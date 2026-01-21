#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Internationalization/Regex.h"

FString FProtoBridgePathHelpers::ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context)
{
	if (InPath.IsEmpty()) return FString();

	TStringBuilder<1024> PathBuilder;
	PathBuilder.Append(InPath);

	FString PathStr = PathBuilder.ToString();
	
	if (PathStr.Contains(FProtoBridgeDefs::TokenProjectDir))
	{
		PathStr = PathStr.Replace(*FProtoBridgeDefs::TokenProjectDir, *Context.ProjectDirectory);
	}
	if (PathStr.Contains(FProtoBridgeDefs::TokenPluginDir))
	{
		PathStr = PathStr.Replace(*FProtoBridgeDefs::TokenPluginDir, *Context.PluginDirectory);
	}

	static const FRegexPattern PluginPattern(FProtoBridgeDefs::TokenPluginMacro + TEXT("([a-zA-Z0-9_]+)\\}"));
	FRegexMatcher Matcher(PluginPattern, PathStr);

	while (Matcher.FindNext())
	{
		FString Placeholder = Matcher.GetCaptureGroup(0);
		FString PluginName = Matcher.GetCaptureGroup(1);

		if (const FString* FoundPath = Context.PluginLocations.Find(PluginName))
		{
			PathStr = PathStr.Replace(*Placeholder, **FoundPath);
		}
	}

	FString FullPath = FPaths::ConvertRelativePathToFull(PathStr);
	FPaths::CollapseRelativeDirectories(FullPath);
	FPaths::NormalizeFilename(FullPath);
	
	return FullPath;
}

FString FProtoBridgePathHelpers::ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.ProtocPath.IsEmpty() && IFileManager::Get().FileExists(*Context.ProtocPath))
	{
		return FPaths::ConvertRelativePathToFull(Context.ProtocPath);
	}
	return GetPlatformBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::ProtocExecutableName);
}

FString FProtoBridgePathHelpers::ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.PluginPath.IsEmpty() && IFileManager::Get().FileExists(*Context.PluginPath))
	{
		return FPaths::ConvertRelativePathToFull(Context.PluginPath);
	}
	return GetPlatformBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::PluginExecutableName);
}

FString FProtoBridgePathHelpers::GetPlatformBinaryPath(const FString& BaseDir, const FString& ExecName)
{
	const FString PlatformSubDir = FPlatformProcess::GetBinariesSubdirectory();
	FString Extension = TEXT("");

#if PLATFORM_WINDOWS
	Extension = TEXT(".exe");
#endif
	
	FString Path = FPaths::Combine(
		BaseDir, 
		FProtoBridgeDefs::SourceFolder, 
		FProtoBridgeDefs::ThirdPartyFolder, 
		FProtoBridgeDefs::BinFolder, 
		PlatformSubDir, 
		ExecName + Extension
	);

	return FPaths::ConvertRelativePathToFull(Path);
}

bool FProtoBridgePathHelpers::IsPathSafe(const FString& InPath, const FProtoBridgeEnvironmentContext& Context)
{
	FString FullPath = FPaths::ConvertRelativePathToFull(InPath);
	FPaths::NormalizeFilename(FullPath);

	if (FPaths::IsUnderDirectory(FullPath, FPaths::ConvertRelativePathToFull(Context.ProjectDirectory))) return true;
	if (FPaths::IsUnderDirectory(FullPath, FPaths::ConvertRelativePathToFull(Context.PluginDirectory))) return true;
	
	for (const auto& Pair : Context.PluginLocations)
	{
		if (FPaths::IsUnderDirectory(FullPath, FPaths::ConvertRelativePathToFull(Pair.Value))) return true;
	}
	return false;
}

bool FProtoBridgeFileScanner::FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& Blacklist, TArray<FString>& OutFiles)
{
	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		return false;
	}

	if (bRecursive)
	{
		IFileManager::Get().FindFilesRecursive(OutFiles, *SourceDir, *FProtoBridgeDefs::ProtoWildcard, true, false);
	}
	else
	{
		IFileManager::Get().FindFiles(OutFiles, *SourceDir, *FProtoBridgeDefs::ProtoWildcard);
		for (FString& File : OutFiles)
		{
			File = FPaths::Combine(SourceDir, File);
		}
	}

	for (int32 i = OutFiles.Num() - 1; i >= 0; --i)
	{
		FString NormalizedFile = OutFiles[i];
		FPaths::NormalizeFilename(NormalizedFile);
		
		bool bIsBlacklisted = false;
		for (const FString& BlacklistPattern : Blacklist)
		{
			if (BlacklistPattern.IsEmpty()) continue;
			if (NormalizedFile.MatchesWildcard(BlacklistPattern) || FPaths::GetCleanFilename(NormalizedFile).MatchesWildcard(BlacklistPattern))
			{
				bIsBlacklisted = true;
				break;
			}
		}

		if (bIsBlacklisted)
		{
			OutFiles.RemoveAt(i);
		}
		else
		{
			OutFiles[i] = MoveTemp(NormalizedFile);
		}
	}

	return true;
}