#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FCommandBuilder
{
public:
	bool BuildContent(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutContent);

private:
	bool IsMacroNameSafe(const FString& Str);
};