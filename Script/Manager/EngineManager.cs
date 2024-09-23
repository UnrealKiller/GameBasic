using Script.CoreUObject;
using Script.Engine;
using System;
using System.Collections.Generic;
using Script.GameBasicCore;

namespace Script.GameBasic
{
    public class UObjectGCRef
    {
        public UObject Object { get; private set; }

        public UObjectGCRef(UObject Object)
        {
            this.Object = Object;
            EngineImplementation.Engine_RegisterObjectRefImplementation(Object.GarbageCollectionHandle);
        }

        ~UObjectGCRef()
        {
            AsyncManager.Get().AddAsyncTask(ENamedThreads.GameThread, () =>
            {
                EngineImplementation.Engine_UnregisterObjectRefImplementation(Object.GarbageCollectionHandle);
            });
        }

        public static UObjectGCRef LoadObject<T>(UObject Outer = null,
            FString Name = null,
            FString Filename = null,
            ELoadFlags LoadFlags = ELoadFlags.LOAD_None,
            UPackageMap Sandbox = null
        ) where T : UObject
        {
            return new UObjectGCRef(Unreal.LoadObject<T>(Outer, Name, Filename, LoadFlags, Sandbox));
        }

        public static UObjectGCRef NewObject<T>(UObject Outer = null,
            UClass Class = null,
            FName Name = null,
            EObjectFlags Flags = EObjectFlags.RF_NoFlags,
            UObject Template = null,
            bool bCopyTransientsFromClassDefaults = false
        ) where T : UObject, IStaticClass
        {
            return new UObjectGCRef(Unreal.NewObject<T>(Outer, Class, Name, Flags, Template, bCopyTransientsFromClassDefaults));
        }
    }

    public sealed class EngineManager
    {
        private static readonly EngineManager Instance = new EngineManager();
        private EngineManager() { }
        public static EngineManager Get() { return Instance; }

        public UEngine GetGEngine()
        {
            return UGameBasicBlueprintLibrary.GetGEngine();
        }

        public bool IsRequestingExit()
        {
            return UGameBasicBlueprintLibrary.IsEngineRequestingExit();
        }

        public void ForceGarbageCollection(bool bFullPurge = false)
        {
            UGameBasicBlueprintLibrary.ForceGarbageCollection(bFullPurge);
        }

        public void Sleep(float TimeSec)
        {
            UGameBasicBlueprintLibrary.Sleep(TimeSec);
        }

        public void ThreadTick()
        {
            UGameBasicBlueprintLibrary.ThreadTick();
        }

        public void RegisterConsoleCommand(FString Name, FString Tooltip, Delegate Callback)
            => EngineImplementation.Engine_RegisterConsoleCommandImplementation(Name.GarbageCollectionHandle, Tooltip.GarbageCollectionHandle, Callback);
        
        public void UnregisterConsoleCommand(FString Name)
            => EngineImplementation.Engine_UnregisterConsoleCommandImplementation(Name.GarbageCollectionHandle);


        public long GetCurrentTimestampUSec()
        {
            return UGameBasicBlueprintLibrary.GetCurrentTimestampUSec();
        }

        public int GetFrameNumber()
        {
            return UGameBasicBlueprintLibrary.GetFrameNumber();
        }

        public double GetSeconds()
        {
            return UGameBasicBlueprintLibrary.GetSeconds();
        }

        public bool IsEditor()
        {
            return UGameBasicBlueprintLibrary.IsEditor();
        }

        public bool IsServer()
        {
            return UGameBasicBlueprintLibrary.IsServer();
        }

        public bool SetConsoleVar(FString Name, bool Value)
        {
            return UGameBasicBlueprintLibrary.SetConsoleVarByBoolean(Name, Value);
        }

        public bool SetConsoleVar(FString Name, FString Value)
        {
            return UGameBasicBlueprintLibrary.SetConsoleVarByString(Name, Value);
        }

        public bool SetConsoleVar(FString Name, int Value)
        {
            return UGameBasicBlueprintLibrary.SetConsoleVarByInteger(Name, Value);
        }

        public bool SetConsoleVar(FString Name, float Value)
        {
            return UGameBasicBlueprintLibrary.SetConsoleVarByFloat(Name, Value);
        }

        public bool GetConsoleVarAsBoolean(FString Name)
        {
            return UGameBasicBlueprintLibrary.GetConsoleVarAsBoolean(Name);
        }

        public FString GetConsoleVarAsString(FString Name)
                {
            return UGameBasicBlueprintLibrary.GetConsoleVarAsString(Name);
        }

        public int GetConsoleVarAsInteger(FString Name)
        {
            return UGameBasicBlueprintLibrary.GetConsoleVarAsInteger(Name);
        }

        public float GetConsoleVarAsFloat(FString Name)
        {
            return UGameBasicBlueprintLibrary.GetConsoleVarAsFloat(Name);
        }

        public bool HasCommandLineParam(FString Name)
        {
            return UGameBasicBlueprintLibrary.HasCommandLineParam(Name);
        }

        public FString GetCommandLineParamValue(FString Name)
        {
            return UGameBasicBlueprintLibrary.GetCommandLineParamValue(Name);
        }
    }
}