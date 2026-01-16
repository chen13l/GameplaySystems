#pragma once

#include "CoreMinimal.h"
#include "STQS_Structs.h"
#include "Components/Widget.h"
#include "DialogueWidgetBase.generated.h"

DECLARE_DELEGATE(FOnDialoguePaintEvent);

class SDialogueWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDialogueWidget)
		{
		};
		SLATE_ARGUMENT(FString, TargetName);
		SLATE_ARGUMENT(FString, ContentText);
		SLATE_ARGUMENT(FSlateBrush, ImageBrush);
		SLATE_ARGUMENT(FSlateBrush, ContentBGBrush);
		SLATE_ARGUMENT(FSlateColor, ContentBGColor);
		SLATE_ARGUMENT(FSlateFontInfo, FontInfo_Name);
		SLATE_ARGUMENT(FSlateFontInfo, FontInfo_Content);
	SLATE_END_ARGS()

public:
	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct(const FArguments& InArgs);
	END_SLATE_FUNCTION_BUILD_OPTIMIZATION

	void SetTargetImage(const FSlateBrush& InBrush);
	void SetContentBGColor(const FSlateColor& InSlateColor);
	void SetContentBGBrush(const FSlateBrush& InBrush);
	void SetContentText(const FString& InContentText);
	void SetContentFontInfo(const FSlateFontInfo& InFontInfo);
	void SetTargetName(const FString& InTargetName);
	void SetNameFontInfo(const FSlateFontInfo& InFontInfo);

	FSlateFontInfo FontInfo_Name;
	FSlateFontInfo FontInfo_Content;
	FSlateBrush ImageBrush;
	FSlateBrush ContentBGBrush;
	FSlateColor ContentBGColor;

private:
	FString TargetName = TEXT("Name");
	FString ContentText = TEXT("Content");

	TSharedPtr<STextBlock> TargetNameWidget;
	TSharedPtr<SImage> TargetIconWidget;
	TSharedPtr<STextBlock> ContentTextWidget;
	TSharedPtr<SBorder> ContentBGWidget;
};

UCLASS()
class STQUESTSYSTEMRUNTIME_API UDialogueWidgetBase : public UWidget
{
	GENERATED_BODY()

#if WITH_EDITOR

public:
	virtual const FText GetPaletteCategory() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> RebuildDesignWidget(TSharedRef<SWidget> Content) override;
#endif

public:
	UDialogueWidgetBase();
	virtual void BeginDestroy() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueWidget | Data")
	FDataTableRowHandle DialogueDataRowHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueWidget | Data")
	FSlateFontInfo FontInfo_Name = FCoreStyle::Get().GetFontStyle("Roboto");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueWidget | Data")
	FSlateFontInfo FontInfo_Content = FCoreStyle::Get().GetFontStyle("Roboto");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueWidget | Data")
	FSlateBrush ImageBrush = *FCoreStyle::Get().GetDefaultBrush();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueWidget | Data")
	FSlateBrush ContentBGBrush = *FCoreStyle::Get().GetDefaultBrush();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueWidget | Data")
	FSlateColor ContentBGColor = FCoreStyle::Get().GetColor("Gary");

private:
	TSharedPtr<SDialogueWidget> DialogueWidget;

	TSharedRef<SDialogueWidget> CreateDialogueWidget();
};
