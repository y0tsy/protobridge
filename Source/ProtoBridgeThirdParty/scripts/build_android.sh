#!/bin/bash
set -e

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME is not set."
    exit 1
fi

GRPC_VERSION="v1.76.0"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."
BUILD_DIR_HOST="$SCRIPT_DIR/build_android_host"
INSTALL_DIR_HOST="$SCRIPT_DIR/install_android_host"
BUILD_DIR="$SCRIPT_DIR/build_android"
INSTALL_DIR="$SCRIPT_DIR/install_android"
FINAL_LIB_DIR="$ROOT_DIR/lib/Android/arm64-v8a"
FINAL_INCLUDE_DIR="$ROOT_DIR/includes"

mkdir -p "$BUILD_DIR_HOST"
mkdir -p "$BUILD_DIR"
mkdir -p "$FINAL_LIB_DIR"

cd "$SCRIPT_DIR"

if [ ! -d "grpc" ]; then
    echo "Cloning gRPC..."
    git clone --recurse-submodules -b $GRPC_VERSION --depth 1 --shallow-submodules https://github.com/grpc/grpc
else
    echo "Updating gRPC..."
    cd grpc
    git fetch --tags
    git checkout $GRPC_VERSION
    git submodule update --init --recursive
    cd ..
fi

echo "Building Host Tools (protoc, grpc_cpp_plugin)..."
cd "$BUILD_DIR_HOST"
cmake ../grpc \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR_HOST" \
    -DBUILD_SHARED_LIBS=OFF \
    -DgRPC_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_TESTS=OFF \
    -DgRPC_SSL_PROVIDER=module \
    -DgRPC_ZLIB_PROVIDER=module \
    -DgRPC_CARES_PROVIDER=module \
    -DgRPC_RE2_PROVIDER=module \
    -DgRPC_PROTOBUF_PROVIDER=module

cmake --build . --target protoc grpc_cpp_plugin

HOST_PROTOC="$BUILD_DIR_HOST/third_party/protobuf/protoc"
HOST_PLUGIN="$BUILD_DIR_HOST/grpc_cpp_plugin"

echo "Building Android Libraries..."
cd "$BUILD_DIR"

cmake ../grpc \
    -GNinja \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="arm64-v8a" \
    -DANDROID_PLATFORM=android-26 \
    -DANDROID_STL=c++_shared \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=ON \
    -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_TESTS=OFF \
    -DgRPC_SSL_PROVIDER=module \
    -DgRPC_ZLIB_PROVIDER=module \
    -DgRPC_CARES_PROVIDER=module \
    -DgRPC_RE2_PROVIDER=module \
    -DgRPC_PROTOBUF_PROVIDER=module \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -D_gRPC_PROTOBUF_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -D_gRPC_CPP_PLUGIN="$HOST_PLUGIN"

cmake --build . --config Release --target install

cp "$INSTALL_DIR"/lib/*.so "$FINAL_LIB_DIR/"
cp -r "$INSTALL_DIR"/include/* "$FINAL_INCLUDE_DIR/"

echo "Android Build Complete."
