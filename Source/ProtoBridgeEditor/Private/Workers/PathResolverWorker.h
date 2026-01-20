#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "ProtoBridgeTypes.h"

class IProtoBridgeFileSystem;

class FPathResolverWorker : public IPathResolverWorker
{
public:
	FPathResolverWorker(const FProtoBridgeEnvironmentContext& InContext, TSharedPtr<IProtoBridgeFileSystem> InFileSystem);
	
	virtual FString ResolveDirectory(const FString& PathWithPlaceholders) const override;
	virtual FString ResolveProtocPath() const override;
	virtual FString ResolvePluginPath() const override;
	virtual void ValidateEnvironment() const override;

private:
	FString GetPlatformSpecificBinaryPath(const FString& ExecutableName) const;
	FString GetExecutableExtension() const;
	void InitializeTokens();

	FProtoBridgeEnvironmentContext Context;
	TSharedPtr<IProtoBridgeFileSystem> FileSystem;
	TMap<FString, FString> PathTokens;
};