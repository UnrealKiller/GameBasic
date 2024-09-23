#include "Binding/Class/FClassBuilder.h"
#include "Environment/FCSharpEnvironment.h"
#include "Async/TaskGraphInterfaces.h"
#include "Binding/Enum/TBindingEnumBuilder.inl"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

BINDING_ENUM_WITH_NAME(ENamedThreads::Type, "ENamedThreads")

namespace
{
	struct FRegisterGameBasicAsync
	{
		static void AddAsyncTask(ENamedThreads::Type ThreadType, MonoObject* Callback) {
			FGarbageCollectionHandle GarbageCollectionHandle = FGarbageCollectionHandle::NewRef(Callback, true);
			AsyncTask(ThreadType, [Callback, GarbageCollectionHandle]()
				{
					FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(Callback, nullptr);
					FDomain::GCHandle_Free_V2(GarbageCollectionHandle);
				});
		}

		static void LoadObjectAsync(const FGarbageCollectionHandle ObjectPathHandle, int32 AsyncLoadPriority, MonoObject* Callback) {
			const auto ObjectPath = FCSharpEnvironment::GetEnvironment().GetString<FString>(ObjectPathHandle);
			FGarbageCollectionHandle GarbageCollectionHandle = FGarbageCollectionHandle::NewRef(Callback, true);
			FSoftObjectPath SoftObjectPath(*ObjectPath);
			FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
			FStreamableDelegate StreamableDelegate = FStreamableDelegate::CreateLambda([SoftObjectPath, Callback, GarbageCollectionHandle]() {
				FSoftObjectPtr SoftObjPtr = FSoftObjectPtr(SoftObjectPath);
				UObject* Obj = SoftObjPtr.Get();
				if (Obj) {
					void * Params[1] = { FCSharpEnvironment::GetEnvironment() .Bind(Obj)};
					FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(Callback, Params);
					FDomain::GCHandle_Free_V2(GarbageCollectionHandle);
				}
				});
			TSharedPtr<FStreamableHandle> Handler = StreamableManager.RequestAsyncLoad(SoftObjectPath, StreamableDelegate, AsyncLoadPriority, false, false, "LoadObjectAsyncByCSharp");
		}

		static void LoadObjectsAsync(const FGarbageCollectionHandle ObjectPathHandle, int32 AsyncLoadPriority, MonoObject* Callback) {
			// TODO
			/*if (const auto ArrayHelper = FCSharpEnvironment::GetEnvironment().GetContainer<FArrayHelper>(ObjectPathHandle)) {
			}
			const auto ObjectPath = FCSharpEnvironment::GetEnvironment().GetString<FString>(ObjectPathHandle);
			FGarbageCollectionHandle GarbageCollectionHandle = FGarbageCollectionHandle::NewRef(Callback, true);
			FSoftObjectPath SoftObjectPath(*ObjectPath);
			FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
			FStreamableDelegate StreamableDelegate = FStreamableDelegate::CreateLambda([SoftObjectPath, Callback, GarbageCollectionHandle]() {
				FSoftObjectPtr SoftObjPtr = FSoftObjectPtr(SoftObjectPath);
				UObject* Obj = SoftObjPtr.Get();
				if (Obj) {
					void* Params[1] = { FCSharpEnvironment::GetEnvironment().Bind(Obj) };
					FCSharpEnvironment::GetEnvironment().GetDomain()->Runtime_Delegate_Invoke(Callback, Params);
					FDomain::GCHandle_Free_V2(GarbageCollectionHandle);
				}
				});
			TSharedPtr<FStreamableHandle> Handler = StreamableManager.RequestAsyncLoad(SoftObjectPath, StreamableDelegate, AsyncLoadPriority, false, false, "LoadObjectAsyncByCSharp");*/
		}

		FRegisterGameBasicAsync()
		{
			TBindingEnumBuilder<ENamedThreads::Type, false>()
				.Enumerator("UnusedAnchor", ENamedThreads::UnusedAnchor)
				.Enumerator("RHIThread", ENamedThreads::RHIThread)
				.Enumerator("GameThread", ENamedThreads::GameThread)
				.Enumerator("ActualRenderingThread", ENamedThreads::ActualRenderingThread)
				.Enumerator("AnyThread", ENamedThreads::AnyThread)
				.Enumerator("GameThread_Local", ENamedThreads::GameThread_Local)
				.Enumerator("ActualRenderingThread_Local", ENamedThreads::ActualRenderingThread_Local)
				.Enumerator("AnyHiPriThreadNormalTask", ENamedThreads::AnyHiPriThreadNormalTask)
				.Enumerator("AnyHiPriThreadHiPriTask", ENamedThreads::AnyHiPriThreadHiPriTask)
				.Enumerator("AnyNormalThreadNormalTask", ENamedThreads::AnyNormalThreadNormalTask)
				.Enumerator("AnyNormalThreadHiPriTask", ENamedThreads::AnyNormalThreadHiPriTask)
				.Enumerator("AnyBackgroundThreadNormalTask", ENamedThreads::AnyBackgroundThreadNormalTask)
				.Enumerator("AnyBackgroundHiPriTask", ENamedThreads::AnyBackgroundHiPriTask)
				;

			FClassBuilder(TEXT("Async"), "GameBasic")
				.Function("AddAsyncTask", AddAsyncTask)
				.Function("LoadObjectAsync", LoadObjectAsync)
				.Function("LoadObjectsAsync", LoadObjectsAsync)
			;
		}
	};

	[[maybe_unused]] FRegisterGameBasicAsync RegisterGameBasicAsync;
}
