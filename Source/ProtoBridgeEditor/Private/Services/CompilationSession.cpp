#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeEventBus.h"
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
	WaitForCompletion();
}

void FCompilationSession::Start(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&SessionMutex);
	if (bIsActive) return;
	bIsActive = true;
	CancellationFlag = false;
	
	FProtoBridgeEventBus::Get().BroadcastCompilationStarted();

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
	FProtoBridgeEventBus::Get().BroadcastLog(TEXT("Starting discovery..."), ELogVerbosity::Display);
	
	FCompilationPlan Plan = FCompilationPlanner::GeneratePlan(Config, CancellationFlag);

	if (Plan.bWasCancelled)
	{
		FinishSession();
		FProtoBridgeEventBus::Get().BroadcastCompilationFinished(false, TEXT("Operation canceled during discovery"));
		return;
	}

	for (const FString& Err : Plan.Errors)
	{
		FProtoBridgeEventBus::Get().BroadcastLog(Err, ELogVerbosity::Error);
	}

	if (Plan.Tasks.Num() == 0)
	{
		bool bHasErrors = Plan.Errors.Num() > 0;
		FinishSession();
		FProtoBridgeEventBus::Get().BroadcastCompilationFinished(!bHasErrors, bHasErrors ? TEXT("Configuration errors detected") : TEXT("No files to compile"));
		return;
	}

	FProtoBridgeEventBus::Get().BroadcastLog(FString::Printf(TEXT("Generated %d tasks"), Plan.Tasks.Num()), ELogVerbosity::Display);

	if (CancellationFlag)
	{
		FinishSession();
		FProtoBridgeEventBus::Get().BroadcastCompilationFinished(false, TEXT("Canceled before execution"));
		return;
	}

	TSharedPtr<FTaskExecutor> NewExecutor = MakeShared<FTaskExecutor>(Config.TimeoutSeconds, Config.MaxConcurrentProcesses);
	
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
		FinishSession();
		FProtoBridgeEventBus::Get().BroadcastCompilationFinished(false, TEXT("Canceled before execution"));
	}
}

void FCompilationSession::FinishSession()
{
	FScopeLock Lock(&SessionMutex);
	bIsActive = false;
	Executor.Reset();
}