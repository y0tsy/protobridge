#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class PROTOBRIDGEEDITOR_API IProtoBridgeService
{
public:
	virtual ~IProtoBridgeService() = default;

	virtual void Compile(const FProtoBridgeConfiguration& Config) = 0;
	virtual void Cancel() = 0;
	virtual void WaitForCompletion() = 0;
	virtual bool IsCompiling() const = 0;
};