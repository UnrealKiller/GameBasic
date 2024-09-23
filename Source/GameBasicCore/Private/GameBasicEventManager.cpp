#include "GameBasicEventManager.h"
#include "Binding/Class/FClassBuilder.h"
#include "Environment/FCSharpEnvironment.h"
#include "Delegate/FUnrealCSharpModuleDelegates.h"

IGameBasicEventListener::IGameBasicEventListener()
{
	mID = IDStack++;
}

uint32 IGameBasicEventListener::IDStack = 0;

FGameBasicEventListenerCSharp::FGameBasicEventListenerCSharp(MonoObject* Owner, MonoObject* Callback)
	: mOwner(Owner)
	, mOwnerRef(FMonoDomain::GCHandle_New_WeakRef_V2(Owner, false))
	, mCallback(Callback)
	, mCallbackRef(FGarbageCollectionHandle::NewRef(Callback, true))
{

}

FGameBasicEventListenerCSharp::~FGameBasicEventListenerCSharp()
{
	FGarbageCollectionHandle::Free<false>(mOwnerRef);
	FGarbageCollectionHandle::Free<false>(mCallbackRef);
}

void FGameBasicEventListenerCSharp::InvokeByCSharp(MonoArray* Params)
{
	auto Length = FMonoDomain::Array_Length(Params);
	TArray<void*> Args;
	Args.SetNum(Length);
	for (int i = 0; i < Length; i++) {
		MonoObject* Param = ARRAY_GET(Params, MonoObject*, i);
		MonoClass* Class = FMonoDomain::Object_Get_Class(Param);
		bool bIsPrimitive = (bool)FMonoDomain::Class_Is_ValueType(Class);
		Args[i] = bIsPrimitive ? FMonoDomain::Object_Unbox(Param) : Param;
	}
	FMonoDomain::Runtime_Delegate_Invoke(mCallback, Args.GetData());
}

bool FGameBasicEventListenerCSharp::IsValid()
{
	if (mOwner) {
		return FMonoDomain::GCHandle_Get_Target_V2(mOwnerRef) != nullptr;
	}
	return false;
}

FGameBasicEventManager* FGameBasicEventManager::Get()
{
	static FGameBasicEventManager Instance;
	return &Instance;
}

void FGameBasicEventManager::Cleanup()
{
	mListenerMap.Empty();
	mIdMap.Empty();
}

bool FGameBasicEventManager::RemoveListener(uint32 ListenerID)
{
	if (auto ListenerInfoPtr = mIdMap.Find(ListenerID)) {
		if (auto ListenerArrayPtr = mListenerMap.Find(ListenerInfoPtr->Key)) {
			for (auto Listener : *ListenerArrayPtr) {
				if (Listener.Get() == ListenerInfoPtr->Value) {
					mIdMap.Remove(ListenerID);
					ListenerArrayPtr->Remove(Listener);
					return true;
				}
			}
		}
	}
	return false;
}

void FGameBasicEventManager::Dump()
{
	UE_LOG(LogGameBasic, Log, TEXT("----------------------[Event Dump Begin]-----------------------"));
	for (const auto& Item : FGameBasicEventManager::Get()->mListenerMap) {
		UE_LOG(LogGameBasic, Log, TEXT("------------- %30s: %d --------------- "), *Item.Key.ToString(), Item.Value.Num());
#if !UE_BUILD_SHIPPING 
		for (auto Listener : Item.Value) {
			UE_LOG(LogGameBasic, Log, TEXT("--------------- Dispatch[%d] Creation Traceback Begin: \n%s\n--------------- Traceback End"), Listener->GetID(), *Listener->CreationTraceback);
		}
#endif
	}
	UE_LOG(LogGameBasic, Log, TEXT("----------------------[Event Dump End  ]-----------------------"));
}

uint32 FGameBasicEventManager::AddListenerByCSharp(MonoString* EventNameStr, MonoObject* Owner, MonoObject* Callback, bool bOnlyOnce)
{
	const auto EventName = StringCast<TCHAR>(FMonoDomain::String_To_UTF8(EventNameStr));
	auto& EventArray = FGameBasicEventManager::Get()->mListenerMap.FindOrAdd(FName(EventName));
	auto& Listener = EventArray.Add_GetRef(MakeShared<FGameBasicEventListenerCSharp>(Owner, Callback));
	if (bOnlyOnce) {
		Listener->SetOnlyOnce(bOnlyOnce);
	}
#if !UE_BUILD_SHIPPING 
	Listener->CreationTraceback = FString(UTF8_TO_TCHAR(FCSharpEnvironment::GetEnvironment().GetDomain()->String_To_UTF8(
		FCSharpEnvironment::GetEnvironment().GetDomain()->GetTraceback())));
#endif
	FGameBasicEventManager::Get()->mIdMap.Add(Listener->GetID(), TPair<FName, IGameBasicEventListener*>(EventName, Listener.Get()));
	return Listener->GetID();
}

bool FGameBasicEventManager::RemoveListenerByCSharp(uint32 ListenerID)
{
	return FGameBasicEventManager::Get()->RemoveListener(ListenerID);
}

void FGameBasicEventManager::DispatchByCSharp(MonoString* EventNameStr, MonoArray* Params)
{
	FName EventName = FName(StringCast<TCHAR>(FMonoDomain::String_To_UTF8(EventNameStr)));
#if !UE_BUILD_SHIPPING
	UE_LOG(LogGameBasic, Log, TEXT("Dispatch[Lua][%s]"), *EventName.ToString());
#endif
	if (auto ListenerArrayPtr = FGameBasicEventManager::Get()->mListenerMap.Find(EventName)) {
		TArray<TSharedPtr<IGameBasicEventListener>> NewIter = *ListenerArrayPtr;
		for (const TSharedPtr<IGameBasicEventListener>& Listener : NewIter) {
			if (Listener) {
				if (Listener->IsValid()) {
					Listener->InvokeByCSharp(Params);
					if (Listener->IsOnlyOnce()) {
						FGameBasicEventManager::Get()->mListenerMap.FindOrAdd(EventName).Remove(Listener);
					}
				}
				else {
					UE_LOG(LogGameBasic, Warning, TEXT("Event[%s] listener has become invalid, we will remove it"), *EventName.ToString())
#if !UE_BUILD_SHIPPING 
						UE_LOG(LogGameBasic, Log, TEXT("--------------- Listener[%d] Creation Traceback Begin: \n%s\n--------------- Traceback End"), Listener->GetID(), *Listener->CreationTraceback);

#endif
					FGameBasicEventManager::Get()->mListenerMap.FindOrAdd(EventName).Remove(Listener);
				}
			}
		}
	}
}

void FGameBasicEventManager::DumpByCSharp()
{
	FGameBasicEventManager::Get()->Dump();
}

namespace
{
	struct FRegisterGameBasicEvent
	{
		FRegisterGameBasicEvent()
		{
			FClassBuilder(TEXT("Event"), "GameBasic")
				.Function("AddListener", FGameBasicEventManager::AddListenerByCSharp)
				.Function("RemoveListener", FGameBasicEventManager::RemoveListenerByCSharp)
				.Function("Dispatch", FGameBasicEventManager::DispatchByCSharp)
				.Function("Dump", FGameBasicEventManager::DumpByCSharp);

			FUnrealCSharpModuleDelegates::OnUnrealCSharpModuleInActive.AddLambda([]() {
				FGameBasicEventManager::Get()->Cleanup();
			});
		}
	};

	[[maybe_unused]] FRegisterGameBasicEvent RegisterGameBasicEvent;
}
