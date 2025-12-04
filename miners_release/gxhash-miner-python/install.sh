#!/bin/bash
# GXC GXHash Miner - Python Installation Script
# Cross-platform installer for Linux and macOS

set -e

echo "======================================"
echo " GXC GXHash Miner - Python Installer"
echo "======================================"
echo ""

# Check Python version
PYTHON=""
if command -v python3 &> /dev/null; then
    PYTHON="python3"
elif command -v python &> /dev/null; then
    PYTHON="python"
else
    echo "ERROR: Python 3.7+ is required but not found."
    echo "Please install Python from https://www.python.org/downloads/"
    exit 1
fi

# Verify Python version
PYTHON_VERSION=$($PYTHON -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
echo "Found Python $PYTHON_VERSION"

MAJOR=$($PYTHON -c 'import sys; print(sys.version_info.major)')
MINOR=$($PYTHON -c 'import sys; print(sys.version_info.minor)')

if [ "$MAJOR" -lt 3 ] || ([ "$MAJOR" -eq 3 ] && [ "$MINOR" -lt 7 ]); then
    echo "ERROR: Python 3.7+ is required (found $PYTHON_VERSION)"
    exit 1
fi

# Install dependencies
echo ""
echo "Installing dependencies..."
$PYTHON -m pip install --upgrade pip
$PYTHON -m pip install -r requirements.txt

# Install the miner package
echo ""
echo "Installing GXC GXHash Miner..."
$PYTHON -m pip install -e .

echo ""
echo "======================================"
echo " Installation Complete!"
echo "======================================"
echo ""
echo "Run the miner with:"
echo "  gxc-gxhash-miner"
echo ""
echo "Or directly:"
echo "  $PYTHON gxhash_miner.py"
echo ""
