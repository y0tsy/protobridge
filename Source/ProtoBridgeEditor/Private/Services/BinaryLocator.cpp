#include "Services/BinaryLocator.h"
#include "Services/PathTokenResolver.h"
#include "ProtoBridgeDefs.h"
#include "HAL/FileManager.h"
#include "Misc/StringBuilder.h"

FString FBinaryLocator::ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.ProtocPath.IsEmpty())
	{
		if (IFileManager::Get().FileExists(*Context.ProtocPath))
		{
			FString Path = Context.ProtocPath;
			FPathTokenResolver::NormalizePath(Path);
			return Path;
		}
	}
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::ProtocExecutableName);
}

FString FBinaryLocator::ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context)
{
	if (!Context.PluginPath.IsEmpty())
	{
		if (IFileManager::Get().FileExists(*Context.PluginPath))
		{
			FString Path = Context.PluginPath;
			FPathTokenResolver::NormalizePath(Path);
			return Path;
		}
	}
	return FindBinaryPath(Context.PluginDirectory, FProtoBridgeDefs::PluginExecutableName);
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