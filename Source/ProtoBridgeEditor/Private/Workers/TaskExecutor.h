#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/ITaskExecutor.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"

class IProtocExecutor;

class FTaskExecutor : public ITaskExecutor, public TSharedFromThis<FTaskExecutor>
{
public:
	FTaskExecutor(TSharedPtr<IProtoBridgeWorkerFactory> InFactory);
	virtual ~FTaskExecutor();

	virtual void ExecutePlan(const FCompilationPlan& Plan) override;
	virtual void Cancel() override;
	virtual bool IsRunning() const override;

	virtual FOnTaskExecutorOutput& OnOutput() override { return OutputDelegate; }
	virtual FOnTaskExecutorFinished& OnFinished() override { return FinishedDelegate; }

private:
	void StartNextTask();
	void HandleProtocOutput(const FString& Output);
	void HandleProtocCompleted(int32 ReturnCode);
	void CleanupTask(const FCompilationTask& Task, bool bSuccess);

	TSharedPtr<IProtoBridgeWorkerFactory> WorkerFactory;
	TSharedPtr<IProtocExecutor> CurrentProcess;
	
	mutable FCriticalSection StateMutex;
	TArray<FCompilationTask> TaskQueue;
	FCompilationTask CurrentTask;
	
	FOnTaskExecutorOutput OutputDelegate;
	FOnTaskExecutorFinished FinishedDelegate;

	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
};