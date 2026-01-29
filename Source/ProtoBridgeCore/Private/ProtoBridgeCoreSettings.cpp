#include "ProtoBridgeCoreSettings.h"

UProtoBridgeCoreSettings::UProtoBridgeCoreSettings()
{
	Int64SerializationStrategy = EProtobufInt64Strategy::AlwaysString;
	MaxAnyPayloadSize = 32 * 1024 * 1024;
	MaxByteArraySize = 64 * 1024 * 1024;
	MaxJsonRecursionDepth = 75;
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