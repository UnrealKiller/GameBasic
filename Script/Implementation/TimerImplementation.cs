using System.Runtime.CompilerServices;
using System;
using Script.Engine;

namespace Script.GameBasic
{
    public static partial class TimerImplementation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Timer_InitializeImplementation(nint InInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Timer_CreateOnceTimerImplementation(float InTimeSec, Delegate InCallback);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Timer_CreateLoopTimerImplementation(float InIntervalTimeSec, float InDelayTimeSec, Delegate InCallback);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Timer_CreateTSTickerImplementation(Delegate InCallback, float InDelayTimeSec = 0.0f);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Timer_IsTimerPausedImplementation(int ID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Timer_PauseTimerImplementation(int ID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Timer_UnPauseTimerImplementation(int ID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Timer_ClearImplementation(int ID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Timer_CleanupImplementation();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Timer_DumpImplementation();
    }
}