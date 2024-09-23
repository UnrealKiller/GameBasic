using Script.CoreUObject;
using Script.Engine;
using Script.GameBasic.UI.MiniMap;
using Script.UMG;
using System;
using System.Collections.Generic;

namespace Script.GameBasic
{
    public class FMiniMapUIC: UIController
    {
        public UMG_MiniMap_C UI;
        public FMiniMapUIC() {
           UI = CreateView<UMG_MiniMap_C>();
        }

        protected override void OnShowEvent()
        {
            base.OnShowEvent();
        }

        public override void OnTickEvent(float Delta)
        {
            var Character = UIManager.Get().PlayerController.Character;
            var CameraMgr = UIManager.Get().PlayerController.PlayerCameraManager;
            if (Character != null && CameraMgr != null)
            {
                FVector Location = Character.GetTransform().Translation;
                FLinearColor Offset = new FLinearColor();
                Offset.G = 1 - (float)((Location.X - (UI.Center.X - UI.Radius)) / (UI.Radius * 2));
                Offset.R = (float)((Location.Y - (UI.Center.Y - UI.Radius)) / (UI.Radius * 2));
                Offset.B = 1.0f;
                UMaterialInstanceDynamic MID = UI.Image_MiniMap.GetDynamicMaterial();
                MID.SetScalarParameterValue("Zoom", (float)UI.Zoom);
                MID.SetVectorParameterValue("Offset", Offset);
                UI.Image_Arrow.SetRenderTransformAngle((float)CameraMgr.K2_GetActorRotation().Yaw - 90);
            }  
        }
    } 
}