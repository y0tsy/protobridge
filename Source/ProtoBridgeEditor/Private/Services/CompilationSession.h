#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeConfiguration.h"
#include "Async/Future.h"

class FTaskExecutor;

class FCompilationSession : public TSharedFromThis<FCompilationSession>
{
public:
	FCompilationSession();
	~FCompilationSession();

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
	
	TAtomic<bool> bIsTearingDown;
	TAtomic<bool> CancellationFlag;
	TFuture<void> WorkerFuture;
};