#include "Actions/GameFeatureAction_AddInputs.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void UGameFeatureAction_AddInputs::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating();

	if (!ensureAlways(ActiveRequests.IsEmpty()))
	{
		ResetExtensions();
	}

	const FGameFeatureStateChangeContext StateChangeContext(Context);

	// When the game instance starts, will perform the modular feature activation behavior
	GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &ThisClass::HandleGameInstanceStart, StateChangeContext);

	// Useful to activate the feature even if the game instance has already started
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (!Context.ShouldApplyToWorldContext(WorldContext))
		{
			continue;
		}

		AddToWorld(WorldContext);
	}
}

void UGameFeatureAction_AddInputs::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartHandle);

	ResetExtensions();
}

UGameFrameworkComponentManager* UGameFeatureAction_AddInputs::GetGameFrameworkComponentManager(const FWorldContext& WorldContext) const
{
	if (!IsValid(WorldContext.World()) || !WorldContext.World()->IsGameWorld())
	{
		return nullptr;
	}

	return UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(WorldContext.OwningGameInstance);
}

void UGameFeatureAction_AddInputs::ResetExtensions()
{
	// remove from all target actors using this gf
	for (auto& [TargetActor,InputBindingData] : ActiveExtensions)
	{
		RemoveActorInputs(TargetActor.Get());
	}
}

void UGameFeatureAction_AddInputs::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (!ChangeContext.ShouldApplyToWorldContext(*GameInstance->GetWorldContext()))
	{
		return;
	}

	AddToWorld(*GameInstance->GetWorldContext());
}

void UGameFeatureAction_AddInputs::AddToWorld(const FWorldContext& WorldContext)
{
	if (UGameFrameworkComponentManager* ComponentManager = GetGameFrameworkComponentManager(WorldContext);
		IsValid(ComponentManager) && !TargetPawnClass.IsNull())
	{
		using FHandlerDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate;
		const FHandlerDelegate ExtensionHandlerDelegate = FHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension);

		ActiveRequests.Add(ComponentManager->AddExtensionHandler(TargetPawnClass, ExtensionHandlerDelegate));
	}
}

void UGameFeatureAction_AddInputs::HandleActorExtension(AActor* Owner, const FName EventName)
{
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveActorInputs(Owner);
		return;
	}

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		// avoid add multi times & only add to the actor has all require tags 
		if (ActiveExtensions.Contains(Owner) || !IsActorHasAllRequiredTags(Owner, RequireTags))
		{
			return;
		}

		// check input mapping context
		if (InputMappingContext.IsNull()) { UE_LOG(LogTemp, Error, TEXT("%s: Input Mapping Context is null."), *FString(__FUNCTION__)); }
		else { AddActorInputs(Owner); }
	}
}

bool UGameFeatureAction_AddInputs::IsActorHasAllRequiredTags(const AActor* TargetActor, const TArray<FName>& InRequireTags) const
{
	if (!IsValid(TargetActor)) { return false; }

	for (const FName& Tag : InRequireTags)
	{
		if (TargetActor->ActorHasTag(Tag))
		{
			return false;
		}
	}

	return true;
}

void UGameFeatureAction_AddInputs::AddActorInputs(AActor* TargetActor)
{
	APawn* const TargetPawn = Cast<APawn>(TargetActor);
	// only add inputs to valid pawn
	if (!IsValid(TargetActor) || !IsValid(TargetPawn)) { return; }

	if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = GetEnhancedInputSubSystemFromPawn(TargetPawn))
	{
		// get or create input data associated to the target actor
		FInputBindingData& NewInputData = ActiveExtensions.FindOrAdd(TargetActor);

		// load and store input mapping context
		UInputMappingContext* const InputMapping = InputMappingContext.LoadSynchronous();

		UE_LOG(LogTemp, Display, TEXT("%s: Adding Enhanced Input Mapping %s to Actor %s."), *FString(__FUNCTION__),
		       *InputMapping->GetName(), *TargetActor->GetName());

		// Add the loaded mapping context into the enhanced input subsystem
		Subsystem->AddMappingContext(InputMapping, MappingPriority);

		// Add the mapping context to the input data
		NewInputData.Mapping = InputMapping;

		auto GetPawnInputOwner = [this](APawn* InPawn, const EInputBindingOwnerOverride& InOwner) -> AActor* {
			if (!IsValid(InPawn)) { return nullptr; }

			switch (InputBindingOwner)
			{
			case EInputBindingOwnerOverride::Default:
				break;
			case EInputBindingOwnerOverride::Pawn:
				return Cast<AActor>(InPawn);
			case EInputBindingOwnerOverride::Controller:
				return Cast<AActor>(InPawn->GetController());
			}

			return nullptr;
		};
		// Get the UObject which owns the specified UFunction that will be used to bind the input
		const TWeakObjectPtr<UObject> FunctionOwner = GetPawnInputOwner(TargetPawn, InputBindingOwner);
		if (!FunctionOwner.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Failed to get the function owner using the Actor %s."),
			       *FString(__FUNCTION__), *TargetActor->GetName());
			return;
		}

		// Get the Enhanced Input component of the target Pawn and check 
		const TWeakObjectPtr<UEnhancedInputComponent> InputComponent = Cast<UEnhancedInputComponent>(
			TargetPawn->GetController()->InputComponent.Get());
		if (!InputComponent.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Failed to find InputComponent on Actor %s."), *FString(__FUNCTION__),
			       *TargetActor->GetName());
			return;
		}

		// setup the action bindings and add the extension to the active map
		SetupActionBindings(TargetActor, FunctionOwner.Get(), InputComponent.Get());
		ActiveExtensions.Add(TargetActor, NewInputData);
	}
	else if (TargetPawn->IsPawnControlled())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Failed to find PlayerController on Actor %s."), *FString(__FUNCTION__),
		       *TargetActor->GetName());
	}
}

