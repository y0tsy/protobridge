#include "Workers/PathResolverWorker.h"
#include "ProtoBridgeDefs.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"

static void AppendExecutableExtension(FString& Path)
{
#if PLATFORM_WINDOWS
	if (!Path.EndsWith(TEXT(".exe")))
	{
		Path += TEXT(".exe");
	}
#endif
}

FString FPathResolverWorker::ResolveProtocPath(const FString& CustomPath) const
{
	if (!CustomPath.IsEmpty() && FPaths::FileExists(CustomPath))
	{
		return CustomPath;
	}

	FString BinaryPath = FPaths::Combine(GetThirdPartyBinDir(), FProtoBridgeDefs::ProtocExecutable);
	AppendExecutableExtension(BinaryPath);

	return FPaths::ConvertRelativePathToFull(BinaryPath);
}

FString FPathResolverWorker::ResolvePluginPath(const FString& CustomPath) const
{
	if (!CustomPath.IsEmpty() && FPaths::FileExists(CustomPath))
	{
		return CustomPath;
	}

	FString PluginPath = FPaths::Combine(GetThirdPartyBinDir(), FProtoBridgeDefs::PluginExecutable);
	AppendExecutableExtension(PluginPath);

	return FPaths::ConvertRelativePathToFull(PluginPath);
}

FString FPathResolverWorker::ResolveDirectory(const FString& PathWithPlaceholders) const
{
	FString Result = PathWithPlaceholders;

	if (Result.Contains(TEXT("{Project}")))
	{
		Result = Result.Replace(TEXT("{Project}"), *FPaths::ProjectDir());
	}

	if (Result.Contains(TEXT("{ProtoBridgePlugin}")))
	{
		if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName))
		{
			Result = Result.Replace(TEXT("{ProtoBridgePlugin}"), *Plugin->GetBaseDir());
		}
	}

	const FRegexPattern PluginPattern(TEXT("\\{Plugin:([a-zA-Z0-9_]+)\\}"));
	FRegexMatcher Matcher(PluginPattern, Result);

	while (Matcher.FindNext())
	{
		FString Placeholder = Matcher.GetCaptureGroup(0);
		FString PluginName = Matcher.GetCaptureGroup(1);

		if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName))
		{
			Result = Result.Replace(*Placeholder, *Plugin->GetBaseDir());
		}
	}

	FString FullPath = FPaths::ConvertRelativePathToFull(Result);
	FPaths::NormalizeDirectoryName(FullPath);
	return FullPath;
}

FString FPathResolverWorker::GetThirdPartyBinDir() const
{
	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName))
	{
		return FPaths::Combine(Plugin->GetBaseDir(), FProtoBridgeDefs::SourceFolder, FProtoBridgeDefs::ThirdPartyFolder, FProtoBridgeDefs::BinFolder, GetPlatformName());
	}
	return FString();
}

FString FPathResolverWorker::GetPlatformName() const
{
#if PLATFORM_WINDOWS
	return TEXT("Win64");
#elif PLATFORM_MAC
	return TEXT("MacOS");
#elif PLATFORM_LINUX
	return TEXT("Linux");
#else
	return TEXT("Unknown");
#endif
}