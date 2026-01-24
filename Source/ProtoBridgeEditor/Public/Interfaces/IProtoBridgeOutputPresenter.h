#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"

class PROTOBRIDGEEDITOR_API IProtoBridgeOutputPresenter
{
public:
	virtual ~IProtoBridgeOutputPresenter() = default;
	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void OnCompilationStarted() = 0;
	virtual void OnCompilationFinished(bool bSuccess, const FString& Message) = 0;
	virtual void OnLogMessage(const FProtoBridgeDiagnostic& Diagnostic) = 0;
};