#include "Binding/Class/FClassBuilder.h"
#include "Environment/FCSharpEnvironment.h"
#include "GameBasicCoreModule.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UnrealCSharpHelper.h"
#include "Binding/Class/TBindingClassBuilder.inl"
#include "Macro/NamespaceMacro.h"

BINDING_CLASS(EKeys);

#define BINDING_EKEYS(KeyName) .Property(#KeyName, BINDING_READONLY_PROPERTY(&EKeys::KeyName))


class FEnhancedInputActionCSharpEventBinding : public FEnhancedInputActionEventBinding {
private:
	FEnhancedInputActionCSharpEventBinding(const FEnhancedInputActionCSharpEventBinding& CloneFrom, EInputBindingClone Clone)
		: FEnhancedInputActionEventBinding(CloneFrom, Clone)
		, Callback(CloneFrom.Callback)
		, CallbackRef(CloneFrom.CallbackRef)
		, RefCounter(CloneFrom.RefCounter)
	{
		(*RefCounter)++;
	}
public:
	FEnhancedInputActionCSharpEventBinding(const UInputAction* Action, ETriggerEvent InTriggerEvent, MonoObject* Callback)
		: FEnhancedInputActionEventBinding(Action, InTriggerEvent)
		, Callback(Callback)
		, CallbackRef(FGarbageCollectionHandle::NewRef(Callback, true))
		, RefCounter(new int)
	{
		*RefCounter = 1;
	}
	~FEnhancedInputActionCSharpEventBinding() {
		(*RefCounter)--;
		if (*RefCounter == 0) {
			FGarbageCollectionHandle::Free<false>(CallbackRef);
			delete RefCounter;
		}
	}
	void Execute(const FInputActionInstance& ActionData) const override {
		FInputActionValue InputActionValue = ActionData.GetValue();
		FVector Value = InputActionValue.Get<FVector>();
		EInputActionValueType Type = InputActionValue.GetValueType();
		static UScriptStruct* VectorStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Vector"));
		MonoClass* Class = FMonoDomain::Class_From_Name(FUnrealCSharpFunctionLibrary::GetClassNameSpace(VectorStruct), FUnrealCSharpFunctionLibrary::GetFullClass(VectorStruct));
		MonoObject* Object = FCSharpEnvironment::GetEnvironment().GetDomain()->Object_New(Class);
		FCSharpEnvironment::GetEnvironment().AddStructReference<false>(VectorStruct, &Value, Object);
		void* Params[2] = { Object, &Type };
		FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(Callback, Params);
	}
	TUniquePtr<FEnhancedInputActionEventBinding> Clone() const override {
		return TUniquePtr<FEnhancedInputActionEventBinding>(new FEnhancedInputActionCSharpEventBinding(*this, EInputBindingClone::ForceClone));
	}
	void SetShouldFireWithEditorScriptGuard(const bool bNewValue) override {}
	bool IsBoundToObject(void const* Object) const override { return false; }
private:
	MonoObject* Callback = nullptr;
	FGarbageCollectionHandle CallbackRef;
	int* RefCounter = nullptr;
};

class FInputDebugKeyBindingCSharp : public FInputDebugKeyBinding {
private:
	FInputDebugKeyBindingCSharp(const FInputDebugKeyBindingCSharp& CloneFrom, EInputBindingClone Clone)
		: FInputDebugKeyBinding(CloneFrom, Clone)
		, Callback(CloneFrom.Callback)
		, CallbackRef(CloneFrom.CallbackRef)
		, RefCounter(CloneFrom.RefCounter)
	{
		(*RefCounter)++;
	}
public:
	FInputDebugKeyBindingCSharp(const FInputChord InChord, const EInputEvent InKeyEvent, bool bInExecuteWhenPaused, MonoObject* Callback)
		: FInputDebugKeyBinding(InChord, InKeyEvent, bInExecuteWhenPaused)
		, Callback(Callback)
		, CallbackRef(FGarbageCollectionHandle::NewRef(Callback, true))
		, RefCounter(new int)
	{
		*RefCounter = 1;
	}
	~FInputDebugKeyBindingCSharp() {
		(*RefCounter)--;
		if (*RefCounter == 0) {
			FGarbageCollectionHandle::Free<false>(CallbackRef);
			delete RefCounter;
		}
	}
	void Execute(const FInputActionValue& InputActionValue) const override {
		FVector Value = InputActionValue.Get<FVector>();
		EInputActionValueType Type = InputActionValue.GetValueType();
		UClass* Object = LoadObject<UClass>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
		TArray<void*> Args;
		UnrealCSharp::MakeArgs<true>(Args, Value, Type);
		FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(Callback, Args.GetData());
	}
	TUniquePtr<FInputDebugKeyBinding> Clone() const override {
		return TUniquePtr<FInputDebugKeyBinding>(new FInputDebugKeyBindingCSharp(*this, EInputBindingClone::ForceClone));
	}
private:
	MonoObject* Callback = nullptr;
	FGarbageCollectionHandle CallbackRef;
	int* RefCounter = nullptr;
};

