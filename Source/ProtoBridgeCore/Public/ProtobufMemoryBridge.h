#pragma once

#include "CoreMinimal.h"
#include "ProtobufIncludes.h"

class PROTOBRIDGECORE_API FProtobufMemoryBridge
{
public:
	static google::protobuf::ArenaOptions GetArenaOptions();
};