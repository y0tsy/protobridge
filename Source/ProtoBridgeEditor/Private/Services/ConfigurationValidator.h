#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"
#include "ProtoBridgeCompilation.h"

class FConfigurationValidator
{
public:
	static bool ValidateSettings(const FProtoBridgeConfiguration& Config, TArray<FProtoBridgeDiagnostic>& OutDiagnostics);
	static bool ValidateMapping(const FProtoBridgeMapping& Mapping, const FProtoBridgeEnvironmentContext& Context, TArray<FProtoBridgeDiagnostic>& OutDiagnostics);
	static bool IsMacroNameSafe(const FString& MacroName);

private:
	static bool IsPathSafe(const FString& RawPath, const FProtoBridgeEnvironmentContext& Context);
};