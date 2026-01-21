#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FCommandBuilder
{
public:
	bool Build(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath);

private:
	bool GenerateArgumentsString(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent);
	bool IsMacroSafe(const FString& Str);
	bool IsValidPathString(const FString& Str);
};