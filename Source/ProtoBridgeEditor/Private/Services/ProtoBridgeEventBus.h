#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeDelegates.h"

class FProtoBridgeEventBus
{
public:
	static FProtoBridgeEventBus& Get();

	FDelegateHandle RegisterOnLog(const FOnProtoBridgeLogMessage::FDelegate& Delegate);
	void UnregisterOnLog(FDelegateHandle Handle);
	void BroadcastLog(const FString& Message, ELogVerbosity::Type Verbosity);

	FDelegateHandle RegisterOnCompilationStarted(const FOnProtoBridgeCompilationStarted::FDelegate& Delegate);
	void UnregisterOnCompilationStarted(FDelegateHandle Handle);
	void BroadcastCompilationStarted();

	FDelegateHandle RegisterOnCompilationFinished(const FOnProtoBridgeCompilationFinished::FDelegate& Delegate);
	void UnregisterOnCompilationFinished(FDelegateHandle Handle);
	void BroadcastCompilationFinished(bool bSuccess, const FString& Message);

private:
	FProtoBridgeEventBus() = default;
	
	FCriticalSection BusMutex;
	FOnProtoBridgeLogMessage LogDelegate;
	FOnProtoBridgeCompilationStarted StartedDelegate;
	FOnProtoBridgeCompilationFinished FinishedDelegate;
};