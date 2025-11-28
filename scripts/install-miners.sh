#!/bin/bash
# GXC Miners Installation Script for Linux/macOS
# Installs all miners including Python GXHash miner

set -e

echo "========================================"
echo "  GXC Miners Installation Script"
echo "========================================"
echo ""

# Detect OS
OS="$(uname -s)"
ARCH="$(uname -m)"

echo "Detected OS: $OS"
echo "Detected Architecture: $ARCH"
echo ""

# Check for Python
if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
    PIP_CMD="pip3"
elif command -v python &> /dev/null; then
    PYTHON_CMD="python"
    PIP_CMD="pip"
else
    echo "❌ Python not found! Please install Python 3.7 or higher."
    exit 1
fi

PYTHON_VERSION=$($PYTHON_CMD --version 2>&1 | awk '{print $2}')
echo "✅ Found Python: $PYTHON_VERSION"

# Check Python version (3.7+)
PYTHON_MAJOR=$(echo $PYTHON_VERSION | cut -d. -f1)
PYTHON_MINOR=$(echo $PYTHON_VERSION | cut -d. -f2)

if [ "$PYTHON_MAJOR" -lt 3 ] || ([ "$PYTHON_MAJOR" -eq 3 ] && [ "$PYTHON_MINOR" -lt 7 ]); then
    echo "❌ Python 3.7 or higher required. Found: $PYTHON_VERSION"
    exit 1
fi

# Install Python GXHash Miner
echo ""
echo "Installing Python GXHash Miner..."
echo "=================================="

cd "$(dirname "$0")/../mining"

# Install using pip
if [ -f "setup.py" ]; then
    echo "Installing from source..."
    $PIP_CMD install -e . --user
    echo "✅ Python GXHash Miner installed successfully!"
    echo ""
    echo "You can now run: gxc-gxhash-miner"
else
    echo "Installing dependencies only..."
    if [ -f "gxhash_miner_requirements.txt" ]; then
        $PIP_CMD install -r gxhash_miner_requirements.txt --user
        echo "✅ Dependencies installed!"
        echo ""
        echo "You can now run: $PYTHON_CMD gxhash_miner.py"
    fi
fi

# Check for C++ miners
echo ""
echo "Checking for C++ miners..."
echo "=========================="

if [ -f "../build/Release/gxc-gxhash-miner" ] || [ -f "../build/gxc-gxhash-miner" ]; then
    echo "✅ C++ miners found in build directory"
    echo ""
    echo "To install C++ miners system-wide, run:"
    echo "  sudo cmake --install ../build --prefix /usr/local"
else
    echo "⚠️  C++ miners not found. Build them first:"
    echo "  cd ../build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON"
    echo "  cmake --build . --target gxc-gxhash-miner"
fi

echo ""
echo "========================================"
echo "  Installation Complete!"
echo "========================================"
echo ""
echo "Quick Start:"
echo "  1. Start GXC blockchain node:"
echo "     ./gxc-node --rpc-port=8545 --rest-port=8080"
echo ""
echo "  2. Start Python miner:"
echo "     gxc-gxhash-miner"
echo "     (or: $PYTHON_CMD -m gxhash_miner)"
echo ""
echo "  3. Or start C++ miner:"
echo "     ./gxc-gxhash-miner --rpc-url=http://localhost:8545"
echo ""
