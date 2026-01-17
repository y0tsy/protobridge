#pragma once

#include <memory>
#include "FieldStrategy.h"
#include "PrimitiveFieldStrategy.h"
#include "StringFieldStrategy.h"
#include "EnumFieldStrategy.h"
#include "MessageFieldStrategy.h"
#include "MapFieldStrategy.h"
#include "UnrealStrategies.h"
#include "../TypeRegistry.h"

class FFieldStrategyFactory
{
public:
    static std::unique_ptr<IFieldStrategy> Create(const FieldDescriptor* Field)
    {
        if (Field->is_map())
        {
            return std::make_unique<FMapFieldStrategy>(Field);
        }

        if (Field->type() == FieldDescriptor::TYPE_MESSAGE)
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

        if (Field->type() == FieldDescriptor::TYPE_STRING || Field->type() == FieldDescriptor::TYPE_BYTES)
        {
            return std::make_unique<FStringFieldStrategy>(Field);
        }

        if (Field->type() == FieldDescriptor::TYPE_ENUM)
        {
            return std::make_unique<FEnumFieldStrategy>(Field);
        }

        return std::make_unique<FPrimitiveFieldStrategy>(Field);
    }
};