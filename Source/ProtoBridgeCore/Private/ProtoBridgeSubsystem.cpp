#include "ProtoBridgeSubsystem.h"
#include "ProtoBridgeEncoderRegistry.h"
#include "ProtobufStringUtils.h"
#include "ProtobufIncludes.h"
#include "ProtoBridgeCoreSettings.h"

void UProtoBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UProtoBridgeCoreSettings* Settings = GetDefault<UProtoBridgeCoreSettings>();
	Int64Strategy = Settings->Int64SerializationStrategy;
	
	{
		FWriteScopeLock Lock(StateLock);
		VariantEncoders = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}

	TMap<EVariantTypes, FVariantEncoder> DefaultEncoders;
	ProtoBridge::EncoderRegistry::GetDefaultEncoders(DefaultEncoders);
	RegisterEncodersBatch(DefaultEncoders);
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
	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> OldMap;
	{
		FReadScopeLock Lock(StateLock);
		OldMap = VariantEncoders;
	}

	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> NewMap;
	if (OldMap.IsValid())
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>(*OldMap);
	}
	else
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}
	
	NewMap->Add(Type, Encoder);

	{
		FWriteScopeLock Lock(StateLock);
		VariantEncoders = NewMap;
	}
}

void UProtoBridgeSubsystem::RegisterEncodersBatch(const TMap<EVariantTypes, FVariantEncoder>& InEncoders)
{
	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> OldMap;
	{
		FReadScopeLock Lock(StateLock);
		OldMap = VariantEncoders;
	}

	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> NewMap;
	if (OldMap.IsValid())
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>(*OldMap);
	}
	else
	{
		NewMap = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}

	NewMap->Append(InEncoders);

	{
		FWriteScopeLock Lock(StateLock);
		VariantEncoders = NewMap;
	}
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