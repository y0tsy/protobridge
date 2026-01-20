#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

bool FCommandBuilder::Build(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath)
{
	if (!CheckUnsafeChars(SourceDir) || !CheckUnsafeChars(DestDir))
	{
		return false;
	}

	TStringBuilder<4096> Builder;

	FString ProtocPath = FProtoBridgeUtils::ResolveProtocPath(Config.Environment);
	FString PluginPath = FProtoBridgeUtils::ResolvePluginPath(Config.Environment);
	FPaths::NormalizeFilename(PluginPath);
	FPaths::NormalizeFilename(ProtocPath);

	FString NormalizedDest = DestDir;
	FPaths::NormalizeFilename(NormalizedDest);
	
	FString NormalizedSource = SourceDir;
	FPaths::NormalizeFilename(NormalizedSource);

	if (PluginPath.Contains(TEXT("\"")) || NormalizedDest.Contains(TEXT("\"")) || NormalizedSource.Contains(TEXT("\"")))
	{
		return false;
	}

	Builder.Appendf(TEXT("--plugin=%s=\"%s\"\n"), *FProtoBridgeDefs::PluginGeneratorCommand, *PluginPath);
	Builder.Appendf(TEXT("--ue_out=\"%s\"\n"), *NormalizedDest);
	Builder.Appendf(TEXT("-I=\"%s\"\n"), *NormalizedSource);

	if (!Config.ApiMacro.IsEmpty())
	{
		if (!CheckUnsafeChars(Config.ApiMacro) || Config.ApiMacro.Contains(TEXT(" ")))
		{
			return false;
		}
		Builder.Appendf(TEXT("--ue_opt=dllexport_macro=%s\n"), *Config.ApiMacro);
	}

	for (const FString& File : Files)
	{
		if (!CheckUnsafeChars(File))
		{
			return false;
		}

		FString CleanFile = File;
		FPaths::NormalizeFilename(CleanFile);
		
		if (CleanFile.Contains(TEXT("\"")))
		{
			return false;
		}
		
		Builder.Appendf(TEXT("\"%s\"\n"), *CleanFile);
	}

	FString ArgContent = Builder.ToString();
	if (SaveArgumentFile(ArgContent, OutArgFilePath))
	{
		OutArgs = FString::Printf(TEXT("@\"%s\""), *OutArgFilePath);
		return true;
	}

	return false;
}

bool FCommandBuilder::CheckUnsafeChars(const FString& Str)
{
	const TCHAR* UnsafeChars = TEXT("\n\r");
	return !Str.Contains(UnsafeChars); 
}

bool FCommandBuilder::SaveArgumentFile(const FString& Content, FString& OutFilePath)
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