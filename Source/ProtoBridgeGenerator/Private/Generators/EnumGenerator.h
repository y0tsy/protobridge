#pragma once

class FGeneratorContext;
namespace google {
    namespace protobuf {
        class EnumDescriptor;
    }
}

class FEnumGenerator
{
public:
    static void Generate(FGeneratorContext& Ctx, const google::protobuf::EnumDescriptor* Enum);

private:
    static void GenerateValues(FGeneratorContext& Ctx, const google::protobuf::EnumDescriptor* Enum, bool bIsBlueprintType);
};