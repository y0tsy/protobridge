#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGE_API IGrpcRequest
{
public:
	virtual ~IGrpcRequest() = default;
	virtual void OnComplete(bool bSuccess) = 0;
};