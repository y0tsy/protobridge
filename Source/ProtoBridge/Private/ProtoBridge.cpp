#include "ProtoBridge.h"
#include "google/protobuf/stubs/common.h"

DEFINE_LOG_CATEGORY(LogProtoBridge);

void FProtoBridgeModule::StartupModule()
{
}

void FProtoBridgeModule::ShutdownModule()
{
	google::protobuf::ShutdownProtobufLibrary();
}

IMPLEMENT_MODULE(FProtoBridgeModule, ProtoBridge)