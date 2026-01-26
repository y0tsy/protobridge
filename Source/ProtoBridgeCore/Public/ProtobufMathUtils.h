#pragma once

#include "CoreMinimal.h"
#include "ProtobufStringUtils.h"
#include <type_traits>

namespace google {
namespace protobuf {
	class Timestamp;
	class Duration;
	template <typename T> class RepeatedField;
}
}

namespace ProtoMathCheck {
	template<typename T, typename = void> struct HasSetX : std::false_type {};
	template<typename T> struct HasSetX<T, std::void_t<decltype(std::declval<T>().set_x(0.0))>> : std::true_type {};

	template<typename T, typename = void> struct HasSetY : std::false_type {};
	template<typename T> struct HasSetY<T, std::void_t<decltype(std::declval<T>().set_y(0.0))>> : std::true_type {};

	template<typename T, typename = void> struct HasSetZ : std::false_type {};
	template<typename T> struct HasSetZ<T, std::void_t<decltype(std::declval<T>().set_z(0.0))>> : std::true_type {};

	template<typename T, typename = void> struct HasSetW : std::false_type {};
	template<typename T> struct HasSetW<T, std::void_t<decltype(std::declval<T>().set_w(0.0))>> : std::true_type {};
	
	template<typename T, typename = void> struct HasX : std::false_type {};
	template<typename T> struct HasX<T, std::void_t<decltype(std::declval<T>().x())>> : std::true_type {};
}

class PROTOBRIDGECORE_API FProtobufMathUtils
{
public:
	static bool FDateTimeToTimestamp(const FDateTime& InDateTime, google::protobuf::Timestamp& OutTimestamp);
	static FDateTime TimestampToFDateTime(const google::protobuf::Timestamp& InTimestamp);

	static void FTimespanToDuration(const FTimespan& InTimespan, google::protobuf::Duration& OutDuration);
	static FTimespan DurationToFTimespan(const google::protobuf::Duration& InDuration);

	static void FMatrixToRepeatedField(const FMatrix& InMatrix, google::protobuf::RepeatedField<double>* OutField);
	static bool RepeatedFieldToFMatrix(const google::protobuf::RepeatedField<double>& InField, FMatrix& OutMatrix);

	template <typename T_Proto>
	static bool FVectorToProto(const FVector& In, T_Proto* Out) {
		static_assert(ProtoMathCheck::HasSetX<T_Proto>::value, "Proto message type must have set_x(double)");
		static_assert(ProtoMathCheck::HasSetY<T_Proto>::value, "Proto message type must have set_y(double)");
		static_assert(ProtoMathCheck::HasSetZ<T_Proto>::value, "Proto message type must have set_z(double)");
		if (!Out) return false;
		Out->set_x(In.X); Out->set_y(In.Y); Out->set_z(In.Z);
		return true;
	}
	template <typename T_Proto>
	static FVector ProtoToFVector(const T_Proto& In) {
		static_assert(ProtoMathCheck::HasX<T_Proto>::value, "Proto message type must have x()");
		return FVector(In.x(), In.y(), In.z());
	}

	template <typename T_Proto>
	static bool FVector2DToProto(const FVector2D& In, T_Proto* Out) {
		static_assert(ProtoMathCheck::HasSetX<T_Proto>::value, "Proto message type must have set_x(double)");
		static_assert(ProtoMathCheck::HasSetY<T_Proto>::value, "Proto message type must have set_y(double)");
		if (!Out) return false;
		Out->set_x(In.X); Out->set_y(In.Y);
		return true;
	}
	template <typename T_Proto>
	static FVector2D ProtoToFVector2D(const T_Proto& In) {
		static_assert(ProtoMathCheck::HasX<T_Proto>::value, "Proto message type must have x()");
		return FVector2D(In.x(), In.y());
	}

	template <typename T_Proto>
	static bool FIntVectorToProto(const FIntVector& In, T_Proto* Out) {
		static_assert(ProtoMathCheck::HasSetX<T_Proto>::value, "Proto message type must have set_x");
		if (!Out) return false;
		Out->set_x(In.X); Out->set_y(In.Y); Out->set_z(In.Z);
		return true;
	}
	template <typename T_Proto>
	static FIntVector ProtoToFIntVector(const T_Proto& In) {
		static_assert(ProtoMathCheck::HasX<T_Proto>::value, "Proto message type must have x()");
		return FIntVector(In.x(), In.y(), In.z());
	}

	template <typename T_Proto>
	static bool FIntPointToProto(const FIntPoint& In, T_Proto* Out) {
		static_assert(ProtoMathCheck::HasSetX<T_Proto>::value, "Proto message type must have set_x");
		if (!Out) return false;
		Out->set_x(In.X); Out->set_y(In.Y);
		return true;
	}
	template <typename T_Proto>
	static FIntPoint ProtoToFIntPoint(const T_Proto& In) {
		static_assert(ProtoMathCheck::HasX<T_Proto>::value, "Proto message type must have x()");
		return FIntPoint(In.x(), In.y());
	}

