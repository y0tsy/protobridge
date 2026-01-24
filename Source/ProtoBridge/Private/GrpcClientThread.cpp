#include "GrpcClientThread.h"
#include "ProtobufStringUtils.h"

FGrpcClientThread::FGrpcClientThread(const FString& InAddress, const FString& InRootCert, const FString& InClientCert, const FString& InPrivateKey)
	: FGrpcWorkerThread(FString::Printf(TEXT("GrpcClient_%s"), *InAddress))
	, Address(InAddress)
	, RootCert(InRootCert)
	, ClientCert(InClientCert)
	, PrivateKey(InPrivateKey)
{
}

FGrpcClientThread::~FGrpcClientThread()
{
}

bool FGrpcClientThread::Init()
{
	if (!FGrpcWorkerThread::Init())
	{
		return false;
	}

	grpc::ChannelArguments Args;
	Args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
	Args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
	Args.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 1);
	Args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, -1);
	Args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, -1);

	std::shared_ptr<grpc::ChannelCredentials> Creds;

	if (!RootCert.IsEmpty())
	{
		grpc::SslCredentialsOptions SslOpts;
		FProtobufStringUtils::FStringToStdString(RootCert, SslOpts.pem_root_certs);
		
		if (!ClientCert.IsEmpty() && !PrivateKey.IsEmpty())
		{
			FProtobufStringUtils::FStringToStdString(ClientCert, SslOpts.pem_cert_chain);
			FProtobufStringUtils::FStringToStdString(PrivateKey, SslOpts.pem_private_key);
		}

		Creds = grpc::SslCredentials(SslOpts);
	}
	else
	{
		Creds = grpc::InsecureChannelCredentials();
	}

	std::string StdAddress;
	FProtobufStringUtils::FStringToStdString(Address, StdAddress);

	Channel = grpc::CreateCustomChannel(StdAddress, Creds, Args);

	return true;
}

std::shared_ptr<grpc::Channel> FGrpcClientThread::GetChannel() const
{
	return Channel;
}