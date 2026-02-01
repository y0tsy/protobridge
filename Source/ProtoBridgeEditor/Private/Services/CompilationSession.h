#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeConfiguration.h"
#include "Templates/Function.h"
#include "HAL/Event.h"
#include "Tasks/Task.h"

class FTaskExecutor;
class FProtoBridgeEventBus;
class FProtoBridgeCacheManager;

enum class ESessionState : uint8
{
	Idle,
	Planning,
	Compiling,
	Canceling,
	Finished
};

class FCompilationSession : public TSharedFromThis<FCompilationSession>
{
public:
	using FExecutorFactory = TFunction<TSharedPtr<FTaskExecutor>(int32 MaxProcesses)>;

	FCompilationSession(TSharedRef<FProtoBridgeEventBus> InEventBus);
	~FCompilationSession();

	void SetExecutorFactory(FExecutorFactory InFactory);
	void Start(const FProtoBridgeConfiguration& Config);
	void Cancel();
	void WaitForCompletion();
	bool IsRunning() const;

private:
	void OnPlanningCompleted(FCompilationPlan Plan, int32 MaxConcurrentProcesses);
	void FinishSession(bool bSuccess, const FString& Message);
	bool TryTransitionState(ESessionState Expected, ESessionState NewState);
	
	TSharedPtr<FTaskExecutor> Executor;
	TSharedPtr<FProtoBridgeCacheManager> CacheManager;
	
	mutable FCriticalSection StateMutex;
	TAtomic<ESessionState> CurrentState;
	
	FExecutorFactory ExecutorFactory;
	TSharedRef<FProtoBridgeEventBus> EventBus;

	TAtomic<bool> CancellationFlag;
	FEvent* CompletionEvent;
};