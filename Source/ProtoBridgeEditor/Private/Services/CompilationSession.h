#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeConfiguration.h"
#include "Async/Future.h"
#include "Templates/Function.h"

class FTaskExecutor;

class FCompilationSession : public TSharedFromThis<FCompilationSession>
{
public:
	using FExecutorFactory = TFunction<TSharedPtr<FTaskExecutor>(int32 MaxProcesses)>;

	FCompilationSession();
	~FCompilationSession();

	void SetExecutorFactory(FExecutorFactory InFactory);
	void Start(const FProtoBridgeConfiguration& Config);
	void Cancel();
	void WaitForCompletion();
	bool IsRunning() const;

private:
	void RunDiscovery(const FProtoBridgeConfiguration& Config);
	void FinishSession();
	
	TSharedPtr<FTaskExecutor> Executor;
	mutable FCriticalSection SessionMutex;
	bool bIsActive;
	
	FExecutorFactory ExecutorFactory;

	TAtomic<bool> bIsTearingDown;
	TAtomic<bool> CancellationFlag;
	TFuture<void> WorkerFuture;
};