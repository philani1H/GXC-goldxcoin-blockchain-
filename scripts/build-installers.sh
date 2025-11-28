#!/bin/bash
# Build installers for all miners across all platforms
# Creates installable packages that users can download and use immediately

set -e

echo "========================================"
echo "  Building Installable Miner Packages"
echo "========================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGES_DIR="$PROJECT_ROOT/packages"
MINING_DIR="$PROJECT_ROOT/mining"
BUILD_DIR="$PROJECT_ROOT/build"

mkdir -p "$PACKAGES_DIR"

# Function to create Python standalone executable using PyInstaller
create_python_executable() {
    local platform=$1
    echo "Creating Python standalone executable for $platform..."
    
    cd "$MINING_DIR"
    
    # Check if PyInstaller is installed
    if ! command -v pyinstaller &> /dev/null; then
        echo "Installing PyInstaller..."
        pip3 install pyinstaller --user
    fi
    
    # Build executable
    if [ -f "pyinstaller.spec" ]; then
        pyinstaller --clean pyinstaller.spec
        if [ -d "dist/gxc-gxhash-miner" ]; then
            local package_name="gxc-gxhash-miner-$platform-standalone"
            cp -r "dist/gxc-gxhash-miner" "$PACKAGES_DIR/$package_name"
            echo "✅ Created standalone executable package: $package_name"
        fi
    else
        # Fallback: create executable without spec file
        pyinstaller --onefile --windowed --name gxc-gxhash-miner gxhash_miner.py
        if [ -f "dist/gxc-gxhash-miner" ] || [ -f "dist/gxc-gxhash-miner.exe" ]; then
            local package_name="gxc-gxhash-miner-$platform-standalone"
            mkdir -p "$PACKAGES_DIR/$package_name"
            cp dist/gxc-gxhash-miner* "$PACKAGES_DIR/$package_name/"
            echo "✅ Created standalone executable: $package_name"
        fi
    fi
}

# Function to package C++ miners
package_cpp_miners() {
    local platform=$1
    echo "Packaging C++ miners for $platform..."
    
    local package_name="gxc-miners-cpp-$platform"
    local package_dir="$PACKAGES_DIR/$package_name"
    
    mkdir -p "$package_dir"
    
    # Find and copy C++ miner executables
    local miners=("gxc-node" "gxc-miner" "gxc-gxhash-miner" "gxc-ethash-miner" "gxc-sha256-miner")
    
    for miner in "${miners[@]}"; do
        local exe_path=""
        if [ "$platform" = "windows" ]; then
            exe_path="$BUILD_DIR/Release/${miner}.exe"
        else
            exe_path="$BUILD_DIR/${miner}"
        fi
        
        if [ -f "$exe_path" ]; then
            cp "$exe_path" "$package_dir/"
            echo "  ✅ Copied: $miner"
        fi
    done
    
    # Create README
    cat > "$package_dir/README.md" << 'EOF'
# GXC C++ Miners - Standalone Package

## Usage

### Start Blockchain Node
```bash
./gxc-node --rpc-port=8545 --rest-port=8080
```

### Start Miner
```bash
./gxc-gxhash-miner --rpc-url=http://localhost:8545
```

## Available Miners

- `gxc-node` - Blockchain node (required)
- `gxc-miner` - Universal miner (all algorithms)
- `gxc-gxhash-miner` - GXHash CPU miner
- `gxc-ethash-miner` - Ethash GPU miner
- `gxc-sha256-miner` - SHA-256 ASIC miner

## Requirements

- GXC blockchain node running
- RPC enabled (port 8545)
EOF
    
    # Create archive
    cd "$PACKAGES_DIR"
    if [ "$platform" = "windows" ]; then
        zip -r "$package_name.zip" "$package_name" > /dev/null
    else
        tar -czf "$package_name.tar.gz" "$package_name"
    fi
    echo "✅ Created: $package_name archive"
}

# Main build process
echo "Step 1: Creating Python packages..."
echo "===================================="
"$SCRIPT_DIR/create-standalone-packages.sh"

echo ""
echo "Step 2: Creating Python standalone executables..."
echo "================================================"
if command -v python3 &> /dev/null; then
    create_python_executable "linux"
    create_python_executable "windows"
    create_python_executable "macos"
else
    echo "⚠️  Python3 not found, skipping standalone executable creation"
fi

echo ""
echo "Step 3: Packaging C++ miners..."
echo "=============================="
if [ -d "$BUILD_DIR" ]; then
    package_cpp_miners "linux"
    package_cpp_miners "windows"
else
    echo "⚠️  Build directory not found. Build miners first:"
    echo "   cd build && cmake .. -DBUILD_MINING_CLIENT=ON && make"
fi

echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"
echo ""
echo "All packages created in: $PACKAGES_DIR"
echo ""
echo "Packages ready for distribution:"
ls -lh "$PACKAGES_DIR"/*.{zip,tar.gz} 2>/dev/null || echo "  (No archives found)"
echo ""
