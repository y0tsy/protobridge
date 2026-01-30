#include "StrategyPool.h"

FStrategyPool::FStrategyPool()
{
	Primitive = std::make_unique<FPrimitiveFieldStrategy>();
	String = std::make_unique<FStringFieldStrategy>();
	Enum = std::make_unique<FEnumFieldStrategy>();
	Message = std::make_unique<FMessageFieldStrategy>();
	Map = std::make_unique<FMapFieldStrategy>();
	UnrealStruct = std::make_unique<FUnrealStructStrategy>();
	UnrealJson = std::make_unique<FUnrealJsonStrategy>();
}