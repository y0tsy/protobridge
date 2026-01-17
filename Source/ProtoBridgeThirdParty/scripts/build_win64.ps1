$ErrorActionPreference = "Stop"

$GRPC_VERSION = "v1.76.0"
$SCRIPT_DIR = $PSScriptRoot
$ROOT_DIR = Join-Path $SCRIPT_DIR ".."
$BUILD_DIR = Join-Path $SCRIPT_DIR "build_win64"
$INSTALL_DIR = Join-Path $SCRIPT_DIR "install_win64"
$FINAL_BIN_DIR = Join-Path $ROOT_DIR "bin\Win64"
$FINAL_LIB_DIR = Join-Path $ROOT_DIR "lib\Win64"
$FINAL_INCLUDE_DIR = Join-Path $ROOT_DIR "includes"

if (!(Test-Path $BUILD_DIR)) { New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null }
if (!(Test-Path $FINAL_BIN_DIR)) { New-Item -ItemType Directory -Path $FINAL_BIN_DIR -Force | Out-Null }
if (!(Test-Path $FINAL_LIB_DIR)) { New-Item -ItemType Directory -Path $FINAL_LIB_DIR -Force | Out-Null }
if (!(Test-Path $FINAL_INCLUDE_DIR)) { New-Item -ItemType Directory -Path $FINAL_INCLUDE_DIR -Force | Out-Null }
Set-Location $SCRIPT_DIR

if (!(Test-Path "grpc")) {
    Write-Host "Cloning gRPC..."
    git clone --recurse-submodules -b $GRPC_VERSION --depth 1 --shallow-submodules https://github.com/grpc/grpc
}
else {
    Write-Host "Updating gRPC..."
    Push-Location "grpc"
    git fetch --tags
    git checkout $GRPC_VERSION
    git submodule update --init --recursive
    Pop-Location
}

Set-Location $BUILD_DIR

cmake ../grpc `
    -GNinja `
    -DCMAKE_BUILD_TYPE=Release `
    "-DCMAKE_INSTALL_PREFIX=$INSTALL_DIR" `
    -DCMAKE_CXX_FLAGS="/w" `
    -DCMAKE_C_FLAGS="/w" `
    -DCMAKE_CXX_STANDARD=20 `
    -DBUILD_SHARED_LIBS=OFF `
    -DgRPC_INSTALL=ON `
    -DgRPC_BUILD_TESTS=OFF `
    -Dprotobuf_BUILD_TESTS=OFF `
    -Dprotobuf_BUILD_PROTOC_BINARIES=ON `
    -DgRPC_BUILD_CODEGEN=ON `
    -DgRPC_MSVC_STATIC_RUNTIME=OFF `
    -Dprotobuf_MSVC_STATIC_RUNTIME=OFF `
    -DgRPC_SSL_PROVIDER=module `
    -DgRPC_ZLIB_PROVIDER=module `
    -DgRPC_CARES_PROVIDER=module `
    -DgRPC_RE2_PROVIDER=module `
    -DgRPC_PROTOBUF_PROVIDER=module

cmake --build . --config Release --target install

Remove-Item -Path (Join-Path $INSTALL_DIR "lib\cmake") -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $INSTALL_DIR "lib\pkgconfig") -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $INSTALL_DIR "share") -Recurse -Force -ErrorAction SilentlyContinue

Copy-Item -Path "$INSTALL_DIR\bin\*" -Destination $FINAL_BIN_DIR -Recurse -Force
Copy-Item -Path "$INSTALL_DIR\lib\*" -Destination $FINAL_LIB_DIR -Recurse -Force
Copy-Item -Path "$INSTALL_DIR\include\*" -Destination $FINAL_INCLUDE_DIR -Recurse -Force

Write-Host "Windows Build Complete."
