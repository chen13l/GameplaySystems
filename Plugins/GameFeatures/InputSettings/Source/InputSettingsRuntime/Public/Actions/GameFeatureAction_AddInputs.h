#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Types/InputSettingStructs.h"
#include "GameFeatureAction_AddInputs.generated.h"

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
struct FComponentRequestHandle;

UCLASS(BlueprintType, meta=(DisplayName="Add Inputs"))
class INPUTSETTINGSRUNTIME_API UGameFeatureAction_AddInputs : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FInputActionSettings InputActionSettings;

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

	TMap<TWeakObjectPtr<AActor>, FInputBindingData> ActiveExtensions;
};
