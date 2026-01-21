#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

bool FCommandBuilder::Build(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath)
{
	FString ArgContent;
	if (!GenerateArgumentsString(Config, SourceDir, DestDir, Files, ArgContent))
	{
		return false;
	}

	if (WriteArgumentFile(ArgContent, OutArgFilePath))
	{
		OutArgs = FString::Printf(TEXT("@\"%s\""), *OutArgFilePath);
		return true;
	}

	return false;
}

bool FCommandBuilder::GenerateArgumentsString(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent)
{
	if (HasUnsafePathChars(SourceDir) || HasUnsafePathChars(DestDir))
	{
		return false;
	}

	TStringBuilder<4096> Builder;

	FString ProtocPath = FProtoBridgePathHelpers::ResolveProtocPath(Config.Environment);
	FString PluginPath = FProtoBridgePathHelpers::ResolvePluginPath(Config.Environment);
	FPaths::NormalizeFilename(PluginPath);
	FPaths::NormalizeFilename(ProtocPath);

	FString NormalizedDest = DestDir;
	FPaths::NormalizeFilename(NormalizedDest);
	
	FString NormalizedSource = SourceDir;
	FPaths::NormalizeFilename(NormalizedSource);

	if (HasUnsafePathChars(PluginPath) || HasUnsafePathChars(NormalizedDest) || HasUnsafePathChars(NormalizedSource))
	{
		return false;
	}

	Builder.Appendf(TEXT("--plugin=%s=\"%s\"\n"), *FProtoBridgeDefs::PluginGeneratorCommand, *PluginPath);
	Builder.Appendf(TEXT("--ue_out=\"%s\"\n"), *NormalizedDest);
	Builder.Appendf(TEXT("-I=\"%s\"\n"), *NormalizedSource);

	if (!Config.ApiMacro.IsEmpty())
	{
		if (!IsMacroSafe(Config.ApiMacro))
		{
			return false;
		}
		Builder.Appendf(TEXT("--ue_opt=dllexport_macro=%s\n"), *Config.ApiMacro);
	}

	for (const FString& File : Files)
	{
		if (HasUnsafePathChars(File))
		{
			return false;
		}

		FString CleanFile = File;
		FPaths::NormalizeFilename(CleanFile);
		
		Builder.Appendf(TEXT("\"%s\"\n"), *CleanFile);
	}

	OutContent = Builder.ToString();
	return true;
}

bool FCommandBuilder::WriteArgumentFile(const FString& Content, FString& OutFilePath)
{
	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / FProtoBridgeDefs::TempFolder;
	IFileManager::Get().MakeDirectory(*TempDir, true);

	FString ArgFilePath = TempDir / FString::Printf(TEXT("cmd_%s%s"), *FGuid::NewGuid().ToString(), *FProtoBridgeDefs::ArgFileExtension);
	FPaths::NormalizeFilename(ArgFilePath);

	if (FFileHelper::SaveStringToFile(Content, *ArgFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutFilePath = ArgFilePath;
		return true;
	}
	return false;
}

bool FCommandBuilder::IsMacroSafe(const FString& Str)
{
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

bool FCommandBuilder::HasUnsafePathChars(const FString& Str)
{
	return Str.Contains(TEXT("\"")); 
}