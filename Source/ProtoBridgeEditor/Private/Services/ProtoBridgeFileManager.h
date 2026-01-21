#pragma once

#include "CoreMinimal.h"

class FProtoBridgeFileManager
{
public:
	static void CleanupOldTempFiles(const FString& TempDirectory, double MaxAgeSeconds);
	static bool WriteArgumentFile(const FString& Content, FString& OutFilePath);
	static void DeleteFile(const FString& FilePath);
};