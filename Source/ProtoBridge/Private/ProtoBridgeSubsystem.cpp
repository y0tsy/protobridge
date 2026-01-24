#include "ProtoBridgeSubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UProtoBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadConfigAndCreateThreads();
}

void UProtoBridgeSubsystem::Deinitialize()
{
	for (auto& Pair : ClientThreads)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->Stop();
		}
	}
	ClientThreads.Empty();
	Super::Deinitialize();
}

TSharedPtr<FGrpcClientThread> UProtoBridgeSubsystem::GetClientThread(const FString& ServiceName)
{
	if (TSharedPtr<FGrpcClientThread>* Found = ClientThreads.Find(ServiceName))
	{
		return *Found;
	}
	return nullptr;
}

void UProtoBridgeSubsystem::LoadConfigAndCreateThreads()
{
	const FString SectionName = TEXT("ProtoBridge.Services");
	
	if (!GConfig->DoesSectionExist(*SectionName, GGameIni))
	{
		return;
	}

	TArray<FString> SectionKeys;
	GConfig->GetSection(*SectionName, SectionKeys, GGameIni);

	TSet<FString> ProcessedServices;

	for (const FString& Key : SectionKeys)
	{
		FString ServiceName, PropName;
		if (!Key.Split(TEXT("."), &ServiceName, &PropName))
		{
			continue;
		}

		if (ProcessedServices.Contains(ServiceName))
		{
			continue;
		}

		FString Address;
		GConfig->GetString(*SectionName, *(ServiceName + TEXT(".Address")), Address, GGameIni);

		bool bEnableSSL = false;
		GConfig->GetBool(*SectionName, *(ServiceName + TEXT(".EnableSSL")), bEnableSSL, GGameIni);

		FString CertPath;
		GConfig->GetString(*SectionName, *(ServiceName + TEXT(".CertPath")), CertPath, GGameIni);

		FString RootCertContent;
		if (bEnableSSL && !CertPath.IsEmpty())
		{
			RootCertContent = LoadCertificate(CertPath);
		}

		if (!Address.IsEmpty())
		{
			TSharedPtr<FGrpcClientThread> NewThread = MakeShared<FGrpcClientThread>(Address, RootCertContent);
			ClientThreads.Add(ServiceName, NewThread);
			ProcessedServices.Add(ServiceName);
		}
	}
}

FString UProtoBridgeSubsystem::LoadCertificate(const FString& RelPath)
{
	FString FullPath = FPaths::ProjectContentDir() / RelPath;
	FString Result;
	if (FFileHelper::LoadFileToString(Result, *FullPath))
	{
		return Result;
	}
	return FString();
}