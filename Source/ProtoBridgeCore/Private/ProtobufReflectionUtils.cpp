#include "ProtobufReflectionUtils.h"
#include "ProtobufIncludes.h"
#include "ProtoBridgeCoreModule.h"
#include "Misc/ScopeRWLock.h"
#include "Misc/ScopeLock.h"
#include "Templates/Atomic.h"

static FRWLock& GetVariantEncodersLock()
{
	static FRWLock Lock;
	return Lock;
}

static FCriticalSection& GetInitializationInfoLock()
{
	static FCriticalSection Lock;
	return Lock;
}

static TMap<EVariantTypes, FProtobufReflectionUtils::FVariantEncoder> VariantEncoders;
static TAtomic<bool> bEncodersInitialized(false);
static TAtomic<EProtobufInt64Strategy> GlobalInt64Strategy(EProtobufInt64Strategy::AlwaysString);

static void InitializeDefaultEncoders()
{
	if (bEncodersInitialized.Load(EMemoryOrder::SequentiallyConsistent)) return;

	FScopeLock InitScopeLock(&GetInitializationInfoLock());
	if (bEncodersInitialized.Load(EMemoryOrder::SequentiallyConsistent)) return;

	FRWScopeLock WriteLock(GetVariantEncodersLock(), SLT_Write);
	
	auto NullEncoder = [](const FVariant&, google::protobuf::Value& Out) -> bool { 
		Out.set_null_value(google::protobuf::NULL_VALUE); 
		return true;
	};
	VariantEncoders.Add(EVariantTypes::Empty, NullEncoder);

	VariantEncoders.Add(EVariantTypes::Bool, [](const FVariant& V, google::protobuf::Value& Out) -> bool {
		Out.set_bool_value(V.GetValue<bool>());
		return true;
	});

	auto IntEncoder = [](const FVariant& V, google::protobuf::Value& Out) -> bool {
		const int64 Val = V.GetValue<int64>();
		const EProtobufInt64Strategy CurrentStrategy = GlobalInt64Strategy.Load(EMemoryOrder::SequentiallyConsistent);
		
		switch (CurrentStrategy)
		{
		case EProtobufInt64Strategy::AlwaysString:
			{
				std::string StrVal;
				FProtobufStringUtils::FStringToStdString(LexToString(Val), StrVal);
				Out.set_string_value(MoveTemp(StrVal));
				return true;
			}
		
		case EProtobufInt64Strategy::AlwaysNumber:
			{
				const int64 MaxSafe = 9007199254740991LL; 
				const int64 MinSafe = -9007199254740991LL;
				if (Val > MaxSafe || Val < MinSafe)
				{
					static bool bLoggedWarning = false;
					if (!bLoggedWarning)
					{
						UE_LOG(LogProtoBridgeCore, Warning, TEXT("Precision loss detected for int64. Strategy is AlwaysNumber. Value: %lld"), Val);
						bLoggedWarning = true;
					}
				}
				Out.set_number_value(static_cast<double>(Val));
				return true;
			}

		case EProtobufInt64Strategy::ErrorOnPrecisionLoss:
			{
				const int64 MaxSafe = 9007199254740991LL; 
				const int64 MinSafe = -9007199254740991LL;
				if (Val > MaxSafe || Val < MinSafe)
				{
					UE_LOG(LogProtoBridgeCore, Error, TEXT("Precision loss detected for int64. Strategy is ErrorOnPrecisionLoss. Value: %lld"), Val);
					return false;
				}
				Out.set_number_value(static_cast<double>(Val));
				return true;
			}
		}
		
		return false;
	};
	
	VariantEncoders.Add(EVariantTypes::Int8, IntEncoder);
	VariantEncoders.Add(EVariantTypes::Int16, IntEncoder);
	VariantEncoders.Add(EVariantTypes::Int32, IntEncoder);
	VariantEncoders.Add(EVariantTypes::Int64, IntEncoder);
	VariantEncoders.Add(EVariantTypes::UInt8, IntEncoder);
	VariantEncoders.Add(EVariantTypes::UInt16, IntEncoder);
	VariantEncoders.Add(EVariantTypes::UInt32, IntEncoder);
	VariantEncoders.Add(EVariantTypes::UInt64, IntEncoder);

	auto FloatEncoder = [](const FVariant& V, google::protobuf::Value& Out) -> bool {
		Out.set_number_value(V.GetValue<double>());
		return true;
	};
	VariantEncoders.Add(EVariantTypes::Float, FloatEncoder);
	VariantEncoders.Add(EVariantTypes::Double, FloatEncoder);

	VariantEncoders.Add(EVariantTypes::String, [](const FVariant& V, google::protobuf::Value& Out) -> bool {
		FProtobufStringUtils::FStringToStdString(V.GetValue<FString>(), *Out.mutable_string_value());
		return true;
	});

	VariantEncoders.Add(EVariantTypes::Name, [](const FVariant& V, google::protobuf::Value& Out) -> bool {
		FProtobufStringUtils::FNameToStdString(V.GetValue<FName>(), *Out.mutable_string_value());
		return true;
	});

	bEncodersInitialized.Store(true, EMemoryOrder::SequentiallyConsistent);
}

