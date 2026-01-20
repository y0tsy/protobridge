#include "Workers/PathResolverWorker.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"
#include "HAL/PlatformProcess.h"

FPathResolverWorker::FPathResolverWorker(const FProtoBridgeEnvironmentContext& InContext)
	: Context(InContext)
{
	InitializeTokens();
}

void FPathResolverWorker::InitializeTokens()
{
	PathTokens.Add(TEXT("{Project}"), Context.ProjectDirectory);
	PathTokens.Add(TEXT("{ProtoBridgePlugin}"), Context.PluginDirectory);
}

FString FPathResolverWorker::ResolveDirectory(const FString& PathWithPlaceholders) const
{
	FString Result = PathWithPlaceholders;

	for (const auto& Pair : PathTokens)
	{
		if (Result.Contains(Pair.Key))
		{
			Result = Result.Replace(*Pair.Key, *Pair.Value);
		}
	}

	static const FRegexPattern PluginPattern(TEXT("\\{Plugin:([a-zA-Z0-9_]+)\\}"));
	FRegexMatcher Matcher(PluginPattern, Result);

	while (Matcher.FindNext())
	{
		FString Placeholder = Matcher.GetCaptureGroup(0);
		FString PluginName = Matcher.GetCaptureGroup(1);

		if (const FString* FoundPath = Context.PluginLocations.Find(PluginName))
		{
			Result = Result.Replace(*Placeholder, **FoundPath);
		}
	}

	FString FullPath = FPaths::ConvertRelativePathToFull(Result);
	FPaths::NormalizeDirectoryName(FullPath);
	return FullPath;
}

FString FPathResolverWorker::ResolveProtocPath() const
{
	if (!Context.ProtocPath.IsEmpty() && FPaths::FileExists(Context.ProtocPath))
	{
		return FPaths::ConvertRelativePathToFull(Context.ProtocPath);
	}

	return GetPlatformSpecificBinaryPath(FProtoBridgeDefs::ProtocExecutableName);
}

FString FPathResolverWorker::ResolvePluginPath() const
{
	if (!Context.PluginPath.IsEmpty() && FPaths::FileExists(Context.PluginPath))
	{
		return FPaths::ConvertRelativePathToFull(Context.PluginPath);
	}

	return GetPlatformSpecificBinaryPath(FProtoBridgeDefs::PluginExecutableName);
}

void FPathResolverWorker::ValidateEnvironment() const
{
}

FString FPathResolverWorker::GetExecutableExtension() const
{
#if PLATFORM_WINDOWS
	return TEXT(".exe");
#else
	return TEXT("");
#endif
}

FString FPathResolverWorker::GetPlatformSpecificBinaryPath(const FString& ExecutableName) const
{
	const FString PlatformSubDir = FPlatformProcess::GetBinariesSubdirectory();
	const FString ExecExtension = GetExecutableExtension();
	
	const FString BinaryName = ExecutableName + ExecExtension;
	
	FString Path = FPaths::Combine(
		Context.PluginDirectory, 
		FProtoBridgeDefs::SourceFolder, 
		FProtoBridgeDefs::ThirdPartyFolder, 
		FProtoBridgeDefs::BinFolder, 
		PlatformSubDir, 
		BinaryName
	);

	return FPaths::ConvertRelativePathToFull(Path);
}