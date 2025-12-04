#!/bin/bash
#
# GXC MINER PRO - macOS Installer
# ================================
# Professional installation script with license agreement
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m'

# Configuration
VERSION="2.0.0"
PRODUCT_NAME="GXC Miner Pro"
INSTALL_DIR="/Applications/GXC-Miner-Pro"
BIN_LINK="/usr/local/bin/gxc-miner-pro"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

# Banner
show_banner() {
    clear
    echo -e "${GREEN}"
    cat << 'EOF'
    
   ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗ 
  ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗
  ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝
  ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗
  ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║
   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝
                                 PRO
EOF
    echo -e "${NC}"
    echo -e "${WHITE}          The Ultimate Mining Software for GXC Blockchain${NC}"
    echo -e "${CYAN}                        Version ${VERSION}${NC}"
    echo ""
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
}

# Show license
show_license() {
    echo -e "${WHITE}LICENSE AGREEMENT${NC}"
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    echo "MIT License"
    echo ""
    echo "Copyright (c) 2024-2025 GXC Foundation"
    echo ""
    echo "Permission is hereby granted, free of charge, to any person obtaining"
    echo "a copy of this software and associated documentation files (the"
    echo "\"Software\"), to deal in the Software without restriction, including"
    echo "without limitation the rights to use, copy, modify, merge, publish,"
    echo "distribute, sublicense, and/or sell copies of the Software."
    echo ""
    echo -e "${RED}MINING DISCLAIMER:${NC}"
    echo "• Cryptocurrency mining profitability is NOT guaranteed"
    echo "• Electricity costs may exceed mining rewards"
    echo "• Hardware wear and damage is the user's responsibility"
    echo "• Check local regulations regarding cryptocurrency mining"
    echo ""
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    read -p "Do you accept the license agreement? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${RED}Installation cancelled.${NC}"
        exit 1
    fi
    echo ""
}

# Check and install Homebrew if needed
check_homebrew() {
    if ! command -v brew &> /dev/null; then
        echo -e "${YELLOW}  [!] Homebrew not found. Installing...${NC}"
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
}

# Check system requirements
check_requirements() {
    echo -e "${CYAN}[1/6] Checking system requirements...${NC}"
    
    # Check macOS version
    OS_VERSION=$(sw_vers -productVersion)
    echo -e "${GREEN}  [✓] macOS ${OS_VERSION}${NC}"
    
    # Check for Python
    if command -v python3 &> /dev/null; then
        PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2)
        echo -e "${GREEN}  [✓] Python ${PYTHON_VERSION} found${NC}"
    else
        echo -e "${YELLOW}  [!] Python 3 not found. Installing...${NC}"
        check_homebrew
        brew install python3
    fi
    
    # Check for Tcl/Tk
    if ! python3 -c "import tkinter" 2>/dev/null; then
        echo -e "${YELLOW}  [!] Tcl/Tk not found. Installing...${NC}"
        check_homebrew
        brew install python-tk
    fi
    
    echo -e "${GREEN}  [✓] System requirements met${NC}"
}

# Install dependencies
install_dependencies() {
    echo -e "${CYAN}[2/6] Installing dependencies...${NC}"
    
    # Upgrade pip
    python3 -m pip install --upgrade pip --quiet 2>/dev/null || true
    
    echo "  Installing customtkinter..."
    python3 -m pip install customtkinter --quiet 2>/dev/null || true
    
    echo "  Installing matplotlib..."
    python3 -m pip install matplotlib --quiet 2>/dev/null || true
    
    echo "  Installing psutil..."
    python3 -m pip install psutil --quiet 2>/dev/null || true
    
    echo "  Installing requests..."
    python3 -m pip install requests --quiet 2>/dev/null || true
    
    echo -e "${GREEN}  [✓] Dependencies installed${NC}"
}

# Create installation directory
create_install_dir() {
    echo -e "${CYAN}[3/6] Creating installation directory...${NC}"
    
    sudo mkdir -p "$INSTALL_DIR"
    sudo chown -R $USER "$INSTALL_DIR"
    
    echo -e "${GREEN}  [✓] Created ${INSTALL_DIR}${NC}"
}

