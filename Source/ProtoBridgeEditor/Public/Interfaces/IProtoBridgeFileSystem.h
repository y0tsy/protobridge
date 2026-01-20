#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGEEDITOR_API IProtoBridgeFileSystem
{
public:
	virtual ~IProtoBridgeFileSystem() = default;

	virtual bool FileExists(const FString& Path) const = 0;
	virtual bool DirectoryExists(const FString& Path) const = 0;
	virtual bool MakeDirectory(const FString& Path, bool bCreateTree) = 0;
	virtual bool DeleteFile(const FString& Path) = 0;
	virtual void FindFiles(TArray<FString>& OutFiles, const FString& Directory, const FString& FileExtension) const = 0;
	virtual void FindFilesRecursive(TArray<FString>& OutFiles, const FString& StartDirectory, const FString& Filename, bool Files, bool Directories) const = 0;
	virtual bool SaveStringToFile(const FString& String, const FString& Filename) = 0;
};