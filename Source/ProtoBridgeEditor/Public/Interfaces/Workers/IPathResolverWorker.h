#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGEEDITOR_API IPathResolverWorker
{
public:
	virtual ~IPathResolverWorker() = default;

	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const = 0;
	virtual FString ResolveProtocPath() const = 0;
	virtual FString ResolvePluginPath() const = 0;
	virtual void ValidateEnvironment() const = 0;
};