namespace
{
	struct FRegisterGameBasicInput
	{
		static int32 BindEnhancedInputAction(const FGarbageCollectionHandle EnhancedInputComponentHandle, const FGarbageCollectionHandle InputActionHandle, ETriggerEvent TriggleEvent, bool bExecuteWhenPaused, MonoObject* Callback)
		{
			UEnhancedInputComponent* EnhancedInputComponent = FCSharpEnvironment::GetEnvironment().GetObject<UEnhancedInputComponent>(EnhancedInputComponentHandle);
			UInputAction* InputAction = FCSharpEnvironment::GetEnvironment().GetObject<UInputAction>(InputActionHandle);
			if (EnhancedInputComponent && InputAction) {
				const TArray<TUniquePtr<FEnhancedInputActionEventBinding>>& Bindings = EnhancedInputComponent->GetActionEventBindings();
				TArray<TUniquePtr<FEnhancedInputActionEventBinding>>* BindingsPtr = const_cast<TArray<TUniquePtr<FEnhancedInputActionEventBinding>>*>(&Bindings);
				const auto& Binding = BindingsPtr->Add_GetRef(MakeUnique<FEnhancedInputActionCSharpEventBinding>(InputAction, TriggleEvent, Callback));
				return Binding->GetHandle();
			}
			UE_LOG(LogGameBasic, Error, TEXT("EnhancedInputComponent is nullptr"));
			return 0;
		}

		static bool UnBindEnhancedInputAction(const FGarbageCollectionHandle EnhancedInputComponentHandle, uint32 BindingHandle)
		{
			UEnhancedInputComponent* EnhancedInputComponent = FCSharpEnvironment::GetEnvironment().GetObject<UEnhancedInputComponent>(EnhancedInputComponentHandle);
			if (EnhancedInputComponent) {
				return EnhancedInputComponent->RemoveActionEventBinding(BindingHandle);
			}
			UE_LOG(LogGameBasic, Error, TEXT("EnhancedInputComponent is nullptr"));
			return false;
		}

		static uint32 BindDebugKey(const FGarbageCollectionHandle EnhancedInputComponentHandle, const FGarbageCollectionHandle InputChordHandle, EInputEvent InputEvent, bool bExecuteWhenPaused, MonoObject* Callback)
		{
			UEnhancedInputComponent* EnhancedInputComponent = FCSharpEnvironment::GetEnvironment().GetObject<UEnhancedInputComponent>(EnhancedInputComponentHandle);
			FInputChord* InputChord = (FInputChord*)FCSharpEnvironment::GetEnvironment().GetStruct(InputChordHandle);
			if (EnhancedInputComponent && InputChord) {
				const TArray<TUniquePtr<FInputDebugKeyBinding>>& Bindings = EnhancedInputComponent->GetDebugKeyBindings();
				TArray<TUniquePtr<FInputDebugKeyBinding>>* BindingsPtr = const_cast<TArray<TUniquePtr<FInputDebugKeyBinding>>*>(&Bindings);
				const auto& Binding = BindingsPtr->Add_GetRef(MakeUnique<FInputDebugKeyBindingCSharp>(*InputChord, InputEvent, bExecuteWhenPaused, Callback));
				return Binding->GetHandle();
			}
			UE_LOG(LogGameBasic, Error, TEXT("EnhancedInputComponent is nullptr"));
			return 0;
		}

		static bool UnBindDebugKey(const FGarbageCollectionHandle EnhancedInputComponentHandle, uint32 BindingHandle) 
		{
			UEnhancedInputComponent* EnhancedInputComponent = FCSharpEnvironment::GetEnvironment().GetObject<UEnhancedInputComponent>(EnhancedInputComponentHandle);
			if (EnhancedInputComponent) {
				return EnhancedInputComponent->RemoveDebugKeyBinding(BindingHandle);
			}
			UE_LOG(LogGameBasic, Error, TEXT("EnhancedInputComponent is nullptr"));
			return false;
		}

