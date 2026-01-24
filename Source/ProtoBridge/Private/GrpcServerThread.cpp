#include "GrpcServerThread.h"
#include "GrpcServerJob.h"
#include "ProtobufStringUtils.h"

FGrpcServerThread::FGrpcServerThread(const FString& InAddress, const FString& InServerCert, const FString& InPrivateKey)
	: FGrpcWorkerThread(TEXT("GrpcServerThread"))
	, Address(InAddress)
	, ServerCert(InServerCert)
	, PrivateKey(InPrivateKey)
{
}

FGrpcServerThread::~FGrpcServerThread()
{
	Stop();
}

bool FGrpcServerThread::Init()
{
	grpc::ServerBuilder Builder;

	std::shared_ptr<grpc::ServerCredentials> Creds;
	if (!ServerCert.IsEmpty() && !PrivateKey.IsEmpty())
	{
		grpc::SslServerCredentialsOptions::PemKeyCertPair KeyCert;
		FProtobufStringUtils::FStringToStdString(PrivateKey, KeyCert.private_key);
		FProtobufStringUtils::FStringToStdString(ServerCert, KeyCert.cert_chain);

		grpc::SslServerCredentialsOptions SslOpts;
		SslOpts.pem_key_cert_pairs.push_back(KeyCert);

		Creds = grpc::SslServerCredentials(SslOpts);
	}
	else
	{
		Creds = grpc::InsecureServerCredentials();
	}

	std::string StdAddr;
	FProtobufStringUtils::FStringToStdString(Address, StdAddr);
	Builder.AddListeningPort(StdAddr, Creds);

	Builder.RegisterAsyncGenericService(&GenericService);
	
	CompletionQueue = Builder.AddCompletionQueue();
	Server = Builder.BuildAndStart();

	if (!Server)
	{
		return false;
	}

	bIsRunning = true;
	return true;
}

void FGrpcServerThread::Stop()
{
	if (Server)
	{
		Server->Shutdown();
	}
	FGrpcWorkerThread::Stop();
}

void FGrpcServerThread::StartGenericLoop(FServiceFinder&& Finder)
{
	if (bIsRunning && CompletionQueue)
	{
		FGrpcServerJob* Job = new FGrpcServerJob(&GenericService, CompletionQueue.get(), MoveTemp(Finder));
		Job->Start();
	}
}