using Script.CoreUObject;
using Script.Engine;
using System;
using System.Collections.Generic;

namespace Script.GameBasic
{
    public sealed class UIManager
    {
        private static readonly UIManager Instance = new UIManager();
        private UIManager() { }
        public static UIManager Get() { return Instance; }

        public UGameInstance GameInstance { get; private set; }
        public APlayerController PlayerController { get; private set; }

        private int? TickerID = null;

        public List<WeakReference<UIController>> Controllers { get; private set; } = new List<WeakReference<UIController>>();

        public bool Initialize(UObject WorldContext)
        {
            if (WorldContext.IsValid())
            {
                this.GameInstance = UGameplayStatics.GetGameInstance(WorldContext);
                this.PlayerController = UGameplayStatics.GetPlayerController(WorldContext, 0);
                TickerID = TimerManager.Get().CreateTSTicker(Tick);
                return true;
            }
            return false;
        }

        public void Deinitialize()
        {
            if (TickerID.HasValue)
            {
                TimerManager.Get().Clear(TickerID.Value);
            }
        }

        public void RegisterUIController(UIController Controller)
        {
            WeakReference<UIController> WeakReference = new WeakReference<UIController> (Controller);
            Controllers.Add(WeakReference);
        }

        public void UnregisterUIController(UIController Controller)
        {
            for (int i = 0; i < Controllers.Count; i++)
            {
                Controllers[i].TryGetTarget(out UIController ControllerInstance);
                if(ControllerInstance == Controller)
                {
                    Controllers.RemoveAt(i);
                    break;
                }
            }
        }

        public bool Tick(float Delta)
        {
            for (int i = 0; i < Controllers.Count; i++)
            {
                if(Controllers[i].TryGetTarget(out UIController ControllerInstance))
                {
                    ControllerInstance.OnTickEvent(Delta);
                }
            }
            return true;
        }

        public void SetAllVisibility(bool bVisible)
        {
            if (bVisible)
            {
                for (int i = 0; i < Controllers.Count; i++)
                {
                    if (Controllers[i].TryGetTarget(out UIController ControllerInstance))
                    {
                        ControllerInstance.SetVisible(bVisible);
                    }
                }
            }
        }
    }
}