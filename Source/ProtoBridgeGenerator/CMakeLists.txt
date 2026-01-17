cmake_minimum_required(VERSION 3.15)

project(ProtoBridgeGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)

add_executable(bridge_generator
    Private/ProtoBridgeGenerator.cpp
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