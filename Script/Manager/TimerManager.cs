using Script.CoreUObject;
using Script.Engine;
using System;

namespace Script.GameBasic
{

    public partial class TimerManager
    {
        private static readonly TimerManager Instance = new TimerManager();
        private TimerManager() { }
        public static TimerManager Get() { return Instance; }

        public delegate bool TickerCallback(float Delta);

        public bool Initialize(UGameInstance InInstance)
            =>
            TimerImplementation.Timer_InitializeImplementation(InInstance.GarbageCollectionHandle);

        public int CreateOnceTimer(float InTimeSec, Action InCallback)
            =>
            TimerImplementation.Timer_CreateOnceTimerImplementation(InTimeSec, InCallback);

        public int CreateLoopTimer(float InIntervalTimeSec, float InDelayTimeSec, Action InCallback)
            =>
            TimerImplementation.Timer_CreateLoopTimerImplementation(InIntervalTimeSec, InDelayTimeSec, InCallback);
        
        public int CreateTSTicker(TickerCallback InCallback, float InDelayTimeSec = 0.0f)
            =>
            TimerImplementation.Timer_CreateTSTickerImplementation(InCallback, InDelayTimeSec);

        public bool IsTimerPaused(int ID)
            =>
            TimerImplementation.Timer_IsTimerPausedImplementation(ID);

        public bool PauseTimer(int ID)
            =>
            TimerImplementation.Timer_PauseTimerImplementation(ID);

        public bool UnPauseTimer(int ID)
            =>
            TimerImplementation.Timer_UnPauseTimerImplementation(ID);

        public bool Clear(int ID)
            =>
            TimerImplementation.Timer_ClearImplementation(ID);

        public void Cleanup()
            =>
            TimerImplementation.Timer_CleanupImplementation();

        public void Dump()
            =>
            TimerImplementation.Timer_DumpImplementation();
    }
}