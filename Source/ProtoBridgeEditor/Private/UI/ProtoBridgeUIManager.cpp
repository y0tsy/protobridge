#include "UI/ProtoBridgeUIManager.h"
#include "UI/ProtoBridgeLogPresenter.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Async/Async.h"

FProtoBridgeUIManager::FProtoBridgeUIManager()
{
}

FProtoBridgeUIManager::~FProtoBridgeUIManager()
{
	Shutdown();
}

void FProtoBridgeUIManager::Initialize()
{
	TSharedPtr<FMessageLogPresenter> LogPresenter = MakeShared<FMessageLogPresenter>();
	LogPresenter->Initialize();
	Presenters.Add(LogPresenter);

	StartedHandle = FProtoBridgeEventBus::Get().RegisterOnCompilationStarted(
		FOnProtoBridgeCompilationStarted::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleCompilationStarted)
	);
	
	FinishedHandle = FProtoBridgeEventBus::Get().RegisterOnCompilationFinished(
		FOnProtoBridgeCompilationFinished::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleCompilationFinished)
	);
	
	LogHandle = FProtoBridgeEventBus::Get().RegisterOnLog(
		FOnProtoBridgeLogMessage::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleLogMessage)
	);
}

void FProtoBridgeUIManager::Shutdown()
{
	FProtoBridgeEventBus::Get().UnregisterOnCompilationStarted(StartedHandle);
	FProtoBridgeEventBus::Get().UnregisterOnCompilationFinished(FinishedHandle);
	FProtoBridgeEventBus::Get().UnregisterOnLog(LogHandle);

	for (TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Presenters)
	{
		Presenter->Shutdown();
	}
	Presenters.Empty();
}

void FProtoBridgeUIManager::HandleCompilationStarted()
{
	TWeakPtr<FProtoBridgeUIManager> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf]()
	{
		if (TSharedPtr<FProtoBridgeUIManager> Self = WeakSelf.Pin())
		{
			for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Self->Presenters)
			{
				Presenter->OnCompilationStarted();
			}
		}
	});
}

void FProtoBridgeUIManager::HandleCompilationFinished(bool bSuccess, const FString& Message)
{
	TWeakPtr<FProtoBridgeUIManager> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, bSuccess, Message]()
	{
		if (TSharedPtr<FProtoBridgeUIManager> Self = WeakSelf.Pin())
		{
			for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Self->Presenters)
			{
				Presenter->OnCompilationFinished(bSuccess, Message);
			}
		}
	});
}

void FProtoBridgeUIManager::HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity)
{
	TWeakPtr<FProtoBridgeUIManager> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, Message, Verbosity]()
	{
		if (TSharedPtr<FProtoBridgeUIManager> Self = WeakSelf.Pin())
		{
			for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Self->Presenters)
			{
				Presenter->OnLogMessage(Message, Verbosity);
			}
		}
	});
}