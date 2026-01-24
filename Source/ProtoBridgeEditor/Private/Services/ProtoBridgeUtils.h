#pragma once

#include "CoreMinimal.h"

class FProtoBridgeFileScanner
{
public:
	static bool FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& Blacklist, TArray<FString>& OutFiles, const TAtomic<bool>& CancellationFlag);
};