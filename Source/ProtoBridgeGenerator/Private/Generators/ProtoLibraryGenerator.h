#pragma once

#include <string>
#include <vector>

class FGeneratorContext;
namespace google {
    namespace protobuf {
        class Descriptor;
    }
}

class FProtoLibraryGenerator
{
public:
    static void GenerateHeader(FGeneratorContext& Ctx, const std::string& BaseName, const std::vector<const google::protobuf::Descriptor*>& Messages);
    static void GenerateSource(FGeneratorContext& Ctx, const std::string& BaseName, const std::vector<const google::protobuf::Descriptor*>& Messages);
};