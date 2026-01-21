#include "UI/ProtoBridgeLogPresenter.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FProtoBridgeLogPresenter"

FMessageLogPresenter::FMessageLogPresenter()
	: LogCategoryName("ProtoBridge")
{
}

FMessageLogPresenter::~FMessageLogPresenter()
{
	Shutdown();
}

void FMessageLogPresenter::Initialize()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowPages = true;
	InitOptions.bShowFilters = true;
	MessageLogModule.RegisterLogListing(LogCategoryName, LOCTEXT("ProtoBridgeLogLabel", "ProtoBridge"), InitOptions);
}

void FMessageLogPresenter::Shutdown()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing(LogCategoryName);
	}
}

void FMessageLogPresenter::OnCompilationStarted()
{
	FMessageLog Logger(LogCategoryName);
	Logger.Open(EMessageSeverity::Info, false); 
	Logger.NewPage(LOCTEXT("CompilationLogPage", "Compilation"));
}

void FMessageLogPresenter::OnCompilationFinished(bool bSuccess, const FString& Message)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.bUseLargeFont = false;

	if (bSuccess)
	{
		Info.ExpireDuration = 2.0f;
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.SuccessImage"));
	}
	else
	{
		Info.ExpireDuration = 5.0f;
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.FailImage"));
		FMessageLog(LogCategoryName).Open(EMessageSeverity::Error, true);
	}

	FSlateNotificationManager::Get().AddNotification(Info);
}

void FMessageLogPresenter::OnLogMessage(const FString& Message, ELogVerbosity::Type Verbosity)
{
	FMessageLog Logger(LogCategoryName);
	
	if (Verbosity == ELogVerbosity::Error)
	{
		Logger.Error(FText::FromString(Message));
	}
	else if (Verbosity == ELogVerbosity::Warning)
	{
		Logger.Warning(FText::FromString(Message));
	}
	else
	{
		Logger.Info(FText::FromString(Message));
	}
}

#undef LOCTEXT_NAMESPACE