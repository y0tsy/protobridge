#include "GrpcServerThread.h"
#include "GrpcServerJob.h"
#include "ProtobufStringUtils.h"
#include "HAL/RunnableThread.h"

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
	if (Thread) return true;

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
	
	int BoundPort = 0;
	Builder.AddListeningPort(StdAddr, Creds, &BoundPort);

	GenericService = std::make_unique<grpc::AsyncGenericService>();
	Builder.RegisterAsyncGenericService(GenericService.get());
	
	CompletionQueue = Builder.AddCompletionQueue();
	Server = Builder.BuildAndStart();

	if (!Server || BoundPort == 0)
	{
		GenericService.reset();
		if (Server) Server->Shutdown();
		Server.reset();
		return false;
	}

	bIsRunning = true;

	Thread = FRunnableThread::Create(this, *ThreadName, 0, TPri_Normal);

	return true;
}

void FGrpcServerThread::Stop()
{
	if (Server)
	{
		Server->Shutdown();
		Server.reset();
	}
	
	if (GenericService)
	{
		GenericService.reset();
	}

	FGrpcWorkerThread::Stop();
}

void FGrpcServerThread::StartGenericLoop(FServiceFinder&& Finder)
{
	if (bIsRunning && CompletionQueue && GenericService)
	{
		FGrpcServerJob* Job = new FGrpcServerJob(GenericService.get(), CompletionQueue.get(), MoveTemp(Finder));
		Job->Start();
	}
}