#include "ProtobufMemoryBridge.h"

LLM_DEFINE_TAG(Protobuf);

namespace
{
	void* ProtobufBlockAlloc(size_t Size)
	{
		LLM_SCOPE_BYTAG(Protobuf);
		return FMemory::Malloc(Size, 16);
	}

	void ProtobufBlockDealloc(void* Ptr, size_t Size)
	{
		FMemory::Free(Ptr);
	}
}

google::protobuf::ArenaOptions FProtobufMemoryBridge::GetArenaOptions()
{
	google::protobuf::ArenaOptions Options;
	Options.block_alloc = &ProtobufBlockAlloc;
	Options.block_dealloc = &ProtobufBlockDealloc;
	return Options;
}