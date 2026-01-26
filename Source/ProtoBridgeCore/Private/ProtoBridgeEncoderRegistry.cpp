#include "ProtoBridgeEncoderRegistry.h"
#include "ProtoBridgeSubsystem.h"
#include "ProtobufStringUtils.h"
#include "ProtobufIncludes.h"
#include "ProtobufReflectionUtils.h"
#include "ProtoBridgeCoreModule.h"

namespace ProtoBridge::EncoderRegistry
{
	namespace
	{
		bool EncodeIntegerVariant(const FVariant& V, google::protobuf::Value& Out, const FProtoSerializationContext& Context)
		{
			const int64 Val = V.GetValue<int64>();
			return FProtobufReflectionUtils::ConvertInt64ToProtoValue(Val, Out, Context.Int64Strategy);
		}

		void RegisterNumericEncoders(TMap<EVariantTypes, FVariantEncoder>& OutEncoders)
		{
			const EVariantTypes IntTypes[] = {
				EVariantTypes::Int8, EVariantTypes::Int16, EVariantTypes::Int32, EVariantTypes::Int64,
				EVariantTypes::UInt8, EVariantTypes::UInt16, EVariantTypes::UInt32, EVariantTypes::UInt64
			};

			for (const EVariantTypes Type : IntTypes)
			{
				OutEncoders.Add(Type, &EncodeIntegerVariant);
			}

			auto FloatEncoder = [](const FVariant& V, google::protobuf::Value& Out, const FProtoSerializationContext&) -> bool {
				Out.set_number_value(V.GetValue<double>());
				return true;
			};
			OutEncoders.Add(EVariantTypes::Float, FloatEncoder);
			OutEncoders.Add(EVariantTypes::Double, FloatEncoder);
		}

		void RegisterPrimitiveEncoders(TMap<EVariantTypes, FVariantEncoder>& OutEncoders)
		{
			auto NullEncoder = [](const FVariant&, google::protobuf::Value& Out, const FProtoSerializationContext&) -> bool { 
				Out.set_null_value(google::protobuf::NULL_VALUE); 
				return true;
			};
			OutEncoders.Add(EVariantTypes::Empty, NullEncoder);

			OutEncoders.Add(EVariantTypes::Bool, [](const FVariant& V, google::protobuf::Value& Out, const FProtoSerializationContext&) -> bool {
				Out.set_bool_value(V.GetValue<bool>());
				return true;
			});
		}
		
		void RegisterStringEncoders(TMap<EVariantTypes, FVariantEncoder>& OutEncoders)
		{
			OutEncoders.Add(EVariantTypes::String, [](const FVariant& V, google::protobuf::Value& Out, const FProtoSerializationContext&) -> bool {
				FProtobufStringUtils::FStringToStdString(V.GetValue<FString>(), *Out.mutable_string_value());
				return true;
			});

			OutEncoders.Add(EVariantTypes::Name, [](const FVariant& V, google::protobuf::Value& Out, const FProtoSerializationContext&) -> bool {
				FProtobufStringUtils::FNameToStdString(V.GetValue<FName>(), *Out.mutable_string_value());
				return true;
			});
		}
	}

	void RegisterDefaultEncoders(UProtoBridgeSubsystem& Subsystem)
	{
		TMap<EVariantTypes, FVariantEncoder> Batch;
		RegisterPrimitiveEncoders(Batch);
		RegisterNumericEncoders(Batch);
		RegisterStringEncoders(Batch);
		Subsystem.RegisterEncodersBatch(Batch);
	}
}