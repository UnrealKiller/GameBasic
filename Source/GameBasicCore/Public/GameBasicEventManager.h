#pragma once

#include "Templates/Function.h"
#include "Containers/Map.h"
#include "GameBasicCoreModule.h"
#include "GarbageCollection/FGarbageCollectionHandle.h"

class IGameBasicEventListener {
public:
	IGameBasicEventListener();
	virtual ~IGameBasicEventListener() {}
	virtual bool IsCreateByCSharp() const { return false; }
	virtual void InvokeByCSharp(MonoArray* Params) = 0;
	uint32 GetID() const { return mID; }
	void SetOnlyOnce(bool inOnce) { bOnlyOnce = inOnce; }
	bool IsOnlyOnce() const { return bOnlyOnce; }
	virtual bool IsValid() { return true; }
private:
	bool bOnlyOnce = false;
	uint32 mID = -1;
	static uint32 IDStack;
#if !UE_BUILD_SHIPPING 
public:
	FString CreationTraceback;
#endif
};

template <typename ...ArgsType>
class FGameBasicEventListenerCpp : public IGameBasicEventListener {
public:
	FGameBasicEventListenerCpp(const TFunction<void(ArgsType...)>& Callback)
		: mCallback(Callback) {}

	void InvokeByCSharp(MonoArray* Params) override {

	}
	void Invoke(ArgsType&&... Args) {
		mCallback(Forward<ArgsType>(Args)...);
	}
private:
	TFunction<void(ArgsType...)> mCallback;
};

class FGameBasicEventListenerCSharp : public IGameBasicEventListener {
public:
	FGameBasicEventListenerCSharp(MonoObject* Owner, MonoObject* Callback);
	~FGameBasicEventListenerCSharp();

	bool IsCreateByCSharp() const override { return true; }

	void InvokeByCSharp(MonoArray* Params) override;

	template <typename ...ArgsType>
	void Invoke(ArgsType&&... Args) {
		//constexpr int Expected = sizeof...(ArgsType);
		//UnCSharp::PushArgs<true>(mL, Forward<ArgsType>(Args)...);
		//CSharpHelper::CallHandle(mL, mCallbackHandle, Expected);
	}

	bool IsValid() override;
private:
	MonoObject* mOwner = nullptr;
	FGarbageCollectionHandle mOwnerRef;
	MonoObject* mCallback = nullptr;
	FGarbageCollectionHandle mCallbackRef;
};

class FGameBasicEventManager
{
public:
	static FGameBasicEventManager* Get();

	void Cleanup();

	template <typename ...ArgsType>
	uint32 AddListener(FName EventName, const TFunction<void(ArgsType...)>& Callback, bool bOnlyOnce = false) {
		auto& Listener = mListenerMap.FindOrAdd(EventName).Add_GetRef(MakeShared<FGameBasicEventListenerCpp<ArgsType...>>(Callback));
		if (bOnlyOnce) {
			Listener->SetOnlyOnce(true);
		}
		mIdMap.Add(Listener->GetID(), TPair<FName, IGameBasicEventListener*>(EventName, Listener.Get()));
		return Listener->GetID();
	}

	bool RemoveListener(uint32 ListenerID);

	template <typename ...ArgsType>
	void Dispatch(FName EventName, ArgsType... Args) {
#if !UE_BUILD_SHIPPING
		UE_LOG(LogGameBasic, Warning, TEXT("Dispatch[C++][%s]"), *EventName.ToString())
#endif
			if (auto ListenerArrayPtr = FGameBasicEventManager::Get()->mListenerMap.Find(EventName)) {
				TArray<TSharedPtr<IGameBasicEventListener>> NewIter = *ListenerArrayPtr;
				for (TSharedPtr<IGameBasicEventListener>& ListenerBase : NewIter) {
					IGameBasicEventListener* Listener = ListenerBase.Get();
					if (Listener) {
						if (Listener->IsCreateByCSharp()) {
							//FGameBasicEventListenerCSharp* CSharpListener = static_cast<FGameBasicEventListenerCSharp*>(Listener);
							//CSharpListener->Invoke(Forward<ArgsType>(Args)...);
						}
						else {
							FGameBasicEventListenerCpp<ArgsType...>* CppListener = static_cast<FGameBasicEventListenerCpp<ArgsType...>*>(Listener);
							CppListener->Invoke(Forward<ArgsType>(Args)...);
						}
						if (Listener->IsOnlyOnce()) {
							FGameBasicEventManager::Get()->mListenerMap.FindOrAdd(EventName).Remove(ListenerBase);
							mIdMap.Remove(Listener->GetID());
						}
					}
				}
			}
	}

	void Dump();

	static uint32 AddListenerByCSharp(MonoString* EventName, MonoObject* Owner, MonoObject* Callback, bool bOnlyOnce = false);
	static bool RemoveListenerByCSharp(uint32 ListenerID);
	static void DispatchByCSharp(MonoString* EventName, MonoArray* Params);
	static void DumpByCSharp();
private:
	TMap<FName, TArray<TSharedPtr<IGameBasicEventListener>>> mListenerMap;
	TMap<uint32, TPair<FName, IGameBasicEventListener*>> mIdMap;
};