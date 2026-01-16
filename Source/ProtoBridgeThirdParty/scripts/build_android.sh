#!/bin/bash
set -e

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME is not set."
    exit 1
fi

GRPC_VERSION="v1.76.0"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."
BUILD_DIR="$SCRIPT_DIR/build_android"
INSTALL_DIR="$SCRIPT_DIR/install_android"
FINAL_LIB_DIR="$ROOT_DIR/lib/Android/arm64-v8a"
FINAL_INCLUDE_DIR="$ROOT_DIR/includes"

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

cd "$BUILD_DIR"

cmake ../grpc \
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
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON

cmake --build . --config Release --target install -j$(nproc)

cp "$INSTALL_DIR"/lib/*.so "$FINAL_LIB_DIR/"
cp -r "$INSTALL_DIR"/include/* "$FINAL_INCLUDE_DIR/"

echo "Android Build Complete."