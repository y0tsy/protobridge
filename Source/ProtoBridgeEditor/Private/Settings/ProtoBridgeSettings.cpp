#include "Settings/ProtoBridgeSettings.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"

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
		
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UProtoBridgeSettings, ApiMacroName))
		{
			if (!ApiMacroName.IsEmpty())
			{
				FRegexPattern ValidMacroPattern(TEXT("^[a-zA-Z0-9_]+$"));
				FRegexMatcher Matcher(ValidMacroPattern, ApiMacroName);
				if (!Matcher.FindNext())
				{
					ApiMacroName.Empty();
				}
			}
		}
	}
}
#endif