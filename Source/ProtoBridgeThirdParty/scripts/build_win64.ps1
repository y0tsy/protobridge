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
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR `
    -DBUILD_SHARED_LIBS=ON `
    -DgRPC_INSTALL=ON `
    -DgRPC_BUILD_TESTS=OFF `
    -Dprotobuf_BUILD_TESTS=OFF `
    -DgRPC_MSVC_STATIC_RUNTIME=OFF `
    -Dprotobuf_MSVC_STATIC_RUNTIME=OFF `
    -DgRPC_SSL_PROVIDER=package `
    -DgRPC_ZLIB_PROVIDER=package `
    -DCMAKE_VS_PLATFORM_TOOLSET_HOST_ARCH=x64

cmake --build . --config Release --target install -j 16

Copy-Item -Path "$INSTALL_DIR\bin\*.dll" -Destination $FINAL_BIN_DIR -Force
Copy-Item -Path "$INSTALL_DIR\lib\*.lib" -Destination $FINAL_LIB_DIR -Force
Copy-Item -Path "$INSTALL_DIR\include\*" -Destination $FINAL_INCLUDE_DIR -Recurse -Force

Write-Host "Windows Build Complete."