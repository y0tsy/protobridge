#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IProtoBridgeService;
class FProtoBridgeUIManager;

class FProtoBridgeEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<IProtoBridgeService> GetService() const;

private:
	void RegisterMenus();
	void OnCompileButtonClicked();
	void CleanupTempFiles();
	
	TSharedPtr<IProtoBridgeService> CompilerService;
	TSharedPtr<FProtoBridgeUIManager> UIManager;
};