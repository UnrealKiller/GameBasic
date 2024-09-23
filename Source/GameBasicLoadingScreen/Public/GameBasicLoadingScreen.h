// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGameBasicPreLoadScreen;
class SGameBasicPreLoadingScreenWidget;

class FGameBasicLoadingScreenModule : public IModuleInterface
{
public:

	virtual bool IsGameModule() const override;
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    virtual void BeginLoadingScreen(const FString& MapName);
    virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	virtual void BeginStreamingPause(class FViewport* Viewport);
	virtual void EndStreamingPause();

	void OnPreLoadScreenManagerCleanUp();
public:
	TSharedPtr<FGameBasicPreLoadScreen> PreLoadingScreen;
	TSharedPtr<SViewport> LevelLoadingScreen;
	FBeginStreamingPauseDelegate BeginDelegate;
	FEndStreamingPauseDelegate EndDelegate;
	bool bMovieWasStarted;
};
