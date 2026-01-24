#include "Services/GeneratedCodePostProcessor.h"
#include "Misc/Paths.h"
#include "Misc/StringBuilder.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/ScopeExit.h"

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
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.FileExists(*FilePath))
	{
		return;
	}

	const int64 BufferSize = 64 * 1024;
	TUniquePtr<uint8[]> Buffer(new uint8[BufferSize]);

	TUniquePtr<IFileHandle> ReadHandle(PlatformFile.OpenRead(*FilePath));
	if (!ReadHandle)
	{
		return;
	}

	int64 FileSize = ReadHandle->Size();
	if (FileSize == 0) return;

	int64 BytesRead = ReadHandle->Read(Buffer.Get(), FMath::Min(FileSize, BufferSize));
	FString HeaderChunk;
	FFileHelper::BufferToString(HeaderChunk, Buffer.Get(), BytesRead);

	if (HeaderChunk.Contains(TEXT("UE_PROTOBRIDGE_MACRO_GUARD")))
	{
		return;
	}

	ReadHandle->Seek(0);

	FString TempFilePath = FilePath + TEXT(".tmp");
	TUniquePtr<IFileHandle> WriteHandle(PlatformFile.OpenWrite(*TempFilePath));
	if (!WriteHandle)
	{
		return;
	}

	auto WriteString = [&WriteHandle](const FString& Str)
	{
		FTCHARToUTF8 Converter(*Str);
		WriteHandle->Write((const uint8*)Converter.Get(), Converter.Length());
	};

	WriteString(TEXT("// UE_PROTOBRIDGE_MACRO_GUARD_START\n"));
	WriteString(TEXT("#ifdef check\n  #pragma push_macro(\"check\")\n  #undef check\n#endif\n"));
	WriteString(TEXT("#ifdef verify\n  #pragma push_macro(\"verify\")\n  #undef verify\n#endif\n\n"));

	int64 BytesRemaining = FileSize;
	while (BytesRemaining > 0)
	{
		int64 BytesToRead = FMath::Min(BytesRemaining, BufferSize);
		if (ReadHandle->Read(Buffer.Get(), BytesToRead))
		{
			WriteHandle->Write(Buffer.Get(), BytesToRead);
			BytesRemaining -= BytesToRead;
		}
		else
		{
			break;
		}
	}

	WriteString(TEXT("\n// UE_PROTOBRIDGE_MACRO_GUARD_END\n"));
	WriteString(TEXT("#ifdef check\n  #pragma pop_macro(\"check\")\n#endif\n"));
	WriteString(TEXT("#ifdef verify\n  #pragma pop_macro(\"verify\")\n#endif\n"));

	ReadHandle.Reset();
	WriteHandle.Reset();

	PlatformFile.DeleteFile(*FilePath);
	PlatformFile.MoveFile(*FilePath, *TempFilePath);
}