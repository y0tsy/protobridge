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
find_package(utf8_range CONFIG)

add_executable(bridge_generator
    Private/ProtobufGenerator.cpp
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
)

target_include_directories(bridge_generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
    ${Protobuf_INCLUDE_DIRS}
)

if(TARGET protobuf::libprotoc AND TARGET protobuf::libprotobuf)
    target_link_libraries(bridge_generator PRIVATE
        protobuf::libprotoc
        protobuf::libprotobuf
    )
else()
    target_link_libraries(bridge_generator PRIVATE
        ${Protobuf_PROTOC_LIBRARY}
        ${Protobuf_LIBRARIES}
    )
endif()
