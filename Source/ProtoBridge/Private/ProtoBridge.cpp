#include "ProtoBridge.h"

#include "grpcpp/grpcpp.h"
#include "google/protobuf/stubs/common.h"

DEFINE_LOG_CATEGORY(LogProtoBridge);

void FProtoBridgeModule::StartupModule()
{
	PerformGRPCHealthCheck();
}

void FProtoBridgeModule::ShutdownModule()
{
	google::protobuf::ShutdownProtobufLibrary();
}

void FProtoBridgeModule::PerformGRPCHealthCheck()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	FString ProtobufVersion = FString(google::protobuf::internal::VersionString(GOOGLE_PROTOBUF_VERSION).c_str());
	FString GRPCVersion = FString(grpc::Version().c_str());

	UE_LOG(LogProtoBridge, Log, TEXT("ProtoBridge: Protobuf version verified: %s"), *ProtobufVersion);
	UE_LOG(LogProtoBridge, Log, TEXT("ProtoBridge: gRPC version: %s"), *GRPCVersion);

	try
	{
		auto Credentials = grpc::InsecureChannelCredentials();
		auto Channel = grpc::CreateChannel("localhost:0", Credentials);
		
		if (Channel)
		{
			UE_LOG(LogProtoBridge, Log, TEXT("ProtoBridge: gRPC runtime sanity check passed."));
		}
	}
	catch (...)
	{
		UE_LOG(LogProtoBridge, Error, TEXT("ProtoBridge: gRPC runtime sanity check failed with exception."));
	}
}

IMPLEMENT_MODULE(FProtoBridgeModule, ProtoBridge)