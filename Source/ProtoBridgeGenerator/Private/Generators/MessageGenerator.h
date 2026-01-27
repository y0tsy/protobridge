#pragma once

class FGeneratorContext;
namespace google {
    namespace protobuf {
        class Descriptor;
    }
}

class FMessageGenerator
{
public:
    static void GenerateHeader(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message);
    static void GenerateSource(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message);
};