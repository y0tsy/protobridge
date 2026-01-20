#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class FCommandBuilder
{
public:
	static bool Build(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath);

private:
	static bool CheckUnsafeChars(const FString& Str);
	static bool SaveArgumentFile(const FString& Content, FString& OutFilePath);
};