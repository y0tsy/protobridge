#pragma once

#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"

struct FProtoBridgeDiagnostic
{
	ELogVerbosity::Type Verbosity;
	FString Message;
	FString FilePath;
	int32 LineNumber = 0;

	FProtoBridgeDiagnostic()
		: Verbosity(ELogVerbosity::Error)
	{}

	FProtoBridgeDiagnostic(ELogVerbosity::Type InVerbosity, const FString& InMessage)
		: Verbosity(InVerbosity)
		, Message(InMessage)
	{}
};

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
	TArray<FProtoBridgeDiagnostic> Diagnostics;
	bool bWasCancelled = false;
};