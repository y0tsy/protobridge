#include "Workers/FileDiscoveryWorker.h"
#include "Interfaces/IProtoBridgeFileSystem.h"
#include "Misc/Paths.h"

FFileDiscoveryWorker::FFileDiscoveryWorker(TSharedPtr<IProtoBridgeFileSystem> InFileSystem)
	: FileSystem(InFileSystem)
{
}

FFileDiscoveryResult FFileDiscoveryWorker::FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const
{
	FFileDiscoveryResult Result;
	
	if (!FileSystem->DirectoryExists(InSourcePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Source directory not found: %s"), *InSourcePath);
		return Result;
	}

	TArray<FString> FoundFiles;
	if (bRecursive)
	{
		FileSystem->FindFilesRecursive(FoundFiles, InSourcePath, TEXT("*.proto"), true, false);
	}
	else
	{
		FileSystem->FindFiles(FoundFiles, InSourcePath, TEXT("*.proto"));
		for (FString& File : FoundFiles)
		{
			File = FPaths::Combine(InSourcePath, File);
		}
	}

	for (int32 i = FoundFiles.Num() - 1; i >= 0; --i)
	{
		FString NormalizedFile = FoundFiles[i];
		FPaths::NormalizeFilename(NormalizedFile);
		
		bool bIsBlacklisted = false;
		for (const FString& BlacklistPattern : InBlacklist)
		{
			if (BlacklistPattern.IsEmpty()) continue;

			if (NormalizedFile.MatchesWildcard(BlacklistPattern) || 
				FPaths::GetCleanFilename(NormalizedFile).MatchesWildcard(BlacklistPattern))
			{
				bIsBlacklisted = true;
				break;
			}
		}

		if (bIsBlacklisted)
		{
			FoundFiles.RemoveAt(i);
		}
		else
		{
			FoundFiles[i] = FPaths::ConvertRelativePathToFull(NormalizedFile);
		}
	}

	Result.Files = FoundFiles;
	Result.bSuccess = true;
	return Result;
}