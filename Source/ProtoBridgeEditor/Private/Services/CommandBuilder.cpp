#include "Services/CommandBuilder.h"
#include "Services/PathTokenResolver.h"
#include "Services/BinaryLocator.h"
#include "ProtoBridgeDefs.h"
#include "Misc/StringBuilder.h"
#include "Misc/Paths.h"

bool FCommandBuilder::BuildContent(const FProtoBridgeConfiguration& Config, const FProtoBridgeMapping& Mapping, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent)
{
	const FString PluginPath = FBinaryLocator::ResolvePluginPath(Config.Environment);
	const FString ProtocPath = FBinaryLocator::ResolveProtocPath(Config.Environment);

	if (SourceDir.IsEmpty() || DestDir.IsEmpty() || PluginPath.IsEmpty() || ProtocPath.IsEmpty())
	{
		return false;
	}

	FString SafeSourceDir = SourceDir;
	FPaths::NormalizeFilename(SafeSourceDir);
	if (SafeSourceDir.EndsWith(TEXT("/"))) SafeSourceDir.LeftChopInline(1);

	FString SafeDestDir = DestDir;
	FPaths::NormalizeFilename(SafeDestDir);
	if (SafeDestDir.EndsWith(TEXT("/"))) SafeDestDir.LeftChopInline(1);

	FString SafePluginPath = PluginPath;
	FPaths::NormalizeFilename(SafePluginPath);

	TStringBuilder<2048> SB;
	
	SB << TEXT("--plugin=protoc-gen-ue=") << SafePluginPath << TEXT("\n");
	SB << TEXT("--ue_out=") << SafeDestDir << TEXT("\n");
	SB << TEXT("--cpp_out=") << SafeDestDir << TEXT("\n");
	
	if (Mapping.bGenerateGrpc)
	{
		const FString GrpcPluginPath = FBinaryLocator::ResolveGrpcPluginPath(Config.Environment);
		if (!GrpcPluginPath.IsEmpty())
		{
			FString SafeGrpcPluginPath = GrpcPluginPath;
			FPaths::NormalizeFilename(SafeGrpcPluginPath);
			SB << TEXT("--plugin=protoc-gen-grpc=") << SafeGrpcPluginPath << TEXT("\n");
			SB << TEXT("--grpc_out=") << SafeDestDir << TEXT("\n");
		}
	}
	
	SB << TEXT("--proto_path=") << SafeSourceDir << TEXT("\n");
	
	const FString StandardIncludePath = FBinaryLocator::FindStandardIncludePath(ProtocPath);
	if (!StandardIncludePath.IsEmpty())
	{
		FString SafeStandardIncludePath = StandardIncludePath;
		FPaths::NormalizeFilename(SafeStandardIncludePath);
		SB << TEXT("--proto_path=") << SafeStandardIncludePath << TEXT("\n");
	}

	if (!Config.ApiMacro.IsEmpty())
	{
		SB << TEXT("--ue_opt=dllexport_macro=") << Config.ApiMacro << TEXT("\n");
	}

	for (const FString& File : Files)
	{
		if (File.IsEmpty()) return false;
		SB << File << TEXT("\n");
	}

	OutContent = SB.ToString();
	return true;
}