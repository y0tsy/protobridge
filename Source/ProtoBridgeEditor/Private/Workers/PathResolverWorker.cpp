#include "Workers/PathResolverWorker.h"
#include "ProtoBridgeDefs.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

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

	FString BinaryPath = FPaths::Combine(GetThirdPartyBinDir(), TEXT("protoc"));
	AppendExecutableExtension(BinaryPath);

	return FPaths::ConvertRelativePathToFull(BinaryPath);
}

FString FPathResolverWorker::ResolvePluginPath(const FString& CustomPath) const
{
	if (!CustomPath.IsEmpty() && FPaths::FileExists(CustomPath))
	{
		return CustomPath;
	}

	FString PluginPath = FPaths::Combine(GetThirdPartyBinDir(), TEXT("bridge_generator"));
	AppendExecutableExtension(PluginPath);

	return FPaths::ConvertRelativePathToFull(PluginPath);
}

FString FPathResolverWorker::ResolveDirectory(const FString& PathWithPlaceholders) const
{
	FString Result = PathWithPlaceholders;

	if (Result.Contains(TEXT("{Project}")))
	{
		Result = Result.Replace(TEXT("{Project}"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()));
	}

	if (Result.Contains(TEXT("{ProtoBridgePlugin}")))
	{
		if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName))
		{
			Result = Result.Replace(TEXT("{ProtoBridgePlugin}"), *FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()));
		}
	}

	int32 StartIdx = 0;
	while ((StartIdx = Result.Find(TEXT("{Plugin:"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIdx)) != INDEX_NONE)
	{
		int32 EndIdx = Result.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIdx);
		if (EndIdx != INDEX_NONE)
		{
			FString Placeholder = Result.Mid(StartIdx, EndIdx - StartIdx + 1);
			FString PluginName = Placeholder.Mid(8, Placeholder.Len() - 9); 

			if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName))
			{
				Result = Result.Replace(*Placeholder, *FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()));
			}
			else
			{
				StartIdx = EndIdx + 1;
			}
		}
		else
		{
			break;
		}
	}

	FPaths::NormalizeDirectoryName(Result);
	return Result;
}

FString FPathResolverWorker::GetThirdPartyBinDir() const
{
	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName))
	{
		return FPaths::Combine(Plugin->GetBaseDir(), TEXT("Source"), TEXT("ProtoBridgeThirdParty"), TEXT("bin"), GetPlatformName());
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