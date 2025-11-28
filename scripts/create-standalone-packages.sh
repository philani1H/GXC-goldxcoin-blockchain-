#!/bin/bash
# Create standalone installable packages for all miners
# Creates packages for Windows, Linux, and macOS

set -e

echo "========================================"
echo "  Creating Standalone Miner Packages"
echo "========================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGES_DIR="$PROJECT_ROOT/packages"
MINING_DIR="$PROJECT_ROOT/mining"

# Create packages directory
mkdir -p "$PACKAGES_DIR"

# Function to create Python package
create_python_package() {
    local platform=$1
    local package_name="gxc-gxhash-miner-$platform"
    local package_dir="$PACKAGES_DIR/$package_name"
    
    echo "Creating Python package for $platform..."
    
    mkdir -p "$package_dir"
    
    # Copy miner files
    cp "$MINING_DIR/gxhash_miner.py" "$package_dir/"
    cp "$MINING_DIR/gxhash_miner_requirements.txt" "$package_dir/requirements.txt"
    cp "$MINING_DIR/setup.py" "$package_dir/" 2>/dev/null || true
    
    # Create installation script
    if [ "$platform" = "windows" ]; then
        cat > "$package_dir/install.bat" << 'EOF'
@echo off
echo Installing GXC GXHash Miner...
echo.

python --version >nul 2>&1
if errorlevel 1 (
    echo Python not found! Please install Python 3.7+ from https://www.python.org/downloads/
    pause
    exit 1
)

echo Installing dependencies...
pip install -r requirements.txt --user
if errorlevel 1 (
    echo Installation failed!
    pause
    exit 1
)

echo.
echo ✅ Installation complete!
echo.
echo You can now run: python gxhash_miner.py
echo.
pause
EOF
    else
        cat > "$package_dir/install.sh" << 'EOF'
#!/bin/bash
echo "Installing GXC GXHash Miner..."
echo ""

if ! command -v python3 &> /dev/null; then
    echo "Python 3 not found! Please install Python 3.7+"
    exit 1
fi

echo "Installing dependencies..."
pip3 install -r requirements.txt --user

echo ""
echo "✅ Installation complete!"
echo ""
echo "You can now run: python3 gxhash_miner.py"
EOF
        chmod +x "$package_dir/install.sh"
    fi
    
    # Create README
    cat > "$package_dir/README.md" << 'EOF'
# GXC GXHash Miner - Standalone Package

## Installation

### Windows
1. Double-click `install.bat`
2. Or run manually:
   ```
   pip install -r requirements.txt --user
   python gxhash_miner.py
   ```

### Linux/macOS
1. Run: `bash install.sh`
2. Or run manually:
   ```
   pip3 install -r requirements.txt --user
   python3 gxhash_miner.py
   ```

## Usage

```bash
python gxhash_miner.py --rpc-url http://localhost:8545
```

## Options

- `--rpc-url URL` - Blockchain RPC URL (default: http://localhost:8545)
- `--rest-url URL` - Blockchain REST URL (default: http://localhost:8080)
- `--threads N` - Number of mining threads (0 = auto-detect)
- `--address ADDRESS` - Miner address for rewards
- `--daemon` - Run in background mode

## Requirements

- Python 3.7 or higher
- GXC blockchain node running
- RPC enabled on node (port 8545)
EOF
    
    # Create archive
    cd "$PACKAGES_DIR"
    if [ "$platform" = "windows" ]; then
        zip -r "$package_name.zip" "$package_name" > /dev/null
        echo "✅ Created: $package_name.zip"
    else
        tar -czf "$package_name.tar.gz" "$package_name"
        echo "✅ Created: $package_name.tar.gz"
    fi
}

# Create packages for all platforms
create_python_package "windows"
create_python_package "linux"
create_python_package "macos"

echo ""
echo "========================================"
echo "  Package Creation Complete!"
echo "========================================"
echo ""
echo "Packages created in: $PACKAGES_DIR"
echo ""
