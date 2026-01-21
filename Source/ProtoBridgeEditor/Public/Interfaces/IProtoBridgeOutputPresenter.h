#pragma once

#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"

class PROTOBRIDGEEDITOR_API IProtoBridgeOutputPresenter
{
public:
	virtual ~IProtoBridgeOutputPresenter() = default;
	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void OnCompilationStarted() = 0;
	virtual void OnCompilationFinished(bool bSuccess, const FString& Message) = 0;
	virtual void OnLogMessage(const FString& Message, ELogVerbosity::Type Verbosity) = 0;
};