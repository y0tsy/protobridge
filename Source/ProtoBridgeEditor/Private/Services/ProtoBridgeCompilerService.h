#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Containers/Ticker.h"

class FCompilationSession;

class FProtoBridgeCompilerService : public IProtoBridgeService, public TSharedFromThis<FProtoBridgeCompilerService>
{
public:
	FProtoBridgeCompilerService();
	virtual ~FProtoBridgeCompilerService();

	virtual void Compile(const FProtoBridgeConfiguration& Config) override;
	virtual void Cancel() override;
	virtual void WaitForCompletion() override;
	virtual bool IsCompiling() const override;

	virtual FDelegateHandle RegisterOnCompilationStarted(const FOnProtoBridgeCompilationStarted::FDelegate& Delegate) override;
	virtual void UnregisterOnCompilationStarted(FDelegateHandle Handle) override;

	virtual FDelegateHandle RegisterOnCompilationFinished(const FOnProtoBridgeCompilationFinished::FDelegate& Delegate) override;
	virtual void UnregisterOnCompilationFinished(FDelegateHandle Handle) override;

	virtual FDelegateHandle RegisterOnLogMessage(const FOnProtoBridgeLogMessage::FDelegate& Delegate) override;
	virtual void UnregisterOnLogMessage(FDelegateHandle Handle) override;

private:
	void OnSessionStarted();
	void OnSessionLog(const FString& Msg);
	void OnSessionFinished(bool bSuccess, const FString& Msg);
	
	void EnsureTickerRegistered();
	bool ProcessLogQueue();

	TSharedPtr<FCompilationSession> CurrentSession;
	
	FOnProtoBridgeCompilationStarted CompilationStartedDelegate;
	FOnProtoBridgeCompilationFinished CompilationFinishedDelegate;
	FOnProtoBridgeLogMessage LogMessageDelegate;

	FCriticalSection LogMutex;
	TArray<FString> LogQueue;
	FTSTicker::FDelegateHandle LogTickerHandle;
};