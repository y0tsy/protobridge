#include "Services/ProtoBridgeCacheManager.h"
#include "Services/PathTokenResolver.h"
#include "Services/BinaryLocator.h"
#include "ProtoBridgeDefs.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformFileManager.h"

FProtoBridgeCacheManager::FProtoBridgeCacheManager(const FProtoBridgeConfiguration& InConfig)
	: Config(InConfig)
	, bIsDirty(false)
{
	CacheFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir() / FProtoBridgeDefs::IntermediateFolder / FProtoBridgeDefs::CacheFileName);
}

void FProtoBridgeCacheManager::LoadCache()
{
	FString JsonContent;
	if (FFileHelper::LoadFileToString(JsonContent, *CacheFilePath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const TSharedPtr<FJsonObject>* EntriesObject;
			if (JsonObject->TryGetObjectField(TEXT("Entries"), EntriesObject))
			{
				for (const auto& Pair : (*EntriesObject)->Values)
				{
					const TSharedPtr<FJsonObject> EntryObj = Pair.Value->AsObject();
					if (EntryObj.IsValid())
					{
						FProtoFileCacheEntry Entry;
						Entry.FileHash = EntryObj->GetStringField(TEXT("FileHash"));
						Entry.ConfigHash = EntryObj->GetStringField(TEXT("ConfigHash"));
						Entry.EffectiveHash = EntryObj->GetStringField(TEXT("EffectiveHash"));
						EntryObj->TryGetStringArrayField(TEXT("Dependencies"), Entry.Dependencies);
						Manifest.Entries.Add(Pair.Key, Entry);
					}
				}
			}
		}
	}
}

void FProtoBridgeCacheManager::SaveCache()
{
	if (!bIsDirty) return;

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> EntriesObject = MakeShared<FJsonObject>();

	for (const auto& Pair : Manifest.Entries)
	{
		TSharedPtr<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("FileHash"), Pair.Value.FileHash);
		EntryObj->SetStringField(TEXT("ConfigHash"), Pair.Value.ConfigHash);
		EntryObj->SetStringField(TEXT("EffectiveHash"), Pair.Value.EffectiveHash);
		
		TArray<TSharedPtr<FJsonValue>> DepArray;
		for (const FString& Dep : Pair.Value.Dependencies)
		{
			DepArray.Add(MakeShared<FJsonValueString>(Dep));
		}
		EntryObj->SetArrayField(TEXT("Dependencies"), DepArray);
		
		EntriesObject->SetField(Pair.Key, MakeShared<FJsonValueObject>(EntryObj));
	}

	RootObject->SetObjectField(TEXT("Entries"), EntriesObject);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(OutputString, *CacheFilePath);
}

bool FProtoBridgeCacheManager::IsFileUpToDate(const FString& FilePath, const FString& ConfigContextHash, const FString& SourceRoot, const FString& DestinationRoot)
{
	FString NormalizedPath = FilePath;
	FPaths::NormalizeFilename(NormalizedPath);

	if (!CheckOutputsExist(NormalizedPath, SourceRoot, DestinationRoot))
	{
		return false;
	}

	if (!Manifest.Entries.Contains(NormalizedPath))
	{
		return false;
	}

	TArray<FString> Dependencies;
	FString CurrentEffectiveHash = CalculateEffectiveHash(NormalizedPath, ConfigContextHash, Dependencies);
	const FProtoFileCacheEntry& CachedEntry = Manifest.Entries[NormalizedPath];

	return CachedEntry.EffectiveHash == CurrentEffectiveHash && CachedEntry.ConfigHash == ConfigContextHash;
}

void FProtoBridgeCacheManager::UpdateFileSuccess(const FString& FilePath, const FString& ConfigContextHash)
{
	FString NormalizedPath = FilePath;
	FPaths::NormalizeFilename(NormalizedPath);

	FProtoFileCacheEntry NewEntry;
	NewEntry.ConfigHash = ConfigContextHash;
	NewEntry.EffectiveHash = CalculateEffectiveHash(NormalizedPath, ConfigContextHash, NewEntry.Dependencies);
	NewEntry.FileHash = CalculateFileHash(NormalizedPath);

	Manifest.Entries.Add(NormalizedPath, NewEntry);
	bIsDirty = true;
}

FString FProtoBridgeCacheManager::CalculateFileHash(const FString& FilePath)
{
	if (FileContentHashMap.Contains(FilePath))
	{
		return FileContentHashMap[FilePath];
	}

	FString Hash = TEXT("MISSING");
	if (FPaths::FileExists(FilePath))
	{
		FMD5 HashGen;
		FString Content;
		if (FFileHelper::LoadFileToString(Content, *FilePath))
		{
			HashGen.Update((uint8*)TCHAR_TO_ANSI(*Content), Content.Len());
			Hash = FMD5::HashAnsiString(*Content);
		}
	}
	
	FileContentHashMap.Add(FilePath, Hash);
	return Hash;
}

