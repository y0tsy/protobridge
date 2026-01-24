#include "Services/BinaryLocator.h"
#include "Services/PathTokenResolver.h"
#include "ProtoBridgeDefs.h"
#include "HAL/FileManager.h"
#include "Misc/StringBuilder.h"

FString FBinaryLocator::ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.ProtocPath.IsEmpty())
	{
		FString ResolvedPath = FPathTokenResolver::ResolvePath(Context.ProtocPath, Context);
		if (IFileManager::Get().FileExists(*ResolvedPath))
		{
			return ResolvedPath;
		}
	}
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::ProtocExecutableName);
}

FString FBinaryLocator::ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.PluginPath.IsEmpty())
	{
		FString ResolvedPath = FPathTokenResolver::ResolvePath(Context.PluginPath, Context);
		if (IFileManager::Get().FileExists(*ResolvedPath))
		{
			return ResolvedPath;
		}
	}
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::PluginExecutableName);
}

FString FBinaryLocator::ResolveGrpcPluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::GrpcPluginExecutableName);
}

FString FBinaryLocator::FindStandardIncludePath(const FString& ProtocPath)
{
	if (ProtocPath.IsEmpty() || !IFileManager::Get().FileExists(*ProtocPath))
	{
		return FString();
	}

	const FString ProtocDir = FPaths::GetPath(ProtocPath);
	FString IncludePath = FPaths::Combine(ProtocDir, TEXT("../"), FProtoBridgeDefs::StandardIncludeFolder);

	FPaths::CollapseRelativeDirectories(IncludePath);
	FPaths::NormalizeDirectoryName(IncludePath);

	if (IFileManager::Get().DirectoryExists(*IncludePath))
	{
		return IncludePath;
	}

	IncludePath = FPaths::Combine(ProtocDir, FProtoBridgeDefs::StandardIncludeFolder);
	FPaths::NormalizeDirectoryName(IncludePath);
	if (IFileManager::Get().DirectoryExists(*IncludePath))
	{
		return IncludePath;
	}

	return FString();
}

FString FBinaryLocator::FindBinaryPath(const FString& BaseDir, const FString& BinaryName)
{
	if (BaseDir.IsEmpty()) return FString();

	FString NameToSearch = BinaryName;

#if PLATFORM_WINDOWS
	if (!NameToSearch.EndsWith(TEXT(".exe")))
	{
		NameToSearch += TEXT(".exe");
	}
	const TCHAR* Platform = TEXT("Win64");
#elif PLATFORM_MAC
	const TCHAR* Platform = TEXT("Mac");
#elif PLATFORM_LINUX
	const TCHAR* Platform = TEXT("Linux");
#else
	const TCHAR* Platform = TEXT("Unknown");
#endif

	const TCHAR* RelPaths[] = {
		TEXT("Source/ProtoBridgeThirdParty/bin"),
		TEXT("Binaries/ThirdParty"),
		TEXT("Resources/Binaries")
	};

	TStringBuilder<260> SB;

	for (const TCHAR* RelPath : RelPaths)
	{
		SB.Reset();
		SB << BaseDir;
		if (SB.Len() > 0 && SB.LastChar() != TEXT('/'))
		{
			SB << TEXT('/');
		}
		
		int32 BaseLen = SB.Len();
		SB << RelPath << TEXT('/') << Platform << TEXT('/') << NameToSearch;
		
		if (IFileManager::Get().FileExists(SB.ToString()))
		{
			FString Result = SB.ToString();
			FPathTokenResolver::NormalizePath(Result);
			return Result;
		}
		
		SB.RemoveAt(BaseLen, SB.Len() - BaseLen);
		SB << RelPath << TEXT('/') << NameToSearch;

		if (IFileManager::Get().FileExists(SB.ToString()))
		{
			FString Result = SB.ToString();
			FPathTokenResolver::NormalizePath(Result);
			return Result;
		}
	}

	return FString();
}