void FProtobufReflectionUtils::Shutdown()
{
	FScopeLock InitScopeLock(&GetInitializationInfoLock());
	FRWScopeLock WriteLock(GetVariantEncodersLock(), SLT_Write);
	VariantEncoders.Empty();
	bEncodersInitialized.Store(false, EMemoryOrder::SequentiallyConsistent);
}

void FProtobufReflectionUtils::SetInt64SerializationStrategy(EProtobufInt64Strategy InStrategy)
{
	GlobalInt64Strategy.Store(InStrategy, EMemoryOrder::SequentiallyConsistent);
}

EProtobufInt64Strategy FProtobufReflectionUtils::GetInt64SerializationStrategy()
{
	return GlobalInt64Strategy.Load(EMemoryOrder::SequentiallyConsistent);
}

void FProtobufReflectionUtils::RegisterVariantEncoder(EVariantTypes Type, FVariantEncoder Encoder)
{
	InitializeDefaultEncoders();
	FRWScopeLock WriteLock(GetVariantEncodersLock(), SLT_Write);
	VariantEncoders.Add(Type, Encoder);
}

bool FProtobufReflectionUtils::FVariantToProtoValue(const FVariant& InVariant, google::protobuf::Value& OutValue)
{
	InitializeDefaultEncoders();

	FVariantEncoder EncoderCopy;
	{
		FRWScopeLock ReadLock(GetVariantEncodersLock(), SLT_ReadOnly);
		if (const FVariantEncoder* Found = VariantEncoders.Find(InVariant.GetType()))
		{
			EncoderCopy = *Found;
		}
	}

	if (EncoderCopy)
	{
		return EncoderCopy(InVariant, OutValue);
	}

	UE_LOG(LogProtoBridgeCore, Error, TEXT("FVariantToProtoValue: Unsupported variant type: %d"), (int32)InVariant.GetType());
	return false;
}

FVariant FProtobufReflectionUtils::ProtoValueToFVariant(const google::protobuf::Value& InValue)
{
	switch (InValue.kind_case()) {
	case google::protobuf::Value::kNullValue: return FVariant();
	case google::protobuf::Value::kNumberValue: return FVariant(InValue.number_value());
	case google::protobuf::Value::kStringValue: return FVariant(FProtobufStringUtils::StdStringToFString(InValue.string_value()));
	case google::protobuf::Value::kBoolValue: return FVariant(InValue.bool_value());
	default: return FVariant();
	}
}

void FProtobufReflectionUtils::AnyToProto(const FProtobufAny& InAny, google::protobuf::Any& OutAny)
{
	FProtobufStringUtils::FStringToStdString(InAny.TypeUrl, *OutAny.mutable_type_url());
	OutAny.set_value(reinterpret_cast<const char*>(InAny.Value.GetData()), InAny.Value.Num());
}

FProtobufAny FProtobufReflectionUtils::ProtoToAny(const google::protobuf::Any& InAny)
{
	FProtobufAny Result;
	FProtobufStringUtils::StdStringToFString(InAny.type_url(), Result.TypeUrl);
	const std::string& Val = InAny.value();
	if (!Val.empty()) {
		Result.Value.SetNumUninitialized(static_cast<int32>(Val.size()));
		FMemory::Memcpy(Result.Value.GetData(), Val.data(), Val.size());
	}
	return Result;
}