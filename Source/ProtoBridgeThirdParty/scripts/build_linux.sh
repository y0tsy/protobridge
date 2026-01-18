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
GENERATOR_SRC="$ROOT_DIR/../ProtoBridgeGenerator"
WORK_DIR="$SCRIPT_DIR/work_linux"
INSTALL_HOST="$WORK_DIR/install_host"

FINAL_BIN_DIR="$ROOT_DIR/bin/Linux"
FINAL_LIB_LINUX="$ROOT_DIR/lib/Linux"
FINAL_INCLUDE="$ROOT_DIR/includes"

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
mkdir -p "$FINAL_BIN_DIR"
mkdir -p "$FINAL_LIB_LINUX"
mkdir -p "$FINAL_INCLUDE"

SKIP_GRPC=false

if [ ! -z "$CACHE_DIR" ]; then
    if [ -f "$CACHE_DIR/completed.marker" ]; then
        echo "Cache Hit! Restoring from $CACHE_DIR..."
        cp -r "$CACHE_DIR/install_host" "$WORK_DIR/"
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

    echo "--- Building Linux Libs & Tools ---"
    cmake -S . -B build_host -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_HOST" \
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
        $LAUNCHER_FLAGS

    cmake --build build_host --config Release --target install

    if [ ! -z "$CACHE_DIR" ]; then
        echo "Updating Cache..."
        rm -rf "$CACHE_DIR"
        mkdir -p "$CACHE_DIR"
        cp -r "$INSTALL_HOST" "$CACHE_DIR/"
        touch "$CACHE_DIR/completed.marker"
    fi
fi

echo "Building ProtoBridgeGenerator (Linux)..."
cmake -S "$GENERATOR_SRC" -B build_gen -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$INSTALL_HOST" \
    -DCMAKE_C_FLAGS="-w" \
    -DCMAKE_CXX_FLAGS="-w" \
    -Dprotobuf_MODULE_COMPATIBLE=ON \
    $LAUNCHER_FLAGS

cmake --build build_gen --config Release

echo "Cleaning up installed packages..."
rm -rf "$INSTALL_HOST"/lib/cmake
rm -rf "$INSTALL_HOST"/lib/pkgconfig
rm -rf "$INSTALL_HOST"/share

echo "Copying final artifacts..."
cp build_gen/bridge_generator "$FINAL_BIN_DIR/"
cp -r "$INSTALL_HOST"/bin/* "$FINAL_BIN_DIR/"
cp -r "$INSTALL_HOST"/lib/* "$FINAL_LIB_LINUX/"
cp -r "$INSTALL_HOST"/include/* "$FINAL_INCLUDE/"

echo "Linux Build Complete."
