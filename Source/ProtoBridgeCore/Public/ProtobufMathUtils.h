#pragma once

#include "CoreMinimal.h"
#include "ProtobufIncludes.h"
#include "ProtobufStringUtils.h"

#ifdef min
#undef min
#endif min

#ifdef max
#undef max
#endif max

#define DEFINE_MATH_CONV_2(UEType, M1, P1, M2, P2) \
	template <typename T_Proto> \
	static void UEType##ToProto(const UEType& In, T_Proto* Out) { \
		if (!Out) return; \
		Out->set_##P1(In.M1); Out->set_##P2(In.M2); \
	} \
	template <typename T_Proto> \
	static UEType ProtoTo##UEType(const T_Proto& In) { \
		return UEType(In.P1(), In.P2()); \
	}

#define DEFINE_MATH_CONV_3(UEType, M1, P1, M2, P2, M3, P3) \
	template <typename T_Proto> \
	static void UEType##ToProto(const UEType& In, T_Proto* Out) { \
		if (!Out) return; \
		Out->set_##P1(In.M1); Out->set_##P2(In.M2); Out->set_##P3(In.M3); \
	} \
	template <typename T_Proto> \
	static UEType ProtoTo##UEType(const T_Proto& In) { \
		return UEType(In.P1(), In.P2(), In.P3()); \
	}

#define DEFINE_MATH_CONV_4(UEType, M1, P1, M2, P2, M3, P3, M4, P4) \
	template <typename T_Proto> \
	static void UEType##ToProto(const UEType& In, T_Proto* Out) { \
		if (!Out) return; \
		Out->set_##P1(In.M1); Out->set_##P2(In.M2); Out->set_##P3(In.M3); Out->set_##P4(In.M4); \
	} \
	template <typename T_Proto> \
	static UEType ProtoTo##UEType(const T_Proto& In) { \
		return UEType(In.P1(), In.P2(), In.P3(), In.P4()); \
	}

class PROTOBRIDGECORE_API FProtobufMathUtils
{
public:
	static void FDateTimeToTimestamp(const FDateTime& InDateTime, google::protobuf::Timestamp& OutTimestamp);
	static FDateTime TimestampToFDateTime(const google::protobuf::Timestamp& InTimestamp);

	static void FTimespanToDuration(const FTimespan& InTimespan, google::protobuf::Duration& OutDuration);
	static FTimespan DurationToFTimespan(const google::protobuf::Duration& InDuration);

	static void FMatrixToRepeatedField(const FMatrix& InMatrix, google::protobuf::RepeatedField<double>* OutField);
	static FMatrix RepeatedFieldToFMatrix(const google::protobuf::RepeatedField<double>& InField);

	DEFINE_MATH_CONV_3(FVector, X, x, Y, y, Z, z);
	DEFINE_MATH_CONV_2(FVector2D, X, x, Y, y);
	DEFINE_MATH_CONV_3(FIntVector, X, x, Y, y, Z, z);
	DEFINE_MATH_CONV_2(FIntPoint, X, x, Y, y);
	DEFINE_MATH_CONV_3(FRotator, Pitch, pitch, Yaw, yaw, Roll, roll);
	DEFINE_MATH_CONV_4(FQuat, X, x, Y, y, Z, z, W, w);
	DEFINE_MATH_CONV_4(FGuid, A, a, B, b, C, c, D, d);

	template <typename T_Proto>
	static void FColorToProto(const FColor& InColor, T_Proto* OutProto) {
		if (!OutProto) return;
		OutProto->set_r(InColor.R); OutProto->set_g(InColor.G); OutProto->set_b(InColor.B); OutProto->set_a(InColor.A);
	}
	template <typename T_Proto>
	static FColor ProtoToFColor(const T_Proto& InProto) {
		return FColor(static_cast<uint8>(InProto.r()), static_cast<uint8>(InProto.g()), static_cast<uint8>(InProto.b()), static_cast<uint8>(InProto.a()));
	}

	template <typename T_Proto>
	static void FLinearColorToProto(const FLinearColor& InColor, T_Proto* OutProto) {
		if (!OutProto) return;
		OutProto->set_r(InColor.R); OutProto->set_g(InColor.G); OutProto->set_b(InColor.B); OutProto->set_a(InColor.A);
	}
	template <typename T_Proto>
	static FLinearColor ProtoToFLinearColor(const T_Proto& InProto) {
		return FLinearColor(InProto.r(), InProto.g(), InProto.b(), InProto.a());
	}

