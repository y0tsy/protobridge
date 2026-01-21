#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

FCompilationSession::FCompilationSession()
	: bIsActive(false)
	, bIsTearingDown(false)
	, CancellationFlag(false)
{
	ExecutorFactory = [](int32 MaxProcesses) {
		return MakeShared<FTaskExecutor>(MaxProcesses);
	};
	CompletionEvent = FPlatformProcess::GetSynchEventFromPool(true);
}

FCompilationSession::~FCompilationSession()
{
	bIsTearingDown = true;
	Cancel();
	WaitForCompletion();
	
	if (CompletionEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
		CompletionEvent = nullptr;
	}
}

void FCompilationSession::SetExecutorFactory(FExecutorFactory InFactory)
{
	if (InFactory)
	{
		ExecutorFactory = InFactory;
	}
}

void FCompilationSession::Start(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&SessionMutex);
	if (bIsActive) return;
	bIsActive = true;
	CancellationFlag = false;
	
	if (CompletionEvent)
	{
		CompletionEvent->Reset();
	}
	
	FProtoBridgeEventBus::Get().BroadcastCompilationStarted();

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	Async(EAsyncExecution::ThreadPool, [WeakSelf, Config]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->RunPlanning(Config);
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
	if (CompletionEvent)
	{
		if (IsRunning())
		{
			CompletionEvent->Wait();
		}
	}
}

bool FCompilationSession::IsRunning() const
{
	FScopeLock Lock(&SessionMutex);
	return bIsActive;
}

void FCompilationSession::RunPlanning(const FProtoBridgeConfiguration& Config)
{
	if (CancellationFlag || bIsTearingDown)
	{
		FinishSession();
		FProtoBridgeEventBus::Get().BroadcastCompilationFinished(false, TEXT("Operation canceled"));
		return;
	}

	FProtoBridgeEventBus::Get().BroadcastLog(TEXT("Starting discovery..."), ELogVerbosity::Display);
	
	FCompilationPlan Plan = FCompilationPlanner::GeneratePlan(Config, CancellationFlag);

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	int32 MaxProcs = Config.MaxConcurrentProcesses;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakSelf, Plan = MoveTemp(Plan), MaxProcs]() mutable
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->OnPlanningCompleted(MoveTemp(Plan), MaxProcs);
		}
	});
}

void FCompilationSession::OnPlanningCompleted(FCompilationPlan Plan, int32 MaxConcurrentProcesses)
{
	if (Plan.bWasCancelled || CancellationFlag || bIsTearingDown)
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

	TSharedPtr<FTaskExecutor> NewExecutor = ExecutorFactory(MaxConcurrentProcesses);
	
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
	{
		FScopeLock Lock(&SessionMutex);
		bIsActive = false;
		Executor.Reset();
	}
	
	if (CompletionEvent)
	{
		CompletionEvent->Trigger();
	}
}