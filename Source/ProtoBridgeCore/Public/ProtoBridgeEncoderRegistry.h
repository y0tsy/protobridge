#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

namespace ProtoBridge::EncoderRegistry
{
	void PROTOBRIDGECORE_API GetDefaultEncoders(TMap<EVariantTypes, FVariantEncoder>& OutEncoders);
}