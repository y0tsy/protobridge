#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/CompilationPlanner.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"

FCompilationSession::FCompilationSession()
	: bIsActive(false)
	, bIsTearingDown(false)
	, CancellationFlag(false)
{
}

FCompilationSession::~FCompilationSession()
{
	bIsTearingDown = true;
	Cancel();
}

void FCompilationSession::Start(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&SessionMutex);
	if (bIsActive) return;
	bIsActive = true;
	CancellationFlag = false;
	
	StartedDelegate.Broadcast();

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	
	WorkerFuture = Async(EAsyncExecution::Thread, [WeakSelf, Config]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->RunDiscovery(Config);
		}
	});
}

void FCompilationSession::Cancel()
{
	CancellationFlag = true;

	TSharedPtr<FTaskExecutor> ExecutorToCancel;
	{
		FScopeLock Lock(&SessionMutex);
		ExecutorToCancel = Executor;
	}

	if (ExecutorToCancel.IsValid())
	{
		ExecutorToCancel->Cancel();
	}
}

void FCompilationSession::WaitForCompletion()
{
	if (WorkerFuture.IsValid())
	{
		WorkerFuture.Wait();
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
	
	FCompilationPlan Plan = FCompilationPlanner::GeneratePlan(Config, CancellationFlag);

	if (Plan.bWasCancelled)
	{
		OnExecutorFinishedInternal(false, TEXT("Operation canceled during discovery"));
		return;
	}

	for (const FString& Err : Plan.Errors)
	{
		DispatchLog(FString::Printf(TEXT("Error: %s"), *Err), true);
	}

	if (Plan.Tasks.Num() == 0)
	{
		bool bHasErrors = Plan.Errors.Num() > 0;
		OnExecutorFinishedInternal(!bHasErrors, bHasErrors ? TEXT("Configuration errors detected") : TEXT("No files to compile"));
		return;
	}

	DispatchLog(FString::Printf(TEXT("Generated %d tasks"), Plan.Tasks.Num()));

	if (CancellationFlag)
	{
		OnExecutorFinishedInternal(false, TEXT("Canceled before execution"));
		return;
	}

	TSharedPtr<FTaskExecutor> NewExecutor = MakeShared<FTaskExecutor>(Config.TimeoutSeconds, Config.MaxConcurrentProcesses);
	NewExecutor->OnOutput().AddSP(this, &FCompilationSession::DispatchLog, false);
	NewExecutor->OnFinished().AddSP(this, &FCompilationSession::OnExecutorFinishedInternal);

	bool bShouldStart = false;
	{
		FScopeLock Lock(&SessionMutex);
		if (bIsActive && !CancellationFlag && !bIsTearingDown)
		{
			Executor = NewExecutor;
			bShouldStart = true;
		}
	}

	if (bShouldStart)
	{
		NewExecutor->Execute(MoveTemp(Plan.Tasks));
	}
	else
	{
		OnExecutorFinishedInternal(false, TEXT("Canceled before execution"));
	}
}

void FCompilationSession::OnExecutorFinishedInternal(bool bSuccess, const FString& Message)
{
	{
		FScopeLock Lock(&SessionMutex);
		bIsActive = false;
		Executor.Reset();
	}

	DispatchFinished(bSuccess, Message);
}

void FCompilationSession::DispatchLog(const FString& Message, bool bIsError)
{
	if (bIsTearingDown) return;

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	FString LogMessage = bIsError ? TEXT("Error: ") + Message : Message;
	
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, LogMessage]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->LogDelegate.Broadcast(LogMessage);
		}
	});
}

void FCompilationSession::DispatchFinished(bool bSuccess, const FString& Message)
{
	if (bIsTearingDown) return;

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, bSuccess, Message]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->FinishedDelegate.Broadcast(bSuccess, Message);
		}
	});
}