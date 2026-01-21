#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeDelegates.h"

class FAsyncLogDispatcher
{
public:
	FDelegateHandle Add(const FOnProtoBridgeLogMessage::FDelegate& InDelegate);
	void Remove(FDelegateHandle Handle);
	void Broadcast(const FString& Message, ELogVerbosity::Type Verbosity);

private:
	FOnProtoBridgeLogMessage LogDelegate;
	FCriticalSection DelegateMutex;
};