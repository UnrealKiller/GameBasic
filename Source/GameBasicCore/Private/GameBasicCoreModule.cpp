#include "GameBasicCoreModule.h"
#include "Setting/UnrealCSharpSetting.h"
#include "Setting/UnrealCSharpEditorSetting.h"

DEFINE_LOG_CATEGORY(LogGameBasic);

void FGameBasicCoreModule::StartupModule()
{
	if (auto Settings = GetMutableDefault<UUnrealCSharpSetting>()) {
		TArray<FCustomProject>* CustomProjects = const_cast<TArray<FCustomProject>*>(&Settings->GetCustomProjects());
		bool bNeedRegister = true;
		for (auto& Project: *CustomProjects) {
			if (Project.Name == "GameBasic") {
				bNeedRegister = false;
				break;
			}
		}
		if (bNeedRegister) {
			FCustomProject Project;
			Project.Name = "GameBasic";
			CustomProjects->Emplace(Project);
			Settings->TryUpdateDefaultConfigFile();
		}
	}
	if (auto Settings = GetMutableDefault<UUnrealCSharpEditorSetting>()) {
		TArray<FString>* SupportedModules = const_cast<TArray<FString>*>(&Settings->GetSupportedModule());
		bool bNeedRegister = true;
		for (auto& ModuleName : *SupportedModules) {
			if (ModuleName == "GameBasic") {
				bNeedRegister = false;
				break;
			}
		}
		if (bNeedRegister) {
			SupportedModules->Add(FString(TEXT("GameBasic")));
			Settings->TryUpdateDefaultConfigFile();
		}
	}
}

void FGameBasicCoreModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FGameBasicCoreModule, GameBasicCore)