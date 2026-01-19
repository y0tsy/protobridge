#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class PROTOBRIDGEEDITOR_API FProtoBridgeEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static TSharedPtr<ISlateStyle> Get();
	static FName GetStyleSetName();

private:
	static TSharedPtr<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};