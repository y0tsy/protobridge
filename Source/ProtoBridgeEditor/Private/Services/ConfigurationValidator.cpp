#include "Services/ConfigurationValidator.h"
#include "Services/PathTokenResolver.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

bool FConfigurationValidator::ValidateSettings(const FProtoBridgeConfiguration& Config, TArray<FProtoBridgeDiagnostic>& OutDiagnostics)
{
	bool bResult = true;

	if (!Config.ApiMacro.IsEmpty())
	{
		if (!IsMacroNameSafe(Config.ApiMacro))
		{
			OutDiagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Unsafe API macro name: %s"), *Config.ApiMacro));
			bResult = false;
		}
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		if (!ValidateMapping(Mapping, Config.Environment, OutDiagnostics))
		{
			bResult = false;
		}
	}

	return bResult;
}

bool FConfigurationValidator::ValidateMapping(const FProtoBridgeMapping& Mapping, const FProtoBridgeEnvironmentContext& Context, TArray<FProtoBridgeDiagnostic>& OutDiagnostics)
{
	FString Source = FPathTokenResolver::ResolvePath(Mapping.SourcePath.Path, Context);
	FString Dest = FPathTokenResolver::ResolvePath(Mapping.DestinationPath.Path, Context);

	if (Source.IsEmpty() || Dest.IsEmpty())
	{
		return false;
	}

	bool bValid = true;

	if (!IsPathSafe(Source, Context))
	{
		OutDiagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Security Error: Unsafe source path detected: %s"), *Source));
		bValid = false;
	}
	else if (!IFileManager::Get().DirectoryExists(*Source))
	{
		OutDiagnostics.Emplace(ELogVerbosity::Warning, FString::Printf(TEXT("Source directory does not exist: %s"), *Source));
		bValid = false;
	}

	if (!IsPathSafe(Dest, Context))
	{
		OutDiagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Security Error: Unsafe destination path detected: %s"), *Dest));
		bValid = false;
	}

	return bValid;
}

bool FConfigurationValidator::IsMacroNameSafe(const FString& MacroName)
{
	if (MacroName.IsEmpty() || FChar::IsDigit(MacroName[0]))
	{
		return false;
	}

	for (int32 i = 0; i < MacroName.Len(); ++i)
	{
		TCHAR C = MacroName[i];
		if (!FChar::IsAlnum(C) && C != TCHAR('_'))
		{
			return false;
		}
	}
	return true;
}

bool FConfigurationValidator::IsPathSafe(const FString& RawPath, const FProtoBridgeEnvironmentContext& Context)
{
	FString FullPath = FPaths::ConvertRelativePathToFull(RawPath);
	FPaths::NormalizeFilename(FullPath);
	FPaths::CollapseRelativeDirectories(FullPath);

	if (FullPath.Contains(TEXT(".."))) return false;

	auto IsUnderSafeDir = [](const FString& PathToCheck, const FString& SafeDir) -> bool
	{
		return FPaths::IsUnderDirectory(PathToCheck, SafeDir);
	};

	bool bIsUnderSafeLocation = IsUnderSafeDir(FullPath, Context.ProjectDirectory) || 
								IsUnderSafeDir(FullPath, Context.PluginDirectory);

	if (!bIsUnderSafeLocation)
	{
		for (const auto& Pair : Context.PluginLocations)
		{
			if (IsUnderSafeDir(FullPath, Pair.Value))
			{
				bIsUnderSafeLocation = true;
				break;
			}
		}
	}

	return bIsUnderSafeLocation;
}