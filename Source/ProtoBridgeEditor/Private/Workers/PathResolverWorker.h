#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IPathResolverWorker.h"

class FPathResolverWorker : public IPathResolverWorker
{
public:
	virtual FString ResolveProtocPath(const FString& CustomPath) const override;
	virtual FString ResolvePluginPath(const FString& CustomPath) const override;
	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const override;

private:
	FString GetThirdPartyBinDir() const;
	FString GetPlatformName() const;
};