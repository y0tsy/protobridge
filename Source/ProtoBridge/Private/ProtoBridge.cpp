#include "ProtoBridge.h"
#include "GrpcIncludes.h"
#include "google/protobuf/stubs/common.h"
#include <grpc/grpc.h>

DEFINE_LOG_CATEGORY(LogProtoBridge);

void FProtoBridgeModule::StartupModule()
{
	grpc_init();
}

void FProtoBridgeModule::ShutdownModule()
{
	grpc_shutdown();
	google::protobuf::ShutdownProtobufLibrary();
}

IMPLEMENT_MODULE(FProtoBridgeModule, ProtoBridge)