#include "Workers/CompilationPlanner.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"
#include "Internationalization/Regex.h"

FCompilationPlanner::FCompilationPlanner(
	TSharedPtr<IPathResolverWorker> InPathResolver,
	TSharedPtr<IFileDiscoveryWorker> InFileDiscovery,
	TSharedPtr<ICommandBuilderWorker> InCommandBuilder
)
	: PathResolver(InPathResolver)
	, FileDiscovery(InFileDiscovery)
	, CommandBuilder(InCommandBuilder)
{
}

FCompilationPlan FCompilationPlanner::CreatePlan(const FProtoBridgeConfiguration& Config)
{
	FCompilationPlan Plan;
	Plan.bIsValid = true;

	if (!Config.ApiMacro.IsEmpty())
	{
		const FRegexPattern ValidMacroPattern(TEXT("^[a-zA-Z0-9_]+$"));
		FRegexMatcher Matcher(ValidMacroPattern, Config.ApiMacro);
		if (!Matcher.FindNext())
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Invalid API Macro name '%s'. It must contain only alphanumeric characters and underscores."), *Config.ApiMacro);
			return Plan;
		}
	}

	FString ResolvedProtoc = PathResolver->ResolveProtocPath();
	FString ResolvedPlugin = PathResolver->ResolvePluginPath();

	if (ResolvedProtoc.IsEmpty())
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = TEXT("Protoc executable path resolution failed.");
		return Plan;
	}

	if (ResolvedPlugin.IsEmpty())
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = TEXT("Plugin executable path resolution failed.");
		return Plan;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		FString SourceDir = PathResolver->ResolveDirectory(Mapping.SourcePath.Path);
		FString DestDir = PathResolver->ResolveDirectory(Mapping.DestinationPath.Path);

		if (SourceDir.IsEmpty() || DestDir.IsEmpty()) continue;

		FFileDiscoveryResult DiscoveryResult = FileDiscovery->FindProtoFiles(SourceDir, Mapping.bRecursive, Mapping.Blacklist);
		
		if (!DiscoveryResult.bSuccess)
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = DiscoveryResult.ErrorMessage;
			return Plan;
		}

		if (DiscoveryResult.Files.Num() == 0) continue;

		FProtoBridgeCommandArgs Args;
		Args.ProtocPath = ResolvedProtoc;
		Args.PluginPath = ResolvedPlugin;
		Args.SourceDirectory = SourceDir;
		Args.DestinationDirectory = DestDir;
		Args.ProtoFiles = DiscoveryResult.Files;
		Args.ApiMacro = Config.ApiMacro;

		FCommandBuildResult BuildResult = CommandBuilder->BuildCommand(Args);
		
		if (!BuildResult.Arguments.IsEmpty())
		{
			FCompilationTask Task;
			Task.ProtocPath = ResolvedProtoc;
			Task.Arguments = BuildResult.Arguments;
			Task.TempArgFilePath = BuildResult.TempArgFilePath;
			Task.SourceDir = SourceDir;
			Task.DestinationDir = DestDir;
			Plan.Tasks.Add(Task);
		}
	}

	if (Plan.Tasks.Num() == 0 && Plan.bIsValid)
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = TEXT("No proto files found or no tasks generated. Check your paths and mappings.");
	}

	return Plan;
}