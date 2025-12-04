#!/bin/bash
# GXC Blockchain Miners - Installation Script
# Standalone installer - No dependencies required!
# Just like installing VSCode - download and run.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION="2.0.0"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_banner() {
    echo -e "${GREEN}"
    echo '╔═══════════════════════════════════════════════════════════╗'
    echo '║                                                           ║'
    echo '║     ██████╗ ██╗  ██╗ ██████╗    ██████╗ ██╗     ██╗      ║'
    echo '║    ██╔════╝ ╚██╗██╔╝██╔════╝    ██╔══██╗██║     ██║      ║'
    echo '║    ██║  ███╗ ╚███╔╝ ██║         ██████╔╝██║     ██║      ║'
    echo '║    ██║   ██║ ██╔██╗ ██║         ██╔══██╗██║     ██║      ║'
    echo '║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██████╔╝███████╗██║      ║'
    echo '║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═════╝ ╚══════╝╚═╝      ║'
    echo '║                                                           ║'
    echo "║         GXC Blockchain Miners Installer v${VERSION}           ║"
    echo '║                  Standalone Edition                       ║'
    echo '║                                                           ║'
    echo '╚═══════════════════════════════════════════════════════════╝'
    echo -e "${NC}"
}

print_banner

# Detect platform
ARCH=$(uname -m)
OS=$(uname -s)

echo -e "${BLUE}Detected: ${OS} ${ARCH}${NC}"
echo ""

# Determine binary directory
if [ "$OS" = "Linux" ]; then
    if [ "$ARCH" = "x86_64" ]; then
        PLATFORM="linux-x64"
    elif [ "$ARCH" = "aarch64" ]; then
        PLATFORM="linux-arm64"
    else
        echo -e "${RED}Unsupported architecture: ${ARCH}${NC}"
        exit 1
    fi
elif [ "$OS" = "Darwin" ]; then
    PLATFORM="macos"
else
    echo -e "${RED}Unsupported OS: ${OS}${NC}"
    exit 1
fi

BINARIES_DIR="$SCRIPT_DIR/$PLATFORM"

if [ ! -d "$BINARIES_DIR" ]; then
    echo -e "${RED}ERROR: Binaries not found for platform: ${PLATFORM}${NC}"
    echo "Looking for: $BINARIES_DIR"
    exit 1
fi

# Installation options
echo "Installation Options:"
echo "  1) System-wide (/usr/local/bin) - requires sudo"
echo "  2) User-local (~/.local/bin) - no sudo needed"
echo "  3) Current directory (./bin) - portable"
echo ""
read -p "Select option [1-3] (default: 2): " CHOICE
CHOICE=${CHOICE:-2}

case $CHOICE in
    1)
        INSTALL_DIR="/usr/local/bin"
        NEED_SUDO=true
        ;;
    2)
        INSTALL_DIR="$HOME/.local/bin"
        NEED_SUDO=false
        ;;
    3)
        INSTALL_DIR="$SCRIPT_DIR/bin"
        NEED_SUDO=false
        ;;
    *)
        echo -e "${RED}Invalid option${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${YELLOW}Installing to: ${INSTALL_DIR}${NC}"
echo ""

# Create directory
if [ "$NEED_SUDO" = true ]; then
    sudo mkdir -p "$INSTALL_DIR"
else
    mkdir -p "$INSTALL_DIR"
fi

# Install binaries
echo -e "${BLUE}Installing GXC miners and tools...${NC}"
echo ""

INSTALLED=0
for binary in "$BINARIES_DIR"/gxc-*; do
    if [ -f "$binary" ]; then
        name=$(basename "$binary")
        echo -e "  ${GREEN}✓${NC} Installing $name..."
        if [ "$NEED_SUDO" = true ]; then
            sudo cp "$binary" "$INSTALL_DIR/"
            sudo chmod +x "$INSTALL_DIR/$name"
        else
            cp "$binary" "$INSTALL_DIR/"
            chmod +x "$INSTALL_DIR/$name"
        fi
        ((INSTALLED++))
    fi
done

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}  Installation Complete! ${INSTALLED} binaries installed.${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════${NC}"
echo ""

# Check if path needs updating
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo -e "${YELLOW}NOTE: Add to your PATH:${NC}"
    echo ""
    echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
    echo ""
    echo "Add this line to ~/.bashrc or ~/.zshrc for permanent access."
    echo ""
fi

echo -e "${BLUE}Installed binaries:${NC}"
echo ""
echo "  MINERS:"
echo "    gxc-miner          - Universal miner (all algorithms)"
echo "    gxc-gxhash-miner   - CPU miner (GXHash)"
echo "    gxc-sha256-miner   - ASIC miner (SHA-256)"
echo "    gxc-ethash-miner   - GPU miner (Ethash)"
echo "    gxc-pool-proxy     - Mining pool proxy"
echo ""
echo "  BLOCKCHAIN:"
echo "    gxc-node           - Full blockchain node"
echo "    gxc-cli            - Command-line interface"
echo ""
echo "  TOOLS:"
echo "    gxc-keygen         - Key/wallet generator"
echo "    gxc-tx             - Transaction builder"
echo "    gxc-explorer       - Block explorer"
echo "    gxc-netdiag        - Network diagnostics"
echo ""

echo -e "${GREEN}Quick Start:${NC}"
echo ""
echo "  1. Start the blockchain node:"
echo "     gxc-node --testnet"
echo ""
echo "  2. Run GXHash miner:"
echo "     gxc-gxhash-miner --help"
echo ""
echo "  3. Run universal miner:"
echo "     gxc-miner -a YOUR_ADDRESS --algorithm=gxhash"
echo ""
echo -e "${GREEN}Thank you for choosing GXC Blockchain!${NC}"
echo ""
