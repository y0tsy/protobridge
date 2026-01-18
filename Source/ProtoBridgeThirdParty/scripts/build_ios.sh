#!/bin/bash
set -e

GRPC_VERSION="v1.76.0"
CACHE_DIR=""
COMPILER_LAUNCHER=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --grpc-version) GRPC_VERSION="$2"; shift ;;
        --cache-dir) CACHE_DIR="$2"; shift ;;
        --compiler-launcher) COMPILER_LAUNCHER="$2"; shift ;;
        *) echo "Unknown parameter: $1"; exit 1 ;;
    esac
    shift
done

LAUNCHER_FLAGS=""
if [ ! -z "$COMPILER_LAUNCHER" ]; then
    LAUNCHER_FLAGS="-DCMAKE_C_COMPILER_LAUNCHER=$COMPILER_LAUNCHER -DCMAKE_CXX_COMPILER_LAUNCHER=$COMPILER_LAUNCHER"
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."
WORK_DIR="$SCRIPT_DIR/work_ios"
INSTALL_HOST="$WORK_DIR/install_host"
INSTALL_IOS="$WORK_DIR/install_ios"

FINAL_LIB_IOS="$ROOT_DIR/lib/IOS"
FINAL_INCLUDE="$ROOT_DIR/includes"

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
mkdir -p "$FINAL_LIB_IOS"
mkdir -p "$FINAL_INCLUDE"

SKIP_GRPC=false

if [ ! -z "$CACHE_DIR" ]; then
    if [ -f "$CACHE_DIR/completed.marker" ]; then
        echo "Cache Hit! Restoring from $CACHE_DIR..."
        cp -r "$CACHE_DIR/install_host" "$WORK_DIR/"
        cp -r "$CACHE_DIR/install_ios" "$WORK_DIR/"
        SKIP_GRPC=true
    else
        echo "Cache Miss. Will build gRPC."
    fi
fi

cd "$WORK_DIR"

if [ "$SKIP_GRPC" = false ]; then
    echo "Cloning gRPC $GRPC_VERSION..."
    git clone --recurse-submodules -b $GRPC_VERSION --depth 1 --shallow-submodules https://github.com/grpc/grpc.git grpc
    cd grpc

    echo "--- PHASE 1: Building Host Tools (Universal) ---"
    cmake -S . -B build_host -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_HOST" \
        -DCMAKE_C_FLAGS="-w" \
        -DCMAKE_CXX_FLAGS="-w" \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
        -DBUILD_SHARED_LIBS=OFF \
        -DgRPC_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_TESTS=OFF \
        -DgRPC_SSL_PROVIDER=module \
        -DgRPC_ZLIB_PROVIDER=module \
        -DgRPC_CARES_PROVIDER=module \
        -DgRPC_RE2_PROVIDER=module \
        -DgRPC_PROTOBUF_PROVIDER=module \
        $LAUNCHER_FLAGS

    cmake --build build_host --config Release --target install

    echo "--- PHASE 2: Building iOS Libs ---"
    HOST_PROTOC="$INSTALL_HOST/bin/protoc"
    HOST_PLUGIN="$INSTALL_HOST/bin/grpc_cpp_plugin"

    cmake -S . -B build_ios -G Ninja \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_ARCHITECTURES="arm64" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
        -DCMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE=NO \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_IOS" \
        -DCMAKE_C_FLAGS="-w" \
        -DCMAKE_CXX_FLAGS="-w" \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DBUILD_SHARED_LIBS=OFF \
        -DgRPC_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_TESTS=OFF \
        -DgRPC_SSL_PROVIDER=module \
        -DgRPC_ZLIB_PROVIDER=module \
        -DgRPC_CARES_PROVIDER=module \
        -DgRPC_RE2_PROVIDER=module \
        -DgRPC_PROTOBUF_PROVIDER=module \
        -D_gRPC_PROTOBUF_PROTOC_EXECUTABLE="$HOST_PROTOC" \
        -D_gRPC_CPP_PLUGIN="$HOST_PLUGIN" \
        $LAUNCHER_FLAGS

    cmake --build build_ios --config Release --target install

    if [ ! -z "$CACHE_DIR" ]; then
        echo "Updating Cache..."
        rm -rf "$CACHE_DIR"
        mkdir -p "$CACHE_DIR"
        cp -r "$INSTALL_HOST" "$CACHE_DIR/"
        cp -r "$INSTALL_IOS" "$CACHE_DIR/"
        touch "$CACHE_DIR/completed.marker"
    fi
fi

echo "Cleaning up installed packages..."
rm -rf "$INSTALL_IOS"/lib/cmake
rm -rf "$INSTALL_IOS"/lib/pkgconfig
rm -rf "$INSTALL_IOS"/share

echo "Copying final artifacts..."
cp -r "$INSTALL_IOS"/lib/* "$FINAL_LIB_IOS/"
cp -r "$INSTALL_IOS"/include/* "$FINAL_INCLUDE/"

echo "iOS Build Complete."
