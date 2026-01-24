#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

PROTOBRIDGECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogProtoBridgeCore, Log, All);

class FProtoBridgeCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};