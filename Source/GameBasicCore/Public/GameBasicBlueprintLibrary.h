#pragma once
#include "CoreMinimal.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameBasicBlueprintLibrary.generated.h"

UCLASS()
class UGameBasicBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "GameBasic", meta = (WorldContext = "WorldContextObject"))
    static UGameViewportSubsystem* GetGameViewportSubsystem(UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputLocalPlayerSubsystem(APlayerController* PlayerController);
    
    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static UEnhancedInputComponent* GetEnhancedInputComponent(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static int GetNetMode(UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static FString GetWorldHost(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static FString GetWorldPort(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static int64 GetCurrentTimestampUSec();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static FString TimestampUSecToString(int64 InTimestampUSec);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static void Sleep(float TimeSec);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static void ThreadTick();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static int32 ShowContentFilesInDirectory(FString Directory);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static bool IsEngineRequestingExit();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static UEngine* GetGEngine();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static void ForceGarbageCollection(bool bFullPurge = false);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static int32 GetFrameNumber();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static double GetSeconds();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static bool IsEditor();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
    static bool IsServer();

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static bool SetConsoleVarByBoolean(FString Name, bool Value);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static bool SetConsoleVarByString(FString Name, FString Value);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static bool SetConsoleVarByInteger(FString Name, int Value);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static bool SetConsoleVarByFloat(FString Name, float Value);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static bool GetConsoleVarAsBoolean(FString Name);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static FString GetConsoleVarAsString(FString Name);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static int GetConsoleVarAsInteger(FString Name);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static float GetConsoleVarAsFloat(FString Name);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static bool HasCommandLineParam(FString Name);

    UFUNCTION(BlueprintCallable, Category = "GameBasic")
	static FString GetCommandLineParamValue(FString Name);

};