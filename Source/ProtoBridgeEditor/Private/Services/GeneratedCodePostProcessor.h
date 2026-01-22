#pragma once

#include "CoreMinimal.h"

class FGeneratedCodePostProcessor
{
public:
	static void ProcessTaskFiles(const FString& SourceDir, const FString& DestDir, const TArray<FString>& InputFiles);

private:
	static void ProcessSingleFile(const FString& FilePath);
	static void InjectMacroGuards(FString& Content);
};