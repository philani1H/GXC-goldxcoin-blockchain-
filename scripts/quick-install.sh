#!/bin/bash
# Quick installation script for GXC miners
# Installs Python miner with minimal user interaction

set -e

echo "========================================"
echo "  GXC Miners Quick Install"
echo "========================================"
echo ""

# Detect Python
if command -v python3 &> /dev/null; then
    PYTHON=python3
    PIP=pip3
elif command -v python &> /dev/null; then
    PYTHON=python
    PIP=pip
else
    echo "❌ Python not found!"
    echo "Please install Python 3.7+ from https://www.python.org/downloads/"
    exit 1
fi

echo "✅ Found Python: $($PYTHON --version)"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MINING_DIR="$(cd "$SCRIPT_DIR/../mining" && pwd)"

echo "Installing GXC GXHash Miner..."
echo "==============================="
echo ""

cd "$MINING_DIR"

# Install
if [ -f "setup.py" ]; then
    echo "Installing from source..."
    $PIP install -e . --user --quiet
    echo "✅ Installation complete!"
    echo ""
    echo "You can now run: gxc-gxhash-miner"
else
    echo "Installing dependencies..."
    $PIP install -r gxhash_miner_requirements.txt --user --quiet
    echo "✅ Dependencies installed!"
    echo ""
    echo "You can now run: $PYTHON gxhash_miner.py"
fi

echo ""
echo "========================================"
echo "  Quick Install Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo "  1. Start blockchain node:"
echo "     ./gxc-node --rpc-port=8545"
echo ""
echo "  2. Start miner:"
if [ -f "setup.py" ]; then
    echo "     gxc-gxhash-miner"
else
    echo "     $PYTHON gxhash_miner.py"
fi
echo ""
