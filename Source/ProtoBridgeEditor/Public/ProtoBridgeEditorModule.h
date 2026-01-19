#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IProtoBridgeService;

class FProtoBridgeEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<IProtoBridgeService> GetService() const;

private:
	void RegisterMenus();
	void OnCompileButtonClicked();
	
	void HandleCompilationStarted();
	void HandleCompilationFinished(bool bSuccess, const FString& Message);
	void HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity);

	TSharedPtr<IProtoBridgeService> CompilerService;
	TSharedPtr<class FUICommandList> PluginCommands;
};