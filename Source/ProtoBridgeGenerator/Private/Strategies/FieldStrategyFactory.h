#pragma once

#include <memory>

namespace google {
namespace protobuf {
	class FieldDescriptor;
}
}

class IFieldStrategy;

class FFieldStrategyFactory
{
public:
	static std::unique_ptr<IFieldStrategy> Create(const google::protobuf::FieldDescriptor* Field);
};