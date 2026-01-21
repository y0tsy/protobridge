#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "ProtoBridgeFileManager.h"
#include "Misc/StringBuilder.h"

bool FCommandBuilder::Build(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath)
{
	FString ArgContent;
	if (!GenerateArgumentsString(Config, SourceDir, DestDir, Files, ArgContent))
	{
		return false;
	}

	if (FProtoBridgeFileManager::WriteArgumentFile(ArgContent, OutArgFilePath))
	{
		OutArgs = FString::Printf(TEXT("@\"%s\""), *OutArgFilePath);
		return true;
	}

	return false;
}

bool FCommandBuilder::GenerateArgumentsString(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent)
{
	FString PluginPath = FProtoBridgePathHelpers::ResolvePluginPath(Config.Environment);
	
	if (!IsValidPathString(SourceDir) || !IsValidPathString(DestDir) || !IsValidPathString(PluginPath))
	{
		return false;
	}

	TStringBuilder<4096> SB;
	SB << TEXT("--plugin=") << FProtoBridgeDefs::PluginGeneratorCommand << TEXT("=\"") << PluginPath << TEXT("\"\n");
	SB << TEXT("--ue_out=\"") << DestDir << TEXT("\"\n");
	SB << TEXT("-I=\"") << SourceDir << TEXT("\"\n");

	if (!Config.ApiMacro.IsEmpty())
	{
		if (!IsMacroSafe(Config.ApiMacro))
		{
			return false;
		}
		SB << TEXT("--ue_opt=dllexport_macro=") << Config.ApiMacro << TEXT("\n");
	}

	for (const FString& File : Files)
	{
		if (!IsValidPathString(File)) return false;
		SB << TEXT("\"") << File << TEXT("\"\n");
	}

	OutContent = SB.ToString();
	return true;
}

bool FCommandBuilder::IsMacroSafe(const FString& Str)
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

bool FCommandBuilder::IsValidPathString(const FString& Str)
{
	if (Str.IsEmpty()) return false;
	
	for (int32 i = 0; i < Str.Len(); ++i)
	{
		TCHAR C = Str[i];
		const bool bIsAlnum = FChar::IsAlnum(C);
		const bool bIsSafeSymbol = (C == TCHAR('_') || C == TCHAR('-') || C == TCHAR('.') || C == TCHAR('/') || C == TCHAR('\\') || C == TCHAR(':') || C == TCHAR(' '));
		
		if (!bIsAlnum && !bIsSafeSymbol)
		{
			return false;
		}
	}
	return true;
}