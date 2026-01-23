#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "Misc/StringBuilder.h"
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
	if (!Context.ProtocPath.IsEmpty())
	{
		if (IFileManager::Get().FileExists(*Context.ProtocPath))
		{
			FString Path = Context.ProtocPath;
			NormalizePath(Path);
			UE_LOG(LogProtoBridge, Display, TEXT("Using custom Protoc path: %s"), *Path);
			return Path;
		}
		UE_LOG(LogProtoBridge, Warning, TEXT("Custom Protoc path specified but file not found: %s"), *Context.ProtocPath);
	}
	
	UE_LOG(LogProtoBridge, Display, TEXT("Attempting to resolve Protoc from Plugin Directory: %s"), *Context.PluginDirectory);
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::ProtocExecutableName);
}

FString FProtoBridgePathHelpers::ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.PluginPath.IsEmpty())
	{
		if (IFileManager::Get().FileExists(*Context.PluginPath))
		{
			FString Path = Context.PluginPath;
			NormalizePath(Path);
			UE_LOG(LogProtoBridge, Display, TEXT("Using custom Plugin path: %s"), *Path);
			return Path;
		}
		UE_LOG(LogProtoBridge, Warning, TEXT("Custom Plugin path specified but file not found: %s"), *Context.PluginPath);
	}
	
	UE_LOG(LogProtoBridge, Display, TEXT("Attempting to resolve Plugin Binary from Plugin Directory: %s"), *Context.PluginDirectory);
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::PluginExecutableName);
}

FString FProtoBridgePathHelpers::FindBinaryPath(const FString& BaseDir, const FString& BinaryName)
{
	if (BaseDir.IsEmpty())
	{
		UE_LOG(LogProtoBridge, Error, TEXT("FindBinaryPath called with empty BaseDir for binary: %s"), *BinaryName);
		return FString();
	}

	FString NameToSearch = BinaryName;

#if PLATFORM_WINDOWS
	if (!NameToSearch.EndsWith(TEXT(".exe")))
	{
		NameToSearch += TEXT(".exe");
	}
	const TCHAR* Platform = TEXT("Win64");
#elif PLATFORM_MAC
	const TCHAR* Platform = TEXT("Mac");
#elif PLATFORM_LINUX
	const TCHAR* Platform = TEXT("Linux");
#else
	const TCHAR* Platform = TEXT("Unknown");
#endif

	const TCHAR* RelPaths[] = {
		TEXT("Source/ProtoBridgeThirdParty/bin"),
		TEXT("Binaries/ThirdParty"),
		TEXT("Resources/Binaries")
	};

	TStringBuilder<260> SB;

	for (const TCHAR* RelPath : RelPaths)
	{
		SB.Reset();
		SB << BaseDir;
		if (SB.Len() > 0 && SB.LastChar() != TEXT('/'))
		{
			SB << TEXT('/');
		}
		
		int32 BaseLen = SB.Len();

		SB << RelPath << TEXT('/') << Platform << TEXT('/') << NameToSearch;
		
		if (IFileManager::Get().FileExists(SB.ToString()))
		{
			FString Result = SB.ToString();
			NormalizePath(Result);
			UE_LOG(LogProtoBridge, Display, TEXT("Found binary at: %s"), *Result);
			return Result;
		}
		
		SB.RemoveAt(BaseLen, SB.Len() - BaseLen);
		SB << RelPath << TEXT('/') << NameToSearch;

		if (IFileManager::Get().FileExists(SB.ToString()))
		{
			FString Result = SB.ToString();
			NormalizePath(Result);
			UE_LOG(LogProtoBridge, Display, TEXT("Found binary at: %s"), *Result);
			return Result;
		}
	}

	UE_LOG(LogProtoBridge, Warning, TEXT("Failed to find binary '%s' in standard paths based on %s"), *NameToSearch, *BaseDir);
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