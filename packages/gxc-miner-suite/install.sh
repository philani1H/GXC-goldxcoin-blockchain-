#!/bin/bash
# GXC Miner Suite Installer for Linux/macOS

set -e

APP_NAME="GXC Miner Suite"
VERSION="2.0.0"
INSTALL_DIR="$HOME/.gxc-miner-suite"
BIN_DIR="$HOME/.local/bin"

# Colors
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                                                               ║"
echo "║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗     ║"
echo "║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║     ║"
echo "║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║     ║"
echo "║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║     ║"
echo "║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║     ║"
echo "║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝     ║"
echo "║                                                               ║"
echo "║              MINER SUITE INSTALLER v${VERSION}                     ║"
echo "║           SHA256 • Ethash • GxHash Mining                     ║"
echo "║                                                               ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check Python
echo -e "${CYAN}[INFO]${NC} Checking Python..."
if command -v python3 &> /dev/null; then
    PYTHON=python3
elif command -v python &> /dev/null; then
    PYTHON=python
else
    echo -e "${RED}[ERROR]${NC} Python 3 not found!"
    echo "Install: sudo apt install python3 python3-pip python3-tk"
    exit 1
fi
echo -e "${GREEN}[✓]${NC} Python found: $($PYTHON --version)"

# License
echo ""
echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}                        LICENSE AGREEMENT                        ${NC}"
echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "This software is provided under the MIT License."
echo ""
echo "⚠  MINING RISKS:"
echo "   • Financial risk from hardware/electricity costs"
echo "   • Hardware may experience increased wear"
echo "   • No profit guarantees"
echo ""
read -p "Accept license terms? [y/N] " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${RED}License not accepted. Cancelled.${NC}"
    exit 1
fi
echo -e "${GREEN}[✓]${NC} License accepted"

# Install
echo -e "${CYAN}[INFO]${NC} Installing..."

mkdir -p "$INSTALL_DIR" "$BIN_DIR"
cp -r src "$INSTALL_DIR/"
cp requirements.txt LICENSE README.md "$INSTALL_DIR/" 2>/dev/null || true

# Install dependencies
echo -e "${CYAN}[INFO]${NC} Installing dependencies..."
$PYTHON -m pip install --upgrade pip
$PYTHON -m pip install -r requirements.txt

# Create launcher
cat > "$BIN_DIR/gxc-miner" << EOF
#!/bin/bash
cd "$INSTALL_DIR"
$PYTHON -m src.gxc_miner "\$@"
EOF
chmod +x "$BIN_DIR/gxc-miner"

# Desktop entry (Linux)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    mkdir -p "$HOME/.local/share/applications"
    cat > "$HOME/.local/share/applications/gxc-miner.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=GXC Miner Suite
Comment=SHA256, Ethash, GxHash Mining
Exec=$BIN_DIR/gxc-miner
Terminal=false
Categories=Utility;Finance;
EOF
fi

echo ""
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}              INSTALLATION COMPLETE!                            ${NC}"
echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  Run:  gxc-miner"
echo ""
echo "  Or add to PATH:"
echo "    export PATH=\"\$PATH:$BIN_DIR\""
echo ""
echo -e "${CYAN}  Happy Mining! ⛏${NC}"
