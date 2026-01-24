#include "GrpcAsyncNodeBase.h"
#include "ProtoBridgeSubsystem.h"
#include "Engine/Engine.h"
#include "ProtobufUtils.h"

UProtoBridgeSubsystem* UGrpcAsyncNodeBase::GetSubsystem() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UProtoBridgeSubsystem>();
}

void UGrpcAsyncNodeBase::PrepareContext(grpc::ClientContext& Context, const TMap<FString, FString>& Metadata, float TimeoutSeconds)
{
	if (TimeoutSeconds > 0.0f)
	{
		std::chrono::system_clock::time_point Deadline = std::chrono::system_clock::now() + 
			std::chrono::milliseconds(static_cast<int64>(TimeoutSeconds * 1000.0f));
		Context.set_deadline(Deadline);
	}

	for (const TPair<FString, FString>& Pair : Metadata)
	{
		std::string Key, Value;
		FProtobufUtils::FStringToStdString(Pair.Key, Key);
		FProtobufUtils::FStringToStdString(Pair.Value, Value);
		Context.AddMetadata(Key, Value);
	}
}