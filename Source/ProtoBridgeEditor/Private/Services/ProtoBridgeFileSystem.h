#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeFileSystem.h"

class FProtoBridgeFileSystem : public IProtoBridgeFileSystem
{
public:
	virtual bool FileExists(const FString& Path) const override;
	virtual bool DirectoryExists(const FString& Path) const override;
	virtual bool MakeDirectory(const FString& Path, bool bCreateTree) override;
	virtual bool DeleteFile(const FString& Path) override;
	virtual void FindFiles(TArray<FString>& OutFiles, const FString& Directory, const FString& FileExtension) const override;
	virtual void FindFilesRecursive(TArray<FString>& OutFiles, const FString& StartDirectory, const FString& Filename, bool Files, bool Directories) const override;
	virtual bool SaveStringToFile(const FString& String, const FString& Filename) override;
};