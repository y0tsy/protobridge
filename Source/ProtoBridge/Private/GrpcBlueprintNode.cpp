#include "GrpcBlueprintNode.h"
#include "ProtoBridgeSubsystem.h"
#include "Engine/Engine.h"
#include "ProtobufStringUtils.h"
#include <chrono>

UProtoBridgeSubsystem* UGrpcBlueprintNode::GetSubsystem() const
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

void UGrpcBlueprintNode::PrepareContext(grpc::ClientContext& Context, const TMap<FString, FString>& Metadata, float TimeoutSeconds)
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
		FProtobufStringUtils::FStringToStdString(Pair.Key, Key);
		FProtobufStringUtils::FStringToStdString(Pair.Value, Value);
		Context.AddMetadata(Key, Value);
	}
}