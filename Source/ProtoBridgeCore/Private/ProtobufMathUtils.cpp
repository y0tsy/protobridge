#include "ProtobufMathUtils.h"
#include "ProtobufIncludes.h"
#include "ProtoBridgeCoreModule.h"

bool FProtobufMathUtils::FDateTimeToTimestamp(const FDateTime& InDateTime, google::protobuf::Timestamp& OutTimestamp)
{
	static const FDateTime MinDate(1, 1, 1);
	static const FDateTime MaxDate(9999, 12, 31, 23, 59, 59, 999);

	if (InDateTime < MinDate || InDateTime > MaxDate)
	{
		UE_LOG(LogProtoBridgeCore, Warning, TEXT("FDateTimeToTimestamp: Date out of RFC3339 range (0001-9999). Clamping."));
	}

	FDateTime ClampedDate = InDateTime;
	if (ClampedDate < MinDate) ClampedDate = MinDate;
	if (ClampedDate > MaxDate) ClampedDate = MaxDate;

	const int64 AllTicks = ClampedDate.GetTicks();
	const int64 TicksPerSec = ETimespan::TicksPerSecond;
	OutTimestamp.set_seconds(AllTicks / TicksPerSec);
	OutTimestamp.set_nanos(static_cast<int32>((AllTicks % TicksPerSec) * 100));
	
	return true;
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
	if (!Data) return;
	
	Data[0] = InMatrix.M[0][0]; Data[1] = InMatrix.M[0][1]; Data[2] = InMatrix.M[0][2]; Data[3] = InMatrix.M[0][3];
	Data[4] = InMatrix.M[1][0]; Data[5] = InMatrix.M[1][1]; Data[6] = InMatrix.M[1][2]; Data[7] = InMatrix.M[1][3];
	Data[8] = InMatrix.M[2][0]; Data[9] = InMatrix.M[2][1]; Data[10] = InMatrix.M[2][2]; Data[11] = InMatrix.M[2][3];
	Data[12] = InMatrix.M[3][0]; Data[13] = InMatrix.M[3][1]; Data[14] = InMatrix.M[3][2]; Data[15] = InMatrix.M[3][3];
}

bool FProtobufMathUtils::RepeatedFieldToFMatrix(const google::protobuf::RepeatedField<double>& InField, FMatrix& OutMatrix)
{
	if (InField.size() < 16)
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("RepeatedFieldToFMatrix: Invalid size %d, expected at least 16"), InField.size());
		OutMatrix = FMatrix::Identity;
		return false;
	}

	const double* Data = InField.data();
	if (!Data)
	{
		OutMatrix = FMatrix::Identity;
		return false;
	}

	OutMatrix.M[0][0] = Data[0]; OutMatrix.M[0][1] = Data[1]; OutMatrix.M[0][2] = Data[2]; OutMatrix.M[0][3] = Data[3];
	OutMatrix.M[1][0] = Data[4]; OutMatrix.M[1][1] = Data[5]; OutMatrix.M[1][2] = Data[6]; OutMatrix.M[1][3] = Data[7];
	OutMatrix.M[2][0] = Data[8]; OutMatrix.M[2][1] = Data[9]; OutMatrix.M[2][2] = Data[10]; OutMatrix.M[2][3] = Data[11];
	OutMatrix.M[3][0] = Data[12]; OutMatrix.M[3][1] = Data[13]; OutMatrix.M[3][2] = Data[14]; OutMatrix.M[3][3] = Data[15];
	
	return true;
}