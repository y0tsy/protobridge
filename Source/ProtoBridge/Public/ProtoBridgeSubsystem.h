#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GrpcClientThread.h"
#include "ProtoBridgeSubsystem.generated.h"

UCLASS()
class PROTOBRIDGE_API UProtoBridgeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TSharedPtr<FGrpcClientThread> GetClientThread(const FString& ServiceName);

private:
	TMap<FString, TSharedPtr<FGrpcClientThread>> ClientThreads;

	void LoadConfigAndCreateThreads();
	FString LoadCertificate(const FString& RelPath);
};