	template <typename T_Proto>
	static bool FRotatorToProto(const FRotator& In, T_Proto* Out) {
		if (!Out) return false;
		Out->set_pitch(In.Pitch); Out->set_yaw(In.Yaw); Out->set_roll(In.Roll);
		return true;
	}
	template <typename T_Proto>
	static FRotator ProtoToFRotator(const T_Proto& In) {
		return FRotator(In.pitch(), In.yaw(), In.roll());
	}

	template <typename T_Proto>
	static bool FQuatToProto(const FQuat& In, T_Proto* Out) {
		static_assert(ProtoMathCheck::HasSetW<T_Proto>::value, "Proto message type must have set_w");
		if (!Out) return false;
		Out->set_x(In.X); Out->set_y(In.Y); Out->set_z(In.Z); Out->set_w(In.W);
		return true;
	}
	template <typename T_Proto>
	static FQuat ProtoToFQuat(const T_Proto& In) {
		static_assert(ProtoMathCheck::HasX<T_Proto>::value, "Proto message type must have x()");
		return FQuat(In.x(), In.y(), In.z(), In.w());
	}

	template <typename T_Proto>
	static bool FGuidToProto(const FGuid& In, T_Proto* Out) {
		if (!Out) return false;
		Out->set_a(In.A); Out->set_b(In.B); Out->set_c(In.C); Out->set_d(In.D);
		return true;
	}
	template <typename T_Proto>
	static FGuid ProtoToFGuid(const T_Proto& In) {
		return FGuid(In.a(), In.b(), In.c(), In.d());
	}

	template <typename T_Proto>
	static bool FColorToProto(const FColor& InColor, T_Proto* OutProto) {
		if (!OutProto) return false;
		OutProto->set_r(InColor.R); OutProto->set_g(InColor.G); OutProto->set_b(InColor.B); OutProto->set_a(InColor.A);
		return true;
	}
	template <typename T_Proto>
	static FColor ProtoToFColor(const T_Proto& InProto) {
		return FColor(static_cast<uint8>(InProto.r()), static_cast<uint8>(InProto.g()), static_cast<uint8>(InProto.b()), static_cast<uint8>(InProto.a()));
	}

	template <typename T_Proto>
	static bool FLinearColorToProto(const FLinearColor& InColor, T_Proto* OutProto) {
		if (!OutProto) return false;
		OutProto->set_r(InColor.R); OutProto->set_g(InColor.G); OutProto->set_b(InColor.B); OutProto->set_a(InColor.A);
		return true;
	}
	template <typename T_Proto>
	static FLinearColor ProtoToFLinearColor(const T_Proto& InProto) {
		return FLinearColor(InProto.r(), InProto.g(), InProto.b(), InProto.a());
	}

	template <typename T_Proto>
	static bool FTransformToProto(const FTransform& InTransform, T_Proto* OutProto) {
		if (!OutProto) return false;
		FVectorToProto(InTransform.GetLocation(), OutProto->mutable_location());
		FQuatToProto(InTransform.GetRotation(), OutProto->mutable_rotation());
		FVectorToProto(InTransform.GetScale3D(), OutProto->mutable_scale());
		return true;
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
	static bool FMatrixToProto(const FMatrix& InMatrix, T_Proto* OutProto) {
		if (!OutProto) return false;
		OutProto->set_m00(InMatrix.M[0][0]); OutProto->set_m01(InMatrix.M[0][1]); OutProto->set_m02(InMatrix.M[0][2]); OutProto->set_m03(InMatrix.M[0][3]);
		OutProto->set_m10(InMatrix.M[1][0]); OutProto->set_m11(InMatrix.M[1][1]); OutProto->set_m12(InMatrix.M[1][2]); OutProto->set_m13(InMatrix.M[1][3]);
		OutProto->set_m20(InMatrix.M[2][0]); OutProto->set_m21(InMatrix.M[2][1]); OutProto->set_m22(InMatrix.M[2][2]); OutProto->set_m23(InMatrix.M[2][3]);
		OutProto->set_m30(InMatrix.M[3][0]); OutProto->set_m31(InMatrix.M[3][1]); OutProto->set_m32(InMatrix.M[3][2]); OutProto->set_m33(InMatrix.M[3][3]);
		return true;
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
	static bool FBoxToProto(const FBox& InBox, T_Proto* OutProto) {
		if (!OutProto) return false;
		FVectorToProto(InBox.Min, OutProto->mutable_min());
		FVectorToProto(InBox.Max, OutProto->mutable_max());
		OutProto->set_is_valid(InBox.IsValid);
		return true;
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
	static bool FBox2DToProto(const FBox2D& InBox, T_Proto* OutProto) {
		if (!OutProto) return false;
		FVector2DToProto(InBox.Min, OutProto->mutable_min());
		FVector2DToProto(InBox.Max, OutProto->mutable_max());
		OutProto->set_is_valid(InBox.bIsValid);
		return true;
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
	static bool FSphereToProto(const FSphere& InSphere, T_Proto* OutProto) {
		if (!OutProto) return false;
		FVectorToProto(InSphere.Center, OutProto->mutable_center());
		OutProto->set_w(InSphere.W);
		return true;
	}
	template <typename T_Proto>
	static FSphere ProtoToFSphere(const T_Proto& InProto) {
		FSphere Result;
		Result.Center = ProtoToFVector(InProto.center());
		Result.W = InProto.w();
		return Result;
	}
};