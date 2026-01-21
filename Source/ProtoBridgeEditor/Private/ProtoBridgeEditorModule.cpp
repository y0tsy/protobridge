#include "ProtoBridgeEditorModule.h"
#include "ProtoBridgeEditorStyle.h" 
#include "ProtoBridgeDefs.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Services/ProtoBridgeCompilerService.h"
#include "Services/ProtoBridgeUtils.h"
#include "UI/ProtoBridgeUIManager.h"
#include "Settings/ProtoBridgeSettings.h"
#include "ProtoBridgeConfiguration.h"
#include "ToolMenus.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"
#include "Async/Async.h"
#include "Services/ProtoBridgeFileManager.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "FProtoBridgeEditorModule"

void FProtoBridgeEditorModule::StartupModule()
{
	FProtoBridgeEditorStyle::Initialize();
	
	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / FProtoBridgeDefs::TempFolder;
	
	Async(EAsyncExecution::ThreadPool, [TempDir]()
	{
		FProtoBridgeFileManager::CleanupOldTempFiles(TempDir, FProtoBridgeDefs::MaxTempFileAgeSeconds);
	});

	CompilerService = MakeShared<FProtoBridgeCompilerService>();
	UIManager = MakeShared<FProtoBridgeUIManager>();
	UIManager->Initialize();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FProtoBridgeEditorModule::RegisterMenus));
}

void FProtoBridgeEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	FProtoBridgeEditorStyle::Shutdown();
	
	if (UIManager.IsValid())
	{
		UIManager->Shutdown();
		UIManager.Reset();
	}

	if (CompilerService.IsValid())
	{
		CompilerService->Cancel();
		CompilerService->WaitForCompletion();
		CompilerService.Reset();
	}
}

TSharedPtr<IProtoBridgeService> FProtoBridgeEditorModule::GetService() const
{
	return CompilerService;
}

void FProtoBridgeEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->FindOrAddSection("PluginTools");

	FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
		"ProtoBridgeCompile",
		FUIAction(
			FExecuteAction::CreateRaw(this, &FProtoBridgeEditorModule::OnCompileButtonClicked),
			FCanExecuteAction::CreateLambda([this]() { return CompilerService.IsValid() && !CompilerService->IsCompiling(); })
		),
		LOCTEXT("CompileButtonLabel", "ProtoBridge"),
		LOCTEXT("CompileButtonTooltip", "Compile all Proto files"),
		FSlateIcon(FProtoBridgeEditorStyle::GetStyleSetName(), "Icons.Build")
	);
	
	Section.AddEntry(Entry);
}

void FProtoBridgeEditorModule::OnCompileButtonClicked()
{
	if (CompilerService.IsValid())
	{
		const UProtoBridgeSettings* Settings = GetDefault<UProtoBridgeSettings>();
		
		FProtoBridgeConfiguration Config;
		Config.Environment.ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		Config.Environment.ProtocPath = Settings->CustomProtocPath.FilePath;
		Config.Environment.PluginPath = Settings->CustomPluginPath.FilePath;
		Config.Mappings = Settings->Mappings;
		Config.ApiMacro = Settings->ApiMacroName;
		Config.TimeoutSeconds = Settings->TimeoutSeconds;
		Config.MaxConcurrentProcesses = Settings->MaxConcurrentProcesses;

		TSharedPtr<IPlugin> SelfPlugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName);
		if (SelfPlugin.IsValid())
		{
			Config.Environment.PluginDirectory = SelfPlugin->GetBaseDir();
		}

		for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
		{
			Config.Environment.PluginLocations.Add(Plugin->GetName(), Plugin->GetBaseDir());
		}

		CompilerService->Compile(Config);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProtoBridgeEditorModule, ProtoBridgeEditor)