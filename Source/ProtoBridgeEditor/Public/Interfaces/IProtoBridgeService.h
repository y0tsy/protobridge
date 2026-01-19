#pragma once

#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE(FOnProtoBridgeCompilationStarted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProtoBridgeCompilationFinished, bool /*bSuccess*/, const FString& /*Message*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProtoBridgeLogMessage, const FString& /*Message*/, ELogVerbosity::Type /*Verbosity*/);

class PROTOBRIDGEEDITOR_API IProtoBridgeService
{
public:
	virtual ~IProtoBridgeService() = default;

	virtual void CompileAll() = 0;
	virtual void Cancel() = 0;
	virtual bool IsCompiling() const = 0;

	virtual FOnProtoBridgeCompilationStarted& OnCompilationStarted() = 0;
	virtual FOnProtoBridgeCompilationFinished& OnCompilationFinished() = 0;
	virtual FOnProtoBridgeLogMessage& OnLogMessage() = 0;
};