void UGameFeatureAction_AddInputs::RemoveActorInputs(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		ActiveExtensions.Remove(TargetActor);
		return;
	}

	APawn* const TargetPawn = Cast<APawn>(TargetActor);
	if (!IsValid(TargetPawn))
	{
		ActiveExtensions.Remove(TargetActor);
		return;
	}

	// Check if there's existing active input data
	if (const FInputBindingData* const ActiveInputData = ActiveExtensions.Find(TargetActor))
	{
		// Try to get the enhanced input subsystem from the pawn
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem = GetEnhancedInputSubSystemFromPawn(TargetPawn))
		{
			UE_LOG(LogTemp, Display, TEXT("%s: Removing Enhanced Input Mapping %s from Actor %s."),
			       *FString(__FUNCTION__), *ActiveInputData->Mapping->GetName(), *TargetActor->GetName());

			// Try to get the enhanced input component of the target pawn
			if (const TWeakObjectPtr<UEnhancedInputComponent> InputComponent =
					Cast<UEnhancedInputComponent>(TargetPawn->GetController()->InputComponent.Get());
				!InputComponent.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("%s: Failed to find InputComponent on Actor %s."),
				       *FString(__FUNCTION__), *TargetActor->GetName());
			}
			else
			{
				// Iterate through the active bindings and remove all
				for (const FInputBindingHandle& InputActionBinding : ActiveInputData->ActionBinding)
				{
					InputComponent->RemoveBinding(InputActionBinding);
				}
			}

			// Remove the mapping context from the subsystem
			Subsystem->RemoveMappingContext(ActiveInputData->Mapping.Get());
		}
	}
	ActiveExtensions.Remove(TargetActor);
}

void UGameFeatureAction_AddInputs::SetupActionBindings(AActor* TargetActor, UObject* FunctionOwner, UEnhancedInputComponent* InputComponent)
{
	// Get the existing input data
	FInputBindingData& NewInputData = ActiveExtensions.FindOrAdd(TargetActor);

	// Iterate through the bindings to add all
	for (const auto& [ActionInput,FunctionBindingData]:ActionsBindings)
	{
		// Check if the action input is valid
		if (ActionInput.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Action Input is null."), *FString(__FUNCTION__));
			continue;
		}

		// Load and store the Action
		UInputAction* const InputAction = ActionInput.LoadSynchronous();

		UE_LOG(LogTemp, Display, TEXT("%s: Binding Action Input %s to Actor %s."), *FString(__FUNCTION__),
			   *InputAction->GetName(), *TargetActor->GetName());

		// Iterate through all function binding data to bind the UFunctions to it's corresponding input
		for (const auto& [FunctionName, Triggers] : FunctionBindingData)
		{
			// Iterate through all triggersto bind the UFunctions to it's corresponding trigger type
			for (const ETriggerEvent& Trigger : Triggers)
			{
				NewInputData.ActionBinding.Add(InputComponent->BindAction(InputAction, Trigger, FunctionOwner, FunctionName));
			}
		}
	}
}

UEnhancedInputLocalPlayerSubsystem* UGameFeatureAction_AddInputs::GetEnhancedInputSubSystemFromPawn(APawn* TargetPawn)
{
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
