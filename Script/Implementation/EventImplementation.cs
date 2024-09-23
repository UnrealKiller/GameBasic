using System.Runtime.CompilerServices;
using System;
using Script.Engine;
using Script.CoreUObject;

namespace Script.GameBasic
{
    public static partial class EventImplementation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Event_AddListenerImplementation(string EventName, object Owner, Delegate Callback, bool bOnlyOnce);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Event_RemoveListenerImplementation(uint ListenerID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Event_DispatchImplementation(string EventName, params object[] Params);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Event_DumpImplementation();

    }
}