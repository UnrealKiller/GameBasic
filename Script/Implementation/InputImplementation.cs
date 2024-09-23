using System.Runtime.CompilerServices;
using System;
using Script.Engine;

namespace Script.GameBasic
{
    public static partial class InputImplementation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Input_BindDebugKeyImplementation(nint EnhancedInputComponent, nint InputChord, EInputEvent InputEvent, bool bExecuteWhenPaused, Delegate Callback);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Input_UnBindDebugKeyImplementation(nint EnhancedInputComponent, uint BindingHandle);

    }
}