#include "FieldStrategyFactory.h"
#include "StrategyPool.h"
#include "UnrealStrategies.h"
#include "../TypeRegistry.h"
#include "../Config/UEDefinitions.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

const IFieldStrategy* FFieldStrategyFactory::GetStrategy(const google::protobuf::FieldDescriptor* Field, const FStrategyPool& Pool)
{
	if (Field->is_map())
	{
		return Pool.GetMapStrategy();
	}

	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		std::string FullName = std::string(Field->message_type()->full_name());
		
		if (const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(FullName))
		{
			if (Info->UtilityClass == UE::Names::Utils::Struct)
			{
				return Pool.GetUnrealJsonStrategy();
			}
			return Pool.GetUnrealStructStrategy();
		}

		return Pool.GetMessageStrategy();
	}

	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_STRING || Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		return Pool.GetStringStrategy();
	}

	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
	{
		return Pool.GetEnumStrategy();
	}

	return Pool.GetPrimitiveStrategy();
}