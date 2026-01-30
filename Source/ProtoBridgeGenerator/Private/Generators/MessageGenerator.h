#pragma once

class FGeneratorContext;
class FStrategyPool;
namespace google {
    namespace protobuf {
        class Descriptor;
    }
}

class FMessageGenerator
{
public:
    static void GenerateHeader(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const FStrategyPool& Pool);
    static void GenerateSource(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const FStrategyPool& Pool);
};