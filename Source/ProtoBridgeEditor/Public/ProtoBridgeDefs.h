#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProtoBridge, Log, All);

struct PROTOBRIDGEEDITOR_API FProtoBridgeDefs
{
	inline static const FString PluginName = TEXT("ProtoBridge");
	inline static const FString ProtocExecutableName = TEXT("protoc");
	inline static const FString PluginExecutableName = TEXT("bridge_generator");
	inline static const FString PluginGeneratorCommand = TEXT("protoc-gen-ue");
	inline static const FString GeneratedFileExtension = TEXT(".h");
	inline static const FString ArgFileExtension = TEXT(".args");
	inline static const FString ProtoExtension = TEXT("proto");
	inline static const FString SourceFolder = TEXT("Source");
	inline static const FString ThirdPartyFolder = TEXT("ProtoBridgeThirdParty");
	inline static const FString BinFolder = TEXT("bin");
	inline static const FString TempFolder = TEXT("Temp");
	inline static const FString ProtoWildcard = TEXT("*.proto");
	
	inline static const FString TokenProjectDir = TEXT("{Project}");
	inline static const FString TokenPluginDir = TEXT("{ProtoBridgePlugin}");
	inline static const FString TokenPluginMacro = TEXT("{Plugin:");

	inline static const double ExecutionTimeoutSeconds = 60.0;
	inline static const double MaxTempFileAgeSeconds = 86400.0;
};