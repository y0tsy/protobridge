#pragma once

namespace google {
	namespace protobuf {
		class FieldDescriptor;
	}
}

class IFieldStrategy;
class FStrategyPool;

class FFieldStrategyFactory
{
public:
	static const IFieldStrategy* GetStrategy(const google::protobuf::FieldDescriptor* Field, const FStrategyPool& Pool);
};