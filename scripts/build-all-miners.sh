#!/bin/bash
# Master script to build and package ALL miners
# Builds C++ miners, packages Python miner, creates all installable packages

set -e

echo "========================================"
echo "  Building ALL GXC Miners"
echo "========================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
PACKAGES_DIR="$PROJECT_ROOT/packages"

# Create directories
mkdir -p "$BUILD_DIR"
mkdir -p "$PACKAGES_DIR"

# Step 1: Build C++ miners
echo "Step 1: Building C++ Miners..."
echo "=============================="
cd "$BUILD_DIR"

if [ ! -f "CMakeCache.txt" ]; then
    echo "Configuring CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON -DBUILD_GUI=ON -DBUILD_TESTS=OFF || {
        echo "⚠️  GUI build failed, trying without GUI..."
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON -DBUILD_TESTS=OFF
    }
fi

echo "Building all miners..."
cmake --build . --target gxc-node -j$(nproc) || echo "⚠️  gxc-node build failed"
cmake --build . --target gxc-miner -j$(nproc) || echo "⚠️  gxc-miner build failed"
cmake --build . --target gxc-gxhash-miner -j$(nproc) || echo "⚠️  gxc-gxhash-miner build failed"
cmake --build . --target gxc-ethash-miner -j$(nproc) || echo "⚠️  gxc-ethash-miner build failed"
cmake --build . --target gxc-sha256-miner -j$(nproc) || echo "⚠️  gxc-sha256-miner build failed"
cmake --build . --target gxc-pool-proxy -j$(nproc) || echo "⚠️  gxc-pool-proxy build failed"

    # Build GUI miners if Qt is available
    echo ""
    echo "Building GUI miners (if Qt available)..."
    cmake --build . --target gxc-mining-gui -j$(nproc) || echo "⚠️  gxc-mining-gui build failed (Qt may not be available)"
    cmake --build . --target gxc-wallet -j$(nproc) || echo "⚠️  gxc-wallet build failed (Qt may not be available)"
    cmake --build . --target gxc-node-gui -j$(nproc) || echo "⚠️  gxc-node-gui build failed (Qt may not be available)"
    cmake --build . --target gxc-miner-gui -j$(nproc) || echo "⚠️  gxc-miner-gui build failed (Qt may not be available)"
    cmake --build . --target gxc-gxhash-miner-gui -j$(nproc) || echo "⚠️  gxc-gxhash-miner-gui build failed (Qt may not be available)"
    cmake --build . --target gxc-sha256-miner-gui -j$(nproc) || echo "⚠️  gxc-sha256-miner-gui build failed (Qt may not be available)"
    cmake --build . --target gxc-ethash-miner-gui -j$(nproc) || echo "⚠️  gxc-ethash-miner-gui build failed (Qt may not be available)"
    cmake --build . --target gxc-pool-proxy-gui -j$(nproc) || echo "⚠️  gxc-pool-proxy-gui build failed (Qt may not be available)"

cd "$PROJECT_ROOT"

# Step 2: Create Python packages
echo ""
echo "Step 2: Creating Python Packages..."
echo "===================================="
if [ -f "$SCRIPT_DIR/create-standalone-packages.sh" ]; then
    bash "$SCRIPT_DIR/create-standalone-packages.sh"
else
    echo "⚠️  Python package script not found"
fi

# Step 3: Package C++ miners
echo ""
echo "Step 3: Packaging C++ Miners..."
echo "==============================="
if [ -f "$SCRIPT_DIR/package-cpp-miners.sh" ]; then
    bash "$SCRIPT_DIR/package-cpp-miners.sh"
else
    echo "⚠️  C++ package script not found"
fi

# Step 4: Create Python standalone executables (optional)
echo ""
echo "Step 4: Creating Python Standalone Executables (optional)..."
echo "============================================================"
if command -v python3 &> /dev/null && command -v pyinstaller &> /dev/null; then
    cd "$PROJECT_ROOT/mining"
    if [ -f "pyinstaller.spec" ]; then
        pyinstaller --clean pyinstaller.spec || echo "⚠️  PyInstaller failed"
    fi
    cd "$PROJECT_ROOT"
else
    echo "⚠️  Python3 or PyInstaller not found, skipping standalone executable"
fi

# Step 5: Summary
echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"
echo ""
echo "Packages created in: $PACKAGES_DIR"
echo ""
echo "Available packages:"
ls -lh "$PACKAGES_DIR"/*.{zip,tar.gz} 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}' || echo "  (No packages found)"
echo ""
echo "Built executables in: $BUILD_DIR"
echo ""
ls -lh "$BUILD_DIR"/gxc-* 2>/dev/null | awk '{print "  " $9}' || echo "  (No executables found)"
echo ""
