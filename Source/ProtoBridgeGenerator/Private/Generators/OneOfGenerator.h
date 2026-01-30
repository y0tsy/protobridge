#pragma once
#include <string>

class FGeneratorContext;
class FStrategyPool;
namespace google {
    namespace protobuf {
        class Descriptor;
        class OneofDescriptor;
    }
}

class FOneOfGenerator
{
public:
    static void GenerateEnums(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& StructName);
    static void GenerateProperties(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& StructName);
    static void GenerateToProto(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& UeType, const FStrategyPool& Pool);
    static void GenerateFromProto(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& UeType, const std::string& ProtoType, const FStrategyPool& Pool);

private:
    static std::string GetOneOfEnumName(FGeneratorContext& Ctx, const google::protobuf::OneofDescriptor* Oneof, const std::string& StructName);
};