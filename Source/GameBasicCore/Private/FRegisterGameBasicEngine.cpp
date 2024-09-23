#include "Binding/Class/FClassBuilder.h"
#include "Environment/FCSharpEnvironment.h"
#include "HAL/ConsoleManager.h"
#include "GameBasicCoreModule.h"
#include "Delegate/FUnrealCSharpModuleDelegates.h"

namespace{
	class FGameBasicGCObject : public FGCObject {
	public:
		static FGameBasicGCObject* Get() {
			static FGameBasicGCObject Instance;
			return &Instance;
		}

		void Cleanup() {
			CSharpRefObjects.Reset();
			for (auto Command : ConsoleCommands) {
				FConsoleManager::Get().UnregisterConsoleObject(*Command.Key.ToString());
				FDomain::GCHandle_Free_V2(Command.Value);
			}
			ConsoleCommands.Reset();
		}
	protected:
		FGameBasicGCObject() {
			FUnrealCSharpModuleDelegates::OnUnrealCSharpModuleInActive.AddLambda([this]() {
				Cleanup();
			});
		}

		virtual void AddReferencedObjects(FReferenceCollector& Collector) override {
			Collector.AddReferencedObjects(CSharpRefObjects);
		}

		virtual FString GetReferencerName() const override
		{
			return TEXT("UnrealCSharpRef");
		}
	public:
		TArray<UObject*> CSharpRefObjects;
		TMap<FName, FGarbageCollectionHandle> ConsoleCommands;
	};

	struct FRegisterGameBasicEngine
	{
		static void RegisterConsoleCommand(const FGarbageCollectionHandle NameHandle, const FGarbageCollectionHandle TooltipHandle, MonoObject* Callback) {
			const FString* Name = FCSharpEnvironment::GetEnvironment().GetString<FString>(NameHandle);
			const FString* Tooltip = FCSharpEnvironment::GetEnvironment().GetString<FString>(TooltipHandle);
			FGarbageCollectionHandle GarbageCollectionHandle = FGarbageCollectionHandle::NewRef(Callback, true);
			FConsoleManager::Get().RegisterConsoleCommand(**Name, **Tooltip, FConsoleCommandWithArgsDelegate::CreateLambda([Callback, GarbageCollectionHandle](const TArray<FString>& Args) {
				TArray<void*> Params;
				const auto FoundMonoClass = TPropertyClass<FString, FString>::Get();
				for (auto& Arg : Args) {
					const auto Object = FCSharpEnvironment::GetEnvironment().GetDomain()->Object_New(FoundMonoClass);
					FCSharpEnvironment::GetEnvironment().AddStringReference<FString, true>(Object, new FString(Arg));
					Params.Add(Object);
				}					
				FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(Callback, Params.GetData());
			}));
			FGameBasicGCObject::Get()->ConsoleCommands.Add(**Name, GarbageCollectionHandle);

		}

		static void UnregisterConsoleCommand(const FGarbageCollectionHandle NameHandle) {
			const FString* Name = FCSharpEnvironment::GetEnvironment().GetString<FString>(NameHandle);
			FConsoleManager::Get().UnregisterConsoleObject(**Name);
			if (FGarbageCollectionHandle* GarbageCollectionHandle =  FGameBasicGCObject::Get()->ConsoleCommands.Find(**Name)) {
				FDomain::GCHandle_Free_V2(*GarbageCollectionHandle);
				FGameBasicGCObject::Get()->ConsoleCommands.Remove(**Name);
			}
		}

		static void RegisterObjectRef(const FGarbageCollectionHandle ObjectHandle) {
			if (UObject* Object = FCSharpEnvironment::GetEnvironment().GetObject<UObject>(ObjectHandle)) {
				FGameBasicGCObject::Get()->CSharpRefObjects.Add(Object);
			}
		}

		static void UnregisterObjectRef(const FGarbageCollectionHandle ObjectHandle) {
			if (UObject* Object = FCSharpEnvironment::GetEnvironment().GetObject<UObject>(ObjectHandle)) {
				FGameBasicGCObject::Get()->CSharpRefObjects.Remove(Object);
			}
		}

		FRegisterGameBasicEngine()
		{
			FClassBuilder(TEXT("Engine"), "GameBasic")
				.Function("RegisterConsoleCommand", RegisterConsoleCommand)
				.Function("UnregisterConsoleCommand", UnregisterConsoleCommand)
				.Function("RegisterObjectRef", RegisterObjectRef)
				.Function("UnregisterObjectRef", UnregisterObjectRef)
			;
		}
	};

	[[maybe_unused]] FRegisterGameBasicEngine RegisterGameBasicEngine;
}
