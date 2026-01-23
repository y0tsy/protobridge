#include "Services/GeneratedCodePostProcessor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/StringBuilder.h"
#include "ProtoBridgeDefs.h"

UE::Tasks::TTask<void> FGeneratedCodePostProcessor::LaunchProcessTaskFiles(const FString& SourceDir, const FString& DestDir, const TArray<FString>& InputFiles)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [SourceDir, DestDir, InputFiles]()
	{
		ProcessTaskFilesInternal(SourceDir, DestDir, InputFiles);
	}, UE::Tasks::ETaskPriority::BackgroundHigh);
}

void FGeneratedCodePostProcessor::ProcessTaskFilesInternal(const FString& SourceDir, const FString& DestDir, const TArray<FString>& InputFiles)
{
	FString SafeSourceDir = SourceDir;
	FPaths::NormalizeDirectoryName(SafeSourceDir);
	if (!SafeSourceDir.EndsWith(TEXT("/")))
	{
		SafeSourceDir += TEXT("/");
	}

	for (const FString& InputFile : InputFiles)
	{
		FString SafeInputFile = InputFile;
		FPaths::NormalizeFilename(SafeInputFile);

		FString RelativePath = SafeInputFile;
		
		if (SafeInputFile.StartsWith(SafeSourceDir))
		{
			RelativePath = SafeInputFile.RightChop(SafeSourceDir.Len());
		}
		else
		{
			FPaths::MakePathRelativeTo(RelativePath, *SafeSourceDir);
		}
		
		if (RelativePath.StartsWith(TEXT("/"))) RelativePath.RightChopInline(1);

		FString BaseDestPath = DestDir / RelativePath;
		FPaths::NormalizeFilename(BaseDestPath);
		
		FString HeaderPath = FPaths::ChangeExtension(BaseDestPath, TEXT(".pb.h"));
		FString SourcePath = FPaths::ChangeExtension(BaseDestPath, TEXT(".pb.cc"));

		ProcessSingleFile(HeaderPath);
		ProcessSingleFile(SourcePath);
	}
}

void FGeneratedCodePostProcessor::ProcessSingleFile(const FString& FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogProtoBridge, Warning, TEXT("PostProcessor: Generated file not found: %s"), *FilePath);
		return;
	}

	FString Content;
	if (FFileHelper::LoadFileToString(Content, *FilePath))
	{
		if (Content.Contains(TEXT("UE_PROTOBRIDGE_MACRO_GUARD")))
		{
			return;
		}

		InjectMacroGuards(Content);
		FFileHelper::SaveStringToFile(Content, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		UE_LOG(LogProtoBridge, Verbose, TEXT("PostProcessor: Patched file %s"), *FilePath);
	}
}

void FGeneratedCodePostProcessor::InjectMacroGuards(FString& Content)
{
	TStringBuilder<32768> SB;

	SB << TEXT("// UE_PROTOBRIDGE_MACRO_GUARD_START\n");
	SB << TEXT("#ifdef check\n");
	SB << TEXT("  #pragma push_macro(\"check\")\n");
	SB << TEXT("  #undef check\n");
	SB << TEXT("#endif\n");
	SB << TEXT("#ifdef verify\n");
	SB << TEXT("  #pragma push_macro(\"verify\")\n");
	SB << TEXT("  #undef verify\n");
	SB << TEXT("#endif\n\n");

	SB << Content;

	SB << TEXT("\n// UE_PROTOBRIDGE_MACRO_GUARD_END\n");
	SB << TEXT("#ifdef check\n");
	SB << TEXT("  #pragma pop_macro(\"check\")\n");
	SB << TEXT("#endif\n");
	SB << TEXT("#ifdef verify\n");
	SB << TEXT("  #pragma pop_macro(\"verify\")\n");
	SB << TEXT("#endif\n");

	Content = SB.ToString();
}