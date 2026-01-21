#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/CompilationPlanner.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

FCompilationSession::FCompilationSession()
	: bIsActive(false)
	, WorkFinishedEvent(nullptr)
{
	WorkFinishedEvent = FPlatformProcess::GetSynchEventFromPool(true);
}

FCompilationSession::~FCompilationSession()
{
	Cancel();
	if (WorkFinishedEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WorkFinishedEvent);
		WorkFinishedEvent = nullptr;
	}
}

void FCompilationSession::Start(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&SessionMutex);
	if (bIsActive) return;
	bIsActive = true;

	if (WorkFinishedEvent)
	{
		WorkFinishedEvent->Reset();
	}
	
	StartedDelegate.Broadcast();

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	
	Async(EAsyncExecution::Thread, [WeakSelf, Config]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->RunDiscovery(Config);
		}
	});
}

void FCompilationSession::Cancel()
{
	TSharedPtr<FTaskExecutor> ExecutorToCancel;
	{
		FScopeLock Lock(&SessionMutex);
		if (!bIsActive) return;
		ExecutorToCancel = Executor;
	}

	if (ExecutorToCancel.IsValid())
	{
		ExecutorToCancel->Cancel();
	}
}

void FCompilationSession::WaitForCompletion()
{
	bool bShouldWait = false;
	{
		FScopeLock Lock(&SessionMutex);
		bShouldWait = bIsActive;
	}

	if (bShouldWait && WorkFinishedEvent)
	{
		WorkFinishedEvent->Wait();
	}
}

bool FCompilationSession::IsRunning() const
{
	FScopeLock Lock(&SessionMutex);
	return bIsActive;
}

void FCompilationSession::RunDiscovery(const FProtoBridgeConfiguration& Config)
{
	DispatchLog(TEXT("Starting discovery..."));
	
	FCompilationPlan Plan = FCompilationPlanner::GeneratePlan(Config);

	if (!Plan.bIsValid)
	{
		OnExecutorFinishedInternal(false, Plan.ErrorMessage);
		return;
	}

	if (Plan.Tasks.Num() == 0)
	{
		OnExecutorFinishedInternal(true, TEXT("No files to compile"));
		return;
	}

	DispatchLog(FString::Printf(TEXT("Generated %d tasks"), Plan.Tasks.Num()));

	FScopeLock Lock(&SessionMutex);
	if (bIsActive)
	{
		Executor = MakeShared<FTaskExecutor>(Config.TimeoutSeconds);
		
		Executor->OnOutput().AddSP(this, &FCompilationSession::DispatchLog);
		Executor->OnFinished().AddSP(this, &FCompilationSession::OnExecutorFinishedInternal);
		
		Executor->Execute(MoveTemp(Plan.Tasks));
	}
}

void FCompilationSession::OnExecutorFinishedInternal(bool bSuccess, const FString& Message)
{
	{
		FScopeLock Lock(&SessionMutex);
		bIsActive = false;
		Executor.Reset();
	}

	if (WorkFinishedEvent)
	{
		WorkFinishedEvent->Trigger();
	}

	DispatchFinished(bSuccess, Message);
}

void FCompilationSession::DispatchLog(const FString& Message)
{
	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, Message]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->LogDelegate.Broadcast(Message);
		}
	});
}

void FCompilationSession::DispatchFinished(bool bSuccess, const FString& Message)
{
	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, bSuccess, Message]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->FinishedDelegate.Broadcast(bSuccess, Message);
		}
	});
}