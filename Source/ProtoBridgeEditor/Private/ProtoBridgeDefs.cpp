#include "ProtoBridgeDefs.h"

const FString FProtoBridgeDefs::PluginName = TEXT("ProtoBridge");
const FString FProtoBridgeDefs::ProtocExecutableName = TEXT("protoc");
const FString FProtoBridgeDefs::PluginExecutableName = TEXT("bridge_generator");
const FString FProtoBridgeDefs::PluginGeneratorCommand = TEXT("protoc-gen-ue");
const FString FProtoBridgeDefs::GeneratedFileExtension = TEXT(".h");
const FString FProtoBridgeDefs::ArgFileExtension = TEXT(".args");
const FString FProtoBridgeDefs::ProtoExtension = TEXT("proto");
const FString FProtoBridgeDefs::SourceFolder = TEXT("Source");
const FString FProtoBridgeDefs::ThirdPartyFolder = TEXT("ProtoBridgeThirdParty");
const FString FProtoBridgeDefs::BinFolder = TEXT("bin");
const FString FProtoBridgeDefs::TempFolder = TEXT("Temp");
const FString FProtoBridgeDefs::ProtoWildcard = TEXT("*.proto");

const FString FProtoBridgeDefs::TokenProjectDir = TEXT("{Project}");
const FString FProtoBridgeDefs::TokenPluginDir = TEXT("{ProtoBridgePlugin}");
const FString FProtoBridgeDefs::TokenPluginMacro = TEXT("{Plugin:");

const double FProtoBridgeDefs::ExecutionTimeoutSeconds = 60.0;
const double FProtoBridgeDefs::LogTimeLimitSeconds = 0.015;