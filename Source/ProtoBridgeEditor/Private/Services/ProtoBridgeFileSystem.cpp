#include "Services/ProtoBridgeFileSystem.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

bool FProtoBridgeFileSystem::FileExists(const FString& Path) const
{
	return IFileManager::Get().FileExists(*Path);
}

bool FProtoBridgeFileSystem::DirectoryExists(const FString& Path) const
{
	return IFileManager::Get().DirectoryExists(*Path);
}

bool FProtoBridgeFileSystem::MakeDirectory(const FString& Path, bool bCreateTree)
{
	return IFileManager::Get().MakeDirectory(*Path, bCreateTree);
}

bool FProtoBridgeFileSystem::DeleteFile(const FString& Path)
{
	return IFileManager::Get().Delete(*Path, false, true);
}

void FProtoBridgeFileSystem::FindFiles(TArray<FString>& OutFiles, const FString& Directory, const FString& FileExtension) const
{
	IFileManager::Get().FindFiles(OutFiles, *Directory, *FileExtension);
}

void FProtoBridgeFileSystem::FindFilesRecursive(TArray<FString>& OutFiles, const FString& StartDirectory, const FString& Filename, bool Files, bool Directories) const
{
	IFileManager::Get().FindFilesRecursive(OutFiles, *StartDirectory, *Filename, Files, Directories);
}

bool FProtoBridgeFileSystem::SaveStringToFile(const FString& String, const FString& Filename)
{
	return FFileHelper::SaveStringToFile(String, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}