#!/bin/bash
# Railway build script for C++ testnet node
# This script builds the C++ node for Railway deployment

set -e

echo "=========================================="
echo "Building C++ Testnet Node for Railway"
echo "=========================================="

# Install dependencies
echo "Installing dependencies..."
apt-get update
apt-get install -y build-essential cmake libssl-dev libsqlite3-dev pkg-config

# Build the node
echo "Building gxc-node..."
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF -DBUILD_TESTS=OFF
make -j$(nproc) gxc-node

# Copy to app root
cp gxc-node /app/gxc-node

echo "Build complete!"
ls -lh /app/gxc-node
