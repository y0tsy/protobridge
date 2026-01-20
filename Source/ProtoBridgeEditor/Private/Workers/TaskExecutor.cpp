#include "Workers/TaskExecutor.h"
#include "Interfaces/IProtocExecutor.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"

FTaskExecutor::FTaskExecutor(TSharedPtr<IProtoBridgeWorkerFactory> InFactory)
	: WorkerFactory(InFactory)
	, bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
{
}

FTaskExecutor::~FTaskExecutor()
{
	Cancel();
}

void FTaskExecutor::ExecutePlan(const FCompilationPlan& Plan)
{
	FScopeLock Lock(&StateMutex);
	if (bIsRunning)
	{
		return;
	}

	bIsRunning = true;
	bIsCancelled = false;
	bHasErrors = false;
	TaskQueue = Plan.Tasks;

	TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
	Async(EAsyncExecution::ThreadPool, [WeakSelf]()
	{
		if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
		{
			Self->StartNextTask();
		}
	});
}

void FTaskExecutor::Cancel()
{
	TSharedPtr<IProtocExecutor> ProcToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
		TaskQueue.Empty();
		ProcToCancel = CurrentProcess;
	}

	if (ProcToCancel.IsValid())
	{
		ProcToCancel->Cancel();
	}
}

bool FTaskExecutor::IsRunning() const
{
	FScopeLock Lock(&StateMutex);
	return bIsRunning;
}

void FTaskExecutor::StartNextTask()
{
	FCompilationTask TaskToRun;
	bool bFoundTask = false;

	{
		FScopeLock Lock(&StateMutex);
		
		if (bIsCancelled)
		{
			bIsRunning = false;
			FinishedDelegate.Broadcast(false);
			return;
		}

		if (TaskQueue.Num() > 0)
		{
			TaskToRun = TaskQueue[0];
			TaskQueue.RemoveAt(0);
			bFoundTask = true;
			CurrentTask = TaskToRun;
		}
		else
		{
			bIsRunning = false;
			FinishedDelegate.Broadcast(!bHasErrors);
			return;
		}
	}

	if (bFoundTask)
	{
		OutputDelegate.Broadcast(FString::Printf(TEXT("Processing: %s"), *TaskToRun.SourceDir));
		
		CurrentProcess = WorkerFactory->CreateProtocExecutor();
		CurrentProcess->OnOutput().AddSP(this, &FTaskExecutor::HandleProtocOutput);
		CurrentProcess->OnCompleted().AddSP(this, &FTaskExecutor::HandleProtocCompleted);

		if (!CurrentProcess->Execute(TaskToRun))
		{
			HandleProtocCompleted(-1);
		}
	}
}

void FTaskExecutor::HandleProtocOutput(const FString& Output)
{
	OutputDelegate.Broadcast(Output);
}

void FTaskExecutor::HandleProtocCompleted(int32 ReturnCode)
{
	bool bSuccess = (ReturnCode == 0);
	
	FCompilationTask FinishedTask;
	{
		FScopeLock Lock(&StateMutex);
		FinishedTask = CurrentTask;
		CurrentProcess.Reset();
		
		if (!bSuccess)
		{
			bHasErrors = true;
		}
	}

	CleanupTask(FinishedTask, bSuccess);

	if (!bSuccess)
	{
		OutputDelegate.Broadcast(FString::Printf(TEXT("Task failed with code %d"), ReturnCode));
	}

	StartNextTask();
}

void FTaskExecutor::CleanupTask(const FCompilationTask& Task, bool bSuccess)
{
	if (!Task.TempArgFilePath.IsEmpty() && bSuccess)
	{
		if (IFileManager::Get().FileExists(*Task.TempArgFilePath))
		{
			IFileManager::Get().Delete(*Task.TempArgFilePath, false, true);
		}
	}
}