#pragma once

#include "CoreMinimal.h"
#include "GrpcIncludes.h"

class FGrpcByteBufferUtils
{
public:
	static void ByteBufferToTArray(const grpc::ByteBuffer& Buffer, TArray<uint8>& OutData)
	{
		OutData.Reset();
		size_t Size = Buffer.Length();
		if (Size == 0) return;

		OutData.SetNumUninitialized(Size);
		
		std::vector<grpc::Slice> Slices;
		if (Buffer.Dump(&Slices).ok())
		{
			size_t Offset = 0;
			for (const auto& Slice : Slices)
			{
				FMemory::Memcpy(OutData.GetData() + Offset, Slice.begin(), Slice.size());
				Offset += Slice.size();
			}
		}
	}

	static grpc::ByteBuffer TArrayToByteBuffer(const TArray<uint8>& InData)
	{
		if (InData.Num() == 0)
		{
			return grpc::ByteBuffer();
		}
		grpc::Slice Slice(InData.GetData(), InData.Num());
		return grpc::ByteBuffer(&Slice, 1);
	}
};