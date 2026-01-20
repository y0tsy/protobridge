#include "UI/ProtoBridgeUIManager.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FProtoBridgeUIManager"

FProtoBridgeUIManager::FProtoBridgeUIManager(TSharedPtr<IProtoBridgeService> InService)
	: Service(InService)
	, LogCategoryName("ProtoBridge")
{
}

FProtoBridgeUIManager::~FProtoBridgeUIManager()
{
	Shutdown();
}

void FProtoBridgeUIManager::Initialize()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowPages = true;
	InitOptions.bShowFilters = true;
	MessageLogModule.RegisterLogListing(LogCategoryName, LOCTEXT("ProtoBridgeLogLabel", "ProtoBridge"), InitOptions);

	if (TSharedPtr<IProtoBridgeService> PinnedService = Service.Pin())
	{
		PinnedService->OnCompilationStarted().AddSP(this, &FProtoBridgeUIManager::HandleCompilationStarted);
		PinnedService->OnCompilationFinished().AddSP(this, &FProtoBridgeUIManager::HandleCompilationFinished);
		PinnedService->OnLogMessage().AddSP(this, &FProtoBridgeUIManager::HandleLogMessage);
	}
}

void FProtoBridgeUIManager::Shutdown()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing(LogCategoryName);
	}
}

void FProtoBridgeUIManager::HandleCompilationStarted()
{
	FMessageLog(LogCategoryName).NewPage(LOCTEXT("CompilationLogPage", "Compilation"));
}

void FProtoBridgeUIManager::HandleCompilationFinished(bool bSuccess, const FString& Message)
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
		FMessageLog(LogCategoryName).Open(EMessageSeverity::Error, true);
	}

	FSlateNotificationManager::Get().AddNotification(Info);
}

void FProtoBridgeUIManager::HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity)
{
	FMessageLog Logger(LogCategoryName);
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