# Copy files
copy_files() {
    echo -e "${CYAN}[4/6] Installing files...${NC}"
    
    # Copy main script
    if [ -f "${PARENT_DIR}/src/gxc_miner_pro.py" ]; then
        cp "${PARENT_DIR}/src/gxc_miner_pro.py" "${INSTALL_DIR}/"
        chmod +x "${INSTALL_DIR}/gxc_miner_pro.py"
    elif [ -f "${PARENT_DIR}/dist/GXC-Miner-Pro" ]; then
        cp "${PARENT_DIR}/dist/GXC-Miner-Pro" "${INSTALL_DIR}/"
        chmod +x "${INSTALL_DIR}/GXC-Miner-Pro"
    elif [ -f "${PARENT_DIR}/dist/GXC Miner Pro.app" ]; then
        cp -R "${PARENT_DIR}/dist/GXC Miner Pro.app" "/Applications/"
    else
        echo -e "${RED}  [✗] Source files not found${NC}"
        exit 1
    fi
    
    # Copy license and readme
    [ -f "${PARENT_DIR}/LICENSE" ] && cp "${PARENT_DIR}/LICENSE" "${INSTALL_DIR}/"
    [ -f "${PARENT_DIR}/README.md" ] && cp "${PARENT_DIR}/README.md" "${INSTALL_DIR}/"
    
    echo -e "${GREEN}  [✓] Files installed${NC}"
}

# Create launcher
create_launcher() {
    echo -e "${CYAN}[5/6] Creating launcher...${NC}"
    
    # Create launcher script
    cat > "${INSTALL_DIR}/gxc-miner-pro" << 'LAUNCHER'
#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0" 2>/dev/null || echo "$0")")"
cd "$SCRIPT_DIR"

if [ -f "./GXC-Miner-Pro" ]; then
    exec ./GXC-Miner-Pro "$@"
elif [ -f "./gxc_miner_pro.py" ]; then
    exec python3 ./gxc_miner_pro.py "$@"
else
    echo "Error: GXC Miner Pro not found"
    exit 1
fi
LAUNCHER
    chmod +x "${INSTALL_DIR}/gxc-miner-pro"
    
    # Create symlink in /usr/local/bin
    sudo mkdir -p /usr/local/bin
    sudo ln -sf "${INSTALL_DIR}/gxc-miner-pro" "${BIN_LINK}" 2>/dev/null || true
    
    echo -e "${GREEN}  [✓] Launcher created${NC}"
}

# Complete installation
complete_install() {
    echo -e "${CYAN}[6/6] Completing installation...${NC}"
    echo -e "${GREEN}  [✓] Installation complete!${NC}"
}

# Show completion message
show_complete() {
    echo ""
    echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${WHITE}           GXC MINER PRO INSTALLED SUCCESSFULLY!${NC}"
    echo -e "${GREEN}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "${CYAN}Installation Location:${NC} ${INSTALL_DIR}"
    echo ""
    echo -e "${WHITE}QUICK START:${NC}"
    echo ""
    echo -e "  ${GREEN}GUI Mode:${NC}"
    echo "    gxc-miner-pro"
    echo ""
    echo -e "  ${GREEN}Pool Mining:${NC}"
    echo "    gxc-miner-pro -o stratum+tcp://pool.gxc.network:3333 -u YOUR_WALLET -p x"
    echo ""
    echo -e "  ${GREEN}Solo Mining:${NC}"
    echo "    gxc-miner-pro --solo -n http://localhost:8332 -u YOUR_WALLET"
    echo ""
    echo -e "  ${GREEN}List Devices:${NC}"
    echo "    gxc-miner-pro --list-devices"
    echo ""
    echo -e "  ${GREEN}Help:${NC}"
    echo "    gxc-miner-pro --help"
    echo ""
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}Happy Mining! For support, visit: https://docs.gxc.network${NC}"
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
}

# Uninstall
uninstall() {
    echo -e "${YELLOW}Uninstalling GXC Miner Pro...${NC}"
    
    sudo rm -rf "${INSTALL_DIR}" 2>/dev/null || true
    sudo rm -f "${BIN_LINK}" 2>/dev/null || true
    sudo rm -rf "/Applications/GXC Miner Pro.app" 2>/dev/null || true
    
    echo -e "${GREEN}Uninstalled successfully.${NC}"
}

# Main
main() {
    if [ "$1" == "--uninstall" ]; then
        uninstall
        exit 0
    fi
    
    show_banner
    show_license
    check_requirements
    install_dependencies
    create_install_dir
    copy_files
    create_launcher
    complete_install
    show_complete
}

main "$@"
