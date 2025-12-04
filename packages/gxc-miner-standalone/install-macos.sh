#!/bin/bash
#
# GXC Miner Installer for macOS
# One-click installation - No dependencies required
#
set -e

VERSION="2.0.0"
INSTALL_DIR="$HOME/.gxc-miner"
BIN_DIR="/usr/local/bin"

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
║                     GXC MINER INSTALLER - macOS                               ║
╚═══════════════════════════════════════════════════════════════════════════════╝
BANNER
echo -e "${NC}"

# License
echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}                        LICENSE AGREEMENT                        ${NC}"
echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  MIT License - Mining involves financial risk. No guarantees."
echo ""
read -p "  Accept terms? [y/N] " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${RED}Cancelled.${NC}"
    exit 1
fi

echo -e "${GREEN}[✓]${NC} License accepted"
echo ""

# Create directories
echo -e "${CYAN}[1/4]${NC} Creating directories..."
mkdir -p "$INSTALL_DIR"
echo -e "${GREEN}[✓]${NC} Done"

# Check if binary exists
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ -f "$SCRIPT_DIR/bin/gxc-miner" ]]; then
    echo -e "${CYAN}[2/4]${NC} Installing binary..."
    cp "$SCRIPT_DIR/bin/gxc-miner" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/gxc-miner"
else
    echo -e "${CYAN}[2/4]${NC} Building from source..."
    
    # Check Python (macOS usually has it)
    if command -v python3 &> /dev/null; then
        PYTHON=python3
    else
        # Try homebrew
        if command -v brew &> /dev/null; then
            echo -e "${YELLOW}[INFO]${NC} Installing Python via Homebrew..."
            brew install python3
            PYTHON=python3
        else
            echo -e "${RED}Error: Python 3 required.${NC}"
            echo "Install Homebrew: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            echo "Then: brew install python3"
            exit 1
        fi
    fi
    
    echo -e "${GREEN}[✓]${NC} Python: $($PYTHON --version)"
    
    $PYTHON -m pip install --quiet --upgrade pip pyinstaller 2>/dev/null || true
    
    cd "$SCRIPT_DIR/src"
    $PYTHON -m PyInstaller --onefile --name gxc-miner --clean --noconfirm --log-level ERROR gxc_miner_cli.py 2>/dev/null
    
    mv dist/gxc-miner "$INSTALL_DIR/"
    rm -rf build dist *.spec
fi

echo -e "${GREEN}[✓]${NC} Binary installed"

# Create symlink
echo -e "${CYAN}[3/4]${NC} Creating command link..."
if [[ -w "$BIN_DIR" ]]; then
    ln -sf "$INSTALL_DIR/gxc-miner" "$BIN_DIR/gxc-miner"
else
    sudo ln -sf "$INSTALL_DIR/gxc-miner" "$BIN_DIR/gxc-miner"
fi
echo -e "${GREEN}[✓]${NC} Command available: gxc-miner"

# Create app alias
echo -e "${CYAN}[4/4]${NC} Setting up..."

# Add to zsh/bash
for rc in "$HOME/.zshrc" "$HOME/.bashrc"; do
    if [[ -f "$rc" ]]; then
        if ! grep -q "gxc-miner" "$rc" 2>/dev/null; then
            echo "" >> "$rc"
            echo "# GXC Miner" >> "$rc"
            echo "alias gxc-miner='$INSTALL_DIR/gxc-miner'" >> "$rc"
        fi
    fi
done

echo -e "${GREEN}[✓]${NC} Setup complete"

echo ""
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}              INSTALLATION COMPLETE!                            ${NC}"
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  Usage:  gxc-miner -o pool.gxc.io:3333 -u YOUR_WALLET -p x"
echo "  Help:   gxc-miner --help"
echo ""
echo -e "${GREEN}  Happy Mining! ⛏${NC}"
echo ""
