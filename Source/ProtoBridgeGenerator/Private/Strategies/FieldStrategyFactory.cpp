#include "FieldStrategyFactory.h"
#include "FieldStrategy.h"
#include "PrimitiveFieldStrategy.h"
#include "StringFieldStrategy.h"
#include "EnumFieldStrategy.h"
#include "MessageFieldStrategy.h"
#include "MapFieldStrategy.h"
#include "UnrealStrategies.h"
#include "../TypeRegistry.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

std::unique_ptr<IFieldStrategy> FFieldStrategyFactory::Create(const google::protobuf::FieldDescriptor* Field)
{
	if (Field->is_map())
	{
		return std::make_unique<FMapFieldStrategy>(Field);
	}

	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		std::string FullName = std::string(Field->message_type()->full_name());
		
		if (const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(FullName))
		{
			if (Info->UtilsFuncPrefix == "Json" || Info->UtilsFuncPrefix == "JsonList")
			{
				return std::make_unique<FUnrealJsonStrategy>(Field);
			}
			return std::make_unique<FUnrealStructStrategy>(Field, Info);
		}

		return std::make_unique<FMessageFieldStrategy>(Field);
	}

	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_STRING || Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		return std::make_unique<FStringFieldStrategy>(Field);
	}

	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
	{
		return std::make_unique<FEnumFieldStrategy>(Field);
	}

	return std::make_unique<FPrimitiveFieldStrategy>(Field);
}