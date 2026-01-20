#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeService.h"
#include "ProtoBridgeTypes.h"

class IProtoBridgeWorkerFactory;
class IProtocExecutor;
struct FProtoBridgeMapping;

class FProtoBridgeCompilerService : public IProtoBridgeService, public TSharedFromThis<FProtoBridgeCompilerService>
{
public:
	FProtoBridgeCompilerService(TSharedPtr<IProtoBridgeWorkerFactory> InFactory);
	virtual ~FProtoBridgeCompilerService();

	virtual void CompileAll() override;
	virtual void Cancel() override;
	virtual bool IsCompiling() const override;

	virtual FOnProtoBridgeCompilationStarted& OnCompilationStarted() override { return CompilationStartedDelegate; }
	virtual FOnProtoBridgeCompilationFinished& OnCompilationFinished() override { return CompilationFinishedDelegate; }
	virtual FOnProtoBridgeLogMessage& OnLogMessage() override { return LogMessageDelegate; }

private:
	void ExecuteCompilation(TSharedPtr<IProtoBridgeWorkerFactory> Factory, const FProtoBridgeEnvironmentContext& Context, const TArray<FProtoBridgeMapping>& Mappings);
	void StartNextTask();
	void HandleExecutorOutput(const FString& Output);
	void HandleExecutorCompleted(int32 ReturnCode);
	void CleanUpTask(const FCompilationTask& Task);
	void FinalizeCompilation();
	void ReportErrorAndStop(const FString& ErrorMsg);
	void DispatchToGameThread(TFunction<void()> Task);

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