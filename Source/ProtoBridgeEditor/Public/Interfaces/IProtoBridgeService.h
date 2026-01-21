#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeDelegates.h"
#include "ProtoBridgeConfiguration.h"

class PROTOBRIDGEEDITOR_API IProtoBridgeService
{
public:
	virtual ~IProtoBridgeService() = default;

	virtual void Compile(const FProtoBridgeConfiguration& Config) = 0;
	virtual void Cancel() = 0;
	virtual void WaitForCompletion() = 0;
	virtual bool IsCompiling() const = 0;

	virtual FDelegateHandle RegisterOnCompilationStarted(const FOnProtoBridgeCompilationStarted::FDelegate& Delegate) = 0;
	virtual void UnregisterOnCompilationStarted(FDelegateHandle Handle) = 0;

	virtual FDelegateHandle RegisterOnCompilationFinished(const FOnProtoBridgeCompilationFinished::FDelegate& Delegate) = 0;
	virtual void UnregisterOnCompilationFinished(FDelegateHandle Handle) = 0;

	virtual FDelegateHandle RegisterOnLogMessage(const FOnProtoBridgeLogMessage::FDelegate& Delegate) = 0;
	virtual void UnregisterOnLogMessage(FDelegateHandle Handle) = 0;
};