#include "Binding/Class/FClassBuilder.h"
#include "Environment/FCSharpEnvironment.h"
#include "GameBasicCoreModule.h"
#include "Delegate/FUnrealCSharpModuleDelegates.h"

namespace
{
	struct FTimerDescription
	{
		int32 ID;
		bool bOnlyOnce;
		float DelayTimeSec;
		float IntervalTimeSec;

		FTimerHandle TimerHandle;
		FTSTicker::FDelegateHandle TickerHandle;

		MonoObject* Callback = nullptr;
		FGarbageCollectionHandle CallbackRef;

#if !UE_BUILD_SHIPPING 
		FString CreationTraceback;
#endif
	};

	struct FCSharpTimerManager
	{
		TWeakObjectPtr<UGameInstance> GameInstance;
		int32 IDStep = 0;
		TMap<int32, FTimerDescription> TimerMap;
	};

	static FCSharpTimerManager CSharpTimerManager;

	struct FRegisterGameBasicTimer
	{
		static bool Initialize(const FGarbageCollectionHandle GameInstance) {
			if (!CSharpTimerManager.GameInstance.IsValid()) {
				CSharpTimerManager.GameInstance = FCSharpEnvironment::GetEnvironment().GetObject<UGameInstance>(GameInstance);
				CSharpTimerManager.IDStep = 0;
				return true;
			}
			return false;
		}

		static void InvokeTimer(int32 TimerID) {
			if (auto TimerDesc = CSharpTimerManager.TimerMap.Find(TimerID)) {
				FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(TimerDesc->Callback, nullptr);
			}
			else {
				UE_LOG(LogGameBasic, Warning, TEXT("Invaild Timer ID: %d"), TimerID);
			}
		}

		static int32 CreateOnceTimer(float InTimeSec, MonoObject* InCallback) {
			if (CSharpTimerManager.GameInstance.IsValid()) {
				FTimerDescription NewTimerDesc;
				NewTimerDesc.ID = ++CSharpTimerManager.IDStep;
				NewTimerDesc.bOnlyOnce = true;
				NewTimerDesc.DelayTimeSec = InTimeSec;
				NewTimerDesc.Callback = InCallback;
				NewTimerDesc.CallbackRef = FGarbageCollectionHandle::NewRef(InCallback, true);
#if !UE_BUILD_SHIPPING 
				NewTimerDesc.CreationTraceback = FString(UTF8_TO_TCHAR(FCSharpEnvironment::GetEnvironment().GetDomain()->String_To_UTF8(
					FCSharpEnvironment::GetEnvironment().GetDomain()->GetTraceback())));
#endif
				FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
				TimerManager.SetTimer(
					NewTimerDesc.TimerHandle,
					[ID = NewTimerDesc.ID]() {
						InvokeTimer(ID);
						Clear(ID);
					},
					InTimeSec,
					false
				);
				CSharpTimerManager.TimerMap.Emplace(NewTimerDesc.ID, NewTimerDesc);
				return NewTimerDesc.ID;
			}
			return 0;
		}

		static int32 CreateLoopTimer(float InIntervalTimeSec, float InDelayTimeSec, MonoObject* InCallback) {
			if (CSharpTimerManager.GameInstance.IsValid()) {
				FTimerDescription NewTimerDesc;
				NewTimerDesc.ID = ++CSharpTimerManager.IDStep;
				NewTimerDesc.bOnlyOnce = false;
				NewTimerDesc.IntervalTimeSec = InIntervalTimeSec;
				NewTimerDesc.DelayTimeSec = InDelayTimeSec;
				NewTimerDesc.Callback = InCallback;
				NewTimerDesc.CallbackRef = FGarbageCollectionHandle::NewRef(InCallback, true);
#if !UE_BUILD_SHIPPING 
				NewTimerDesc.CreationTraceback = FString(UTF8_TO_TCHAR(FCSharpEnvironment::GetEnvironment().GetDomain()->String_To_UTF8(
					FCSharpEnvironment::GetEnvironment().GetDomain()->GetTraceback())));
#endif
				FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
				TimerManager.SetTimer(
					NewTimerDesc.TimerHandle,
					[ID = NewTimerDesc.ID]() {
						InvokeTimer(ID);
					},
					NewTimerDesc.IntervalTimeSec,
					true,
					NewTimerDesc.DelayTimeSec
				);
				CSharpTimerManager.TimerMap.Emplace(NewTimerDesc.ID, NewTimerDesc);
				return NewTimerDesc.ID;
			}
			return 0;
		}

