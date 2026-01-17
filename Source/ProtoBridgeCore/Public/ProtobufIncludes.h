#pragma once

#include "CoreMinimal.h"

#pragma push_macro("check")
#undef check

#pragma push_macro("verify")
#undef verify

#pragma push_macro("TEXT")
#undef TEXT

#pragma push_macro("ChangeDisplaySettings")
#undef ChangeDisplaySettings

#pragma push_macro("GetMessage")
#undef GetMessage

#pragma warning(push)
#pragma warning(disable: 4800)
#pragma warning(disable: 4125)
#pragma warning(disable: 4668)
#pragma warning(disable: 4541)
#pragma warning(disable: 4946)

#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/time_util.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>
#include <google/protobuf/wrappers.pb.h>
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/any.pb.h>

#pragma warning(pop)

#pragma pop_macro("GetMessage")
#pragma pop_macro("ChangeDisplaySettings")
#pragma pop_macro("TEXT")
#pragma pop_macro("verify")
#pragma pop_macro("check")