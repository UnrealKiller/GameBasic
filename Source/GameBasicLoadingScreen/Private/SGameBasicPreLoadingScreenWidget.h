// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/GCObject.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Fonts/SlateFontInfo.h"

class FReferenceCollector;

class SGameBasicPreLoadingScreenWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameBasicPreLoadingScreenWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
private:
    TSharedPtr<ISlateBrushSource> BrushSource;
    FSlateFontInfo Font;
};
