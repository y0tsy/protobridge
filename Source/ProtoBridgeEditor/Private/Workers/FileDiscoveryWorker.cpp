#include "Workers/FileDiscoveryWorker.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

TArray<FString> FFileDiscoveryWorker::FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const
{
	TArray<FString> FoundFiles;
	IFileManager& FileManager = IFileManager::Get();

	if (!FileManager.DirectoryExists(*InSourcePath))
	{
		return FoundFiles;
	}

	if (bRecursive)
	{
		FileManager.FindFilesRecursive(FoundFiles, *InSourcePath, TEXT("*.proto"), true, false, false);
	}
	else
	{
		FileManager.FindFiles(FoundFiles, *InSourcePath, TEXT("*.proto"));
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

	return FoundFiles;
}