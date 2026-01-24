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

void FMessageLogPresenter::OnLogMessage(const FProtoBridgeDiagnostic& Diagnostic)
{
	FMessageLog Logger(LogCategoryName);
	TSharedRef<FTokenizedMessage> TokenizedMessage = FTokenizedMessage::Create(
		static_cast<EMessageSeverity::Type>(Diagnostic.Verbosity)
	);

	if (!Diagnostic.FilePath.IsEmpty())
	{
		TokenizedMessage->AddToken(FTextToken::Create(FText::FromString(FString::Printf(TEXT("%s: "), *Diagnostic.FilePath))));
	}

	TokenizedMessage->AddToken(FTextToken::Create(FText::FromString(Diagnostic.Message)));
	Logger.AddMessage(TokenizedMessage);
}

#undef LOCTEXT_NAMESPACE