#include "GrpcWorkerThread.h"
#include "HAL/RunnableThread.h"

FGrpcWorkerThread::FGrpcWorkerThread(const FString& InThreadName)
	: ThreadName(InThreadName)
	, Thread(nullptr)
	, bIsRunning(false)
{
	Thread = FRunnableThread::Create(this, *ThreadName, 0, TPri_Normal);
}

FGrpcWorkerThread::~FGrpcWorkerThread()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}
}

bool FGrpcWorkerThread::Init()
{
	bIsRunning = true;
	CompletionQueue = std::make_unique<grpc::CompletionQueue>();
	return true;
}

uint32 FGrpcWorkerThread::Run()
{
	void* Tag;
	bool bOk;

	while (bIsRunning && CompletionQueue->Next(&Tag, &bOk))
	{
		if (Tag)
		{
			IGrpcOperation* Operation = static_cast<IGrpcOperation*>(Tag);
			Operation->OnEvent(bOk, Tag);
		}
	}

	return 0;
}

void FGrpcWorkerThread::Stop()
{
	bIsRunning = false;
	ShutdownQueue();
}

void FGrpcWorkerThread::Exit()
{
}

void FGrpcWorkerThread::WaitForCompletion()
{
	if (Thread)
	{
		Thread->WaitForCompletion();
	}
}

grpc::CompletionQueue* FGrpcWorkerThread::GetCompletionQueue()
{
	return CompletionQueue.get();
}

void FGrpcWorkerThread::ShutdownQueue()
{
	if (CompletionQueue)
	{
		CompletionQueue->Shutdown();
	}
}