using Script.CoreUObject;
using Script.Engine;
using System;
using System.Collections.Generic;

namespace Script.GameBasic
{
    public sealed class EventManager
    {
        public delegate bool LoadObjectAsyncCallback(UObject Object);

        private static readonly EventManager Instance = new EventManager();
        private EventManager() { }
        public static EventManager Get() { return Instance; }

        public void Dispatch(string EventName, params object[] Params)
            => EventImplementation.Event_DispatchImplementation(EventName, Params);
        
        public uint AddListener(string EventName, object Owner, Delegate Callback, bool bOnlyOnce = false)
            => EventImplementation.Event_AddListenerImplementation(EventName, Owner, Callback, bOnlyOnce);

        public uint AddListener(string EventName, Delegate Callback, bool bOnlyOnce = false)
            => AddListener(EventName, null, Callback, bOnlyOnce);

        public bool RemoveListener(uint ListenerID)
            => EventImplementation.Event_RemoveListenerImplementation(ListenerID);
        
        public void Dump()
            => EventImplementation.Event_DumpImplementation();
        
    }
}