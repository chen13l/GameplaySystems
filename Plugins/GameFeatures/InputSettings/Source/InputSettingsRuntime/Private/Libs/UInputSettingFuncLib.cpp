#include "Libs/UInputSettingFuncLib.h"

UEnhancedInputLocalPlayerSubsystem* UInputSettingFuncLib::GetEnhancedInputSubSystemFromActor(AActor* TargetActor)
{
	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!IsValid(TargetPawn)) { return nullptr; }

	if (const APlayerController* const PlayerController = Cast<APlayerController>(TargetPawn->GetController()))
	{
		// only add inputs to local player
		if (!PlayerController->IsLocalController()) { return nullptr; }

		if (const ULocalPlayer* const LocalPlayer = PlayerController->GetLocalPlayer())
		{
			// Get subsystem of the local player and check 
			UEnhancedInputLocalPlayerSubsystem* const Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			if (!IsValid(Subsystem))
			{
				UE_LOG(LogTemp, Error, TEXT("%s: LocalPlayer %s has no EnhancedInputLocalPlayerSubsystem."),
				       *FString(__FUNCTION__), *LocalPlayer->GetName());
				return nullptr;
			}

			return Subsystem;
		}
	}

	return nullptr;
}

UEnhancedInputComponent* UInputSettingFuncLib::GetInputComponentFromActor(AActor* TargetActor)
{
	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!IsValid(TargetPawn)) { return nullptr; }

	return Cast<UEnhancedInputComponent>(TargetPawn->GetController()->InputComponent.Get());
}

UObject* UInputSettingFuncLib::GetInputOwnerObject(UObject* InObject, const EInputBindingOwnerOverride& InOwner)
{
	if (!IsValid(InObject)) { return nullptr; }

	switch (InOwner)
	{
	case EInputBindingOwnerOverride::Default:
		return InObject;
	case EInputBindingOwnerOverride::Pawn:
		return Cast<APawn>(InObject);
	case EInputBindingOwnerOverride::Controller:
		if (APawn* InPawn = Cast<APawn>(InObject))
		{
			return InPawn->GetController();
		}
		UE_LOG(LogTemp, Error, TEXT("%s's controller - Invalid InputBinding Owner."), *InObject->GetName());
		return nullptr;
	}

	return nullptr;
}

TArray<FInputBindingHandle> UInputSettingFuncLib::AddActorInputs(AActor* TargetActor, const FInputActionSettings& ActionSettings)
{
	return AddActorInputs(TargetActor, GetInputOwnerObject(TargetActor, ActionSettings.InputBindingOwner), ActionSettings);
}

TArray<FInputBindingHandle> UInputSettingFuncLib::AddActorInputs(AActor* TargetActor, UObject* BoundFuncSource,
                                                                 const FInputActionSettings& ActionSettings)
{
	TArray<FInputBindingHandle> OutHandles;

	APawn* const TargetPawn = Cast<APawn>(TargetActor);
	// only add inputs to valid pawn
	if (!IsValid(TargetActor) || !IsValid(TargetPawn)) { return OutHandles; }

	if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = GetEnhancedInputSubSystemFromActor(TargetPawn))
	{
		// load and store input mapping context
		UInputMappingContext* const InputMapping = ActionSettings.InputMappingContext.LoadSynchronous();

		UE_LOG(LogTemp, Warning, TEXT("%s: Adding Enhanced Input Mapping %s to Actor %s."), *FString(__FUNCTION__),
		       *InputMapping->GetName(), *TargetActor->GetName());

		// Add the loaded mapping context into the enhanced input subsystem
		Subsystem->AddMappingContext(InputMapping, ActionSettings.MappingPriority);

		if (!IsValid(BoundFuncSource))
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Failed to get the function owner using the Actor %s."),
			       *FString(__FUNCTION__), *TargetActor->GetName());
			return OutHandles;
		}

		// Get the Enhanced Input component of the target Pawn and check 
		const TWeakObjectPtr<UEnhancedInputComponent> InputComponent = Cast<UEnhancedInputComponent>(
			TargetPawn->GetController()->InputComponent.Get());
		if (!InputComponent.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Failed to find InputComponent on Actor %s."), *FString(__FUNCTION__),
			       *TargetActor->GetName());
			return OutHandles;
		}

		// setup the action bindings and add the extension to the active map
		SetupInputBindings(TargetActor, BoundFuncSource, ActionSettings);
	}
	else if (TargetPawn->IsPawnControlled())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Failed to find PlayerController on Actor %s."), *FString(__FUNCTION__),
		       *TargetActor->GetName());
	}

	return OutHandles;
}

bool UInputSettingFuncLib::RemoveActorInputs(AActor* TargetActor, const TArray<FInputBindingHandle>& BindingHandles,
                                             const UInputMappingContext* MappingContext)
{
	bool bRemoveActionSucceeded = true;

	// Try to get the enhanced input subsystem from the pawn
	if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = GetEnhancedInputSubSystemFromActor(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Removing Enhanced Input Mapping %s from Actor %s."),
		       *FString(__FUNCTION__), *MappingContext->GetName(), *TargetActor->GetName());

		// Try to get the enhanced input component of the target pawn
		UEnhancedInputComponent* InputComponent = GetInputComponentFromActor(TargetActor);
		if (!IsValid(InputComponent))

		{
			UE_LOG(LogTemp, Error, TEXT("%s: Failed to find InputComponent on Actor %s."),
			       *FString(__FUNCTION__), *TargetActor->GetName());

			bRemoveActionSucceeded = false;
		}
		else
		{
			// Iterate through the active bindings and remove all
			for (const FInputBindingHandle& Handle : BindingHandles)
			{
				InputComponent->RemoveBinding(Handle);
			}
		}

		// Remove the mapping context from the subsystem
		Subsystem->RemoveMappingContext(MappingContext);
	}

	return bRemoveActionSucceeded;
}

TArray<FInputBindingHandle> UInputSettingFuncLib::SetupInputBindings(AActor* InActor, UObject* FunctionOwner,
                                                                     const FInputActionSettings& ActionSettings)
{
	TArray<FInputBindingHandle> OutArr;

	UEnhancedInputComponent* InputComponent = GetInputComponentFromActor(InActor);

	// Iterate through the bindings to add all
	for (const auto& [ActionInput,FunctionBindingData] : ActionSettings.ActionsBindings)
	{
		// Check if the action input is valid
		if (ActionInput.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Action Input is null."), *FString(__FUNCTION__));
			continue;
		}

		// Load and store the Action
		UInputAction* const InputAction = ActionInput.LoadSynchronous();

		UE_LOG(LogTemp, Warning, TEXT("%s: Binding Action Input %s to Actor %s."), *FString(__FUNCTION__),
		       *InputAction->GetName(), *InActor->GetName());

		// Iterate through all function binding data to bind the UFunctions to it's corresponding input
		for (const auto& [FunctionName, Triggers] : FunctionBindingData)
		{
			// Iterate through all triggersto bind the UFunctions to it's corresponding trigger type
			for (const ETriggerEvent& Trigger : Triggers)
			{
				OutArr.Add(InputComponent->BindAction(InputAction, Trigger, FunctionOwner, FunctionName));
			}
		}
	}
	return OutArr;
}
