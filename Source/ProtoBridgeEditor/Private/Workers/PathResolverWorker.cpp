#include "Workers/PathResolverWorker.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"

FPathResolverWorker::FPathResolverWorker(const FProtoBridgeEnvironmentContext& InContext)
	: Context(InContext)
{
}

FString FPathResolverWorker::ResolveDirectory(const FString& PathWithPlaceholders) const
{
	FString Result = PathWithPlaceholders;

	if (Result.Contains(TEXT("{Project}")))
	{
		Result = Result.Replace(TEXT("{Project}"), *Context.ProjectDirectory);
	}

	if (Result.Contains(TEXT("{ProtoBridgePlugin}")))
	{
		Result = Result.Replace(TEXT("{ProtoBridgePlugin}"), *Context.PluginDirectory);
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