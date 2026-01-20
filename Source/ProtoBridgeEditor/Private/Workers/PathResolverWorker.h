#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "ProtoBridgeTypes.h"

class FPathResolverWorker : public IPathResolverWorker
{
public:
	FPathResolverWorker(const FProtoBridgeEnvironmentContext& InContext);
	
	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const override;
	virtual FString ResolveProtocPath() const override;
	virtual FString ResolvePluginPath() const override;
	virtual void ValidateEnvironment() const override;

private:
	FString GetPlatformSpecificBinaryPath(const FString& ExecutableName) const;
	FString GetExecutableExtension() const;
	void InitializeTokens();

	FProtoBridgeEnvironmentContext Context;
	TMap<FString, FString> PathTokens;
};