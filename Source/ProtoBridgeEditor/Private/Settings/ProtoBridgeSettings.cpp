#include "Settings/ProtoBridgeSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "Services/ProtoBridgeUtils.h"
#include "ProtoBridgeDefs.h"

UProtoBridgeSettings::UProtoBridgeSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("ProtoBridge");
	TimeoutSeconds = 60.0;
	MaxConcurrentProcesses = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
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
			auto ValidatePath = [](const FDirectoryPath& DirEntry, const TCHAR* Label)
			{
				if (!DirEntry.Path.IsEmpty() && !DirEntry.Path.Contains(TEXT("{")))
				{
					if (!IFileManager::Get().DirectoryExists(*DirEntry.Path))
					{
						UE_LOG(LogProtoBridge, Warning, TEXT("%s path does not exist: %s"), Label, *DirEntry.Path);
					}
				}
			};

			for (const FProtoBridgeMapping& Mapping : Mappings)
			{
				ValidatePath(Mapping.SourcePath, TEXT("Source"));
				ValidatePath(Mapping.DestinationPath, TEXT("Destination"));
			}
		}
	}
}
#endif