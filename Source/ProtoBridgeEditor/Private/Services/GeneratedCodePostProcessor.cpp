#include "Services/GeneratedCodePostProcessor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProtoBridgeDefs.h"

void FGeneratedCodePostProcessor::ProcessTaskFiles(const FString& SourceDir, const FString& DestDir, const TArray<FString>& InputFiles)
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
	FString HeaderGuard;
	HeaderGuard += TEXT("// UE_PROTOBRIDGE_MACRO_GUARD_START\n");
	HeaderGuard += TEXT("#ifdef check\n");
	HeaderGuard += TEXT("  #pragma push_macro(\"check\")\n");
	HeaderGuard += TEXT("  #undef check\n");
	HeaderGuard += TEXT("#endif\n");
	HeaderGuard += TEXT("#ifdef verify\n");
	HeaderGuard += TEXT("  #pragma push_macro(\"verify\")\n");
	HeaderGuard += TEXT("  #undef verify\n");
	HeaderGuard += TEXT("#endif\n\n");

	FString FooterGuard;
	FooterGuard += TEXT("\n// UE_PROTOBRIDGE_MACRO_GUARD_END\n");
	FooterGuard += TEXT("#ifdef check\n");
	FooterGuard += TEXT("  #pragma pop_macro(\"check\")\n");
	FooterGuard += TEXT("#endif\n");
	FooterGuard += TEXT("#ifdef verify\n");
	FooterGuard += TEXT("  #pragma pop_macro(\"verify\")\n");
	FooterGuard += TEXT("#endif\n");

	Content.InsertAt(0, HeaderGuard);
	Content.Append(FooterGuard);
}