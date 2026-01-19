#include "ProtoBridgeEditorStyle.h"
#include "ProtoBridgeDefs.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Slate/SlateGameResources.h"

TSharedPtr<FSlateStyleSet> FProtoBridgeEditorStyle::StyleInstance = nullptr;

void FProtoBridgeEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FProtoBridgeEditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

void FProtoBridgeEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

FName FProtoBridgeEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ProtoBridgeEditorStyle"));
	return StyleSetName;
}

TSharedPtr<ISlateStyle> FProtoBridgeEditorStyle::Get()
{
	return StyleInstance;
}

TSharedPtr<FSlateStyleSet> FProtoBridgeEditorStyle::Create()
{
	TSharedPtr<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName);
	if (Plugin.IsValid())
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	return Style;
}