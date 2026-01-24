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

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/status_code_enum.h>

#pragma pop_macro("TEXT")
#pragma pop_macro("verify")
#pragma pop_macro("check")

#if defined(_MSC_VER)
#pragma warning(pop)
#endif