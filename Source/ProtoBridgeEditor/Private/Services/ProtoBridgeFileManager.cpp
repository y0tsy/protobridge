#include "Services/ProtoBridgeFileManager.h"
#include "ProtoBridgeDefs.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"

void FProtoBridgeFileManager::CleanupOldTempFiles(const FString& TempDirectory, double MaxAgeSeconds)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FDateTime Now = FDateTime::UtcNow();

	TArray<FString> FilesToDelete;
	PlatformFile.IterateDirectory(*TempDirectory, [&FilesToDelete, &PlatformFile, Now, MaxAgeSeconds](const TCHAR* FilenameOrDirectory, bool bIsDirectory)
	{
		if (!bIsDirectory)
		{
			FFileStatData StatData = PlatformFile.GetStatData(FilenameOrDirectory);
			if (StatData.bIsValid)
			{
				FTimespan Age = Now - StatData.ModificationTime;
				if (Age.GetTotalSeconds() > MaxAgeSeconds)
				{
					FilesToDelete.Add(FilenameOrDirectory);
				}
			}
		}
		return true;
	});

	for (const FString& File : FilesToDelete)
	{
		PlatformFile.DeleteFile(*File);
	}
}

bool FProtoBridgeFileManager::WriteArgumentFile(const FString& Content, FString& OutFilePath)
{
	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / FProtoBridgeDefs::TempFolder;
	IFileManager::Get().MakeDirectory(*TempDir, true);

	FString ArgFilePath = TempDir / FString::Printf(TEXT("cmd_%s%s"), *FGuid::NewGuid().ToString(), *FProtoBridgeDefs::ArgFileExtension);
	FPaths::NormalizeFilename(ArgFilePath);

	if (FFileHelper::SaveStringToFile(Content, *ArgFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutFilePath = ArgFilePath;
		return true;
	}
	return false;
}

void FProtoBridgeFileManager::DeleteFile(const FString& FilePath)
{
	if (!FilePath.IsEmpty())
	{
		IFileManager::Get().Delete(*FilePath, false, true);
	}
}