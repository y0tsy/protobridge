#include "ProtobufMathUtils.h"

void FProtobufMathUtils::FDateTimeToTimestamp(const FDateTime& InDateTime, google::protobuf::Timestamp& OutTimestamp)
{
	const int64 AllTicks = InDateTime.GetTicks();
	const int64 TicksPerSec = ETimespan::TicksPerSecond;
	OutTimestamp.set_seconds(AllTicks / TicksPerSec);
	OutTimestamp.set_nanos(static_cast<int32>((AllTicks % TicksPerSec) * 100));
}

FDateTime FProtobufMathUtils::TimestampToFDateTime(const google::protobuf::Timestamp& InTimestamp)
{
	return FDateTime(InTimestamp.seconds() * ETimespan::TicksPerSecond + InTimestamp.nanos() / 100);
}

void FProtobufMathUtils::FTimespanToDuration(const FTimespan& InTimespan, google::protobuf::Duration& OutDuration)
{
	const int64 TotalTicks = InTimespan.GetTicks();
	const int64 TicksPerSec = ETimespan::TicksPerSecond;
	OutDuration.set_seconds(TotalTicks / TicksPerSec);
	OutDuration.set_nanos(static_cast<int32>((TotalTicks % TicksPerSec) * 100));
}

FTimespan FProtobufMathUtils::DurationToFTimespan(const google::protobuf::Duration& InDuration)
{
	return FTimespan(InDuration.seconds() * ETimespan::TicksPerSecond + InDuration.nanos() / 100);
}

void FProtobufMathUtils::FMatrixToRepeatedField(const FMatrix& InMatrix, google::protobuf::RepeatedField<double>* OutField)
{
	if (!OutField) return;
	OutField->Clear();
	OutField->Resize(16, 0.0);
	double* Data = OutField->mutable_data();
	
	Data[0] = InMatrix.M[0][0]; Data[1] = InMatrix.M[0][1]; Data[2] = InMatrix.M[0][2]; Data[3] = InMatrix.M[0][3];
	Data[4] = InMatrix.M[1][0]; Data[5] = InMatrix.M[1][1]; Data[6] = InMatrix.M[1][2]; Data[7] = InMatrix.M[1][3];
	Data[8] = InMatrix.M[2][0]; Data[9] = InMatrix.M[2][1]; Data[10] = InMatrix.M[2][2]; Data[11] = InMatrix.M[2][3];
	Data[12] = InMatrix.M[3][0]; Data[13] = InMatrix.M[3][1]; Data[14] = InMatrix.M[3][2]; Data[15] = InMatrix.M[3][3];
}

FMatrix FProtobufMathUtils::RepeatedFieldToFMatrix(const google::protobuf::RepeatedField<double>& InField)
{
	FMatrix Result = FMatrix::Identity;
	if (InField.size() >= 16)
	{
		const double* Data = InField.data();
		Result.M[0][0] = Data[0]; Result.M[0][1] = Data[1]; Result.M[0][2] = Data[2]; Result.M[0][3] = Data[3];
		Result.M[1][0] = Data[4]; Result.M[1][1] = Data[5]; Result.M[1][2] = Data[6]; Result.M[1][3] = Data[7];
		Result.M[2][0] = Data[8]; Result.M[2][1] = Data[9]; Result.M[2][2] = Data[10]; Result.M[2][3] = Data[11];
		Result.M[3][0] = Data[12]; Result.M[3][1] = Data[13]; Result.M[3][2] = Data[14]; Result.M[3][3] = Data[15];
	}
	return Result;
}