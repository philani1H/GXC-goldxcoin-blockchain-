#!/bin/bash
# Install dependencies for building GXC miners

echo "========================================"
echo "  Installing Build Dependencies"
echo "========================================"
echo ""

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Detected: Linux"
    
    if command -v apt-get &> /dev/null; then
        echo "Installing dependencies (Ubuntu/Debian)..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            libssl-dev \
            libsqlite3-dev \
            qt5-default \
            libqt5widgets5 \
            python3 \
            python3-pip
    elif command -v yum &> /dev/null; then
        echo "Installing dependencies (RHEL/CentOS)..."
        sudo yum install -y \
            gcc-c++ \
            cmake \
            openssl-devel \
            sqlite-devel \
            qt5-qtbase-devel \
            python3 \
            python3-pip
    elif command -v dnf &> /dev/null; then
        echo "Installing dependencies (Fedora)..."
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            openssl-devel \
            sqlite-devel \
            qt5-qtbase-devel \
            python3 \
            python3-pip
    fi
    
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected: macOS"
    
    if command -v brew &> /dev/null; then
        echo "Installing dependencies (Homebrew)..."
        brew install cmake openssl sqlite qt5 python3
    else
        echo "❌ Homebrew not found. Install from https://brew.sh"
        exit 1
    fi
    
else
    echo "❌ Unsupported OS: $OSTYPE"
    exit 1
fi

echo ""
echo "✅ Dependencies installed!"
echo ""
echo "You can now run: bash scripts/build-all-miners.sh"
echo ""
