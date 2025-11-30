#!/bin/bash
# Build C++ node locally and prepare for Railway deployment

set -e

echo "=========================================="
echo "Building C++ Testnet Node for Railway"
echo "=========================================="

# Step 1: Build the node
echo "Step 1: Building C++ node..."
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF -DBUILD_TESTS=OFF
make -j$(nproc 2>/dev/null || echo 4) gxc-node

cd ..

# Step 2: Verify build
if [ ! -f "build/gxc-node" ]; then
    echo "❌ Build failed - gxc-node not found"
    exit 1
fi

echo "✅ Build successful!"
ls -lh build/gxc-node

# Step 3: Check dependencies
echo ""
echo "Step 2: Checking dependencies..."
ldd build/gxc-node 2>/dev/null | grep -E "libssl|libsqlite|libcrypto" || echo "Dependencies check complete"

echo ""
echo "=========================================="
echo "✅ Node built successfully!"
echo "=========================================="
echo ""
echo "The node is ready for Railway deployment."
echo "Railway will use Dockerfile.testnet.runtime which"
echo "copies the pre-built binary and just runs it."
echo ""
echo "To deploy to Railway:"
echo "  railway up"
echo ""
