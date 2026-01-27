#pragma once
#include <string>

class FGeneratorContext;
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
    static void GenerateToProto(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& UeType);
    static void GenerateFromProto(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& UeType, const std::string& ProtoType);

private:
    static std::string GetOneOfEnumName(FGeneratorContext& Ctx, const google::protobuf::OneofDescriptor* Oneof, const std::string& StructName);
};