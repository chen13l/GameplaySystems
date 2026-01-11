#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeatureAction_AddInputs.generated.h"

class UGameplayAbility;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
struct FComponentRequestHandle;

UENUM(BlueprintType, Category="Extra Actions | Enums")
enum class EInputBindingOwnerOverride :uint8
{
	Default,
	Pawn,
	Controller
};

USTRUCT(BlueprintType, Category = "Extra Actions | Modular Structs")
struct FFunctionStackedData
{
	GENERATED_BODY()

	/* UFunction name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FName FunctionName;

	/* Input Trigger event type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TArray<ETriggerEvent> Triggers;
};

USTRUCT(BlueprintType, Category = "Extra Actions | Modular Structs")
struct FInputMappingStack
{
	GENERATED_BODY()

	/* Enhanced Input Action to bind with these settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TSoftObjectPtr<UInputAction> ActionInput;

	/* UFunction and Triggers to bind activation by Enhanced Input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (DisplayName = "UFunction Bindings"))
	TArray<FFunctionStackedData> FunctionBindingData;
};


UCLASS(BlueprintType, meta=(DisplayName="Add Inputs"))
class STQUESTSYSTEMRUNTIME_API UGameFeatureAction_AddInputs : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/* Target pawn to which Input Mapping will be given */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (AllowedClasses = "/Script/Engine.Pawn", OnlyPlaceable = "true"))
	TSoftClassPtr<APawn> TargetPawnClass;

	/* Determines whether the binding will be performed within the controller class or within the pawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	EInputBindingOwnerOverride InputBindingOwner = EInputBindingOwnerOverride::Default;

	/* Tags required on the target to apply this action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TArray<FName> RequireTags;

	/* Enhanced Input Mapping Context to be added */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TSoftObjectPtr<UInputMappingContext> InputMappingContext;

	/* Input Mapping priority */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	int32 MappingPriority = 1;

	/* Enhanced Input Actions binding stacked data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (DisplayName = "Actions Bindings", ShowOnlyInnerProperties))
	TArray<FInputMappingStack> ActionsBindings;

protected:
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	void AddToWorld(const FWorldContext& WorldContext);

	UGameFrameworkComponentManager* GetGameFrameworkComponentManager(const FWorldContext& WorldContext) const;
	TArray<TSharedPtr<FComponentRequestHandle>> ActiveRequests;

	void ResetExtensions();

private:
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);
	FDelegateHandle GameInstanceStartHandle;

	void HandleActorExtension(AActor* Owner, const FName EventName);
	bool IsActorHasAllRequiredTags(const AActor* TargetActor, const TArray<FName>& RequireTags) const;

	void AddActorInputs(AActor* TargetActor);
	void RemoveActorInputs(AActor* TargetActor);

	void SetupActionBindings(AActor* TargetActor, UObject* FunctionOwner, UEnhancedInputComponent* InputComponent);

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubSystemFromPawn(APawn* TargetPawn);

	struct FInputBindingData
	{
		TArray<FInputBindingHandle> ActionBinding;
		TWeakObjectPtr<UInputMappingContext> Mapping;
	};

	TMap<TWeakObjectPtr<AActor>, FInputBindingData> ActiveExtensions;
	TArray<TWeakObjectPtr<UInputAction>> InputActions;
};
