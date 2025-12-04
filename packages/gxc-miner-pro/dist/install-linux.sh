#!/bin/bash
#
# GXC MINER PRO - Linux Installer
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
NC='\033[0m' # No Color

# Configuration
VERSION="2.0.0"
PRODUCT_NAME="GXC Miner Pro"
INSTALL_DIR="/opt/gxc-miner-pro"
BIN_LINK="/usr/local/bin/gxc-miner-pro"
DESKTOP_FILE="/usr/share/applications/gxc-miner-pro.desktop"
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

# Check system requirements
check_requirements() {
    echo -e "${CYAN}[1/6] Checking system requirements...${NC}"
    
    # Check if running as root for system install
    if [ "$EUID" -ne 0 ]; then
        echo -e "${YELLOW}[!] Not running as root. Installing to user directory.${NC}"
        INSTALL_DIR="$HOME/.local/share/gxc-miner-pro"
        BIN_LINK="$HOME/.local/bin/gxc-miner-pro"
        DESKTOP_FILE="$HOME/.local/share/applications/gxc-miner-pro.desktop"
        mkdir -p "$HOME/.local/bin"
        mkdir -p "$HOME/.local/share/applications"
    fi
    
    # Check for Python
    if command -v python3 &> /dev/null; then
        PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2)
        echo -e "${GREEN}  [✓] Python ${PYTHON_VERSION} found${NC}"
    else
        echo -e "${YELLOW}  [!] Python 3 not found. Installing...${NC}"
        if command -v apt-get &> /dev/null; then
            sudo apt-get update && sudo apt-get install -y python3 python3-pip python3-tk
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y python3 python3-pip python3-tkinter
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm python python-pip tk
        else
            echo -e "${RED}  [✗] Cannot install Python. Please install manually.${NC}"
            exit 1
        fi
    fi
    
    echo -e "${GREEN}  [✓] System requirements met${NC}"
}

# Install dependencies
install_dependencies() {
    echo -e "${CYAN}[2/6] Installing dependencies...${NC}"
    
    # Upgrade pip
    python3 -m pip install --upgrade pip --quiet 2>/dev/null || true
    
    # Install required packages
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
    
    mkdir -p "$INSTALL_DIR"
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
    else
        echo -e "${RED}  [✗] Source files not found${NC}"
        exit 1
    fi
    
    # Copy license
    if [ -f "${PARENT_DIR}/LICENSE" ]; then
        cp "${PARENT_DIR}/LICENSE" "${INSTALL_DIR}/"
    fi
    
    # Copy README
    if [ -f "${PARENT_DIR}/README.md" ]; then
        cp "${PARENT_DIR}/README.md" "${INSTALL_DIR}/"
    fi
    
    echo -e "${GREEN}  [✓] Files installed${NC}"
}

# Create launcher script and links
create_launcher() {
    echo -e "${CYAN}[5/6] Creating launcher...${NC}"
    
    # Create launcher script
    cat > "${INSTALL_DIR}/gxc-miner-pro" << 'LAUNCHER'
#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
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
    
    # Create symlink
    ln -sf "${INSTALL_DIR}/gxc-miner-pro" "${BIN_LINK}" 2>/dev/null || true
    
    # Create desktop entry
    cat > "${DESKTOP_FILE}" << DESKTOP
[Desktop Entry]
Name=GXC Miner Pro
Comment=Ultimate Mining Software for GXC Blockchain
Exec=${INSTALL_DIR}/gxc-miner-pro
Icon=utilities-terminal
Terminal=false
Type=Application
Categories=Utility;Finance;
Keywords=mining;cryptocurrency;gxc;blockchain;
DESKTOP
    
    echo -e "${GREEN}  [✓] Launcher created${NC}"
}

# Complete installation
complete_install() {
    echo -e "${CYAN}[6/6] Completing installation...${NC}"
    
    # Update desktop database
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database "$(dirname "$DESKTOP_FILE")" 2>/dev/null || true
    fi
    
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

# Uninstall function
uninstall() {
    echo -e "${YELLOW}Uninstalling GXC Miner Pro...${NC}"
    
    rm -rf "${INSTALL_DIR}" 2>/dev/null || true
    rm -f "${BIN_LINK}" 2>/dev/null || true
    rm -f "${DESKTOP_FILE}" 2>/dev/null || true
    
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
