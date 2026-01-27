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
    #enter point
    
    Private/ProtobufGenerator.cpp
    
    # declarations
    Private/CodeBuilder.h
    Private/DependencySorter.h
    Private/GeneratorContext.h
    Private/TypeRegistry.h
    Private/UECodeGenerator.h
    Private/Generators/EnumGenerator.h
    Private/Generators/MessageGenerator.h
    Private/Generators/OneOfGenerator.h
    Private/Generators/ProtoLibraryGenerator.h
    Private/Strategies/EnumFieldStrategy.h
    Private/Strategies/FieldStrategy.h
    Private/Strategies/FieldStrategyFactory.h
    Private/Strategies/MapFieldStrategy.h
    Private/Strategies/MessageFieldStrategy.h
    Private/Strategies/PrimitiveFieldStrategy.h
    Private/Strategies/StringFieldStrategy.h
    Private/Strategies/UnrealStrategies.h
    
    # implementations
    Private/CodeBuilder.cpp
    Private/DependencySorter.cpp
    Private/GeneratorContext.cpp
    Private/TypeRegistry.cpp
    Private/UECodeGenerator.cpp
    Private/Generators/EnumGenerator.cpp
    Private/Generators/MessageGenerator.cpp
    Private/Generators/OneOfGenerator.cpp
    Private/Generators/ProtoLibraryGenerator.cpp
    Private/Strategies/EnumFieldStrategy.cpp
    Private/Strategies/FieldStrategy.cpp
    Private/Strategies/FieldStrategyFactory.cpp
    Private/Strategies/MapFieldStrategy.cpp
    Private/Strategies/MessageFieldStrategy.cpp
    Private/Strategies/PrimitiveFieldStrategy.cpp
    Private/Strategies/StringFieldStrategy.cpp
    Private/Strategies/UnrealStrategies.cpp    
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