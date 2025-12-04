#!/bin/bash
#
# GXC Miner Installer for Linux
# One-click installation - No dependencies required
#
set -e

VERSION="2.0.0"
INSTALL_DIR="$HOME/.gxc-miner"
BIN_DIR="$HOME/.local/bin"

# Colors
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${CYAN}"
cat << 'BANNER'
╔═══════════════════════════════════════════════════════════════════════════════╗
║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗      ║
║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗     ║
║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝     ║
║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗     ║
║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║     ║
║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝     ║
╠═══════════════════════════════════════════════════════════════════════════════╣
║                         GXC MINER INSTALLER                                   ║
║                    Version 2.0.0 - Pool Mining                                ║
╚═══════════════════════════════════════════════════════════════════════════════╝
BANNER
echo -e "${NC}"

# License agreement
echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}                        LICENSE AGREEMENT                        ${NC}"
echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  This software is provided under the MIT License."
echo ""
echo "  ⚠  IMPORTANT MINING DISCLAIMER:"
echo ""
echo "  • Mining cryptocurrency involves financial risk"
echo "  • No profit or earnings are guaranteed"
echo "  • Electricity and hardware costs apply"
echo "  • You are solely responsible for compliance with local laws"
echo ""
read -p "  Do you accept these terms? [y/N] " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${RED}Installation cancelled.${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}[✓]${NC} License accepted"
echo ""

# Create directories
echo -e "${CYAN}[1/4]${NC} Creating directories..."
mkdir -p "$INSTALL_DIR" "$BIN_DIR"
echo -e "${GREEN}[✓]${NC} Directories created"

# Check if standalone binary exists in current dir
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ -f "$SCRIPT_DIR/bin/gxc-miner" ]]; then
    echo -e "${CYAN}[2/4]${NC} Installing pre-built binary..."
    cp "$SCRIPT_DIR/bin/gxc-miner" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/gxc-miner"
    
elif [[ -f "$SCRIPT_DIR/src/gxc_miner_cli.py" ]]; then
    # Need to build from source
    echo -e "${CYAN}[2/4]${NC} Building from source..."
    
    # Check Python
    if command -v python3 &> /dev/null; then
        PYTHON=python3
    elif command -v python &> /dev/null; then
        PYTHON=python
    else
        echo -e "${YELLOW}[INFO]${NC} Installing Python..."
        
        # Try package managers
        if command -v apt-get &> /dev/null; then
            sudo apt-get update && sudo apt-get install -y python3 python3-pip
            PYTHON=python3
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y python3 python3-pip
            PYTHON=python3
        elif command -v yum &> /dev/null; then
            sudo yum install -y python3 python3-pip
            PYTHON=python3
        elif command -v pacman &> /dev/null; then
            sudo pacman -Sy python python-pip
            PYTHON=python3
        else
            echo -e "${RED}Error: Could not install Python. Please install manually.${NC}"
            exit 1
        fi
    fi
    
    echo -e "${GREEN}[✓]${NC} Python: $($PYTHON --version)"
    
    # Install PyInstaller and build
    $PYTHON -m pip install --quiet --upgrade pip pyinstaller 2>/dev/null || true
    
    cd "$SCRIPT_DIR/src"
    $PYTHON -m PyInstaller --onefile --name gxc-miner --clean --noconfirm --log-level ERROR gxc_miner_cli.py 2>/dev/null
    
    mv dist/gxc-miner "$INSTALL_DIR/"
    rm -rf build dist *.spec
    
else
    echo -e "${RED}Error: No source or binary found to install.${NC}"
    exit 1
fi

echo -e "${GREEN}[✓]${NC} Binary installed"

# Create symlink
echo -e "${CYAN}[3/4]${NC} Creating command link..."
ln -sf "$INSTALL_DIR/gxc-miner" "$BIN_DIR/gxc-miner"
echo -e "${GREEN}[✓]${NC} Command link created"

# Add to PATH if needed
echo -e "${CYAN}[4/4]${NC} Checking PATH..."

if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    # Add to shell config
    SHELL_CONFIG=""
    if [[ -f "$HOME/.bashrc" ]]; then
        SHELL_CONFIG="$HOME/.bashrc"
    elif [[ -f "$HOME/.zshrc" ]]; then
        SHELL_CONFIG="$HOME/.zshrc"
    elif [[ -f "$HOME/.profile" ]]; then
        SHELL_CONFIG="$HOME/.profile"
    fi
    
    if [[ -n "$SHELL_CONFIG" ]]; then
        echo "" >> "$SHELL_CONFIG"
        echo "# GXC Miner" >> "$SHELL_CONFIG"
        echo "export PATH=\"\$PATH:$BIN_DIR\"" >> "$SHELL_CONFIG"
        echo -e "${GREEN}[✓]${NC} Added to PATH in $SHELL_CONFIG"
    fi
fi

# Done
echo ""
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}              INSTALLATION COMPLETE!                            ${NC}"
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  Location:   $INSTALL_DIR/gxc-miner"
echo "  Command:    gxc-miner"
echo ""
echo "  ┌─────────────────────────────────────────────────────────────┐"
echo "  │ QUICK START:                                                │"
echo "  │                                                             │"
echo "  │   gxc-miner -o pool.gxc.io:3333 -u YOUR_WALLET -p x        │"
echo "  │                                                             │"
echo "  │ OPTIONS:                                                    │"
echo "  │   -a gxhash   CPU mining (default)                         │"
echo "  │   -a sha256   ASIC/CPU mining                              │"
echo "  │   -a ethash   GPU mining                                   │"
echo "  │   -t 4        Use 4 threads                                │"
echo "  │                                                             │"
echo "  │ HELP:                                                       │"
echo "  │   gxc-miner --help                                         │"
echo "  └─────────────────────────────────────────────────────────────┘"
echo ""
echo -e "${CYAN}  If 'gxc-miner' command not found, run: source ~/.bashrc${NC}"
echo ""
echo -e "${GREEN}  Happy Mining! ⛏${NC}"
echo ""
