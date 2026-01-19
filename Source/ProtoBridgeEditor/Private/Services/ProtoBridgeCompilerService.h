#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Misc/MonitoredProcess.h"

class IProtoBridgeWorkerFactory;
struct FProtoBridgeCommandArgs;

struct FCompilationTask
{
	FString ProtocPath;
	FString Arguments;
	FString SourceDir;
};

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
	void StartNextTask();
	void ProcessTask(const FCompilationTask& Task);
	
	void HandleProcessOutput(FString Output);
	void HandleProcessCompleted(int32 ReturnCode);
	void HandleProcessCanceled();

	TSharedPtr<IProtoBridgeWorkerFactory> WorkerFactory;
	TSharedPtr<FMonitoredProcess> CurrentProcess;
	TArray<FCompilationTask> TaskQueue;
	
	FOnProtoBridgeCompilationStarted CompilationStartedDelegate;
	FOnProtoBridgeCompilationFinished CompilationFinishedDelegate;
	FOnProtoBridgeLogMessage LogMessageDelegate;

	bool bIsActive;
	bool bHasErrors;
};