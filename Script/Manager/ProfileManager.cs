using Script.CoreUObject;
using Script.Engine;
using System;
using System.Collections.Generic;

namespace Script.GameBasic
{
    public sealed class ProfileManager
    {
        public delegate bool LoadObjectAsyncCallback(UObject Object);

        private static readonly ProfileManager Instance = new ProfileManager();
        private ProfileManager() { }
        public static ProfileManager Get() { return Instance; }

        public void TraceObjectDeleted(UObject Object)
            => ProfileImplementation.Profile_TraceObjectDeletedImplementation(Object.GarbageCollectionHandle);

        public double TraceCPUScope(string ProfileName, Action ScopeCallback)
            => ProfileImplementation.Profile_TraceCPUScopeImplementation(ProfileName, ScopeCallback);
    }
}