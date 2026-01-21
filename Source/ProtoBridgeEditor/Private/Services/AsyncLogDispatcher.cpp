#include "Services/AsyncLogDispatcher.h"

FDelegateHandle FAsyncLogDispatcher::Add(const FOnProtoBridgeLogMessage::FDelegate& InDelegate)
{
	FScopeLock Lock(&DelegateMutex);
	return LogDelegate.Add(InDelegate);
}

void FAsyncLogDispatcher::Remove(FDelegateHandle Handle)
{
	FScopeLock Lock(&DelegateMutex);
	LogDelegate.Remove(Handle);
}

void FAsyncLogDispatcher::Broadcast(const FString& Message, ELogVerbosity::Type Verbosity)
{
	FOnProtoBridgeLogMessage DelegateCopy;
	{
		FScopeLock Lock(&DelegateMutex);
		if (LogDelegate.IsBound())
		{
			DelegateCopy = LogDelegate;
		}
	}

	if (DelegateCopy.IsBound())
	{
		DelegateCopy.Broadcast(Message, Verbosity);
	}
}