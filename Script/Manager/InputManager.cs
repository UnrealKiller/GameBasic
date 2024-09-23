using Script.CoreUObject;
using Script.Engine;
using Script.EnhancedInput;
using Script.InputCore;
using Script.Slate;
using Script.GameBasicCore;
using System;

namespace Script.GameBasic
{
    public partial class InputManager
    {
        public delegate bool Tic1Callback(float Delta);

        private static readonly InputManager Instance = new InputManager();
        private InputManager() { }
        public static InputManager Get() { return Instance; }

        public UEnhancedInputLocalPlayerSubsystem EnhancedInputLocalPlayerSubsystem = null;
        public UEnhancedInputComponent EnhancedInputComponent = null;

        public void Initialize(APlayerController PlayerController)
        {
            EnhancedInputComponent = UGameBasicBlueprintLibrary.GetEnhancedInputComponent(PlayerController);
            EnhancedInputLocalPlayerSubsystem = UGameBasicBlueprintLibrary.GetEnhancedInputLocalPlayerSubsystem(PlayerController);
        }

        public void BindAction(UInputAction Action, ETriggerEvent TriggerEvent, Action<FVector, EInputActionValueType> Callback)
        {
            
        }

        public bool UnBindAction(UInputAction Action) 
        { 
            return false;
        }

        public uint BindDebugKey(FInputChord InputChord, EInputEvent InputEvent, bool bExecuteWhenPaused, Delegate Callback)
            =>InputImplementation.Input_BindDebugKeyImplementation(EnhancedInputComponent.GarbageCollectionHandle, InputChord.GarbageCollectionHandle, InputEvent, bExecuteWhenPaused, Callback);

        public uint BindDebugKey(FKey Key, EInputEvent InputEvent, bool bExecuteWhenPaused, Delegate Callback)
        {
            FInputChord InputChord = new FInputChord();
            InputChord.Key = Key;
            return BindDebugKey(InputChord, InputEvent, bExecuteWhenPaused, Callback);
        }

        public bool UnBindDebugKey(uint BindingHandle)
            => InputImplementation.Input_UnBindDebugKeyImplementation(EnhancedInputComponent.GarbageCollectionHandle, BindingHandle);

        public void AddMappingContext(UInputMappingContext Context, int Priority = 0 , FModifyContextOptions ModifyContextOptions = null)
        {
            EnhancedInputLocalPlayerSubsystem.AddMappingContext(Context, Priority, ModifyContextOptions != null ? ModifyContextOptions : new FModifyContextOptions());
        }

        public void RemoveMappingContext(UInputMappingContext Context, FModifyContextOptions ModifyContextOptions)
        {
            EnhancedInputLocalPlayerSubsystem.RemoveMappingContext(Context, ModifyContextOptions != null ? ModifyContextOptions : new FModifyContextOptions());
        }

        public void ClearAllMappings()
        {
            EnhancedInputLocalPlayerSubsystem.ClearAllMappings();
        }
    }
}