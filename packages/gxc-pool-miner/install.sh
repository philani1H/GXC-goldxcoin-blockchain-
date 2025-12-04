#!/bin/bash
#===============================================================================
#
#          FILE:  install.sh
#
#         USAGE:  ./install.sh
#
#   DESCRIPTION:  GXC Pool Miner Installation Script for Linux/macOS
#
#       OPTIONS:  --uninstall    Remove GXC Pool Miner
#                 --update       Update existing installation
#                 --help         Show this help
#
#  REQUIREMENTS:  Python 3.8+, pip, tkinter
#
#        AUTHOR:  GXC Blockchain
#       VERSION:  2.0.0
#       CREATED:  2024
#
#===============================================================================

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
APP_NAME="GXC Pool Miner"
APP_VERSION="2.0.0"
INSTALL_DIR="$HOME/.gxc-pool-miner"
BIN_DIR="$HOME/.local/bin"
DESKTOP_DIR="$HOME/.local/share/applications"

# Banner
print_banner() {
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
    echo "║                 POOL MINER INSTALLER v${APP_VERSION}                   ║"
    echo "║                                                               ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

log_error() {
    echo -e "${RED}[✗]${NC} $1"
}

# Check Python
check_python() {
    log_info "Checking Python installation..."
    
    if command -v python3 &> /dev/null; then
        PYTHON_CMD="python3"
    elif command -v python &> /dev/null; then
        PYTHON_CMD="python"
    else
        log_error "Python 3 is not installed!"
        echo "Please install Python 3.8 or higher:"
        echo "  Ubuntu/Debian: sudo apt install python3 python3-pip python3-tk"
        echo "  macOS: brew install python3 python-tk"
        echo "  Fedora: sudo dnf install python3 python3-pip python3-tkinter"
        exit 1
    fi
    
    # Check version
    PYTHON_VERSION=$($PYTHON_CMD -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
    PYTHON_MAJOR=$($PYTHON_CMD -c 'import sys; print(sys.version_info.major)')
    PYTHON_MINOR=$($PYTHON_CMD -c 'import sys; print(sys.version_info.minor)')
    
    if [ "$PYTHON_MAJOR" -lt 3 ] || ([ "$PYTHON_MAJOR" -eq 3 ] && [ "$PYTHON_MINOR" -lt 8 ]); then
        log_error "Python 3.8+ is required. Found: $PYTHON_VERSION"
        exit 1
    fi
    
    log_success "Python $PYTHON_VERSION found"
}

# Check pip
check_pip() {
    log_info "Checking pip..."
    
    if ! $PYTHON_CMD -m pip --version &> /dev/null; then
        log_warning "pip not found, attempting to install..."
        $PYTHON_CMD -m ensurepip --upgrade || {
            log_error "Failed to install pip"
            exit 1
        }
    fi
    
    log_success "pip is available"
}

# Check tkinter
check_tkinter() {
    log_info "Checking tkinter..."
    
    if ! $PYTHON_CMD -c "import tkinter" &> /dev/null; then
        log_error "tkinter is not installed!"
        echo "Please install tkinter:"
        echo "  Ubuntu/Debian: sudo apt install python3-tk"
        echo "  Fedora: sudo dnf install python3-tkinter"
        echo "  macOS: brew install python-tk"
        exit 1
    fi
    
    log_success "tkinter is available"
}

# Install dependencies
install_dependencies() {
    log_info "Installing dependencies..."
    
    $PYTHON_CMD -m pip install --upgrade pip
    $PYTHON_CMD -m pip install -r requirements.txt
    
    log_success "Dependencies installed"
}

# Create installation directory
create_install_dir() {
    log_info "Creating installation directory..."
    
    mkdir -p "$INSTALL_DIR"
    mkdir -p "$INSTALL_DIR/logs"
    mkdir -p "$INSTALL_DIR/config"
    mkdir -p "$BIN_DIR"
    mkdir -p "$DESKTOP_DIR"
    
    log_success "Directories created"
}

# Copy files
copy_files() {
    log_info "Copying files..."
    
    cp -r src "$INSTALL_DIR/"
    cp requirements.txt "$INSTALL_DIR/"
    cp LICENSE "$INSTALL_DIR/"
    cp README.md "$INSTALL_DIR/" 2>/dev/null || true
    
    log_success "Files copied"
}

# Create launcher script
create_launcher() {
    log_info "Creating launcher..."
    
    cat > "$BIN_DIR/gxc-pool-miner" << EOF
#!/bin/bash
# GXC Pool Miner Launcher
cd "$INSTALL_DIR"
$PYTHON_CMD -m src.gxc_pool_miner "\$@"
EOF
    
    chmod +x "$BIN_DIR/gxc-pool-miner"
    
    log_success "Launcher created: $BIN_DIR/gxc-pool-miner"
}

# Create desktop entry (Linux)
create_desktop_entry() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        log_info "Creating desktop entry..."
        
        cat > "$DESKTOP_DIR/gxc-pool-miner.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=GXC Pool Miner
Comment=Universal Mining Management System for GXC Blockchain
Exec=$BIN_DIR/gxc-pool-miner
Icon=utilities-terminal
Terminal=false
Categories=Utility;Finance;
Keywords=mining;cryptocurrency;gxc;blockchain;
EOF
        
        # Update desktop database
        if command -v update-desktop-database &> /dev/null; then
            update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
        fi
        
        log_success "Desktop entry created"
    fi
}

# Show license
show_license() {
    echo ""
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}                         LICENSE AGREEMENT                        ${NC}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "This software is provided under the MIT License."
    echo ""
    echo "IMPORTANT NOTICES:"
    echo "  • Mining involves financial risk"
    echo "  • Hardware utilization may affect device lifespan"
    echo "  • You are responsible for local law compliance"
    echo ""
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    read -p "Do you accept the license terms? [y/N] " -n 1 -r
    echo ""
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log_error "License not accepted. Installation cancelled."
        exit 1
    fi
    
    log_success "License accepted"
}

# Uninstall
uninstall() {
    log_info "Uninstalling $APP_NAME..."
    
    rm -rf "$INSTALL_DIR"
    rm -f "$BIN_DIR/gxc-pool-miner"
    rm -f "$DESKTOP_DIR/gxc-pool-miner.desktop"
    
    log_success "$APP_NAME has been uninstalled"
    exit 0
}

# Show help
show_help() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  --uninstall    Remove GXC Pool Miner"
    echo "  --update       Update existing installation"
    echo "  --help         Show this help"
    echo ""
    exit 0
}

# Main installation
install() {
    print_banner
    
    echo ""
    log_info "Starting $APP_NAME installation..."
    echo ""
    
    show_license
    
    check_python
    check_pip
    check_tkinter
    
    create_install_dir
    copy_files
    install_dependencies
    create_launcher
    create_desktop_entry
    
    echo ""
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GREEN}              INSTALLATION COMPLETED SUCCESSFULLY!                ${NC}"
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "  Installation directory: $INSTALL_DIR"
    echo ""
    echo "  To start mining:"
    echo "    gxc-pool-miner"
    echo ""
    echo "  Or add to PATH:"
    echo "    export PATH=\"\$PATH:$BIN_DIR\""
    echo ""
    echo -e "${CYAN}  Happy Mining! ⛏${NC}"
    echo ""
}

# Parse arguments
case "$1" in
    --uninstall)
        uninstall
        ;;
    --update)
        uninstall
        install
        ;;
    --help|-h)
        show_help
        ;;
    *)
        install
        ;;
esac
