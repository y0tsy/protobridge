#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

struct FProtoFileCacheEntry
{
	FString FileHash;
	FString ConfigHash;
	FString EffectiveHash;
	TArray<FString> Dependencies;
};

struct FProtoCacheManifest
{
	TMap<FString, FProtoFileCacheEntry> Entries;
};

class FProtoBridgeCacheManager
{
public:
	FProtoBridgeCacheManager(const FProtoBridgeConfiguration& InConfig);

	void LoadCache();
	void SaveCache();

	bool IsFileUpToDate(const FString& FilePath, const FString& ConfigContextHash);
	void UpdateFileSuccess(const FString& FilePath, const FString& ConfigContextHash);

private:
	FString CalculateFileHash(const FString& FilePath);
	FString CalculateEffectiveHash(const FString& FilePath, const FString& ConfigContextHash, TArray<FString>& OutDependencies);
	FString ResolveImportPath(const FString& ImportName, const FString& CurrentFileDir);
	void ExtractImports(const FString& FileContent, TArray<FString>& OutImports);

	FProtoBridgeConfiguration Config;
	FProtoCacheManifest Manifest;
	TMap<FString, FString> FileContentHashMap;
	TMap<FString, FString> EffectiveHashMap;
	FString CacheFilePath;
	bool bIsDirty;
};