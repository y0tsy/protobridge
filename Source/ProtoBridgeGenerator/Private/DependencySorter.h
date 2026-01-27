#pragma once

#include <vector>

namespace google {
    namespace protobuf {
        class Descriptor;
        class FileDescriptor;
    }
}

class FDependencySorter
{
public:
    static std::vector<const google::protobuf::Descriptor*> Sort(const google::protobuf::FileDescriptor* File);
};