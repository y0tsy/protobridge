#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"

FString FProtoBridgePathHelpers::ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context)
{
	if (InPath.IsEmpty()) return FString();

	FString PathStr = InPath;
	
	PathStr.ReplaceInline(*FProtoBridgeDefs::TokenProjectDir, *Context.ProjectDirectory, ESearchCase::IgnoreCase);
	PathStr.ReplaceInline(*FProtoBridgeDefs::TokenPluginDir, *Context.PluginDirectory, ESearchCase::IgnoreCase);

	int32 PluginMacroIndex = PathStr.Find(FProtoBridgeDefs::TokenPluginMacroStart);
	int32 IterationCount = 0;
	const int32 MaxIterations = 10;

	while (PluginMacroIndex != INDEX_NONE)
	{
		if (++IterationCount > MaxIterations)
		{
			break;
		}

		int32 EndIndex = PathStr.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, PluginMacroIndex);
		if (EndIndex != INDEX_NONE)
		{
			FString Placeholder = PathStr.Mid(PluginMacroIndex, EndIndex - PluginMacroIndex + 1);
			FString PluginName = Placeholder.Mid(FProtoBridgeDefs::TokenPluginMacroStart.Len(), Placeholder.Len() - FProtoBridgeDefs::TokenPluginMacroStart.Len() - 1);
			
			if (const FString* FoundPath = Context.PluginLocations.Find(PluginName))
			{
				PathStr.ReplaceInline(*Placeholder, **FoundPath);
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
		PluginMacroIndex = PathStr.Find(FProtoBridgeDefs::TokenPluginMacroStart);
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

	auto IsUnderSafeDir = [](const FString& PathToCheck, const FString& SafeDir) -> bool
	{
		return FPaths::IsUnderDirectory(PathToCheck, SafeDir);
	};

	bool bIsUnderSafeLocation = IsUnderSafeDir(FullPath, Context.ProjectDirectory) || 
								IsUnderSafeDir(FullPath, Context.PluginDirectory);

	if (!bIsUnderSafeLocation)
	{
		for (const auto& Pair : Context.PluginLocations)
		{
			if (IsUnderSafeDir(FullPath, Pair.Value))
			{
				bIsUnderSafeLocation = true;
				break;
			}
		}
	}

	return bIsUnderSafeLocation;
}

void FProtoBridgePathHelpers::NormalizePath(FString& Path)
{
	if (Path.IsEmpty()) return;
	Path = FPaths::ConvertRelativePathToFull(Path);
	FPaths::NormalizeFilename(Path);
	FPaths::CollapseRelativeDirectories(Path);
}

bool FProtoBridgeFileScanner::FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& ExcludeList, TArray<FString>& OutFiles, const TAtomic<bool>& CancellationFlag)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*SourceDir))
	{
		return false;
	}

	class FScannerVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FScannerVisitor(TArray<FString>& InFiles, const TArray<FString>& InExcludeList, const TAtomic<bool>& InCancelFlag)
			: Files(InFiles)
			, ExcludeList(InExcludeList)
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
				
				bool bIsExcluded = false;
				for (const FString& Pattern : ExcludeList)
				{
					if (Pattern.IsEmpty()) continue;
					if (FilePath.MatchesWildcard(Pattern) || FPaths::GetCleanFilename(FilePath).MatchesWildcard(Pattern))
					{
						bIsExcluded = true;
						break;
					}
				}

				if (!bIsExcluded)
				{
					Files.Add(MoveTemp(FilePath));
				}
			}
			return true;
		}

	private:
		TArray<FString>& Files;
		const TArray<FString>& ExcludeList;
		const TAtomic<bool>& CancelFlag;
	};

	FScannerVisitor Visitor(OutFiles, ExcludeList, CancellationFlag);
	
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