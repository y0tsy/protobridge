#pragma once

#include "CoreMinimal.h"

class UProtoBridgeSubsystem;

namespace ProtoBridge::EncoderRegistry
{
	void PROTOBRIDGECORE_API RegisterDefaultEncoders(UProtoBridgeSubsystem& Subsystem);
}