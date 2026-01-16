#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/InputSettingStructs.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UInputSettingFuncLib.generated.h"

UCLASS()
class INPUTSETTINGSRUNTIME_API UInputSettingFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubSystemFromActor(AActor* TargetActor);

	static UEnhancedInputComponent* GetInputComponentFromActor(AActor* TargetActor);

	static UObject* GetInputOwnerObject(UObject* InObject, const EInputBindingOwnerOverride& InOwner);

	static TArray<FInputBindingHandle> AddActorInputs(AActor* TargetActor, const FInputActionSettings& ActionSettings);
	
	static TArray<FInputBindingHandle> AddActorInputs(AActor* TargetActor,UObject* BoundFuncSource, const FInputActionSettings& ActionSettings);

	static bool RemoveActorInputs(AActor* TargetActor, const TArray<FInputBindingHandle>& BindingHandles,
	                              const UInputMappingContext* MappingContext);

private:
	static TArray<FInputBindingHandle> SetupInputBindings(AActor* InActor, UObject* FunctionOwner, const FInputActionSettings& ActionSettings);
};
