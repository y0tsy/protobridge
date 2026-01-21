#include "UI/ProtoBridgeUIManager.h"
#include "Interfaces/IProtoBridgeService.h"
#include "UI/ProtoBridgeLogPresenter.h"

FProtoBridgeUIManager::FProtoBridgeUIManager(TSharedPtr<IProtoBridgeService> InService)
	: Service(InService)
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

	if (TSharedPtr<IProtoBridgeService> PinnedService = Service.Pin())
	{
		StartedHandle = PinnedService->RegisterOnCompilationStarted(
			FOnProtoBridgeCompilationStarted::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleCompilationStarted)
		);
		
		FinishedHandle = PinnedService->RegisterOnCompilationFinished(
			FOnProtoBridgeCompilationFinished::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleCompilationFinished)
		);
		
		LogHandle = PinnedService->RegisterOnLogMessage(
			FOnProtoBridgeLogMessage::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleLogMessage)
		);
	}
}

void FProtoBridgeUIManager::Shutdown()
{
	for (TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Presenters)
	{
		Presenter->Shutdown();
	}
	Presenters.Empty();

	if (TSharedPtr<IProtoBridgeService> PinnedService = Service.Pin())
	{
		if (StartedHandle.IsValid())
		{
			PinnedService->UnregisterOnCompilationStarted(StartedHandle);
			StartedHandle.Reset();
		}
		if (FinishedHandle.IsValid())
		{
			PinnedService->UnregisterOnCompilationFinished(FinishedHandle);
			FinishedHandle.Reset();
		}
		if (LogHandle.IsValid())
		{
			PinnedService->UnregisterOnLogMessage(LogHandle);
			LogHandle.Reset();
		}
	}
}

void FProtoBridgeUIManager::HandleCompilationStarted()
{
	for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Presenters)
	{
		Presenter->OnCompilationStarted();
	}
}

void FProtoBridgeUIManager::HandleCompilationFinished(bool bSuccess, const FString& Message)
{
	for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Presenters)
	{
		Presenter->OnCompilationFinished(bSuccess, Message);
	}
}

void FProtoBridgeUIManager::HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity)
{
	for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Presenters)
	{
		Presenter->OnLogMessage(Message, Verbosity);
	}
}