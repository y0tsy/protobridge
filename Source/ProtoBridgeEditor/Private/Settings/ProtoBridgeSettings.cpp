#include "Settings/ProtoBridgeSettings.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

UProtoBridgeSettings::UProtoBridgeSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("ProtoBridge");
}

#if WITH_EDITOR
void UProtoBridgeSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UProtoBridgeSettings, CustomProtocPath))
		{
			if (!CustomProtocPath.FilePath.IsEmpty() && !FPaths::FileExists(CustomProtocPath.FilePath))
			{
				CustomProtocPath.FilePath.Empty();
			}
		}

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UProtoBridgeSettings, CustomPluginPath))
		{
			if (!CustomPluginPath.FilePath.IsEmpty() && !FPaths::FileExists(CustomPluginPath.FilePath))
			{
				CustomPluginPath.FilePath.Empty();
			}
		}
	}
}
#endif