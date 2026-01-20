#include "ProtoBridgeEditorModule.h"
#include "ProtoBridgeEditorStyle.h" 
#include "ProtoBridgeDefs.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Services/ProtoBridgeCompilerService.h"
#include "Workers/ProtoBridgeWorkerFactory.h"
#include "ToolMenus.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FProtoBridgeEditorModule"

static const FName ProtoBridgeMessageLogName = FName("ProtoBridge");

void FProtoBridgeEditorModule::StartupModule()
{
	FProtoBridgeEditorStyle::Initialize();
	FProtoBridgeEditorStyle::ReloadTextures();
	
	CleanupTempFiles();

	CompilerService = MakeShared<FProtoBridgeCompilerService>(MakeShared<FProtoBridgeWorkerFactory>());
	
	CompilerService->OnCompilationStarted().AddLambda([this]() {
		HandleCompilationStarted();
	});
	
	CompilerService->OnCompilationFinished().AddLambda([this](bool bSuccess, const FString& Message) {
		HandleCompilationFinished(bSuccess, Message);
	});

	CompilerService->OnLogMessage().AddLambda([this](const FString& Message, ELogVerbosity::Type Verbosity) {
		HandleLogMessage(Message, Verbosity);
	});

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowPages = true;
	InitOptions.bShowFilters = true;
	MessageLogModule.RegisterLogListing(ProtoBridgeMessageLogName, LOCTEXT("ProtoBridgeLogLabel", "ProtoBridge"), InitOptions);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FProtoBridgeEditorModule::RegisterMenus));
}

void FProtoBridgeEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FProtoBridgeEditorStyle::Shutdown();

	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing(ProtoBridgeMessageLogName);
	}

	if (CompilerService.IsValid())
	{
		CompilerService->Cancel();
		CompilerService->OnCompilationStarted().RemoveAll(this);
		CompilerService->OnCompilationFinished().RemoveAll(this);
		CompilerService->OnLogMessage().RemoveAll(this);
		CompilerService.Reset();
	}
}

TSharedPtr<IProtoBridgeService> FProtoBridgeEditorModule::GetService() const
{
	return CompilerService;
}

void FProtoBridgeEditorModule::CleanupTempFiles()
{
	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / FProtoBridgeDefs::TempFolder;
	if (IFileManager::Get().DirectoryExists(*TempDir))
	{
		IFileManager::Get().DeleteDirectory(*TempDir, false, true);
	}
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
		CompilerService->CompileAll();
	}
}

void FProtoBridgeEditorModule::HandleCompilationStarted()
{
	FMessageLog(ProtoBridgeMessageLogName).NewPage(LOCTEXT("CompilationLogPage", "Compilation"));
}

void FProtoBridgeEditorModule::HandleCompilationFinished(bool bSuccess, const FString& Message)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.ExpireDuration = 3.0f;
	Info.bUseLargeFont = false;

	if (bSuccess)
	{
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.SuccessImage"));
	}
	else
	{
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.FailImage"));
		FMessageLog(ProtoBridgeMessageLogName).Open(EMessageSeverity::Error, true);
	}

	FSlateNotificationManager::Get().AddNotification(Info);
}

void FProtoBridgeEditorModule::HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity)
{
	FMessageLog Logger(ProtoBridgeMessageLogName);

	TSharedPtr<FTokenizedMessage> TokenizedMessage;

	if (Verbosity == ELogVerbosity::Error)
	{
		TokenizedMessage = Logger.Error(FText::FromString(Message));
	}
	else if (Verbosity == ELogVerbosity::Warning)
	{
		TokenizedMessage = Logger.Warning(FText::FromString(Message));
	}
	else
	{
		TokenizedMessage = Logger.Info(FText::FromString(Message));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProtoBridgeEditorModule, ProtoBridgeEditor)