#include "Workers/CommandBuilderWorker.h"
#include "ProtoBridgeDefs.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

FString FCommandBuilderWorker::BuildCommand(const FProtoBridgeCommandArgs& Args) const
{
	FString ArgumentsContent;
	
	FString PluginPath = Args.PluginPath;
	FPaths::NormalizeFilename(PluginPath);

	FString DestDir = Args.DestinationDirectory;
	FPaths::NormalizeFilename(DestDir);
	
	FString SourceDir = Args.SourceDirectory;
	FPaths::NormalizeFilename(SourceDir);

	ArgumentsContent += FString::Printf(TEXT("--plugin=protoc-gen-ue=\"%s\"\n"), *PluginPath);
	ArgumentsContent += FString::Printf(TEXT("--ue_out=\"%s\"\n"), *DestDir);
	ArgumentsContent += FString::Printf(TEXT("-I=\"%s\"\n"), *SourceDir);

	if (!Args.ApiMacro.IsEmpty())
	{
		ArgumentsContent += FString::Printf(TEXT("--ue_opt=dllexport_macro=%s\n"), *Args.ApiMacro);
	}

	for (const FString& File : Args.ProtoFiles)
	{
		FString NormalizedFile = File;
		FPaths::NormalizeFilename(NormalizedFile);
		ArgumentsContent += FString::Printf(TEXT("\"%s\"\n"), *NormalizedFile);
	}

	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / TEXT("Temp");
	IFileManager::Get().MakeDirectory(*TempDir, true);

	FString ArgFilePath = TempDir / FString::Printf(TEXT("cmd_%s%s"), *FGuid::NewGuid().ToString(), *FProtoBridgeDefs::ArgFileExtension);
	FPaths::NormalizeFilename(ArgFilePath);

	if (FFileHelper::SaveStringToFile(ArgumentsContent, *ArgFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		return FString::Printf(TEXT("@\"%s\""), *ArgFilePath);
	}

	return FString();
}