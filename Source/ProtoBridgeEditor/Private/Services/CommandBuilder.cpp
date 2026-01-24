#include "Services/CommandBuilder.h"
#include "Services/PathTokenResolver.h"
#include "ProtoBridgeDefs.h"
#include "Misc/StringBuilder.h"
#include "Misc/Paths.h"
#include "BinaryLocator.h"

bool FCommandBuilder::BuildContent(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent)
{
	FString PluginPath = FPathTokenResolver::ResolvePath(Config.Environment.PluginPath, Config.Environment);
	PluginPath = FBinaryLocator::ResolvePluginPath(Config.Environment);
	if (SourceDir.IsEmpty() || DestDir.IsEmpty() || PluginPath.IsEmpty())
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
	SB << TEXT("--proto_path=") << SafeSourceDir << TEXT("\n");

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