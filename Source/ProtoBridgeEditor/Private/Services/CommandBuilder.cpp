#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/StringBuilder.h"
#include "Misc/Paths.h"

bool FCommandBuilder::BuildContent(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent)
{
	FString PluginPath = FProtoBridgePathHelpers::ResolvePluginPath(Config.Environment);
	
	if (!IsPathSafeForCommand(SourceDir) || !IsPathSafeForCommand(DestDir) || !IsPathSafeForCommand(PluginPath))
	{
		return false;
	}

	TStringBuilder<2048> SB;
	
	SB << TEXT("--plugin=") << FProtoBridgeDefs::PluginGeneratorCommand << TEXT("=\"") << PluginPath << TEXT("\"\n");
	SB << TEXT("--ue_out=\"") << DestDir << TEXT("\"\n");
	SB << TEXT("-I=\"") << SourceDir << TEXT("\"\n");

	if (!Config.ApiMacro.IsEmpty())
	{
		if (!IsMacroNameSafe(Config.ApiMacro))
		{
			return false;
		}
		SB << TEXT("--ue_opt=dllexport_macro=") << Config.ApiMacro << TEXT("\n");
	}

	for (const FString& File : Files)
	{
		if (!IsPathSafeForCommand(File)) return false;
		SB << TEXT("\"") << File << TEXT("\"\n");
	}

	OutContent = SB.ToString();
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

bool FCommandBuilder::IsPathSafeForCommand(const FString& Str)
{
	if (Str.IsEmpty()) return false;
	if (Str.Contains(TEXT("\""))) return false; 
	if (Str.Contains(TEXT("\n")) || Str.Contains(TEXT("\r"))) return false;

	static const FString AllowedSpecialChars = TEXT(" _-./\\:");
	
	for (int32 i = 0; i < Str.Len(); ++i)
	{
		TCHAR C = Str[i];
		if (!FChar::IsAlnum(C) && !AllowedSpecialChars.Contains(FString(1, &C)))
		{
			return false;
		}
	}
	return true;
}