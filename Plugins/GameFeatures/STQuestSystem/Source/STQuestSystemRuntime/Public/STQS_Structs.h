#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "STQS_Structs.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FDialogueData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue | Data")
	UTexture2D* FaceImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue | Data")
	FString TargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue | Data")
	FString ContentText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue | Data")
	USoundBase* InteractSound;
};
