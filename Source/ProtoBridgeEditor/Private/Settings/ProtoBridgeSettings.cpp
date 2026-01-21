#include "Settings/ProtoBridgeSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Services/ProtoBridgeUtils.h"

UProtoBridgeSettings::UProtoBridgeSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("ProtoBridge");
	TimeoutSeconds = 60.0;
}

#if WITH_EDITOR
void UProtoBridgeSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		const FName PropName = PropertyChangedEvent.Property->GetFName();
		
		if (PropName == GET_MEMBER_NAME_CHECKED(FProtoBridgeMapping, SourcePath) ||
			PropName == GET_MEMBER_NAME_CHECKED(FProtoBridgeMapping, DestinationPath))
		{
			for (const FProtoBridgeMapping& Mapping : Mappings)
			{
				if (!Mapping.SourcePath.Path.IsEmpty() && !Mapping.SourcePath.Path.Contains(TEXT("{"))) 
				{
					if (!IFileManager::Get().DirectoryExists(*Mapping.SourcePath.Path))
					{
						UE_LOG(LogTemp, Warning, TEXT("[ProtoBridge] Source path does not exist: %s"), *Mapping.SourcePath.Path);
					}
				}
				if (!Mapping.DestinationPath.Path.IsEmpty() && !Mapping.DestinationPath.Path.Contains(TEXT("{")))
				{
					if (!IFileManager::Get().DirectoryExists(*Mapping.DestinationPath.Path))
					{
						UE_LOG(LogTemp, Warning, TEXT("[ProtoBridge] Destination path does not exist: %s"), *Mapping.DestinationPath.Path);
					}
				}
			}
		}
	}
}
#endif