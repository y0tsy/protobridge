#!/bin/bash
set -e

PROTOBUF_VERSION="v33.4"
CACHE_DIR=""
COMPILER_LAUNCHER=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --protobuf-version) PROTOBUF_VERSION="$2"; shift ;;
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

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME is not set."
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."
WORK_DIR="$SCRIPT_DIR/work_android"
INSTALL_HOST="$WORK_DIR/install_host"
INSTALL_ANDROID="$WORK_DIR/install_android"

FINAL_LIB_ANDROID="$ROOT_DIR/lib/Android/arm64-v8a"
FINAL_INCLUDE="$ROOT_DIR/includes"

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
mkdir -p "$FINAL_LIB_ANDROID"
mkdir -p "$FINAL_INCLUDE"

SKIP_PROTO=false

if [ ! -z "$CACHE_DIR" ]; then
    if [ -f "$CACHE_DIR/completed.marker" ]; then
        echo "Cache Hit! Restoring from $CACHE_DIR..."
        cp -r "$CACHE_DIR/install_host" "$WORK_DIR/"
        cp -r "$CACHE_DIR/install_android" "$WORK_DIR/"
        SKIP_PROTO=true
    else
        echo "Cache Miss. Will build Protobuf."
    fi
fi

cd "$WORK_DIR"

if [ "$SKIP_PROTO" = false ]; then
    echo "Cloning Protobuf $PROTOBUF_VERSION..."
    git clone --recurse-submodules -b $PROTOBUF_VERSION --depth 1 --shallow-submodules https://github.com/protocolbuffers/protobuf.git protobuf
    cd protobuf

    echo "--- PHASE 1: Building Host Tools ---"
    cmake -S . -B build_host -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_HOST" \
        -DCMAKE_C_FLAGS="-w" \
        -DCMAKE_CXX_FLAGS="-w" \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DBUILD_SHARED_LIBS=OFF \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=module \
        $LAUNCHER_FLAGS

    cmake --build build_host --config Release --target install

    echo "--- PHASE 2: Building Android Libs ---"
    HOST_PROTOC="$INSTALL_HOST/bin/protoc"

    cmake -S . -B build_android -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="arm64-v8a" \
        -DANDROID_PLATFORM=android-26 \
        -DANDROID_STL=c++_shared \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_ANDROID" \
        -DCMAKE_C_FLAGS="-w" \
        -DCMAKE_CXX_FLAGS="-w" \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DBUILD_SHARED_LIBS=OFF \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=module \
        -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
        $LAUNCHER_FLAGS

    cmake --build build_android --config Release --target install

    if [ ! -z "$CACHE_DIR" ]; then
        echo "Updating Cache..."
        rm -rf "$CACHE_DIR"
        mkdir -p "$CACHE_DIR"
        cp -r "$INSTALL_HOST" "$CACHE_DIR/"
        cp -r "$INSTALL_ANDROID" "$CACHE_DIR/"
        touch "$CACHE_DIR/completed.marker"
    fi
fi

echo "Cleaning up installed packages..."
rm -rf "$INSTALL_ANDROID"/lib/cmake
rm -rf "$INSTALL_ANDROID"/lib/pkgconfig
rm -rf "$INSTALL_ANDROID"/share

echo "Copying final artifacts..."
cp -r "$INSTALL_ANDROID"/lib/* "$FINAL_LIB_ANDROID/"
cp -r "$INSTALL_ANDROID"/include/* "$FINAL_INCLUDE/"

echo "Android Build Complete."