#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "ProtoBridgeDefs.h"

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
	UE_LOG(LogProtoBridge, Verbose, TEXT("FCompilationSession: Destructor"));
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
	
	FProtoBridgeEventBus::Get().BroadcastLog(TEXT("Starting discovery..."), ELogVerbosity::Display);

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	
	UE::Tasks::TTask<FCompilationPlan> PlanTask = FCompilationPlanner::LaunchPlan(Config, &CancellationFlag);

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [WeakSelf, PlanTask, Config]() mutable
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->OnPlanningCompleted(PlanTask.GetResult(), Config.MaxConcurrentProcesses);
		}
	}, PlanTask);
}

void FCompilationSession::Cancel()
{
	UE_LOG(LogProtoBridge, Display, TEXT("FCompilationSession: Cancel requested"));
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
			UE_LOG(LogProtoBridge, Display, TEXT("FCompilationSession: Waiting for completion event..."));
			CompletionEvent->Wait();
			UE_LOG(LogProtoBridge, Display, TEXT("FCompilationSession: Completion event received."));
		}
	}
}

bool FCompilationSession::IsRunning() const
{
	FScopeLock Lock(&SessionMutex);
	return bIsActive;
}

void FCompilationSession::OnPlanningCompleted(FCompilationPlan Plan, int32 MaxConcurrentProcesses)
{
	if (Plan.bWasCancelled || CancellationFlag || bIsTearingDown)
	{
		FinishSession();
		FProtoBridgeEventBus::Get().BroadcastCompilationFinished(false, TEXT("Operation canceled"));
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

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	NewExecutor->OnFinished.BindLambda([WeakSelf]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			UE_LOG(LogProtoBridge, Verbose, TEXT("FCompilationSession: Executor finished delegate called"));
			Self->FinishSession();
		}
	});

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
	UE_LOG(LogProtoBridge, Verbose, TEXT("FCompilationSession: Finishing session"));
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