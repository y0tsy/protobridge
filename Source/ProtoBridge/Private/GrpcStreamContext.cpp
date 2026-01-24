#include "GrpcStreamContext.h"

void UGrpcStreamContext::Write(const TArray<uint8>& Data)
{
	if (!bIsCancelled)
	{
		OutgoingQueue.Enqueue(FGrpcStreamPacket(Data));
	}
}

void UGrpcStreamContext::Complete()
{
	if (!bIsCancelled)
	{
		OutgoingQueue.Enqueue(FGrpcStreamPacket::MakeDone());
	}
}

void UGrpcStreamContext::Cancel()
{
	bIsCancelled = true;
}

bool UGrpcStreamContext::DequeuePacket(FGrpcStreamPacket& OutPacket)
{
	return OutgoingQueue.Dequeue(OutPacket);
}

bool UGrpcStreamContext::IsCancelled() const
{
	return bIsCancelled;
}