	template <typename T_Proto>
	static void FTransformToProto(const FTransform& InTransform, T_Proto* OutProto) {
		if (!OutProto) return;
		FVectorToProto(InTransform.GetLocation(), OutProto->mutable_location());
		FQuatToProto(InTransform.GetRotation(), OutProto->mutable_rotation());
		FVectorToProto(InTransform.GetScale3D(), OutProto->mutable_scale());
	}
	template <typename T_Proto>
	static FTransform ProtoToFTransform(const T_Proto& InProto) {
		FTransform Result;
		Result.SetLocation(ProtoToFVector(InProto.location()));
		Result.SetRotation(ProtoToFQuat(InProto.rotation()));
		Result.SetScale3D(ProtoToFVector(InProto.scale()));
		return Result;
	}

	template <typename T_Proto>
	static void FMatrixToProto(const FMatrix& InMatrix, T_Proto* OutProto) {
		if (!OutProto) return;
		OutProto->set_m00(InMatrix.M[0][0]); OutProto->set_m01(InMatrix.M[0][1]); OutProto->set_m02(InMatrix.M[0][2]); OutProto->set_m03(InMatrix.M[0][3]);
		OutProto->set_m10(InMatrix.M[1][0]); OutProto->set_m11(InMatrix.M[1][1]); OutProto->set_m12(InMatrix.M[1][2]); OutProto->set_m13(InMatrix.M[1][3]);
		OutProto->set_m20(InMatrix.M[2][0]); OutProto->set_m21(InMatrix.M[2][1]); OutProto->set_m22(InMatrix.M[2][2]); OutProto->set_m23(InMatrix.M[2][3]);
		OutProto->set_m30(InMatrix.M[3][0]); OutProto->set_m31(InMatrix.M[3][1]); OutProto->set_m32(InMatrix.M[3][2]); OutProto->set_m33(InMatrix.M[3][3]);
	}
	template <typename T_Proto>
	static FMatrix ProtoToFMatrix(const T_Proto& InProto) {
		FMatrix Result;
		Result.M[0][0] = InProto.m00(); Result.M[0][1] = InProto.m01(); Result.M[0][2] = InProto.m02(); Result.M[0][3] = InProto.m03();
		Result.M[1][0] = InProto.m10(); Result.M[1][1] = InProto.m11(); Result.M[1][2] = InProto.m12(); Result.M[1][3] = InProto.m13();
		Result.M[2][0] = InProto.m20(); Result.M[2][1] = InProto.m21(); Result.M[2][2] = InProto.m22(); Result.M[2][3] = InProto.m23();
		Result.M[3][0] = InProto.m30(); Result.M[3][1] = InProto.m31(); Result.M[3][2] = InProto.m32(); Result.M[3][3] = InProto.m33();
		return Result;
	}

	template <typename T_Proto>
	static void FBoxToProto(const FBox& InBox, T_Proto* OutProto) {
		if (!OutProto) return;
		FVectorToProto(InBox.Min, OutProto->mutable_min());
		FVectorToProto(InBox.Max, OutProto->mutable_max());
		OutProto->set_is_valid(InBox.IsValid);
	}
	template <typename T_Proto>
	static FBox ProtoToFBox(const T_Proto& InProto) {
		FBox Result;
		Result.Min = ProtoToFVector(InProto.min());
		Result.Max = ProtoToFVector(InProto.max());
		Result.IsValid = InProto.is_valid();
		return Result;
	}

	template <typename T_Proto>
	static void FBox2DToProto(const FBox2D& InBox, T_Proto* OutProto) {
		if (!OutProto) return;
		FVector2DToProto(InBox.Min, OutProto->mutable_min());
		FVector2DToProto(InBox.Max, OutProto->mutable_max());
		OutProto->set_is_valid(InBox.bIsValid);
	}
	template <typename T_Proto>
	static FBox2D ProtoToFBox2D(const T_Proto& InProto) {
		FBox2D Result;
		Result.Min = ProtoToFVector2D(InProto.min());
		Result.Max = ProtoToFVector2D(InProto.max());
		Result.bIsValid = InProto.is_valid();
		return Result;
	}

	template <typename T_Proto>
	static void FSphereToProto(const FSphere& InSphere, T_Proto* OutProto) {
		if (!OutProto) return;
		FVectorToProto(InSphere.Center, OutProto->mutable_center());
		OutProto->set_w(InSphere.W);
	}
	template <typename T_Proto>
	static FSphere ProtoToFSphere(const T_Proto& InProto) {
		FSphere Result;
		Result.Center = ProtoToFVector(InProto.center());
		Result.W = InProto.w();
		return Result;
	}
};