#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"

class IProtoBridgeOutputPresenter;
class FProtoBridgeEventBus;

class FProtoBridgeUIManager : public TSharedFromThis<FProtoBridgeUIManager>
{
public:
	FProtoBridgeUIManager();
	~FProtoBridgeUIManager();

	void Initialize();
	void Shutdown();

private:
	void HandleCompilationStarted();
	void HandleCompilationFinished(bool bSuccess, const FString& Message);
	void HandleLogMessage(const FProtoBridgeDiagnostic& Diagnostic);

	TArray<TSharedPtr<IProtoBridgeOutputPresenter>> Presenters;
	TWeakPtr<FProtoBridgeEventBus> WeakEventBus;

	FDelegateHandle StartedHandle;
	FDelegateHandle FinishedHandle;
	FDelegateHandle LogHandle;
};