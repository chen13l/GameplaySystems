#include "UI/DialogueWidgetBase.h"

UDialogueWidgetBase::UDialogueWidgetBase()
{
}

void SDialogueWidget::Construct(const FArguments& InArgs)
{
	TargetName = InArgs._TargetName;
	ContentText = InArgs._ContentText;
	FontInfo_Name = InArgs._FontInfo_Name;
	FontInfo_Content = InArgs._FontInfo_Content;
	ImageBrush = InArgs._ImageBrush;
	ContentBGBrush = InArgs._ContentBGBrush;
	ContentBGColor = InArgs._ContentBGColor;

	ChildSlot[
		SNew(SGridPanel)
		.FillRow(0, .15f)
		.FillRow(1, .85f)
		.FillColumn(0, .15f)
		.FillColumn(1, .85f)

		+ SGridPanel::Slot(0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SAssignNew(TargetNameWidget, STextBlock)
			.Text(FText::FromString(TargetName))
			.Font(FontInfo_Name)
			.Justification(ETextJustify::Center)
		]

		+ SGridPanel::Slot(0, 1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.MinDesiredHeight(150.f)
			.MinDesiredWidth(150.f)
			.Padding(2.f)
			[
				SAssignNew(TargetIconWidget, SImage)
				.Image(&ImageBrush)
			]
		]

		+ SGridPanel::Slot(1, 1)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SAssignNew(ContentBGWidget, SBorder)
			.BorderBackgroundColor(ContentBGColor)
			.BorderImage(&ContentBGBrush)
			.Padding(10.f)
			[
				SNew(SScrollBox)

				+ SScrollBox::Slot()
				.Padding(10.f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(ContentTextWidget, STextBlock)
					.Font(FontInfo_Content)
					.AutoWrapText(true)
					.Text(FText::FromString(ContentText))
				]
			]
		]
	];
}

void SDialogueWidget::SetTargetImage(const FSlateBrush& InBrush)
{
	ImageBrush = InBrush;
	TargetIconWidget.Get()->SetImage(&ImageBrush);
}

void SDialogueWidget::SetContentBGColor(const FSlateColor& InSlateColor)
{
	ContentBGColor = InSlateColor;
	ContentBGWidget->SetBorderBackgroundColor(ContentBGColor);
}

void SDialogueWidget::SetContentBGBrush(const FSlateBrush& InBrush)
{
	ContentBGBrush = InBrush;
	ContentBGWidget->SetBorderImage(&ContentBGBrush);
}

void SDialogueWidget::SetContentText(const FString& InContentText)
{
	ContentText = InContentText;
	ContentTextWidget->SetText(FText::FromString(InContentText));
}

void SDialogueWidget::SetTargetName(const FString& InTargetName)
{
	TargetName = InTargetName;
	TargetNameWidget->SetText(FText::FromString(InTargetName));
}

void SDialogueWidget::SetContentFontInfo(const FSlateFontInfo& InFontInfo)
{
	FontInfo_Content = InFontInfo;
	ContentTextWidget->SetFont(InFontInfo);
}

void SDialogueWidget::SetNameFontInfo(const FSlateFontInfo& InFontInfo)
{
	FontInfo_Name = InFontInfo;
	TargetNameWidget->SetFont(InFontInfo);
}

#if WITH_EDITOR
const FText UDialogueWidgetBase::GetPaletteCategory()
{
	return NSLOCTEXT("DialogueWidgets", "Category", "DialogueWidgetBase");
}

void UDialogueWidgetBase::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	if (Event.Property == nullptr || !DialogueWidget.IsValid()) { return; }
	if (Event.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, DialogueDataRowHandle)))
	{
		FDialogueData* DialogueData = DialogueDataRowHandle.GetRow<FDialogueData>(DialogueDataRowHandle.RowName.ToString());
		DialogueWidget->SetContentText(DialogueData->ContentText);
		DialogueWidget->SetTargetName(DialogueData->TargetName);
		ImageBrush.SetResourceObject(DialogueData->FaceImage);
		DialogueWidget->SetTargetImage(ImageBrush);
	}
	if (Event.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, FontInfo_Name)))
	{
		DialogueWidget->SetNameFontInfo(FontInfo_Name);
	}
	if (Event.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, FontInfo_Content)))
	{
		DialogueWidget->SetContentFontInfo(FontInfo_Content);
	}
	if (Event.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, ImageBrush)))
	{
		DialogueWidget->SetTargetImage(ImageBrush);
	}
	if (Event.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, ContentBGBrush)))
	{
		DialogueWidget->SetContentBGBrush(ContentBGBrush);
	}
	if (Event.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, ContentBGColor)))
	{
		DialogueWidget->SetContentBGColor(ContentBGColor);
	}
}

TSharedRef<SWidget> UDialogueWidgetBase::RebuildWidget()
{
	if (!DialogueWidget.IsValid())
	{
		CreateDialogueWidget();
	}

	return DialogueWidget.ToSharedRef();
}

TSharedRef<SWidget> UDialogueWidgetBase::RebuildDesignWidget(TSharedRef<SWidget> Content)
{
	return Super::RebuildDesignWidget(Content);
}
#endif

void UDialogueWidgetBase::BeginDestroy()
{
	DialogueWidget.Reset();

	Super::BeginDestroy();
}

void UDialogueWidgetBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	DialogueWidget.Reset();
}

TSharedRef<SDialogueWidget> UDialogueWidgetBase::CreateDialogueWidget()
{
	if (DialogueDataRowHandle.IsNull())
	{
		return SAssignNew(DialogueWidget, SDialogueWidget);
	}
	FDialogueData* DialogueData = DialogueDataRowHandle.GetRow<FDialogueData>(DialogueDataRowHandle.RowName.ToString());
	SAssignNew(DialogueWidget, SDialogueWidget)
	.TargetName(DialogueData->TargetName)
	.ContentText(DialogueData->ContentText)
	.ImageBrush(ImageBrush)
	.FontInfo_Name(FontInfo_Name)
	.FontInfo_Content(FontInfo_Content)
	.ContentBGBrush(ContentBGBrush)
	.ContentBGColor(ContentBGColor);

	return DialogueWidget.ToSharedRef();
}
