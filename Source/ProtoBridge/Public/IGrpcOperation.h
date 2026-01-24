#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGE_API IGrpcOperation
{
public:
	virtual ~IGrpcOperation() = default;

	virtual void OnEvent(bool bSuccess, const void* Tag) = 0;
	
	void* GetTag()
	{
		return static_cast<void*>(this);
	}
};