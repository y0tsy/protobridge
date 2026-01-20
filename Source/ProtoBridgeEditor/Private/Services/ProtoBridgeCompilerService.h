#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Async/Async.h"

class IProtoBridgeWorkerFactory;
class FCompilationSession;

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
	void OnSessionStarted();
	void OnSessionLog(const FString& Msg);
	void OnSessionFinished(bool bSuccess, const FString& Msg);
	void ProcessLogQueue();

	TSharedPtr<IProtoBridgeWorkerFactory> WorkerFactory;
	TSharedPtr<FCompilationSession> CurrentSession;
	
	FOnProtoBridgeCompilationStarted CompilationStartedDelegate;
	FOnProtoBridgeCompilationFinished CompilationFinishedDelegate;
	FOnProtoBridgeLogMessage LogMessageDelegate;

	FCriticalSection LogMutex;
	TArray<FString> LogQueue;
	FTSTicker::FDelegateHandle LogTickerHandle;
};