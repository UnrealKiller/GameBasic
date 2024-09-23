using System.Runtime.CompilerServices;
using System;
using Script.Engine;

namespace Script.GameBasic
{
    public static partial class EngineImplementation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Engine_RegisterConsoleCommandImplementation(nint NameHandle, nint TooltipHandle, Delegate Callback);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Engine_UnregisterConsoleCommandImplementation(nint NameHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Engine_RegisterObjectRefImplementation(nint ObjectHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Engine_UnregisterObjectRefImplementation(nint ObjectHandle);
    }
}