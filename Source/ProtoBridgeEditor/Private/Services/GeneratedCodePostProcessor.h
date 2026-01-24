#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"

class FGeneratedCodePostProcessor
{
public:
	static UE::Tasks::TTask<void> LaunchProcessTaskFiles(const FString& SourceDir, const FString& DestDir, const TArray<FString>& InputFiles);

private:
	static void ProcessTaskFilesInternal(const FString& SourceDir, const FString& DestDir, const TArray<FString>& InputFiles);
	static void ProcessSingleFile(const FString& FilePath);
};