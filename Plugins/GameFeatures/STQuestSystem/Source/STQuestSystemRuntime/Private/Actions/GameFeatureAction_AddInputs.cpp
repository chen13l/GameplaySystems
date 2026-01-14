#include "Actions/GameFeatureAction_AddInputs.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Libs/STQFuncLib.h"

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
		IsValid(ComponentManager) && !InputActionSettings.TargetPawnClass.IsNull())
	{
		using FHandlerDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate;
		const FHandlerDelegate ExtensionHandlerDelegate = FHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension);

		ActiveRequests.Add(ComponentManager->AddExtensionHandler(InputActionSettings.TargetPawnClass, ExtensionHandlerDelegate));
	}
}

void UGameFeatureAction_AddInputs::HandleActorExtension(AActor* Owner, const FName EventName)
{
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveActorInputs(Owner);
		return;
	}

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady
		|| EventName == UGameFrameworkComponentManager::NAME_ReceiverAdded)
	{
		// avoid add multi times & only add to the actor has all require tags 
		if (ActiveExtensions.Contains(Owner) || !IsActorHasAllRequiredTags(Owner, InputActionSettings.RequireTags))
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Input Mapping Context had add."), *FString(__FUNCTION__));
			return;
		}

		// check input mapping context
		if (InputActionSettings.InputMappingContext.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Input Mapping Context is null."), *FString(__FUNCTION__));
		}
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
	if (USTQFuncLib::AddActorInputs(TargetActor, InputActionSettings))
	{
		// get or create input data associated to the target actor
		FInputBindingData& NewInputData = ActiveExtensions.FindOrAdd(TargetActor);
		// Add the mapping context to the input data
		NewInputData.Mapping = InputActionSettings.InputMappingContext.LoadSynchronous();
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
		const auto& [TargetHandles,TargetMapping] = ActiveExtensions[TargetActor];
		USTQFuncLib::RemoveActorInputs(TargetActor, TargetHandles, TargetMapping.Get());
	}
	
	ActiveExtensions.Remove(TargetActor);
}

void UGameFeatureAction_AddInputs::SetupActionBindings(AActor* TargetActor, UObject* FunctionOwner, UEnhancedInputComponent* InputComponent)
{
	// Get the existing input data
	FInputBindingData& NewInputData = ActiveExtensions.FindOrAdd(TargetActor);

	TArray<FInputBindingHandle> BindingHandles = USTQFuncLib::SetupInputBindings(TargetActor,FunctionOwner,InputActionSettings);
	NewInputData.ActionBindingHandle.Append(BindingHandles);
}
