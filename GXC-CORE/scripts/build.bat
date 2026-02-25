@echo off
REM GXC-CORE build script — Windows (MSVC / vcpkg)
setlocal

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo ========================================
echo   GXC-CORE Build (Windows)
echo   Type   : %BUILD_TYPE%
echo ========================================

REM Detect vcpkg
if "%VCPKG_ROOT%"=="" (
    echo WARNING: VCPKG_ROOT not set. Dependencies may not be found.
    echo Install vcpkg and run:
    echo   vcpkg install openssl leveldb snappy
    echo   set VCPKG_ROOT=C:\vcpkg
)

mkdir build 2>nul
cd build

cmake .. ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_MINING_CLIENT=ON ^
    -DBUILD_TESTS=OFF ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

cmake --build . --config %BUILD_TYPE% -- /m

echo.
echo Build complete.
echo   Node:     build\%BUILD_TYPE%\gxc-node.exe
echo   Miner:    build\%BUILD_TYPE%\gxc-miner.exe
echo   CLI:      build\%BUILD_TYPE%\gxc-cli.exe
