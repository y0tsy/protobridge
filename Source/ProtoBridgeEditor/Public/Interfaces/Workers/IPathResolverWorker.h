#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGEEDITOR_API IPathResolverWorker
{
public:
	virtual ~IPathResolverWorker() = default;

	virtual FString ResolveProtocPath(const FString& CustomPath) const = 0;
	virtual FString ResolvePluginPath(const FString& CustomPath) const = 0;
	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const = 0;
};