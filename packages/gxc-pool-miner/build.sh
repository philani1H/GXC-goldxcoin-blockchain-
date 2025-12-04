#!/bin/bash
#===============================================================================
#
#          FILE:  build.sh
#
#         USAGE:  ./build.sh [target]
#
#   DESCRIPTION:  Build GXC Pool Miner packages and executables
#
#       OPTIONS:  all       - Build all targets
#                 exe       - Build standalone executable
#                 pip       - Build pip package
#                 zip       - Create distribution zip
#                 clean     - Clean build artifacts
#
#===============================================================================

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# Configuration
APP_NAME="gxc-pool-miner"
VERSION="2.0.0"
BUILD_DIR="build"
DIST_DIR="dist"

log_info() { echo -e "${CYAN}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[✓]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[!]${NC} $1"; }
log_error() { echo -e "${RED}[✗]${NC} $1"; }

# Print banner
print_banner() {
    echo -e "${CYAN}"
    echo "╔═══════════════════════════════════════════════════════════════╗"
    echo "║            GXC POOL MINER BUILD SYSTEM v${VERSION}                 ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

# Clean build artifacts
clean() {
    log_info "Cleaning build artifacts..."
    rm -rf "$BUILD_DIR"
    rm -rf "$DIST_DIR"
    rm -rf *.egg-info
    rm -rf __pycache__
    rm -rf src/__pycache__
    rm -rf .pytest_cache
    find . -name "*.pyc" -delete
    find . -name "*.pyo" -delete
    log_success "Clean complete"
}

# Build pip package
build_pip() {
    log_info "Building pip package..."
    
    python3 -m pip install --upgrade build
    python3 -m build
    
    log_success "Pip package built: $DIST_DIR/"
}

# Build standalone executable
build_exe() {
    log_info "Building standalone executable..."
    
    # Install PyInstaller if needed
    python3 -m pip install --upgrade pyinstaller
    
    # Build
    pyinstaller pyinstaller.spec --clean
    
    log_success "Executable built: $DIST_DIR/$APP_NAME"
}

# Create distribution zip
build_zip() {
    log_info "Creating distribution archive..."
    
    mkdir -p "$DIST_DIR"
    
    # Determine OS
    OS=$(uname -s | tr '[:upper:]' '[:lower:]')
    ARCH=$(uname -m)
    
    ZIP_NAME="${APP_NAME}-${VERSION}-${OS}-${ARCH}"
    
    # Create temp directory for packaging
    TEMP_DIR=$(mktemp -d)
    mkdir -p "$TEMP_DIR/$APP_NAME"
    
    # Copy files
    cp -r src "$TEMP_DIR/$APP_NAME/"
    cp requirements.txt "$TEMP_DIR/$APP_NAME/"
    cp install.sh "$TEMP_DIR/$APP_NAME/"
    cp install.bat "$TEMP_DIR/$APP_NAME/"
    cp LICENSE "$TEMP_DIR/$APP_NAME/"
    cp README.md "$TEMP_DIR/$APP_NAME/"
    cp setup.py "$TEMP_DIR/$APP_NAME/"
    
    # Create zip
    cd "$TEMP_DIR"
    zip -r "$ZIP_NAME.zip" "$APP_NAME"
    mv "$ZIP_NAME.zip" "$OLDPWD/$DIST_DIR/"
    cd "$OLDPWD"
    
    # Create tar.gz
    cd "$TEMP_DIR"
    tar -czvf "$ZIP_NAME.tar.gz" "$APP_NAME"
    mv "$ZIP_NAME.tar.gz" "$OLDPWD/$DIST_DIR/"
    cd "$OLDPWD"
    
    # Cleanup
    rm -rf "$TEMP_DIR"
    
    log_success "Archives created:"
    log_success "  - $DIST_DIR/$ZIP_NAME.zip"
    log_success "  - $DIST_DIR/$ZIP_NAME.tar.gz"
}

# Build all
build_all() {
    clean
    build_pip
    build_exe
    build_zip
    
    echo ""
    log_success "All builds completed!"
    echo ""
    echo "Build artifacts:"
    ls -la "$DIST_DIR/"
}

# Verify installation
verify() {
    log_info "Verifying installation..."
    
    python3 -c "from src.gxc_pool_miner import GXCPoolMinerApp; print('✓ Import successful')"
    
    log_success "Verification passed"
}

# Main
print_banner

case "${1:-all}" in
    clean)
        clean
        ;;
    pip)
        build_pip
        ;;
    exe)
        build_exe
        ;;
    zip)
        build_zip
        ;;
    verify)
        verify
        ;;
    all)
        build_all
        ;;
    *)
        echo "Usage: $0 {all|exe|pip|zip|clean|verify}"
        exit 1
        ;;
esac
