#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/STQStructs.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "STQFuncLib.generated.h"

UCLASS()
class STQUESTSYSTEMRUNTIME_API USTQFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubSystemFromActor(AActor* TargetActor);

	static UEnhancedInputComponent* GetInputComponentFromActor(AActor* TargetActor);

	static UObject* GetInputOwnerObject(UObject* InObject, const EInputBindingOwnerOverride& InOwner);

	static bool AddActorInputs(AActor* Target, const FInputActionSettings& ActionSettings);

	static bool RemoveActorInputs(AActor* TargetActor, const TArray<FInputBindingHandle>& BindingHandles,
	                              const UInputMappingContext* MappingContext);

	static TArray<FInputBindingHandle> SetupInputBindings(AActor* InActor, UObject* FunctionOwner, const FInputActionSettings& ActionSettings);
};