		FRegisterGameBasicInput()
		{
			FClassBuilder(TEXT("Input"), "GameBasic")
			.Function("BindEnhancedInputAction", BindEnhancedInputAction)
			.Function("UnBindEnhancedInputAction", UnBindEnhancedInputAction)
			.Function("BindDebugKey", BindDebugKey)
			.Function("UnBindDebugKey", UnBindDebugKey)
			;

			TBindingClassBuilder<EKeys>(NAMESPACE_BINDING)
				BINDING_EKEYS(AnyKey)

				BINDING_EKEYS(MouseX)
				BINDING_EKEYS(MouseY)
				BINDING_EKEYS(Mouse2D)
				BINDING_EKEYS(MouseScrollUp)
				BINDING_EKEYS(MouseScrollDown)
				BINDING_EKEYS(MouseWheelAxis)

				BINDING_EKEYS(LeftMouseButton)
				BINDING_EKEYS(RightMouseButton)
				BINDING_EKEYS(MiddleMouseButton)
				BINDING_EKEYS(ThumbMouseButton)
				BINDING_EKEYS(ThumbMouseButton2)

				BINDING_EKEYS(BackSpace)
				BINDING_EKEYS(Tab)
				BINDING_EKEYS(Enter)
				BINDING_EKEYS(Pause)

				BINDING_EKEYS(CapsLock)
				BINDING_EKEYS(Escape)
				BINDING_EKEYS(SpaceBar)
				BINDING_EKEYS(PageUp)
				BINDING_EKEYS(PageDown)
				BINDING_EKEYS(End)
				BINDING_EKEYS(Home)

				BINDING_EKEYS(Left)
				BINDING_EKEYS(Up)
				BINDING_EKEYS(Right)
				BINDING_EKEYS(Down)

				BINDING_EKEYS(Insert)
				BINDING_EKEYS(Delete)

				BINDING_EKEYS(Zero)
				BINDING_EKEYS(One)
				BINDING_EKEYS(Two)
				BINDING_EKEYS(Three)
				BINDING_EKEYS(Four)
				BINDING_EKEYS(Five)
				BINDING_EKEYS(Six)
				BINDING_EKEYS(Seven)
				BINDING_EKEYS(Eight)
				BINDING_EKEYS(Nine)

				BINDING_EKEYS(A)
				BINDING_EKEYS(B)
				BINDING_EKEYS(C)
				BINDING_EKEYS(D)
				BINDING_EKEYS(E)
				BINDING_EKEYS(F)
				BINDING_EKEYS(G)
				BINDING_EKEYS(H)
				BINDING_EKEYS(I)
				BINDING_EKEYS(J)
				BINDING_EKEYS(K)
				BINDING_EKEYS(L)
				BINDING_EKEYS(M)
				BINDING_EKEYS(N)
				BINDING_EKEYS(O)
				BINDING_EKEYS(P)
				BINDING_EKEYS(Q)
				BINDING_EKEYS(R)
				BINDING_EKEYS(S)
				BINDING_EKEYS(T)
				BINDING_EKEYS(U)
				BINDING_EKEYS(V)
				BINDING_EKEYS(W)
				BINDING_EKEYS(X)
				BINDING_EKEYS(Y)
				BINDING_EKEYS(Z)

				BINDING_EKEYS(NumPadZero)
				BINDING_EKEYS(NumPadOne)
				BINDING_EKEYS(NumPadTwo)
				BINDING_EKEYS(NumPadThree)
				BINDING_EKEYS(NumPadFour)
				BINDING_EKEYS(NumPadFive)
				BINDING_EKEYS(NumPadSix)
				BINDING_EKEYS(NumPadSeven)
				BINDING_EKEYS(NumPadEight)
				BINDING_EKEYS(NumPadNine)

				BINDING_EKEYS(Multiply)
				BINDING_EKEYS(Add)
				BINDING_EKEYS(Subtract)
				BINDING_EKEYS(Decimal)
				BINDING_EKEYS(Divide)

				BINDING_EKEYS(F1)
				BINDING_EKEYS(F2)
				BINDING_EKEYS(F3)
				BINDING_EKEYS(F4)
				BINDING_EKEYS(F5)
				BINDING_EKEYS(F6)
				BINDING_EKEYS(F7)
				BINDING_EKEYS(F8)
				BINDING_EKEYS(F9)
				BINDING_EKEYS(F10)
				BINDING_EKEYS(F11)
				BINDING_EKEYS(F12)

				BINDING_EKEYS(NumLock)

				BINDING_EKEYS(ScrollLock)

				BINDING_EKEYS(LeftShift)
				BINDING_EKEYS(RightShift)
				BINDING_EKEYS(LeftControl)
				BINDING_EKEYS(RightControl)
				BINDING_EKEYS(LeftAlt)
				BINDING_EKEYS(RightAlt)
				BINDING_EKEYS(LeftCommand)
				BINDING_EKEYS(RightCommand)

				BINDING_EKEYS(Semicolon)
				.Property("EqualsKey", BINDING_READONLY_PROPERTY(&EKeys::Equals))
				BINDING_EKEYS(Comma)
				BINDING_EKEYS(Underscore)
				BINDING_EKEYS(Hyphen)
				BINDING_EKEYS(Period)
				BINDING_EKEYS(Slash)
				BINDING_EKEYS(Tilde)
				BINDING_EKEYS(LeftBracket)
				BINDING_EKEYS(Backslash)
				BINDING_EKEYS(RightBracket)
				BINDING_EKEYS(Apostrophe)

				BINDING_EKEYS(Ampersand)
				BINDING_EKEYS(Asterix)
				BINDING_EKEYS(Caret)
				BINDING_EKEYS(Colon)
				BINDING_EKEYS(Dollar)
				BINDING_EKEYS(Exclamation)
				BINDING_EKEYS(LeftParantheses)
				BINDING_EKEYS(RightParantheses)
				BINDING_EKEYS(Quote)

				BINDING_EKEYS(A_AccentGrave)
				BINDING_EKEYS(E_AccentGrave)
				BINDING_EKEYS(E_AccentAigu)
				BINDING_EKEYS(C_Cedille)
				BINDING_EKEYS(Section)

				// Platform Keys
				// These keys platform specific versions of keys that go by different names.
				// The delete key is a good example, on Windows Delete is the virtual key for Delete.
				// On Macs, the Delete key is the virtual key for BackSpace.
				BINDING_EKEYS(Platform_Delete)

				// Gamepad Keys
				BINDING_EKEYS(Gamepad_Left2D)
				BINDING_EKEYS(Gamepad_LeftX)
				BINDING_EKEYS(Gamepad_LeftY)
				BINDING_EKEYS(Gamepad_Right2D)
				BINDING_EKEYS(Gamepad_RightX)
				BINDING_EKEYS(Gamepad_RightY)
				BINDING_EKEYS(Gamepad_LeftTriggerAxis)
				BINDING_EKEYS(Gamepad_RightTriggerAxis)

				BINDING_EKEYS(Gamepad_LeftThumbstick)
				BINDING_EKEYS(Gamepad_RightThumbstick)
				BINDING_EKEYS(Gamepad_Special_Left)
				BINDING_EKEYS(Gamepad_Special_Left_X)
				BINDING_EKEYS(Gamepad_Special_Left_Y)
				BINDING_EKEYS(Gamepad_Special_Right)
				BINDING_EKEYS(Gamepad_FaceButton_Bottom)
				BINDING_EKEYS(Gamepad_FaceButton_Right)
				BINDING_EKEYS(Gamepad_FaceButton_Left)
				BINDING_EKEYS(Gamepad_FaceButton_Top)
				BINDING_EKEYS(Gamepad_LeftShoulder)
				BINDING_EKEYS(Gamepad_RightShoulder)
				BINDING_EKEYS(Gamepad_LeftTrigger)
				BINDING_EKEYS(Gamepad_RightTrigger)
				BINDING_EKEYS(Gamepad_DPad_Up)
				BINDING_EKEYS(Gamepad_DPad_Down)
				BINDING_EKEYS(Gamepad_DPad_Right)
				BINDING_EKEYS(Gamepad_DPad_Left)

				// Virtual key codes used for input axis button press/release emulation
				BINDING_EKEYS(Gamepad_LeftStick_Up)
				BINDING_EKEYS(Gamepad_LeftStick_Down)
				BINDING_EKEYS(Gamepad_LeftStick_Right)
				BINDING_EKEYS(Gamepad_LeftStick_Left)

				BINDING_EKEYS(Gamepad_RightStick_Up)
				BINDING_EKEYS(Gamepad_RightStick_Down)
				BINDING_EKEYS(Gamepad_RightStick_Right)
				BINDING_EKEYS(Gamepad_RightStick_Left)

				// static const FKey Vector axes (FVector) not float)
				BINDING_EKEYS(Tilt)
				BINDING_EKEYS(RotationRate)
				BINDING_EKEYS(Gravity)
				BINDING_EKEYS(Acceleration)

				// Gestures
				BINDING_EKEYS(Gesture_Pinch)
				BINDING_EKEYS(Gesture_Flick)
				BINDING_EKEYS(Gesture_Rotate)

				// Steam Controller Specific
				BINDING_EKEYS(Steam_Touch_0)
				BINDING_EKEYS(Steam_Touch_1)
				BINDING_EKEYS(Steam_Touch_2)
				BINDING_EKEYS(Steam_Touch_3)
				BINDING_EKEYS(Steam_Back_Left)
				BINDING_EKEYS(Steam_Back_Right)

				// Xbox One global speech commands
				BINDING_EKEYS(Global_Menu)
				BINDING_EKEYS(Global_View)
				BINDING_EKEYS(Global_Pause)
				BINDING_EKEYS(Global_Play)
				BINDING_EKEYS(Global_Back)

				// Android-specific
				BINDING_EKEYS(Android_Back)
				BINDING_EKEYS(Android_Volume_Up)
				BINDING_EKEYS(Android_Volume_Down)
				BINDING_EKEYS(Android_Menu)

				// HTC Vive Controller
				BINDING_EKEYS(Vive_Left_Grip_Click)
				BINDING_EKEYS(Vive_Left_Menu_Click)
				BINDING_EKEYS(Vive_Left_Trigger_Click)
				BINDING_EKEYS(Vive_Left_Trigger_Axis)
				BINDING_EKEYS(Vive_Left_Trackpad_2D)
				BINDING_EKEYS(Vive_Left_Trackpad_X)
				BINDING_EKEYS(Vive_Left_Trackpad_Y)
				BINDING_EKEYS(Vive_Left_Trackpad_Click)
				BINDING_EKEYS(Vive_Left_Trackpad_Touch)
				BINDING_EKEYS(Vive_Left_Trackpad_Up)
				BINDING_EKEYS(Vive_Left_Trackpad_Down)
				BINDING_EKEYS(Vive_Left_Trackpad_Left)
				BINDING_EKEYS(Vive_Left_Trackpad_Right)
				BINDING_EKEYS(Vive_Right_Grip_Click)
				BINDING_EKEYS(Vive_Right_Menu_Click)
				BINDING_EKEYS(Vive_Right_Trigger_Click)
				BINDING_EKEYS(Vive_Right_Trigger_Axis)
				BINDING_EKEYS(Vive_Right_Trackpad_2D)
				BINDING_EKEYS(Vive_Right_Trackpad_X)
				BINDING_EKEYS(Vive_Right_Trackpad_Y)
				BINDING_EKEYS(Vive_Right_Trackpad_Click)
				BINDING_EKEYS(Vive_Right_Trackpad_Touch)
				BINDING_EKEYS(Vive_Right_Trackpad_Up)
				BINDING_EKEYS(Vive_Right_Trackpad_Down)
				BINDING_EKEYS(Vive_Right_Trackpad_Left)
				BINDING_EKEYS(Vive_Right_Trackpad_Right)

				// Microsoft Mixed Reality Motion Controller
				BINDING_EKEYS(MixedReality_Left_Menu_Click)
				BINDING_EKEYS(MixedReality_Left_Grip_Click)
				BINDING_EKEYS(MixedReality_Left_Trigger_Click)
				BINDING_EKEYS(MixedReality_Left_Trigger_Axis)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_2D)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_X)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_Y)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_Click)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_Up)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_Down)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_Left)
				BINDING_EKEYS(MixedReality_Left_Thumbstick_Right)
				BINDING_EKEYS(MixedReality_Left_Trackpad_2D)
				BINDING_EKEYS(MixedReality_Left_Trackpad_X)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Y)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Click)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Touch)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Up)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Down)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Left)
				BINDING_EKEYS(MixedReality_Left_Trackpad_Right)
				BINDING_EKEYS(MixedReality_Right_Menu_Click)
				BINDING_EKEYS(MixedReality_Right_Grip_Click)
				BINDING_EKEYS(MixedReality_Right_Trigger_Click)
				BINDING_EKEYS(MixedReality_Right_Trigger_Axis)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_2D)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_X)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_Y)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_Click)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_Up)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_Down)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_Left)
				BINDING_EKEYS(MixedReality_Right_Thumbstick_Right)
				BINDING_EKEYS(MixedReality_Right_Trackpad_2D)
				BINDING_EKEYS(MixedReality_Right_Trackpad_X)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Y)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Click)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Touch)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Up)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Down)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Left)
				BINDING_EKEYS(MixedReality_Right_Trackpad_Right)

				// Oculus Touch Controller
				BINDING_EKEYS(OculusTouch_Left_X_Click)
				BINDING_EKEYS(OculusTouch_Left_Y_Click)
				BINDING_EKEYS(OculusTouch_Left_X_Touch)
				BINDING_EKEYS(OculusTouch_Left_Y_Touch)
				BINDING_EKEYS(OculusTouch_Left_Menu_Click)
				BINDING_EKEYS(OculusTouch_Left_Grip_Click)
				BINDING_EKEYS(OculusTouch_Left_Grip_Axis)
				BINDING_EKEYS(OculusTouch_Left_Trigger_Click)
				BINDING_EKEYS(OculusTouch_Left_Trigger_Axis)
				BINDING_EKEYS(OculusTouch_Left_Trigger_Touch)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_2D)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_X)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Y)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Click)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Touch)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Up)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Down)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Left)
				BINDING_EKEYS(OculusTouch_Left_Thumbstick_Right)
				BINDING_EKEYS(OculusTouch_Right_A_Click)
				BINDING_EKEYS(OculusTouch_Right_B_Click)
				BINDING_EKEYS(OculusTouch_Right_A_Touch)
				BINDING_EKEYS(OculusTouch_Right_B_Touch)
				BINDING_EKEYS(OculusTouch_Right_Grip_Click)
				BINDING_EKEYS(OculusTouch_Right_Grip_Axis)
				BINDING_EKEYS(OculusTouch_Right_Trigger_Click)
				BINDING_EKEYS(OculusTouch_Right_Trigger_Axis)
				BINDING_EKEYS(OculusTouch_Right_Trigger_Touch)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_2D)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_X)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Y)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Click)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Touch)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Up)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Down)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Left)
				BINDING_EKEYS(OculusTouch_Right_Thumbstick_Right)

				// Valve Index Controller
				BINDING_EKEYS(ValveIndex_Left_A_Click)
				BINDING_EKEYS(ValveIndex_Left_B_Click)
				BINDING_EKEYS(ValveIndex_Left_A_Touch)
				BINDING_EKEYS(ValveIndex_Left_B_Touch)
				BINDING_EKEYS(ValveIndex_Left_Grip_Axis)
				BINDING_EKEYS(ValveIndex_Left_Grip_Force)
				BINDING_EKEYS(ValveIndex_Left_Trigger_Click)
				BINDING_EKEYS(ValveIndex_Left_Trigger_Axis)
				BINDING_EKEYS(ValveIndex_Left_Trigger_Touch)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_2D)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_X)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Y)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Click)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Touch)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Up)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Down)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Left)
				BINDING_EKEYS(ValveIndex_Left_Thumbstick_Right)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_2D)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_X)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Y)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Force)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Touch)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Up)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Down)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Left)
				BINDING_EKEYS(ValveIndex_Left_Trackpad_Right)
				BINDING_EKEYS(ValveIndex_Right_A_Click)
				BINDING_EKEYS(ValveIndex_Right_B_Click)
				BINDING_EKEYS(ValveIndex_Right_A_Touch)
				BINDING_EKEYS(ValveIndex_Right_B_Touch)
				BINDING_EKEYS(ValveIndex_Right_Grip_Axis)
				BINDING_EKEYS(ValveIndex_Right_Grip_Force)
				BINDING_EKEYS(ValveIndex_Right_Trigger_Click)
				BINDING_EKEYS(ValveIndex_Right_Trigger_Axis)
				BINDING_EKEYS(ValveIndex_Right_Trigger_Touch)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_2D)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_X)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Y)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Click)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Touch)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Up)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Down)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Left)
				BINDING_EKEYS(ValveIndex_Right_Thumbstick_Right)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_2D)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_X)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Y)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Force)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Touch)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Up)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Down)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Left)
				BINDING_EKEYS(ValveIndex_Right_Trackpad_Right)

				// Virtual buttons that use other buttons depending on the platform
				BINDING_EKEYS(Virtual_Accept)
				BINDING_EKEYS(Virtual_Back)

				BINDING_EKEYS(Invalid);

			//static const int32 NUM_TOUCH_KEYS = 11;
			//BINDING_EKEYS(TouchKeys[NUM_TOUCH_KEYS];

		}
	};

	[[maybe_unused]] FRegisterGameBasicInput RegisterGameBasicInput;
}
