cmake_minimum_required(VERSION 3.15)

set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(ProtoBridgeGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_executable(bridge_generator
    Private/Config/UEDefinitions.h
    
    Private/Context/NameResolver.cpp
    Private/Context/NameResolver.h
    Private/Context/NameValidator.cpp
    Private/Context/NameValidator.h
    
    Private/Generators/EnumGenerator.cpp
    Private/Generators/EnumGenerator.h
    Private/Generators/MessageGenerator.cpp
    Private/Generators/MessageGenerator.h
    Private/Generators/OneOfGenerator.cpp
    Private/Generators/OneOfGenerator.h
    Private/Generators/ProtoLibraryGenerator.cpp
    Private/Generators/ProtoLibraryGenerator.h
    
    Private/Strategies/EnumFieldStrategy.cpp
    Private/Strategies/EnumFieldStrategy.h
    Private/Strategies/FieldStrategy.cpp
    Private/Strategies/FieldStrategy.h
    Private/Strategies/FieldStrategyFactory.cpp
    Private/Strategies/FieldStrategyFactory.h
    Private/Strategies/MapFieldStrategy.cpp
    Private/Strategies/MapFieldStrategy.h
    Private/Strategies/MessageFieldStrategy.cpp
    Private/Strategies/MessageFieldStrategy.h
    Private/Strategies/PrimitiveFieldStrategy.cpp
    Private/Strategies/PrimitiveFieldStrategy.h
    Private/Strategies/StrategyPool.cpp
    Private/Strategies/StrategyPool.h
    Private/Strategies/StringFieldStrategy.cpp
    Private/Strategies/StringFieldStrategy.h
    Private/Strategies/UnrealStrategies.cpp
    Private/Strategies/UnrealStrategies.h
    
    Private/CodeBuilder.cpp
    Private/CodeBuilder.h
    Private/DependencySorter.cpp
    Private/DependencySorter.h
    Private/GeneratorContext.cpp
    Private/GeneratorContext.h
    Private/ProtobufGenerator.cpp
    Private/TypeRegistry.cpp
    Private/TypeRegistry.h
    Private/UECodeGenerator.cpp
    Private/UECodeGenerator.h
)

target_include_directories(bridge_generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

if(MSVC)
        target_compile_options(bridge_generator PRIVATE /EHsc)
endif()

target_link_libraries(bridge_generator PRIVATE
    protobuf::libprotoc
    protobuf::libprotobuf
)