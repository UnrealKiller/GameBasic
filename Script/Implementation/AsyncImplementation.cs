using System.Runtime.CompilerServices;
using System;
using Script.Engine;
using Script.CoreUObject;

namespace Script.GameBasic
{
    public static partial class AsyncImplementation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Async_AddAsyncTaskImplementation(ENamedThreads Type, Delegate InCallback);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Async_LoadObjectAsyncImplementation(nint ObjectNameHandle, int AsyncLoadPriority, Delegate InCallback);
        
    }
}