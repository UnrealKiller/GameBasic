using Script.CoreUObject;
using Script.Engine;
using Script.UMG;
using System.Collections.Generic;
using System;
using System.Collections;
using Script.PhysicsCore;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using System.Text;

namespace Script.GameBasic
{
    public partial class UIController
    {
        public int ZOrder = 0;
        private List<UUserWidget> Views = new List<UUserWidget>();
        private Dictionary<UUserWidget, ESlateVisibility> CachedVisibilities = new Dictionary<UUserWidget, ESlateVisibility>();
        public bool bVisible { get; private set; } = false;
        private bool bHasDestroyed = false;
        private bool bHasAddToViewport = false;
        public UIController()
        {
            UIManager.Get().RegisterUIController(this);
        }

        ~UIController()
        {
            if (!bHasDestroyed)
            {
                AsyncManager.Get().AddAsyncTask(ENamedThreads.GameThread, delegate ()
                {
                    Destory();
                });
            }
        }

        public void SetVisible(bool bVisible)
        {
            if (this.bHasDestroyed)
            {
                Console.WriteLine("UI {0} is destroyed", this.GetType().Name);
                return;
            }
                
            if (this.bVisible == bVisible)
                return;
            this.bVisible = bVisible;
            if (this.bVisible)
            {
                if (!bHasAddToViewport)
                {
                    foreach (var View in Views)
                    {
                        View.AddToViewport(ZOrder);
                        CachedVisibilities.Add(View, View.GetVisibility());
                    }
                    this.OnInitialize();
                    this.bHasAddToViewport = true;
                }
                else
                {
                    foreach (var View in Views)
                    {
                        View.SetVisibility(CachedVisibilities[View]);
                    }
                }
                this.OnShowEvent();
            }
            else
            {
                foreach (var View in Views)
                {
                    View.SetVisibility(ESlateVisibility.Hidden);
                }
                this.OnHideEvent();
            }
        }

        public void Show() { SetVisible(true); }

        public void Hide() { SetVisible(false); }   

        public T CreateView<T>(bool IsTopLevel = true) where T : UUserWidget
        {
            UClass UMGClass = Unreal.LoadObject<UClass>(null, Utils.GetPathName(typeof(T)));
            if (!UMGClass.IsValid())
                return null;
            UUserWidget UserWidget = UWidgetBlueprintLibrary.Create(UIManager.Get().GameInstance, UMGClass, UIManager.Get().PlayerController);
            if (IsTopLevel)
            {
                Views.Add(UserWidget);
            }
            return UserWidget as T;
        }

        public void Destory()
        {
            if (!bHasDestroyed)
            {
                bHasDestroyed = true;
                if (bVisible)
                {
                    this.OnHideEvent();
                }
                foreach (var View in Views)
                {
                    View.RemoveFromParent();
                }
                Views.Clear();
                CachedVisibilities.Clear();
                OnDestroy();
                UIManager.Get().UnregisterUIController(this);
            }
        }

        protected virtual void OnInitialize() { }

        protected virtual void OnDestroy() { }

        protected virtual void OnShowEvent() { }

        protected virtual void OnHideEvent() { }

        public virtual void OnTickEvent(float Delta) { }

    }
}