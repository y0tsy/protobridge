#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeService.h"
#include "ProtoBridgeTypes.h"
#include "Async/Async.h"

class IProtoBridgeWorkerFactory;
class IProtocExecutor;
struct FProtoBridgeMapping;

class FProtoBridgeCompilerService : public IProtoBridgeService, public TSharedFromThis<FProtoBridgeCompilerService>
{
public:
	FProtoBridgeCompilerService(TSharedPtr<IProtoBridgeWorkerFactory> InFactory);
	virtual ~FProtoBridgeCompilerService();

	virtual void Compile(const FProtoBridgeConfiguration& Config) override;
	virtual void Cancel() override;
	virtual bool IsCompiling() const override;

	virtual FOnProtoBridgeCompilationStarted& OnCompilationStarted() override { return CompilationStartedDelegate; }
	virtual FOnProtoBridgeCompilationFinished& OnCompilationFinished() override { return CompilationFinishedDelegate; }
	virtual FOnProtoBridgeLogMessage& OnLogMessage() override { return LogMessageDelegate; }

private:
	void ExecuteCompilation(TSharedPtr<IProtoBridgeWorkerFactory> Factory, const FProtoBridgeConfiguration& Config);
	void StartNextTask();
	void HandleExecutorOutput(const FString& Output);
	void HandleExecutorCompleted(int32 ReturnCode);
	void CleanUpTask(const FCompilationTask& Task, bool bSuccess);
	void FinalizeCompilation();
	void ReportErrorAndStop(const FString& ErrorMsg);

	template<typename FuncType>
	void DispatchToGameThread(FuncType&& Func);

	TSharedPtr<IProtoBridgeWorkerFactory> WorkerFactory;
	TSharedPtr<IProtocExecutor> CurrentExecutor;
	
	mutable FCriticalSection StateMutex;
	TArray<FCompilationTask> TaskQueue;
	FCompilationTask CurrentTask;
	
	FOnProtoBridgeCompilationStarted CompilationStartedDelegate;
	FOnProtoBridgeCompilationFinished CompilationFinishedDelegate;
	FOnProtoBridgeLogMessage LogMessageDelegate;

	bool bIsActive;
	bool bHasErrors;
};

template<typename FuncType>
void FProtoBridgeCompilerService::DispatchToGameThread(FuncType&& Func)
{
	if (IsInGameThread())
	{
		Func();
	}
	else
	{
		TWeakPtr<FProtoBridgeCompilerService> WeakSelf = AsShared();
		AsyncTask(ENamedThreads::GameThread, [WeakSelf, Task = Forward<FuncType>(Func)]()
		{
			if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
			{
				Task();
			}
		});
	}
}