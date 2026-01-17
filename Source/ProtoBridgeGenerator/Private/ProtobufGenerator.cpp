#ifdef PROTOBUF_USE_DLLS
#undef PROTOBUF_USE_DLLS
#endif

#include <iostream>
#include <google/protobuf/compiler/plugin.h>
#include "UECodeGenerator.h"

int main(int argc, char* argv[])
{
	try
	{
		FUeCodeGenerator Generator;
		return google::protobuf::compiler::PluginMain(argc, argv, &Generator);
	}
	catch (const std::exception& e)
	{
		std::cerr << "[UEGen] CRITICAL ERROR: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "[UEGen] UNKNOWN CRITICAL ERROR" << std::endl;
		return 1;
	}
}