#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Internationalization/Regex.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"

FString FProtoBridgePathHelpers::ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context)
{
	if (InPath.IsEmpty()) return FString();

	FString PathStr = InPath;
	
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

	NormalizePath(PathStr);
	return PathStr;
}

FString FProtoBridgePathHelpers::ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.ProtocPath.IsEmpty() && IFileManager::Get().FileExists(*Context.ProtocPath))
	{
		FString Path = Context.ProtocPath;
		NormalizePath(Path);
		return Path;
	}
	
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::ProtocExecutableName);
}

FString FProtoBridgePathHelpers::ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.PluginPath.IsEmpty() && IFileManager::Get().FileExists(*Context.PluginPath))
	{
		FString Path = Context.PluginPath;
		NormalizePath(Path);
		return Path;
	}
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::PluginExecutableName);
}

FString FProtoBridgePathHelpers::FindBinaryPath(const FString& BaseDir, const FString& BinaryName)
{
	TArray<FString> SearchPaths;
	SearchPaths.Add(BaseDir / TEXT("Source") / TEXT("ThirdParty") / TEXT("bin"));
	SearchPaths.Add(BaseDir / TEXT("Binaries") / TEXT("ThirdParty"));
	SearchPaths.Add(BaseDir / TEXT("Resources") / TEXT("Binaries"));

#if PLATFORM_WINDOWS
	FString Platform = TEXT("Win64");
#elif PLATFORM_MAC
	FString Platform = TEXT("Mac");
#elif PLATFORM_LINUX
	FString Platform = TEXT("Linux");
#else
	FString Platform = TEXT("Unknown");
#endif

	for (const FString& Path : SearchPaths)
	{
		FString FullPath = Path / Platform / BinaryName;
		if (IFileManager::Get().FileExists(*FullPath))
		{
			NormalizePath(FullPath);
			return FullPath;
		}
		
		FullPath = Path / BinaryName;
		if (IFileManager::Get().FileExists(*FullPath))
		{
			NormalizePath(FullPath);
			return FullPath;
		}
	}

	return FString();
}

bool FProtoBridgePathHelpers::IsPathSafe(const FString& RawPath, const FProtoBridgeEnvironmentContext& Context)
{
	FString FullPath = FPaths::ConvertRelativePathToFull(RawPath);
	FPaths::NormalizeFilename(FullPath);
	FPaths::CollapseRelativeDirectories(FullPath);

	if (FullPath.Contains(TEXT(".."))) return false;

	if (FPaths::IsUnderDirectory(FullPath, Context.ProjectDirectory)) return true;
	if (FPaths::IsUnderDirectory(FullPath, Context.PluginDirectory)) return true;
	
	for (const auto& Pair : Context.PluginLocations)
	{
		if (FPaths::IsUnderDirectory(FullPath, Pair.Value)) return true;
	}
	return false;
}

void FProtoBridgePathHelpers::NormalizePath(FString& Path)
{
	if (Path.IsEmpty()) return;
	Path = FPaths::ConvertRelativePathToFull(Path);
	FPaths::NormalizeFilename(Path);
	FPaths::CollapseRelativeDirectories(Path);
}

bool FProtoBridgeFileScanner::FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& Blacklist, TArray<FString>& OutFiles, const TAtomic<bool>& CancellationFlag)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*SourceDir))
	{
		return false;
	}

	class FScannerVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FScannerVisitor(TArray<FString>& InFiles, const TArray<FString>& InBlacklist, const TAtomic<bool>& InCancelFlag)
			: Files(InFiles)
			, Blacklist(InBlacklist)
			, CancelFlag(InCancelFlag)
		{}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (CancelFlag)
			{
				return false;
			}

			if (bIsDirectory)
			{
				return true;
			}

			FString FilePath = FilenameOrDirectory;
			if (FPaths::GetExtension(FilePath) == FProtoBridgeDefs::ProtoExtension)
			{
				FProtoBridgePathHelpers::NormalizePath(FilePath);
				
				bool bIsBlacklisted = false;
				for (const FString& Pattern : Blacklist)
				{
					if (Pattern.IsEmpty()) continue;
					if (FilePath.MatchesWildcard(Pattern) || FPaths::GetCleanFilename(FilePath).MatchesWildcard(Pattern))
					{
						bIsBlacklisted = true;
						break;
					}
				}

				if (!bIsBlacklisted)
				{
					Files.Add(MoveTemp(FilePath));
				}
			}
			return true;
		}

	private:
		TArray<FString>& Files;
		const TArray<FString>& Blacklist;
		const TAtomic<bool>& CancelFlag;
	};

	FScannerVisitor Visitor(OutFiles, Blacklist, CancellationFlag);
	
	if (bRecursive)
	{
		PlatformFile.IterateDirectoryRecursively(*SourceDir, Visitor);
	}
	else
	{
		PlatformFile.IterateDirectory(*SourceDir, Visitor);
	}

	return !CancellationFlag;
}