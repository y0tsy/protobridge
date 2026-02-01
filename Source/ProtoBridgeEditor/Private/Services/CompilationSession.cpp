#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Services/ProtoBridgeCacheManager.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "ProtoBridgeDefs.h"

FCompilationSession::FCompilationSession(TSharedRef<FProtoBridgeEventBus> InEventBus)
	: CurrentState(ESessionState::Idle)
	, EventBus(InEventBus)
	, CancellationFlag(false)
{
	ExecutorFactory = [InEventBus](int32 MaxProcesses) {
		return MakeShared<FTaskExecutor>(MaxProcesses, InEventBus);
	};
	CompletionEvent = FPlatformProcess::GetSynchEventFromPool(true);
}

FCompilationSession::~FCompilationSession()
{
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
	if (!TryTransitionState(ESessionState::Idle, ESessionState::Planning))
	{
		return;
	}

	CancellationFlag = false;
	
	if (CompletionEvent)
	{
		CompletionEvent->Reset();
	}
	
	EventBus->BroadcastCompilationStarted();
	EventBus->BroadcastLog(FProtoBridgeDiagnostic(ELogVerbosity::Display, TEXT("Starting discovery...")));

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	
	UE::Tasks::TTask<FCompilationPlan> PlanTask = FCompilationPlanner::LaunchPlan(Config, EventBus, &CancellationFlag);

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
	FScopeLock Lock(&StateMutex);
	if (CurrentState == ESessionState::Idle || CurrentState == ESessionState::Finished || CurrentState == ESessionState::Canceling)
	{
		return;
	}
	
	CurrentState = ESessionState::Canceling;
	CancellationFlag = true;

	if (Executor.IsValid())
	{
		Executor->Cancel();
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
	ESessionState State = CurrentState;
	return State != ESessionState::Idle && State != ESessionState::Finished;
}

void FCompilationSession::OnPlanningCompleted(FCompilationPlan Plan, int32 MaxConcurrentProcesses)
{
	if (!TryTransitionState(ESessionState::Planning, ESessionState::Compiling))
	{
		FinishSession(false, TEXT("Operation canceled during planning"));
		return;
	}

	if (Plan.bWasCancelled || CancellationFlag)
	{
		FinishSession(false, TEXT("Operation canceled"));
		return;
	}

	CacheManager = Plan.CacheManager;

	for (const FProtoBridgeDiagnostic& Diag : Plan.Diagnostics)
	{
		EventBus->BroadcastLog(Diag);
	}

	if (Plan.Tasks.Num() == 0)
	{
		bool bHasErrors = Plan.Diagnostics.ContainsByPredicate([](const FProtoBridgeDiagnostic& D){ return D.Verbosity == ELogVerbosity::Error; });
		FinishSession(!bHasErrors, bHasErrors ? TEXT("Configuration errors detected") : TEXT("All files up to date"));
		return;
	}

	EventBus->BroadcastLog(FProtoBridgeDiagnostic(ELogVerbosity::Display, FString::Printf(TEXT("Generated %d tasks"), Plan.Tasks.Num())));

	TSharedPtr<FTaskExecutor> NewExecutor = ExecutorFactory(MaxConcurrentProcesses);
	Executor = NewExecutor;

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	TArray<FCompilationTask> TasksCopy = Plan.Tasks;
	
	NewExecutor->OnFinished.BindLambda([WeakSelf, TasksCopy, NewExecutor]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			if (Self->CacheManager.IsValid() && !NewExecutor->HasErrors() && !NewExecutor->IsCancelled())
			{
				for (const FCompilationTask& Task : TasksCopy)
				{
					for (const FString& File : Task.InputFiles)
					{
						Self->CacheManager->UpdateFileSuccess(File, Task.ConfigHash);
					}
				}
				Self->CacheManager->SaveCache();
			}
			Self->FinishSession(!NewExecutor->HasErrors(), NewExecutor->HasErrors() ? TEXT("Finished with errors") : TEXT("Success"));
		}
	});

	NewExecutor->Execute(MoveTemp(Plan.Tasks));
}

void FCompilationSession::FinishSession(bool bSuccess, const FString& Message)
{
	{
		FScopeLock Lock(&StateMutex);
		CurrentState = ESessionState::Finished;
		Executor.Reset();
		CacheManager.Reset();
	}

	if (CompletionEvent)
	{
		CompletionEvent->Trigger();
	}
	
	if (!Message.IsEmpty())
	{
		EventBus->BroadcastCompilationFinished(bSuccess, Message);
	}
}

bool FCompilationSession::TryTransitionState(ESessionState Expected, ESessionState NewState)
{
	FScopeLock Lock(&StateMutex);
	if (CurrentState == Expected)
	{
		CurrentState = NewState;
		return true;
	}
	if (CurrentState == ESessionState::Canceling)
	{
		return false;
	}
	return false;
}