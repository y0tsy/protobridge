#include "Workers/CommandBuilderWorker.h"
#include "Interfaces/IProtoBridgeFileSystem.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"

FCommandBuilderWorker::FCommandBuilderWorker(TSharedPtr<IProtoBridgeFileSystem> InFileSystem)
	: FileSystem(InFileSystem)
{
}

FCommandBuildResult FCommandBuilderWorker::BuildCommand(const FProtoBridgeCommandArgs& Args) const
{
	FCommandBuildResult Result;
	TStringBuilder<4096> Builder;
	
	FString PluginPath = Args.PluginPath;
	FPaths::NormalizeFilename(PluginPath);

	FString DestDir = Args.DestinationDirectory;
	FPaths::NormalizeFilename(DestDir);
	
	FString SourceDir = Args.SourceDirectory;
	FPaths::NormalizeFilename(SourceDir);

	Builder.Appendf(TEXT("--plugin=protoc-gen-ue=\"%s\"\n"), *PluginPath);
	Builder.Appendf(TEXT("--ue_out=\"%s\"\n"), *DestDir);
	Builder.Appendf(TEXT("-I=\"%s\"\n"), *SourceDir);

	if (!Args.ApiMacro.IsEmpty())
	{
		Builder.Appendf(TEXT("--ue_opt=dllexport_macro=%s\n"), *Args.ApiMacro);
	}

	for (const FString& File : Args.ProtoFiles)
	{
		FString NormalizedFile = File;
		FPaths::NormalizeFilename(NormalizedFile);
		Builder.Appendf(TEXT("\"%s\"\n"), *NormalizedFile);
	}

	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / FProtoBridgeDefs::TempFolder;
	FileSystem->MakeDirectory(TempDir, true);

	FString ArgFilePath = TempDir / FString::Printf(TEXT("cmd_%s%s"), *FGuid::NewGuid().ToString(), *FProtoBridgeDefs::ArgFileExtension);
	FPaths::NormalizeFilename(ArgFilePath);

	if (FileSystem->SaveStringToFile(Builder.ToString(), ArgFilePath))
	{
		Result.TempArgFilePath = ArgFilePath;
		Result.Arguments = FString::Printf(TEXT("@\"%s\""), *ArgFilePath);
	}

	return Result;
}