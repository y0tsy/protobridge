#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/StringBuilder.h"
#include "Misc/Paths.h"

bool FCommandBuilder::BuildContent(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent)
{
	FString PluginPath = FProtoBridgePathHelpers::ResolvePluginPath(Config.Environment);
	
	if (SourceDir.IsEmpty() || DestDir.IsEmpty() || PluginPath.IsEmpty())
	{
		UE_LOG(LogProtoBridge, Error, TEXT("CommandBuilder: Missing required paths. Source: '%s', Dest: '%s', Plugin: '%s'"), *SourceDir, *DestDir, *PluginPath);
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
	
	SB << TEXT("--plugin=protoc-gen-ue=\"") << SafePluginPath << TEXT("\"\n");
	SB << TEXT("--ue_out=\"") << SafeDestDir << TEXT("\"\n");
	SB << TEXT("-I=\"") << SafeSourceDir << TEXT("\"\n");

	if (!Config.ApiMacro.IsEmpty())
	{
		if (!IsMacroNameSafe(Config.ApiMacro))
		{
			UE_LOG(LogProtoBridge, Error, TEXT("CommandBuilder: Unsafe API macro name: %s"), *Config.ApiMacro);
			return false;
		}
		SB << TEXT("--ue_opt=dllexport_macro=") << Config.ApiMacro << TEXT("\n");
	}

	for (const FString& File : Files)
	{
		if (File.IsEmpty()) return false;
		SB << TEXT("\"") << File << TEXT("\"\n");
	}

	OutContent = SB.ToString();
	UE_LOG(LogProtoBridge, Verbose, TEXT("Generated Arguments Content:\n%s"), *OutContent);
	
	return true;
}

bool FCommandBuilder::IsMacroNameSafe(const FString& Str)
{
	if (Str.IsEmpty() || FChar::IsDigit(Str[0]))
	{
		return false;
	}

	for (int32 i = 0; i < Str.Len(); ++i)
	{
		TCHAR C = Str[i];
		if (!FChar::IsAlnum(C) && C != TCHAR('_'))
		{
			return false;
		}
	}
	return true;
}