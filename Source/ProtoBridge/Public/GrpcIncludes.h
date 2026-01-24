#pragma once

#include "CoreMinimal.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4125)
#pragma warning(disable: 4668)
#pragma warning(disable: 4541)
#pragma warning(disable: 4800)
#pragma warning(disable: 4946)
#pragma warning(disable: 4251)
#pragma warning(disable: 4244)
#endif

#pragma push_macro("check")
#undef check

#pragma push_macro("verify")
#undef verify

#pragma push_macro("TEXT")
#undef TEXT

#pragma push_macro("CreateService")
#undef CreateService

#pragma push_macro("GetObject")
#undef GetObject

#pragma push_macro("GetMessage")
#undef GetMessage

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/status_code_enum.h>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/generic/generic_stub.h>

#pragma pop_macro("GetMessage")
#pragma pop_macro("GetObject")
#pragma pop_macro("CreateService")
#pragma pop_macro("TEXT")
#pragma pop_macro("verify")
#pragma pop_macro("check")

#if defined(_MSC_VER)
#pragma warning(pop)
#endif