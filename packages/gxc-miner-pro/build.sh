#!/bin/bash
#
# GXC MINER PRO - Build Script
# =============================
# Creates standalone executables for all platforms
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║           GXC MINER PRO - BUILD SYSTEM                        ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check for Python
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 required"
    exit 1
fi

# Install build dependencies
echo -e "${YELLOW}[1/4] Installing build dependencies...${NC}"
python3 -m pip install --upgrade pip
python3 -m pip install pyinstaller customtkinter matplotlib psutil requests

# Create __init__.py for src directory
touch src/__init__.py

# Clean previous builds
echo -e "${YELLOW}[2/4] Cleaning previous builds...${NC}"
rm -rf build dist *.egg-info

# Build with PyInstaller
echo -e "${YELLOW}[3/4] Building standalone executable...${NC}"
python3 -m PyInstaller pyinstaller.spec --clean --noconfirm

# Check if build succeeded
if [ -f "dist/GXC-Miner-Pro" ] || [ -f "dist/GXC-Miner-Pro.exe" ]; then
    echo -e "${GREEN}[✓] Build successful!${NC}"
else
    echo -e "${YELLOW}[!] PyInstaller build failed. Trying simpler build...${NC}"
    
    python3 -m PyInstaller \
        --onefile \
        --name "GXC-Miner-Pro" \
        --hidden-import customtkinter \
        --hidden-import matplotlib \
        --hidden-import psutil \
        --hidden-import requests \
        --hidden-import tkinter \
        src/gxc_miner_pro.py
fi

# Create distribution packages
echo -e "${YELLOW}[4/4] Creating distribution packages...${NC}"

mkdir -p packages

# Determine OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    OS="windows"
fi

# Copy installer
cp -r installer/* dist/ 2>/dev/null || true
cp LICENSE dist/ 2>/dev/null || true
cp README.md dist/ 2>/dev/null || true
cp requirements.txt dist/ 2>/dev/null || true
cp src/gxc_miner_pro.py dist/ 2>/dev/null || true

# Create tarball (Linux/macOS)
if [[ "$OS" != "windows" ]]; then
    cd dist
    tar -czvf "../packages/gxc-miner-pro-${OS}.tar.gz" .
    cd ..
fi

# Create zip
cd dist
zip -r "../packages/gxc-miner-pro-${OS}.zip" .
cd ..

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}BUILD COMPLETE!${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
echo ""
echo "Packages created in: packages/"
ls -la packages/
echo ""
echo "Executable location: dist/"
ls -la dist/
echo ""