		static int32 CreateTSTicker(MonoObject* InCallback, float InDelayTimeSec = 0.0f) {
			FTimerDescription NewTimerDesc;
			NewTimerDesc.ID = ++CSharpTimerManager.IDStep;
			NewTimerDesc.bOnlyOnce = false;
			NewTimerDesc.DelayTimeSec = InDelayTimeSec;
			NewTimerDesc.Callback = InCallback;
			NewTimerDesc.CallbackRef = FGarbageCollectionHandle::NewRef(InCallback, true);
#if !UE_BUILD_SHIPPING 
			NewTimerDesc.CreationTraceback = FString(UTF8_TO_TCHAR(FCSharpEnvironment::GetEnvironment().GetDomain()->String_To_UTF8(
				FCSharpEnvironment::GetEnvironment().GetDomain()->GetTraceback())));
#endif
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([TimerID = NewTimerDesc.ID](float DeltaSeconds) {
				if (auto TimerDesc = CSharpTimerManager.TimerMap.Find(TimerID)) {
					bool NeedContinue = false;
					MonoMethod* Method = FCSharpEnvironment::GetEnvironment().GetDomain()->Delegate_Get_Method(TimerDesc->Callback);
					auto Attrs = FMonoDomain::Custom_Attrs_From_Method(Method);
					void* Params[1] = { &DeltaSeconds };
					if (MonoObject* ReturnValue = FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(TimerDesc->Callback, Params)) {
						NeedContinue = *static_cast<bool*>(FCSharpEnvironment::GetEnvironment().GetDomain()->Object_Unbox(ReturnValue));
						if (!NeedContinue) {
							FRegisterGameBasicTimer::Clear(TimerID);
						}
					}
					return NeedContinue;
				}
				return false;
				}), InDelayTimeSec);
			CSharpTimerManager.TimerMap.Emplace(NewTimerDesc.ID, NewTimerDesc);
			return NewTimerDesc.ID;
		}

		static bool PauseTimer(int32 ID) {
			if (auto TimerDesc = CSharpTimerManager.TimerMap.Find(ID)) {
				FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
				if (TimerDesc->TimerHandle.IsValid()) {
					TimerManager.PauseTimer(TimerDesc->TimerHandle);
					return true;
				}
				return false;
			}
			return false;
		}

		static bool UnPauseTimer(int32 ID) {
			if (auto TimerDesc = CSharpTimerManager.TimerMap.Find(ID)) {
				FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
				if (TimerDesc->TimerHandle.IsValid()) {
					TimerManager.UnPauseTimer(TimerDesc->TimerHandle);
					return true;
				}
				return false;
			}
			return false;
		}

		static bool IsTimerPaused(int32 ID) {
			if (auto TimerDesc = CSharpTimerManager.TimerMap.Find(ID)) {
				FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
				if (TimerDesc->TimerHandle.IsValid()) {
					return TimerManager.IsTimerPaused(TimerDesc->TimerHandle);;
				}
				return false;
			}
			return false;
		}

		static bool Clear(int32 ID) {
			if (auto TimerDesc = CSharpTimerManager.TimerMap.Find(ID)) {
				if (TimerDesc->TickerHandle.IsValid()) {
					FTSTicker::GetCoreTicker().RemoveTicker(TimerDesc->TickerHandle);
					TimerDesc->TickerHandle.Reset();
				}
				else if (TimerDesc->TimerHandle.IsValid() && CSharpTimerManager.GameInstance.IsValid()) {
					FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
					TimerManager.ClearTimer(TimerDesc->TimerHandle);
					TimerDesc->TimerHandle.Invalidate();
				}
				FGarbageCollectionHandle::Free<false>(TimerDesc->CallbackRef);
				CSharpTimerManager.TimerMap.Remove(ID);
				return true;
			}
			return false;
		}

		static void Cleanup() {
			for (auto TimerDescPair : CSharpTimerManager.TimerMap) {
				auto& TimerDesc = TimerDescPair.Value;
				if (TimerDesc.TickerHandle.IsValid()) {
					FTSTicker::GetCoreTicker().RemoveTicker(TimerDesc.TickerHandle);
					TimerDesc.TickerHandle.Reset();
				}
				else if (TimerDesc.TimerHandle.IsValid() && CSharpTimerManager.GameInstance.IsValid()) {
					FTimerManager& TimerManager = CSharpTimerManager.GameInstance->GetTimerManager();
					TimerManager.ClearTimer(TimerDesc.TimerHandle);
					TimerDesc.TimerHandle.Invalidate();
				}
			}
			CSharpTimerManager.TimerMap.Reset();
		}

		static void Dump() {
			UE_LOG(LogGameBasic, Log, TEXT("------- Timer Count: %d"), CSharpTimerManager.TimerMap.Num());
#if !UE_BUILD_SHIPPING 
			for (auto TimerDescPair : CSharpTimerManager.TimerMap) {
				UE_LOG(LogGameBasic, Log, TEXT("--------------- Timer[ID: %d] Creation Traceback Begin: \n%s\n--------------- Traceback End"), TimerDescPair.Value.ID, *TimerDescPair.Value.CreationTraceback);
			}
#endif
		}

		FRegisterGameBasicTimer()
		{
			FClassBuilder(TEXT("Timer"), "GameBasic")
				.Function("Initialize", Initialize)
				.Function("CreateOnceTimer", CreateOnceTimer)
				.Function("CreateLoopTimer", CreateLoopTimer)
				.Function("CreateTSTicker", CreateTSTicker)
				.Function("IsTimerPaused", IsTimerPaused)
				.Function("PauseTimer", PauseTimer)
				.Function("UnPauseTimer", UnPauseTimer)
				.Function("Clear", Clear)
				.Function("Cleanup", Cleanup)
				.Function("Dump", Dump)
			;
			FUnrealCSharpModuleDelegates::OnUnrealCSharpModuleInActive.AddLambda([](){
				FRegisterGameBasicTimer::Cleanup();
			});
		}
	};

	[[maybe_unused]] FRegisterGameBasicTimer RegisterGameBasicTimer;
}
