using System.Runtime.CompilerServices;
using System;
using Script.Engine;

namespace Script.GameBasic
{
    public static partial class ProfileImplementation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Profile_TraceObjectDeletedImplementation(nint ObjectHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern double Profile_TraceCPUScopeImplementation(string EventScopeName, Action Callback);

    }
}