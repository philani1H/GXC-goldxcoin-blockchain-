#!/bin/bash
#
# GXC Miner - Build Standalone Executables
# Creates self-contained binaries that don't require Python
#
set -e

VERSION="2.0.0"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$ROOT_DIR/src"
BIN_DIR="$ROOT_DIR/bin"
DIST_DIR="$ROOT_DIR/dist"

# Colors
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║           GXC MINER - STANDALONE BUILD                        ║"
echo "║           Building self-contained executables                 ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check Python
echo -e "${CYAN}[1/5]${NC} Checking Python..."
if command -v python3 &> /dev/null; then
    PYTHON=python3
elif command -v python &> /dev/null; then
    PYTHON=python
else
    echo "Error: Python 3 required to build"
    exit 1
fi
echo -e "${GREEN}[✓]${NC} Python: $($PYTHON --version)"

# Install dependencies
echo -e "${CYAN}[2/5]${NC} Installing build dependencies..."
$PYTHON -m pip install --quiet --upgrade pip
$PYTHON -m pip install --quiet pyinstaller

# Clean previous builds
echo -e "${CYAN}[3/5]${NC} Cleaning previous builds..."
rm -rf "$BIN_DIR" "$DIST_DIR" build *.spec
mkdir -p "$BIN_DIR" "$DIST_DIR"

# Detect OS
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

if [[ "$OS" == "darwin" ]]; then
    OS="macos"
    EXT=""
elif [[ "$OS" == "linux" ]]; then
    EXT=""
else
    OS="windows"
    EXT=".exe"
fi

echo -e "${CYAN}[4/5]${NC} Building executables for ${OS}-${ARCH}..."

# Build CLI miner
echo "  Building gxc-miner (CLI)..."
cd "$SRC_DIR"
$PYTHON -m PyInstaller \
    --onefile \
    --name "gxc-miner" \
    --clean \
    --noconfirm \
    --log-level WARN \
    gxc_miner_cli.py

mv dist/gxc-miner* "$BIN_DIR/"

# Clean PyInstaller artifacts
rm -rf build dist *.spec

echo -e "${GREEN}[✓]${NC} Built: $BIN_DIR/gxc-miner$EXT"

# Create distribution packages
echo -e "${CYAN}[5/5]${NC} Creating distribution packages..."

cd "$ROOT_DIR"

# Copy README and LICENSE
cat > "$BIN_DIR/README.txt" << 'EOF'
GXC MINER v2.0.0
================

USAGE:
  ./gxc-miner -o stratum+tcp://pool.gxc.io:3333 -u YOUR_WALLET -p x

OPTIONS:
  -o, --url      Pool URL (required)
  -u, --user     Wallet address (required)
  -p, --pass     Pool password (default: x)
  -a, --algo     Algorithm: gxhash, sha256, ethash (default: gxhash)
  -t, --threads  Number of threads (default: auto)
  -w, --worker   Worker name (default: hostname)
  -v, --verbose  Verbose output

EXAMPLES:
  ./gxc-miner -o pool.gxc.io:3333 -u GXC1abc... -p x
  ./gxc-miner -o localhost:3333 -u tGXC... --algo sha256 -t 4

ALGORITHMS:
  gxhash  - CPU mining, ASIC resistant (recommended)
  sha256  - Double SHA256, ASIC/CPU
  ethash  - Keccak256, GPU optimized

For more information: https://gxc.network

MIT License - Copyright (c) 2024 GXC Blockchain
EOF

cat > "$BIN_DIR/LICENSE" << 'EOF'
MIT License

Copyright (c) 2024 GXC Blockchain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

DISCLAIMER: Mining involves financial risk. No profit guarantee.
EOF

# Create tar.gz and zip
PACKAGE_NAME="gxc-miner-${VERSION}-${OS}-${ARCH}"

cd "$BIN_DIR"
tar -czvf "$DIST_DIR/${PACKAGE_NAME}.tar.gz" .
zip -r "$DIST_DIR/${PACKAGE_NAME}.zip" .

echo ""
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}                    BUILD COMPLETE!                             ${NC}"
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  Executables:  $BIN_DIR/"
echo "  Packages:     $DIST_DIR/"
echo ""
ls -lh "$DIST_DIR/"
echo ""
echo "  Test:  $BIN_DIR/gxc-miner --help"
echo ""
