#pragma once

#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "STQStructs.generated.h"

UENUM(BlueprintType, Category="Extra Actions | Enums")
enum class EInputBindingOwnerOverride :uint8
{
	Default,
	Pawn,
	Controller,
	Component
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

USTRUCT(BlueprintType, Category = "Extra Actions | Modular Structs")
struct FInputActionSettings
{
	GENERATED_BODY()

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
};

struct FInputBindingData
{
	TArray<FInputBindingHandle> ActionBindingHandle;
	TWeakObjectPtr<UInputMappingContext> Mapping;
};