FString FProtoBridgeCacheManager::CalculateEffectiveHash(const FString& FilePath, const FString& ConfigContextHash, TArray<FString>& OutDependencies)
{
	if (EffectiveHashMap.Contains(FilePath))
	{
		return EffectiveHashMap[FilePath];
	}

	FString SelfHash = CalculateFileHash(FilePath);
	FString CombinedHash = SelfHash + ConfigContextHash;

	FString Content;
	if (FFileHelper::LoadFileToString(Content, *FilePath))
	{
		ExtractImports(Content, OutDependencies);
	}

	for (const FString& Import : OutDependencies)
	{
		FString ResolvedPath = ResolveImportPath(Import, FPaths::GetPath(FilePath));
		if (!ResolvedPath.IsEmpty())
		{
			TArray<FString> DummyDeps;
			CombinedHash += CalculateEffectiveHash(ResolvedPath, ConfigContextHash, DummyDeps);
		}
	}

	FString FinalHash = FMD5::HashAnsiString(*CombinedHash);
	EffectiveHashMap.Add(FilePath, FinalHash);
	return FinalHash;
}

void FProtoBridgeCacheManager::ExtractImports(const FString& FileContent, TArray<FString>& OutImports)
{
	const FString Marker = TEXT("import \"");
	int32 StartIdx = 0;
	while ((StartIdx = FileContent.Find(Marker, ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIdx)) != INDEX_NONE)
	{
		StartIdx += Marker.Len();
		int32 EndIdx = FileContent.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIdx);
		if (EndIdx != INDEX_NONE)
		{
			FString ImportPath = FileContent.Mid(StartIdx, EndIdx - StartIdx);
			OutImports.Add(ImportPath);
			StartIdx = EndIdx + 1;
		}
		else
		{
			break;
		}
	}
}

FString FProtoBridgeCacheManager::ResolveImportPath(const FString& ImportName, const FString& CurrentFileDir)
{
	FString LocalPath = FPaths::Combine(CurrentFileDir, ImportName);
	FPaths::CollapseRelativeDirectories(LocalPath);
	FPaths::NormalizeFilename(LocalPath);
	if (FPaths::FileExists(LocalPath))
	{
		return LocalPath;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		FString SourceDir = FPathTokenResolver::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString CheckPath = FPaths::Combine(SourceDir, ImportName);
		FPaths::CollapseRelativeDirectories(CheckPath);
		FPaths::NormalizeFilename(CheckPath);
		
		if (FPaths::FileExists(CheckPath))
		{
			return CheckPath;
		}
	}

	FString StdInclude = FBinaryLocator::FindStandardIncludePath(Config.Environment);
	if (!StdInclude.IsEmpty())
	{
		FString CheckPath = FPaths::Combine(StdInclude, ImportName);
		FPaths::CollapseRelativeDirectories(CheckPath);
		FPaths::NormalizeFilename(CheckPath);
		if (FPaths::FileExists(CheckPath))
		{
			return CheckPath;
		}
	}

	return FString();
}

bool FProtoBridgeCacheManager::CheckOutputsExist(const FString& FilePath, const FString& SourceRoot, const FString& DestinationRoot)
{
	FString NormalizedSource = SourceRoot;
	FPaths::NormalizeDirectoryName(NormalizedSource);
	if (!NormalizedSource.EndsWith(TEXT("/"))) NormalizedSource += TEXT("/");

	FString NormalizedDest = DestinationRoot;
	FPaths::NormalizeDirectoryName(NormalizedDest);
	if (!NormalizedDest.EndsWith(TEXT("/"))) NormalizedDest += TEXT("/");

	FString RelativePath = FilePath;
	if (RelativePath.StartsWith(NormalizedSource))
	{
		RelativePath = RelativePath.RightChop(NormalizedSource.Len());
	}
	else
	{
		FPaths::MakePathRelativeTo(RelativePath, *NormalizedSource);
	}

	FString BaseDestPath = NormalizedDest + RelativePath;
	FPaths::NormalizeFilename(BaseDestPath);

	FString HeaderPath = FPaths::ChangeExtension(BaseDestPath, TEXT(".pb.h"));
	FString SourcePath = FPaths::ChangeExtension(BaseDestPath, TEXT(".pb.cc"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.FileExists(*HeaderPath) && PlatformFile.FileExists(*SourcePath);
}