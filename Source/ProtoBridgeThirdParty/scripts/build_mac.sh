#!/bin/bash
set -e

GRPC_VERSION="v1.76.0"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."
BUILD_DIR="$SCRIPT_DIR/build_mac"
INSTALL_DIR="$SCRIPT_DIR/install_mac"
FINAL_BIN_DIR="$ROOT_DIR/bin/Mac"
FINAL_LIB_DIR="$ROOT_DIR/lib/Mac"
FINAL_INCLUDE_DIR="$ROOT_DIR/includes"

mkdir -p "$BUILD_DIR"
mkdir -p "$FINAL_BIN_DIR"
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
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=ON \
    -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_TESTS=OFF \
    -DCMAKE_MACOSX_RPATH=ON \
    -DCMAKE_CXX_VISIBILITY_PRESET=hidden

cmake --build . --config Release --target install -j$(sysctl -n hw.ncpu)

cp "$INSTALL_DIR"/lib/*.dylib "$FINAL_BIN_DIR/"
cp "$INSTALL_DIR"/lib/*.dylib "$FINAL_LIB_DIR/"
cp -r "$INSTALL_DIR"/include/* "$FINAL_INCLUDE_DIR/"

echo "Mac Build Complete."