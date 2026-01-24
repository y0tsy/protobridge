#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeDelegates.h"
#include "ProtoBridgeCompilation.h"

class FProtoBridgeEventBus : public TSharedFromThis<FProtoBridgeEventBus>
{
public:
	FProtoBridgeEventBus() = default;

	FDelegateHandle RegisterOnLog(const FOnProtoBridgeLogMessage::FDelegate& Delegate);
	void UnregisterOnLog(FDelegateHandle Handle);
	void BroadcastLog(const FProtoBridgeDiagnostic& Diagnostic);

	FDelegateHandle RegisterOnCompilationStarted(const FOnProtoBridgeCompilationStarted::FDelegate& Delegate);
	void UnregisterOnCompilationStarted(FDelegateHandle Handle);
	void BroadcastCompilationStarted();

	FDelegateHandle RegisterOnCompilationFinished(const FOnProtoBridgeCompilationFinished::FDelegate& Delegate);
	void UnregisterOnCompilationFinished(FDelegateHandle Handle);
	void BroadcastCompilationFinished(bool bSuccess, const FString& Message);

private:
	FCriticalSection BusMutex;
	FOnProtoBridgeLogMessage LogDelegate;
	FOnProtoBridgeCompilationStarted StartedDelegate;
	FOnProtoBridgeCompilationFinished FinishedDelegate;
};