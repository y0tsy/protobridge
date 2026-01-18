param (
    [string]$GrpcVersion = "v1.76.0",
    [string]$CacheDir = "",
    [string]$CompilerLauncher = ""
)

$ErrorActionPreference = "Stop"

$ScriptDir = $PSScriptRoot
$RootDir = Join-Path $ScriptDir ".."
$GeneratorSourceDir = Join-Path (Join-Path $RootDir "..") "ProtoBridgeGenerator"

$WorkDir = Join-Path $ScriptDir "work_windows"
$InstallDir = Join-Path $WorkDir "install"

$FinalBinDir = Join-Path $RootDir "bin\Win64"
$FinalLibDir = Join-Path $RootDir "lib\Win64"
$FinalIncludeDir = Join-Path $RootDir "includes"

if (Test-Path $WorkDir) { Remove-Item -Path $WorkDir -Recurse -Force }
New-Item -ItemType Directory -Path $WorkDir | Out-Null

if (!(Test-Path $FinalBinDir)) { New-Item -ItemType Directory -Path $FinalBinDir -Force | Out-Null }
if (!(Test-Path $FinalLibDir)) { New-Item -ItemType Directory -Path $FinalLibDir -Force | Out-Null }
if (!(Test-Path $FinalIncludeDir)) { New-Item -ItemType Directory -Path $FinalIncludeDir -Force | Out-Null }

$SkipGrpcBuild = $false

if (-not [string]::IsNullOrEmpty($CacheDir)) {
    $CacheInstall = Join-Path $CacheDir "install"
    $CacheMarker = Join-Path $CacheDir "completed.marker"

    if ((Test-Path $CacheMarker) -and (Test-Path $CacheInstall)) {
        Write-Host "Cache Hit! Restoring gRPC from $CacheDir..."
        Copy-Item -Path $CacheInstall -Destination $WorkDir -Recurse -Force
        $SkipGrpcBuild = $true
    } else {
        Write-Host "Cache Miss or Invalid. Will build gRPC."
    }
}

Set-Location $WorkDir

$Launcher_Arg = ""

if (-not [string]::IsNullOrEmpty($CompilerLauncher)) {
    Write-Host "Using Compiler Launcher: $CompilerLauncher"
    $Launcher_Arg = "-DCMAKE_C_COMPILER_LAUNCHER=$CompilerLauncher -DCMAKE_CXX_COMPILER_LAUNCHER=$CompilerLauncher"
}

if (-not $SkipGrpcBuild) {
    Write-Host "Cloning gRPC $GrpcVersion..."
    git clone --recurse-submodules -b $GrpcVersion --depth 1 --shallow-submodules https://github.com/grpc/grpc.git
    Set-Location "grpc"

    Write-Host "--- Building gRPC (Dynamic CRT /MD) ---"
    $BuildDir = Join-Path $WorkDir "build"
    $Cmd = "cmake -S . -B $BuildDir -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$InstallDir -DCMAKE_C_FLAGS='/w' -DCMAKE_CXX_FLAGS='/w' -DCMAKE_CXX_STANDARD=20 -DBUILD_SHARED_LIBS=OFF -DgRPC_BUILD_TESTS=OFF -Dprotobuf_BUILD_TESTS=OFF -DgRPC_MSVC_STATIC_RUNTIME=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF -DgRPC_SSL_PROVIDER=module -DgRPC_ZLIB_PROVIDER=module -DgRPC_CARES_PROVIDER=module -DgRPC_RE2_PROVIDER=module -DgRPC_PROTOBUF_PROVIDER=module $Launcher_Arg"
    Invoke-Expression $Cmd
    cmake --build $BuildDir --config Release --target install

    if (-not [string]::IsNullOrEmpty($CacheDir)) {
        Write-Host "Updating Cache at $CacheDir..."
        if (Test-Path $CacheDir) { Remove-Item -Path $CacheDir -Recurse -Force }
        New-Item -ItemType Directory -Path $CacheDir | Out-Null
        Copy-Item -Path $InstallDir -Destination $CacheDir -Recurse -Force
        New-Item -ItemType File -Path (Join-Path $CacheDir "completed.marker") | Out-Null
    }
}

Write-Host "Building ProtoBridgeGenerator..."
$BuildGenDir = Join-Path $WorkDir "build_generator"
$CmdGen = "cmake -S $GeneratorSourceDir -B $BuildGenDir -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$InstallDir -DCMAKE_C_FLAGS='/w' -DCMAKE_CXX_FLAGS='/w' -Dprotobuf_MODULE_COMPATIBLE=ON -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDLL' $Launcher_Arg"
Invoke-Expression $CmdGen
cmake --build $BuildGenDir --config Release

Write-Host "Cleaning up installed packages..."
Remove-Item -Path (Join-Path $InstallDir "lib\cmake") -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $InstallDir "lib\pkgconfig") -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $InstallDir "share") -Recurse -Force -ErrorAction SilentlyContinue

Write-Host "Copying final artifacts..."
Copy-Item -Path (Join-Path $BuildGenDir "bridge_generator.exe") -Destination $FinalBinDir -Force
Copy-Item -Path (Join-Path $InstallDir "bin\*") -Destination $FinalBinDir -Recurse -Force
Copy-Item -Path (Join-Path $InstallDir "lib\*") -Destination $FinalLibDir -Recurse -Force
Copy-Item -Path (Join-Path $InstallDir "include\*") -Destination $FinalIncludeDir -Recurse -Force

Write-Host "Windows Build Complete."
