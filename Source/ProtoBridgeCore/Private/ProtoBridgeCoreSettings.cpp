#include "ProtoBridgeCoreSettings.h"

UProtoBridgeCoreSettings::UProtoBridgeCoreSettings()
{
	Int64SerializationStrategy = EProtobufInt64Strategy::AlwaysString;
	bBestEffortJsonParsing = false;
	MaxAnyPayloadSize = 32 * 1024 * 1024;
	MaxByteArraySize = 64 * 1024 * 1024;
	MaxJsonRecursionDepth = 75;
}

void UProtoBridgeCoreSettings::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (MaxAnyPayloadSize < 1024) MaxAnyPayloadSize = 1024;
	if (MaxByteArraySize < 1024) MaxByteArraySize = 1024;
	if (MaxJsonRecursionDepth < 1) MaxJsonRecursionDepth = 1;
	if (MaxJsonRecursionDepth > 1000) MaxJsonRecursionDepth = 1000;
}

FName UProtoBridgeCoreSettings::GetContainerName() const
{
	return FName("Project");
}

FName UProtoBridgeCoreSettings::GetCategoryName() const
{
	return FName("Game");
}

FName UProtoBridgeCoreSettings::GetSectionName() const
{
	return FName("ProtoBridge Core");
}