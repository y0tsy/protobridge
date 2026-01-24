#include "Services/ProtoBridgeUtils.h"
#include "Services/PathTokenResolver.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

bool FProtoBridgeFileScanner::FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& ExcludeList, TArray<FString>& OutFiles, const TAtomic<bool>& CancellationFlag)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*SourceDir))
	{
		return false;
	}

	class FScannerVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FScannerVisitor(TArray<FString>& InFiles, const TArray<FString>& InExcludeList, const TAtomic<bool>& InCancelFlag)
			: Files(InFiles)
			, ExcludeList(InExcludeList)
			, CancelFlag(InCancelFlag)
		{}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (CancelFlag)
			{
				return false;
			}

			if (bIsDirectory)
			{
				return true;
			}

			FString FilePath = FilenameOrDirectory;
			if (FPaths::GetExtension(FilePath) == FProtoBridgeDefs::ProtoExtension)
			{
				FPathTokenResolver::NormalizePath(FilePath);
				
				bool bIsExcluded = false;
				for (const FString& Pattern : ExcludeList)
				{
					if (Pattern.IsEmpty()) continue;
					if (FilePath.MatchesWildcard(Pattern) || FPaths::GetCleanFilename(FilePath).MatchesWildcard(Pattern))
					{
						bIsExcluded = true;
						break;
					}
				}

				if (!bIsExcluded)
				{
					Files.Add(MoveTemp(FilePath));
				}
			}
			return true;
		}

	private:
		TArray<FString>& Files;
		const TArray<FString>& ExcludeList;
		const TAtomic<bool>& CancelFlag;
	};

	FScannerVisitor Visitor(OutFiles, ExcludeList, CancellationFlag);
	
	if (bRecursive)
	{
		PlatformFile.IterateDirectoryRecursively(*SourceDir, Visitor);
	}
	else
	{
		PlatformFile.IterateDirectory(*SourceDir, Visitor);
	}

	return !CancellationFlag;
}