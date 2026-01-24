#include "UI/ProtoBridgeUIManager.h"
#include "UI/ProtoBridgeLogPresenter.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Subsystems/ProtoBridgeCompilerSubsystem.h"
#include "Editor.h"
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

	if (GEditor)
	{
		if (UProtoBridgeCompilerSubsystem* Subsystem = GEditor->GetEditorSubsystem<UProtoBridgeCompilerSubsystem>())
		{
			if (TSharedPtr<FProtoBridgeEventBus> Bus = Subsystem->GetEventBus())
			{
				WeakEventBus = Bus;
				StartedHandle = Bus->RegisterOnCompilationStarted(
					FOnProtoBridgeCompilationStarted::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleCompilationStarted)
				);
				
				FinishedHandle = Bus->RegisterOnCompilationFinished(
					FOnProtoBridgeCompilationFinished::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleCompilationFinished)
				);
				
				LogHandle = Bus->RegisterOnLog(
					FOnProtoBridgeLogMessage::FDelegate::CreateSP(this, &FProtoBridgeUIManager::HandleLogMessage)
				);
			}
		}
	}
}

void FProtoBridgeUIManager::Shutdown()
{
	if (TSharedPtr<FProtoBridgeEventBus> Bus = WeakEventBus.Pin())
	{
		Bus->UnregisterOnCompilationStarted(StartedHandle);
		Bus->UnregisterOnCompilationFinished(FinishedHandle);
		Bus->UnregisterOnLog(LogHandle);
	}

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

void FProtoBridgeUIManager::HandleLogMessage(const FProtoBridgeDiagnostic& Diagnostic)
{
	TWeakPtr<FProtoBridgeUIManager> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, Diagnostic]()
	{
		if (TSharedPtr<FProtoBridgeUIManager> Self = WeakSelf.Pin())
		{
			for (const TSharedPtr<IProtoBridgeOutputPresenter>& Presenter : Self->Presenters)
			{
				Presenter->OnLogMessage(Diagnostic);
			}
		}
	});
}