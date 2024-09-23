#include "GameBasicBlueprintLibrary.h"
#include "HAL/ThreadManager.h"
#include "IPlatformFilePak.h"
#include "SocketSubsystem.h"
#include "GameBasicCoreModule.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionRuntimeCell.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"
#include <chrono>

UGameViewportSubsystem* UGameBasicBlueprintLibrary::GetGameViewportSubsystem(UObject* WorldContext)
{
	if (WorldContext == nullptr || WorldContext->GetWorld() == nullptr)
		return UGameViewportSubsystem::Get();
	return UGameViewportSubsystem::Get(WorldContext->GetWorld());
}

UEnhancedInputLocalPlayerSubsystem* UGameBasicBlueprintLibrary::GetEnhancedInputLocalPlayerSubsystem(APlayerController* PlayerController)
{
	if (PlayerController == nullptr)
		return nullptr;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
		return Subsystem;
	}
	return nullptr;
}

UEnhancedInputComponent* UGameBasicBlueprintLibrary::GetEnhancedInputComponent(APlayerController* PlayerController)
{
	if (PlayerController == nullptr)
		return nullptr;
	return Cast<UEnhancedInputComponent>(PlayerController->GetComponentByClass(UEnhancedInputComponent::StaticClass()));
}

int UGameBasicBlueprintLibrary::GetNetMode(UObject* WorldContext)
{
	if (WorldContext == nullptr || WorldContext->GetWorld() == nullptr)
		return false;
	UWorld* World = WorldContext->GetWorld();
	ENetMode NetMode = World->GetNetMode();
	return NetMode;
}

FString UGameBasicBlueprintLibrary::GetWorldHost(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			bool canBind = false;
			TSharedRef<FInternetAddr> localIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);
			return (localIp->IsValid() ? localIp->ToString(false) : World->URL.Host);
		}
	}
	return "WorldContextObject is invaild";
}

FString UGameBasicBlueprintLibrary::GetWorldPort(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			return FString::FromInt(World->URL.Port);
		}
	}
	return "WorldContextObject is invaild";
}

int64 UGameBasicBlueprintLibrary::GetCurrentTimestampUSec()
{
	FDateTime Now = FDateTime::Now();
	return (Now.GetTicks() - FDateTime(1970, 1, 1, 8).GetTicks()) / ETimespan::TicksPerMicrosecond;
}

FString UGameBasicBlueprintLibrary::TimestampUSecToString(int64 InTimestampUSec)
{
	FDateTime DateTime = FDateTime(1970, 1, 1, 8) + FTimespan(InTimestampUSec * ETimespan::TicksPerMicrosecond);
	return DateTime.ToString(TEXT("%Y.%m.%d-%H:%M:%S.%s"));;
}

void UGameBasicBlueprintLibrary::Sleep(float TimeSec)
{
	FPlatformProcess::Sleep(TimeSec);
}

void UGameBasicBlueprintLibrary::ThreadTick()
{
	FTSTicker::GetCoreTicker().Tick(FApp::GetDeltaTime());
	FThreadManager::Get().Tick();
}

int32 UGameBasicBlueprintLibrary::ShowContentFilesInDirectory(FString Directory)
{
	TArray<FString> Files;
	TArray<FString> Dirs;

	FPakPlatformFile* PakFileMgr = static_cast<FPakPlatformFile*>(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));
	auto FallArrayDirVisitor = [&Files, &Dirs](const TCHAR* InItem, bool bInDir)->bool
		{
			if (bInDir)
			{
				Dirs.AddUnique(InItem);
				UE_LOG(LogTemp, Log, TEXT("find dirs %s"), InItem);
			}
			else
			{
				Files.AddUnique(InItem);
				UE_LOG(LogTemp, Log, TEXT("find files %s"), InItem);
			}
			return true;
		};
	PakFileMgr->IterateDirectoryRecursively(*Directory, FallArrayDirVisitor);

	return 1;
}

bool UGameBasicBlueprintLibrary::IsEngineRequestingExit()
{
	return GIsRequestingExit;
}

UEngine* UGameBasicBlueprintLibrary::GetGEngine()
{
	return GEngine;
}

void UGameBasicBlueprintLibrary::ForceGarbageCollection(bool bFullPurge /*= false*/)
{
	GEngine->ForceGarbageCollection(bFullPurge);
}

int32 UGameBasicBlueprintLibrary::GetFrameNumber()
{
	return GFrameNumber;
}

double UGameBasicBlueprintLibrary::GetSeconds()
{
	return FPlatformTime::Seconds();
}

bool UGameBasicBlueprintLibrary::IsEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

bool UGameBasicBlueprintLibrary::IsServer()
{
#if UE_SERVER
	return true;
#else
	return false;
#endif
}

bool UGameBasicBlueprintLibrary::SetConsoleVarByBoolean(FString Name, bool Value)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return false;
	}
	Variable->Set(Value);
	return true;
}

bool UGameBasicBlueprintLibrary::SetConsoleVarByString(FString Name, FString Value)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return false;
	}
	Variable->Set(UTF8_TO_TCHAR(*Value));
	return true;
}

bool UGameBasicBlueprintLibrary::SetConsoleVarByInteger(FString Name, int Value)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return false;
	}
	Variable->Set(Value);
	return true;
}

bool UGameBasicBlueprintLibrary::SetConsoleVarByFloat(FString Name, float Value)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return false;
	}
	Variable->Set(Value);
	return true;
}

bool UGameBasicBlueprintLibrary::GetConsoleVarAsBoolean(FString Name)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return false;
	}
	return Variable->GetBool();
}

FString UGameBasicBlueprintLibrary::GetConsoleVarAsString(FString Name)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return FString();
	}
	return Variable->GetString();
}

int UGameBasicBlueprintLibrary::GetConsoleVarAsInteger(FString Name)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return 0;
	}
	return Variable->GetInt();
}

float UGameBasicBlueprintLibrary::GetConsoleVarAsFloat(FString Name)
{
	IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (!Variable) {
		UE_LOG(LogGameBasic, Warning, TEXT("Failed to find console variable '%s'."), *Name);
		return 0.f;
	}
	return Variable->GetFloat();
}

bool UGameBasicBlueprintLibrary::HasCommandLineParam(FString Name)
{
	return FParse::Param(FCommandLine::Get(), *Name);
}

FString UGameBasicBlueprintLibrary::GetCommandLineParamValue(FString Name)
{
	FString Value;
	FParse::Value(FCommandLine::Get(), *Name, Value);
	return Value;
}
