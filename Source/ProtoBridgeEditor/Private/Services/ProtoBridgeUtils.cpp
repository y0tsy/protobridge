#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Internationalization/Regex.h"

FString FProtoBridgeUtils::ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context)
{
	static const TArray<FString> StaticKeys = {
		FProtoBridgeDefs::TokenPluginDir,
		FProtoBridgeDefs::TokenProjectDir
	};

	FString Result = InPath;
	
	if (Result.Contains(FProtoBridgeDefs::TokenProjectDir))
	{
		Result = Result.Replace(*FProtoBridgeDefs::TokenProjectDir, *Context.ProjectDirectory);
	}
	if (Result.Contains(FProtoBridgeDefs::TokenPluginDir))
	{
		Result = Result.Replace(*FProtoBridgeDefs::TokenPluginDir, *Context.PluginDirectory);
	}

	static const FRegexPattern PluginPattern(FProtoBridgeDefs::TokenPluginMacro + TEXT("([a-zA-Z0-9_]+)\\}"));
	FRegexMatcher Matcher(PluginPattern, Result);

	while (Matcher.FindNext())
	{
		FString Placeholder = Matcher.GetCaptureGroup(0);
		FString PluginName = Matcher.GetCaptureGroup(1);

		if (const FString* FoundPath = Context.PluginLocations.Find(PluginName))
		{
			Result = Result.Replace(*Placeholder, **FoundPath);
		}
	}

	FString FullPath = FPaths::ConvertRelativePathToFull(Result);
	FPaths::CollapseRelativeDirectories(FullPath);
	FPaths::NormalizeFilename(FullPath);
	
	return FullPath;
}

FString FProtoBridgeUtils::ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.ProtocPath.IsEmpty() && IFileManager::Get().FileExists(*Context.ProtocPath))
	{
		return FPaths::ConvertRelativePathToFull(Context.ProtocPath);
	}
	return GetPlatformBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::ProtocExecutableName);
}

FString FProtoBridgeUtils::ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.PluginPath.IsEmpty() && IFileManager::Get().FileExists(*Context.PluginPath))
	{
		return FPaths::ConvertRelativePathToFull(Context.PluginPath);
	}
	return GetPlatformBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::PluginExecutableName);
}

FString FProtoBridgeUtils::GetPlatformBinaryPath(const FString& BaseDir, const FString& ExecName)
{
	const FString PlatformSubDir = FPlatformProcess::GetBinariesSubdirectory();
	FString Extension = TEXT("");
#if PLATFORM_WINDOWS
	Extension = TEXT(".exe");
#endif
	
	FString Path = FPaths::Combine(
		BaseDir, 
		FProtoBridgeDefs::SourceFolder, 
		FProtoBridgeDefs::ThirdPartyFolder, 
		FProtoBridgeDefs::BinFolder, 
		PlatformSubDir, 
		ExecName + Extension
	);

	return FPaths::ConvertRelativePathToFull(Path);
}

bool FProtoBridgeUtils::FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& Blacklist, TArray<FString>& OutFiles)
{
	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		return false;
	}

	if (bRecursive)
	{
		IFileManager::Get().FindFilesRecursive(OutFiles, *SourceDir, *FProtoBridgeDefs::ProtoWildcard, true, false);
	}
	else
	{
		IFileManager::Get().FindFiles(OutFiles, *SourceDir, *FProtoBridgeDefs::ProtoWildcard);
		for (FString& File : OutFiles)
		{
			File = FPaths::Combine(SourceDir, File);
		}
	}

	for (int32 i = OutFiles.Num() - 1; i >= 0; --i)
	{
		FString NormalizedFile = OutFiles[i];
		FPaths::NormalizeFilename(NormalizedFile);
		
		bool bIsBlacklisted = false;
		for (const FString& BlacklistPattern : Blacklist)
		{
			if (BlacklistPattern.IsEmpty()) continue;
			if (NormalizedFile.MatchesWildcard(BlacklistPattern) || FPaths::GetCleanFilename(NormalizedFile).MatchesWildcard(BlacklistPattern))
			{
				bIsBlacklisted = true;
				break;
			}
		}

		if (bIsBlacklisted)
		{
			OutFiles.RemoveAt(i);
		}
		else
		{
			OutFiles[i] = NormalizedFile;
		}
	}

	return true;
}

bool FProtoBridgeUtils::BuildCommandArguments(const FProtoBridgeConfiguration& Config, const FString& SourceDir, const FString& DestDir, const TArray<FString>& Files, FString& OutArgs, FString& OutArgFilePath)
{
	TStringBuilder<4096> Builder;

	FString ProtocPath = ResolveProtocPath(Config.Environment);
	FString PluginPath = ResolvePluginPath(Config.Environment);
	FPaths::NormalizeFilename(PluginPath);
	FPaths::NormalizeFilename(ProtocPath);

	FString NormalizedDest = DestDir;
	FPaths::NormalizeFilename(NormalizedDest);
	
	FString NormalizedSource = SourceDir;
	FPaths::NormalizeFilename(NormalizedSource);

	if (PluginPath.Contains(TEXT("\"")) || NormalizedDest.Contains(TEXT("\"")) || NormalizedSource.Contains(TEXT("\"")))
	{
		return false;
	}

	Builder.Appendf(TEXT("--plugin=protoc-gen-ue=\"%s\"\n"), *PluginPath);
	Builder.Appendf(TEXT("--ue_out=\"%s\"\n"), *NormalizedDest);
	Builder.Appendf(TEXT("-I=\"%s\"\n"), *NormalizedSource);

	if (!Config.ApiMacro.IsEmpty())
	{
		if (Config.ApiMacro.Contains(TEXT("\"")) || Config.ApiMacro.Contains(TEXT(" "))) return false;
		Builder.Appendf(TEXT("--ue_opt=dllexport_macro=%s\n"), *Config.ApiMacro);
	}

	for (const FString& File : Files)
	{
		FString CleanFile = File;
		FPaths::NormalizeFilename(CleanFile);
		if (CleanFile.Contains(TEXT("\""))) return false;
		Builder.Appendf(TEXT("\"%s\"\n"), *CleanFile);
	}

	FString TempDir = FPaths::ProjectSavedDir() / FProtoBridgeDefs::PluginName / FProtoBridgeDefs::TempFolder;
	IFileManager::Get().MakeDirectory(*TempDir, true);

	FString ArgFilePath = TempDir / FString::Printf(TEXT("cmd_%s%s"), *FGuid::NewGuid().ToString(), *FProtoBridgeDefs::ArgFileExtension);
	FPaths::NormalizeFilename(ArgFilePath);

	if (FFileHelper::SaveStringToFile(Builder.ToString(), *ArgFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutArgFilePath = ArgFilePath;
		OutArgs = FString::Printf(TEXT("@\"%s\""), *ArgFilePath);
		return true;
	}

	return false;
}