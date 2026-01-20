#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGEEDITOR_API IPathResolverWorker
{
public:
	virtual ~IPathResolverWorker() = default;

	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const = 0;
};