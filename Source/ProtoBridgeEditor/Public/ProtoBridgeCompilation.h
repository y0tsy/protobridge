#pragma once

#include "CoreMinimal.h"

struct FCompilationTask
{
	FString SourceDir;
	FString DestinationDir;
	FString TempArgFilePath;
	FString ProtocPath;
	FString Arguments;
	TArray<FString> InputFiles;
};

struct FCompilationPlan
{
	TArray<FCompilationTask> Tasks;
	TArray<FString> Errors;
	bool bWasCancelled = false;
};