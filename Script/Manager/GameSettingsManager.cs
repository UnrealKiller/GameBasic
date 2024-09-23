using Script.CoreUObject;
using System;
using System.Collections.Generic;

namespace Script.GameBasic
{
    public sealed class GameSettingsManager
    {
        private static readonly GameSettingsManager Instance = new GameSettingsManager();
        public static GameSettingsManager Get() { return Instance; }

        private GameSettingsManager()
        {
        }

        public void Show() 
        { 
           
        }

        public void Hide()
        {

        }
    }
}