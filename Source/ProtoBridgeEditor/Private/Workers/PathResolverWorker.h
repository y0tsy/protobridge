#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "ProtoBridgeTypes.h"

class FPathResolverWorker : public IPathResolverWorker
{
public:
	FPathResolverWorker(const FProtoBridgeEnvironmentContext& InContext);
	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const override;

private:
	FProtoBridgeEnvironmentContext Context;
};