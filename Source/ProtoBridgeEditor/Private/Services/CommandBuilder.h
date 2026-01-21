#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FCommandBuilder
{
public:
	static bool Build(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath);

private:
	static bool GenerateArgumentsString(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent);
	static bool WriteArgumentFile(const FString& Content, FString& OutFilePath);
	static bool IsMacroSafe(const FString& Str);
	static bool HasUnsafePathChars(const FString& Str);
};