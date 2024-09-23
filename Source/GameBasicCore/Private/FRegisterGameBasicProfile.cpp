#include "Binding/Class/FClassBuilder.h"
#include "Environment/FCSharpEnvironment.h"
#include "GameBasicCoreModule.h"

namespace
{
	class FDeleteListener : public FUObjectArray::FUObjectDeleteListener {
	public:
		static FDeleteListener* Instance();
		void TraceObjectDeleted(UObject* InObject);
	protected:
		FDeleteListener();
		virtual void NotifyUObjectDeleted(const class UObjectBase* Object, int32 Index) override;
		virtual void OnUObjectArrayShutdown() override;
	private:
		TSet<UObject*> mTraceObjects;
	};

	FDeleteListener* FDeleteListener::Instance()
	{
		static FDeleteListener Inst;
		return &Inst;
	}

	FDeleteListener::FDeleteListener()
	{
		GUObjectArray.AddUObjectDeleteListener(this);
	}

	void FDeleteListener::TraceObjectDeleted(UObject* InObject)
	{
		if (InObject) {
			UE_LOG(LogGameBasic, Warning, TEXT("Begin Trace Object: %p\n\t\tObjectPath: %s\n\t\tClassPath : %s\n\t\tOuterPath : %s")
				, InObject
				, *InObject->GetPathName()
				, *InObject->GetClass()->GetPathName()
				, (InObject->GetOuter() ? *InObject->GetOuter()->GetPathName() : TEXT("null")));
			mTraceObjects.Add(InObject);
		}
	}

	void FDeleteListener::NotifyUObjectDeleted(const UObjectBase* ObjectBase, int32 Index)
	{
		UObject* Object = (UObject*)ObjectBase;
		int32 Num = mTraceObjects.Remove(Object);
		if (Num) {
			UE_LOG(LogGameBasic, Warning, TEXT("End Trace Object: %p\n\t\tObjectPath: %s\n\t\tClassPath : %s\n\t\tOuterPath : %s")
				, Object
				, *Object->GetPathName()
				, *Object->GetClass()->GetPathName()
				, (Object->GetOuter() ? *Object->GetOuter()->GetPathName() : TEXT("null")));
		}
	}

	void FDeleteListener::OnUObjectArrayShutdown()
	{
		GUObjectArray.RemoveUObjectDeleteListener(this);
	}

	static void TraceObjectDeleted(FGarbageCollectionHandle ObjectHandle)
	{
		if (UObject* Object = FCSharpEnvironment::GetEnvironment().GetObject<UObject>(ObjectHandle)) {
			FDeleteListener::Instance()->TraceObjectDeleted(Object);
		}
	}

	static double TraceCPUScope(MonoString*  EventScopeName, MonoObject* InScopeCallback)
	{
		const auto EventName = FString(StringCast<TCHAR>(FMonoDomain::String_To_UTF8(EventScopeName)));
		uint64 StartTime = FPlatformTime::Cycles64();
		{
			TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(UTF8_TO_TCHAR(*EventName));
			FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(InScopeCallback, nullptr);
		}
		double Duration = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartTime);
		return Duration;
	}

	struct FRegisterGameBasicProfile
	{
		FRegisterGameBasicProfile()
		{
			FClassBuilder(TEXT("Profile"), "GameBasic")
			.Function("TraceObjectDeleted", TraceObjectDeleted)
			.Function("TraceCPUScope", TraceCPUScope)
		;
		}
	};

	[[maybe_unused]] FRegisterGameBasicProfile RegisterGameBasicProfile;
}
