#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

DECLARE_MULTICAST_DELEGATE(FOnProtoBridgeCompilationStarted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProtoBridgeCompilationFinished, bool, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProtoBridgeLogMessage, const FString&, ELogVerbosity::Type);

class PROTOBRIDGEEDITOR_API IProtoBridgeService
{
public:
	virtual ~IProtoBridgeService() = default;

	virtual void Compile(const FProtoBridgeConfiguration& Config) = 0;
	virtual void Cancel() = 0;
	virtual void WaitForCompletion() = 0;
	virtual bool IsCompiling() const = 0;

	virtual FOnProtoBridgeCompilationStarted& OnCompilationStarted() = 0;
	virtual FOnProtoBridgeCompilationFinished& OnCompilationFinished() = 0;
	virtual FOnProtoBridgeLogMessage& OnLogMessage() = 0;
};