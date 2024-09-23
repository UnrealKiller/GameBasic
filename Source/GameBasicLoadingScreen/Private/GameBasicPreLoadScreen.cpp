// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameBasicPreLoadScreen.h"

#include "Misc/App.h"
#include "SGameBasicPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "GameBasicPreLoadingScreen"

void FGameBasicPreLoadScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		EngineLoadingWidget = SNew(SGameBasicPreLoadingScreenWidget);
	}
}

#undef LOCTEXT_NAMESPACE
