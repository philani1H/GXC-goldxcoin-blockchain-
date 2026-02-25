#!/usr/bin/env bash
# GXC-CORE build script — Linux / macOS
set -euo pipefail

JOBS=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

echo "========================================"
echo "  GXC-CORE Build"
echo "  Type   : $BUILD_TYPE"
echo "  Threads: $JOBS"
echo "========================================"

# Check dependencies
check_dep() {
    command -v "$1" >/dev/null 2>&1 || { echo "ERROR: '$1' not found. Please install it."; exit 1; }
}
check_dep cmake
check_dep make || check_dep ninja

# Install system dependencies hint
if [ "$(uname)" = "Linux" ]; then
    if ! pkg-config --exists leveldb 2>/dev/null; then
        echo ""
        echo "Missing LevelDB. Run:"
        echo "  sudo apt-get install libleveldb-dev libsnappy-dev libssl-dev"
        echo ""
    fi
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_MINING_CLIENT=ON \
    -DBUILD_TESTS=OFF \
    -DENABLE_CCACHE=ON

cmake --build . -- -j"$JOBS"

echo ""
echo "Build complete. Binaries are in: $BUILD_DIR/"
echo ""
echo "  Node:        ./$BUILD_DIR/gxc-node"
echo "  Miner:       ./$BUILD_DIR/gxc-miner"
echo "  CLI:         ./$BUILD_DIR/gxc-cli"
echo "  Key gen:     ./$BUILD_DIR/gxc-keygen"
echo "  Explorer:    ./$BUILD_DIR/gxc-explorer"
echo ""
