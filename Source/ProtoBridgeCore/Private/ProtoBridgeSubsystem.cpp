#include "ProtoBridgeSubsystem.h"
#include "ProtoBridgeEncoderRegistry.h"
#include "ProtobufStringUtils.h"
#include "ProtobufIncludes.h"
#include "ProtoBridgeCoreSettings.h"
#include "ProtoBridgeLogs.h"

void UProtoBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	bIsInitialized = false;

	const UProtoBridgeCoreSettings* Settings = GetDefault<UProtoBridgeCoreSettings>();
	Int64Strategy = Settings->Int64SerializationStrategy;
	
	{
		FWriteScopeLock Lock(StateLock);
		VariantEncoders = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}

	TMap<EVariantTypes, FVariantEncoder> DefaultEncoders;
	ProtoBridge::EncoderRegistry::GetDefaultEncoders(DefaultEncoders);
	RegisterEncodersBatch(DefaultEncoders);

	bIsInitialized = true;
}

void UProtoBridgeSubsystem::Deinitialize()
{
	bIsInitialized = false;
	{
		FWriteScopeLock Lock(StateLock);
		VariantEncoders = MakeShared<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe>();
	}
	Super::Deinitialize();
}

void UProtoBridgeSubsystem::RegisterVariantEncoder(EVariantTypes Type, FVariantEncoder Encoder)
{
	if (bIsInitialized)
	{
		UE_LOG(LogProtoBridgeCore, Warning, TEXT("Performance Warning: RegisterVariantEncoder called after initialization. This triggers a full map copy under lock. Prefer registering encoders during module startup."));
	}

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
	if (bIsInitialized)
	{
		UE_LOG(LogProtoBridgeCore, Warning, TEXT("Performance Warning: RegisterEncodersBatch called after initialization. This triggers a full map copy under lock."));
	}

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
	const UProtoBridgeCoreSettings* Settings = GetDefault<UProtoBridgeCoreSettings>();
	
	FProtoSerializationContext Context;
	Context.Int64Strategy = Int64Strategy;
	Context.Encoders = VariantEncoders;
	Context.MaxAnyPayloadSize = Settings->MaxAnyPayloadSize;
	Context.MaxByteArraySize = Settings->MaxByteArraySize;
	Context.MaxJsonRecursionDepth = Settings->MaxJsonRecursionDepth;
	Context.bBestEffortJsonParsing = Settings->bBestEffortJsonParsing;
	
	return Context;
}