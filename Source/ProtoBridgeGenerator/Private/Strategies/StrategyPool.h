#pragma once

#include <memory>
#include "PrimitiveFieldStrategy.h"
#include "StringFieldStrategy.h"
#include "EnumFieldStrategy.h"
#include "MessageFieldStrategy.h"
#include "MapFieldStrategy.h"
#include "UnrealStrategies.h"

class FStrategyPool
{
public:
	FStrategyPool();

	const FPrimitiveFieldStrategy* GetPrimitiveStrategy() const { return Primitive.get(); }
	const FStringFieldStrategy* GetStringStrategy() const { return String.get(); }
	const FEnumFieldStrategy* GetEnumStrategy() const { return Enum.get(); }
	const FMessageFieldStrategy* GetMessageStrategy() const { return Message.get(); }
	const FMapFieldStrategy* GetMapStrategy() const { return Map.get(); }
	const FUnrealStructStrategy* GetUnrealStructStrategy() const { return UnrealStruct.get(); }
	const FUnrealJsonStrategy* GetUnrealJsonStrategy() const { return UnrealJson.get(); }

private:
	std::unique_ptr<FPrimitiveFieldStrategy> Primitive;
	std::unique_ptr<FStringFieldStrategy> String;
	std::unique_ptr<FEnumFieldStrategy> Enum;
	std::unique_ptr<FMessageFieldStrategy> Message;
	std::unique_ptr<FMapFieldStrategy> Map;
	std::unique_ptr<FUnrealStructStrategy> UnrealStruct;
	std::unique_ptr<FUnrealJsonStrategy> UnrealJson;
};