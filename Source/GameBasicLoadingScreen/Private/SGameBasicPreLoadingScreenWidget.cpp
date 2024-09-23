// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGameBasicPreLoadingScreenWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SThrobber.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SBoxPanel.h"
#include "Components/Widget.h"
#include "Engine/Font.h"
#include "Widgets/Layout/SScaleBox.h"

class FReferenceCollector;

void SGameBasicPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	UTexture2D* Texture = LoadObject<UTexture2D>(nullptr,TEXT("/GameBasic/UI/PreLoading/T_UE_Logo.T_UE_Logo"));
	BrushSource = StaticCastSharedRef<ISlateBrushSource>(FDeferredCleanupSlateBrush::CreateBrush(Texture));

	UFont* FontRoboto = Cast<UFont>(StaticLoadObject(UFont::StaticClass(), nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto")));
	Font = FSlateFontInfo(FontRoboto, 18, FName(TEXT("Bold")));

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SScaleBox)
			.Stretch(EStretch::Fill)
			[
				SNew(SImage)
				.Image(BrushSource->GetSlateBrush())
			]

		]
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Right)
				.Padding(FMargin(10.0f))
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Right)
						.Padding(5)
						[
							SNew(SThrobber)
						]
				]
		]
	];
}
