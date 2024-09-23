using Script.CoreUObject;
using Script.Engine;
using System;
using System.Collections.Generic;

namespace Script.GameBasic
{
    public sealed class AsyncManager
    {
        public delegate bool LoadObjectAsyncCallback(UObject Object);

        private static readonly AsyncManager Instance = new AsyncManager();
        private AsyncManager() { }
        public static AsyncManager Get() { return Instance; }

        public void AddAsyncTask(ENamedThreads Type, Action Callback)
            => AsyncImplementation.Async_AddAsyncTaskImplementation(Type, Callback);

        public void LoadObjectAsync(FString ObjectNameHandle, int AsyncLoadPriority, LoadObjectAsyncCallback Callback)
            => AsyncImplementation.Async_LoadObjectAsyncImplementation(ObjectNameHandle.GarbageCollectionHandle, AsyncLoadPriority, Callback);
    }
}