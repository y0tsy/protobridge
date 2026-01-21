#pragma once

#include "CoreMinimal.h"

struct FCompilationTask
{
	FString SourceDir;
	FString DestinationDir;
	FString TempArgFilePath;
	FString ProtocPath;
	FString Arguments;
};

struct FCompilationPlan
{
	TArray<FCompilationTask> Tasks;
	bool bIsValid = false;
	FString ErrorMessage;
};