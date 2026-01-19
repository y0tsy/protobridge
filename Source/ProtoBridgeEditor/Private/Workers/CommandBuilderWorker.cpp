#include "Workers/CommandBuilderWorker.h"
#include "Misc/Paths.h"

FString FCommandBuilderWorker::BuildCommand(const FProtoBridgeCommandArgs& Args) const
{
	FString Command;
	
	FString PluginPath = Args.PluginPath;
	FPaths::NormalizeFilename(PluginPath);

	FString DestDir = Args.DestinationDirectory;
	FPaths::NormalizeFilename(DestDir);
	
	FString SourceDir = Args.SourceDirectory;
	FPaths::NormalizeFilename(SourceDir);

	Command += FString::Printf(TEXT("--plugin=protoc-gen-ue=\"%s\" "), *PluginPath);
	Command += FString::Printf(TEXT("--ue_out=\"%s\" "), *DestDir);
	Command += FString::Printf(TEXT("-I=\"%s\" "), *SourceDir);

	if (!Args.ApiMacro.IsEmpty())
	{
		Command += FString::Printf(TEXT("--ue_opt=dllexport_macro=%s "), *Args.ApiMacro);
	}

	for (const FString& File : Args.ProtoFiles)
	{
		FString NormalizedFile = File;
		FPaths::NormalizeFilename(NormalizedFile);
		Command += FString::Printf(TEXT("\"%s\" "), *NormalizedFile);
	}

	return Command;
}