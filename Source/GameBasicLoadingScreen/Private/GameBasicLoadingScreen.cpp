#include "GameBasicLoadingScreen.h"
#include "GameBasicPreLoadScreen.h"
#include "MoviePlayer.h"
#include "Widgets/SViewport.h"
#include "Widgets/Images/SThrobber.h"
#include "PreLoadScreenManager.h"
#include "SGameBasicPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "GameBasicLoadingScreen"

bool FGameBasicLoadingScreenModule::IsGameModule() const
{
	return true;
}

void FGameBasicLoadingScreenModule::StartupModule()
{
	//if (!IsRunningDedicatedServer())
	//{
	//	bMovieWasStarted = false;
	//	FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &FGameBasicLoadingScreenModule::BeginLoadingScreen);
	//	FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FGameBasicLoadingScreenModule::EndLoadingScreen);
	//	PreLoadingScreen = MakeShared<FGameBasicPreLoadScreen>();
	//	PreLoadingScreen->Init();

	//	if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
	//	{
	//		FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
	//		FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FGameBasicLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
	//	}
	//}
}

void FGameBasicLoadingScreenModule::ShutdownModule()
{
	//BeginDelegate.Unbind();
	//EndDelegate.Unbind();
}

void FGameBasicLoadingScreenModule::BeginLoadingScreen(const FString& MapName)
{
	if (GetMoviePlayer()->IsInitialized() && IsMoviePlayerEnabled() && MapName.Contains("Main_Map_WP")) {
		BeginDelegate.BindRaw(this, &FGameBasicLoadingScreenModule::BeginStreamingPause);
		EndDelegate.BindRaw(this, &FGameBasicLoadingScreenModule::EndStreamingPause);
		GEngine->RegisterBeginStreamingPauseRenderingDelegate(&BeginDelegate);
		GEngine->RegisterEndStreamingPauseRenderingDelegate(&EndDelegate);

		TRACE_CPUPROFILER_EVENT_SCOPE(FGameBasicLoadingScreenModule::BeginStreamingPause);
		LevelLoadingScreen = SNew(SViewport)
			.EnableGammaCorrection(false);
		LevelLoadingScreen->SetContent
		(
			SNew(SGameBasicPreLoadingScreenWidget)
		);

		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bWaitForManualStop = true;
		LoadingScreen.bAllowEngineTick = true;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		LoadingScreen.PlaybackType = EMoviePlaybackType::MT_Looped;
		LoadingScreen.WidgetLoadingScreen = LevelLoadingScreen; // SViewport from above
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		GetMoviePlayer()->PlayMovie();
		bMovieWasStarted = true;
	}
}

void FGameBasicLoadingScreenModule::EndLoadingScreen(UWorld* InLoadedWorld)
{
}

void FGameBasicLoadingScreenModule::BeginStreamingPause(class FViewport* Viewport)
{
	if (GetMoviePlayer()->IsInitialized() && IsMoviePlayerEnabled() && !bMovieWasStarted) {
		TRACE_CPUPROFILER_EVENT_SCOPE(FGameBasicLoadingScreenModule::BeginStreamingPause);
		LevelLoadingScreen = SNew(SViewport)
			.EnableGammaCorrection(false);

		LevelLoadingScreen->SetContent
		(
			SNew(SGameBasicPreLoadingScreenWidget)
		);

		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bWaitForManualStop = true;
		LoadingScreen.bAllowEngineTick = true;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		LoadingScreen.PlaybackType = EMoviePlaybackType::MT_Looped;
		LoadingScreen.WidgetLoadingScreen = LevelLoadingScreen; // SViewport from above
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		GetMoviePlayer()->PlayMovie();
		bMovieWasStarted = true;
	}
}

void FGameBasicLoadingScreenModule::EndStreamingPause()
{
	if (bMovieWasStarted)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FGameBasicLoadingScreenModule::EndStreamingPause);
		GetMoviePlayer()->StopMovie();
		GetMoviePlayer()->WaitForMovieToFinish();
		LevelLoadingScreen.Reset();
		FlushRenderingCommands();
		bMovieWasStarted = false;
	}
}


void FGameBasicLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	PreLoadingScreen.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameBasicLoadingScreenModule, GameBasicLoadingScreen)