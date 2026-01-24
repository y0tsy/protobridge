#include "GrpcClientThread.h"
#include "IGrpcRequest.h"
#include "HAL/RunnableThread.h"

FGrpcClientThread::FGrpcClientThread(const FString& InAddress, const FString& InRootCert)
	: Address(InAddress)
	, RootCert(InRootCert)
	, Thread(nullptr)
	, bIsRunning(false)
{
	Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("GrpcThread_%s"), *InAddress), 0, TPri_Normal);
}

FGrpcClientThread::~FGrpcClientThread()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}
}

bool FGrpcClientThread::Init()
{
	bIsRunning = true;
	
	grpc::ChannelArguments Args;
	Args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
	Args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
	Args.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 1);

	std::shared_ptr<grpc::ChannelCredentials> Creds;

	if (!RootCert.IsEmpty())
	{
		grpc::SslCredentialsOptions SslOpts;
		SslOpts.pem_root_certs = std::string(TCHAR_TO_UTF8(*RootCert));
		Creds = grpc::SslCredentials(SslOpts);
	}
	else
	{
		Creds = grpc::InsecureChannelCredentials();
	}

	Channel = grpc::CreateCustomChannel(std::string(TCHAR_TO_UTF8(*Address)), Creds, Args);
	CompletionQueue = std::make_unique<grpc::CompletionQueue>();

	return true;
}

uint32 FGrpcClientThread::Run()
{
	void* Tag;
	bool bOk;

	while (bIsRunning && CompletionQueue->Next(&Tag, &bOk))
	{
		if (Tag)
		{
			IGrpcRequest* Request = static_cast<IGrpcRequest*>(Tag);
			Request->OnComplete(bOk);
		}
	}

	return 0;
}

void FGrpcClientThread::Stop()
{
	bIsRunning = false;
	if (CompletionQueue)
	{
		CompletionQueue->Shutdown();
	}
}

void FGrpcClientThread::Exit()
{
}

std::shared_ptr<grpc::Channel> FGrpcClientThread::GetChannel() const
{
	return Channel;
}

grpc::CompletionQueue* FGrpcClientThread::GetCompletionQueue()
{
	return CompletionQueue.get();
}