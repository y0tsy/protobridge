#include "ProtoBridgeSubsystem.h"
#include "ProtoBridgeEncoderRegistry.h"
#include "ProtobufStringUtils.h"
#include "ProtobufIncludes.h"

void UProtoBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Int64Strategy = EProtobufInt64Strategy::AlwaysString;
	
	{
		FWriteScopeLock Lock(StateLock);
		if (!VariantEncoders.IsValid())
		{
			VariantEncoders = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
		}
	}

	ProtoBridge::EncoderRegistry::RegisterDefaultEncoders(*this);
}

void UProtoBridgeSubsystem::Deinitialize()
{
	{
		FWriteScopeLock Lock(StateLock);
		VariantEncoders = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}
	Super::Deinitialize();
}

void UProtoBridgeSubsystem::RegisterVariantEncoder(EVariantTypes Type, FVariantEncoder Encoder)
{
	FWriteScopeLock Lock(StateLock);
	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> NewMap;
	if (VariantEncoders.IsValid())
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>(*VariantEncoders);
	}
	else
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}
	NewMap->Add(Type, Encoder);
	VariantEncoders = NewMap;
}

void UProtoBridgeSubsystem::RegisterEncodersBatch(const TMap<EVariantTypes, FVariantEncoder>& InEncoders)
{
	FWriteScopeLock Lock(StateLock);
	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> NewMap;
	if (VariantEncoders.IsValid())
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>(*VariantEncoders);
	}
	else
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}
	NewMap->Append(InEncoders);
	VariantEncoders = NewMap;
}

void UProtoBridgeSubsystem::SetInt64SerializationStrategy(EProtobufInt64Strategy InStrategy)
{
	FWriteScopeLock Lock(StateLock);
	Int64Strategy = InStrategy;
}

EProtobufInt64Strategy UProtoBridgeSubsystem::GetInt64SerializationStrategy() const
{
	FReadScopeLock Lock(StateLock);
	return Int64Strategy;
}

FProtoSerializationContext UProtoBridgeSubsystem::CreateSerializationContext() const
{
	FReadScopeLock Lock(StateLock);
	FProtoSerializationContext Context;
	Context.Int64Strategy = Int64Strategy;
	Context.Encoders = VariantEncoders;
	return Context;
}