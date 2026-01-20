#include "Services/CompilationSession.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "Interfaces/Workers/ICompilationPlanner.h"
#include "Interfaces/Workers/ITaskExecutor.h"
#include "Async/Async.h"
#include "Misc/ScopeLock.h"

FCompilationSession::FCompilationSession(TSharedPtr<IProtoBridgeWorkerFactory> InFactory)
	: Factory(InFactory)
	, bIsActive(false)
{
}

FCompilationSession::~FCompilationSession()
{
	Cancel();
}

void FCompilationSession::Start(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&StateMutex);
	if (bIsActive) return;
	bIsActive = true;

	StartedDelegate.Broadcast();

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	Async(EAsyncExecution::ThreadPool, [WeakSelf, Config]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->RunInternal(Config);
		}
	});
}

void FCompilationSession::Cancel()
{
	TSharedPtr<ITaskExecutor> ExecutorToCancel;
	{
		FScopeLock Lock(&StateMutex);
		if (!bIsActive) return;
		ExecutorToCancel = Executor;
	}

	if (ExecutorToCancel.IsValid())
	{
		ExecutorToCancel->Cancel();
	}
}

bool FCompilationSession::IsRunning() const
{
	FScopeLock Lock(&StateMutex);
	return bIsActive;
}

void FCompilationSession::RunInternal(FProtoBridgeConfiguration Config)
{
	TSharedPtr<ICompilationPlanner> Planner = Factory->CreatePlanner(Config.Environment);
	if (!Planner.IsValid())
	{
		FScopeLock Lock(&StateMutex);
		bIsActive = false;
		FinishedDelegate.Broadcast(false, TEXT("Failed to create Planner"));
		return;
	}

	LogDelegate.Broadcast(TEXT("Analyzing files..."));
	FCompilationPlan Plan = Planner->CreatePlan(Config);

	if (!Plan.bIsValid)
	{
		FScopeLock Lock(&StateMutex);
		bIsActive = false;
		FinishedDelegate.Broadcast(false, Plan.ErrorMessage);
		return;
	}

	LogDelegate.Broadcast(FString::Printf(TEXT("Generated %d compilation tasks."), Plan.Tasks.Num()));

	{
		FScopeLock Lock(&StateMutex);
		if (!bIsActive) return; 
		Executor = Factory->CreateTaskExecutor();
	}

	if (Executor.IsValid())
	{
		Executor->OnOutput().AddSP(this, &FCompilationSession::HandleExecutorOutput);
		Executor->OnFinished().AddSP(this, &FCompilationSession::HandleExecutorFinished);
		Executor->ExecutePlan(Plan);
	}
	else
	{
		FScopeLock Lock(&StateMutex);
		bIsActive = false;
		FinishedDelegate.Broadcast(false, TEXT("Failed to create Executor"));
	}
}

void FCompilationSession::HandleExecutorOutput(const FString& Msg)
{
	LogDelegate.Broadcast(Msg);
}

void FCompilationSession::HandleExecutorFinished(bool bSuccess)
{
	FScopeLock Lock(&StateMutex);
	bIsActive = false;
	Executor.Reset();
	
	FString Msg = bSuccess ? TEXT("Compilation success") : TEXT("Compilation failed");
	FinishedDelegate.Broadcast(bSuccess, Msg);
}