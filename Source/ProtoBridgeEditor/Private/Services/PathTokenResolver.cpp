#include "Services/PathTokenResolver.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"

FString FPathTokenResolver::ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context)
{
	if (InPath.IsEmpty()) return FString();

	FString PathStr = InPath;
	
	PathStr.ReplaceInline(*FProtoBridgeDefs::TokenProjectDir, *Context.ProjectDirectory, ESearchCase::IgnoreCase);
	PathStr.ReplaceInline(*FProtoBridgeDefs::TokenPluginDir, *Context.PluginDirectory, ESearchCase::IgnoreCase);

	int32 PluginMacroIndex = PathStr.Find(FProtoBridgeDefs::TokenPluginMacroStart);
	int32 IterationCount = 0;
	const int32 MaxIterations = 10;

	while (PluginMacroIndex != INDEX_NONE)
	{
		if (++IterationCount > MaxIterations)
		{
			break;
		}

		int32 EndIndex = PathStr.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, PluginMacroIndex);
		if (EndIndex != INDEX_NONE)
		{
			FString Placeholder = PathStr.Mid(PluginMacroIndex, EndIndex - PluginMacroIndex + 1);
			FString PluginName = Placeholder.Mid(FProtoBridgeDefs::TokenPluginMacroStart.Len(), Placeholder.Len() - FProtoBridgeDefs::TokenPluginMacroStart.Len() - 1);
			
			if (const FString* FoundPath = Context.PluginLocations.Find(PluginName))
			{
				PathStr.ReplaceInline(*Placeholder, **FoundPath);
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
		PluginMacroIndex = PathStr.Find(FProtoBridgeDefs::TokenPluginMacroStart);
	}

	NormalizePath(PathStr);
	return PathStr;
}

void FPathTokenResolver::NormalizePath(FString& Path)
{
	if (Path.IsEmpty()) return;
	Path = FPaths::ConvertRelativePathToFull(Path);
	FPaths::NormalizeFilename(Path);
	FPaths::CollapseRelativeDirectories(Path);
}