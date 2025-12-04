#!/bin/bash
# GXC Blockchain Miners - Installation Script
# Installs all miners to /usr/local/bin (or custom prefix)

set -e

PREFIX="${1:-/usr/local}"
BINDIR="$PREFIX/bin"

echo "======================================"
echo " GXC Blockchain Miners - Installer"
echo "======================================"
echo ""
echo "Installation prefix: $PREFIX"
echo "Binary directory: $BINDIR"
echo ""

# Check if running as root for system-wide install
if [ "$PREFIX" = "/usr/local" ] && [ "$(id -u)" -ne 0 ]; then
    echo "Note: Installing to /usr/local requires root privileges."
    echo "Run with sudo or specify a different prefix:"
    echo "  ./install.sh ~/.local"
    echo ""
    read -p "Install to $HOME/.local instead? [Y/n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Nn]$ ]]; then
        echo "Aborting. Run with: sudo ./install.sh"
        exit 1
    fi
    PREFIX="$HOME/.local"
    BINDIR="$PREFIX/bin"
fi

# Create directories
mkdir -p "$BINDIR"

# Detect architecture
ARCH=$(uname -m)
OS=$(uname -s)

if [ "$OS" = "Linux" ]; then
    if [ "$ARCH" = "x86_64" ]; then
        PLATFORM="linux-x64"
    elif [ "$ARCH" = "aarch64" ]; then
        PLATFORM="linux-arm64"
    else
        echo "Unsupported architecture: $ARCH"
        exit 1
    fi
elif [ "$OS" = "Darwin" ]; then
    PLATFORM="macos"
else
    echo "Unsupported OS: $OS"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARIES_DIR="$SCRIPT_DIR/$PLATFORM"

if [ ! -d "$BINARIES_DIR" ]; then
    echo "ERROR: Binaries not found for platform: $PLATFORM"
    echo "Looking for: $BINARIES_DIR"
    exit 1
fi

# Install binaries
echo "Installing GXC miners..."
for binary in "$BINARIES_DIR"/gxc-*; do
    if [ -f "$binary" ]; then
        name=$(basename "$binary")
        echo "  Installing $name..."
        cp "$binary" "$BINDIR/"
        chmod +x "$BINDIR/$name"
    fi
done

echo ""
echo "======================================"
echo " Installation Complete!"
echo "======================================"
echo ""
echo "Installed binaries:"
for binary in "$BINDIR"/gxc-*; do
    if [ -f "$binary" ]; then
        echo "  - $(basename "$binary")"
    fi
done

echo ""
echo "Add to PATH if not already (add to ~/.bashrc):"
echo "  export PATH=\"$BINDIR:\$PATH\""
echo ""
echo "Quick start:"
echo "  1. Start the node:     gxc-node --testnet"
echo "  2. Run GXHash miner:   gxc-gxhash-miner --rpc-url http://localhost:8545"
echo "  3. Run SHA256 miner:   gxc-sha256-miner --rpc-url http://localhost:8545"
echo ""
