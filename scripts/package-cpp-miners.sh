#!/bin/bash
# Package C++ miners for installation
# Creates installable packages for all C++ miners (command-line and GUI)

set -e

echo "========================================"
echo "  Packaging C++ Miners"
echo "========================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGES_DIR="$PROJECT_ROOT/packages"
BUILD_DIR="$PROJECT_ROOT/build"

mkdir -p "$PACKAGES_DIR"

# Detect platform
OS="$(uname -s)"
if [[ "$OS" == "Linux" ]]; then
    PLATFORM="linux"
    EXE_EXT=""
    EXE_DIR=""
elif [[ "$OS" == "Darwin" ]]; then
    PLATFORM="macos"
    EXE_EXT=""
    EXE_DIR=""
elif [[ "$OS" == *"MINGW"* ]] || [[ "$OS" == *"MSYS"* ]] || [[ "$OS" == *"CYGWIN"* ]]; then
    PLATFORM="windows"
    EXE_EXT=".exe"
    EXE_DIR="Release/"
else
    PLATFORM="linux"
    EXE_EXT=""
    EXE_DIR=""
fi

echo "Platform: $PLATFORM"
echo ""

# Function to package command-line miners
package_cli_miners() {
    local package_name="gxc-miners-cli-$PLATFORM"
    local package_dir="$PACKAGES_DIR/$package_name"
    
    echo "Packaging command-line miners..."
    echo "================================"
    
    if [ -d "$package_dir" ]; then
        rm -rf "$package_dir"
    fi
    mkdir -p "$package_dir"
    
    # List of command-line miners
    local miners=(
        "gxc-node"
        "gxc-miner"
        "gxc-gxhash-miner"
        "gxc-ethash-miner"
        "gxc-sha256-miner"
        "gxc-pool-proxy"
    )
    
    local found_count=0
    
    for miner in "${miners[@]}"; do
        local exe_path="$BUILD_DIR/$EXE_DIR${miner}${EXE_EXT}"
        
        if [ -f "$exe_path" ]; then
            cp "$exe_path" "$package_dir/"
            echo "  ✅ Copied: $miner"
            found_count=$((found_count + 1))
        else
            echo "  ⚠️  Not found: $miner"
        fi
    done
    
    if [ $found_count -eq 0 ]; then
        echo "❌ No miners found! Build them first:"
        echo "   cd build && cmake .. -DBUILD_MINING_CLIENT=ON && make"
        rm -rf "$package_dir"
        return 1
    fi
    
    # Create README
    cat > "$package_dir/README.md" << 'EOF'
# GXC Command-Line Miners

## Available Miners

- **gxc-node** - Blockchain node (required for mining)
- **gxc-miner** - Universal miner (supports all algorithms)
- **gxc-gxhash-miner** - GXHash CPU miner
- **gxc-ethash-miner** - Ethash GPU miner (NVIDIA/AMD)
- **gxc-sha256-miner** - SHA-256 ASIC miner
- **gxc-pool-proxy** - Mining pool proxy

## Quick Start

### 1. Start Blockchain Node

```bash
./gxc-node --rpc-port=8545 --rest-port=8080
```

### 2. Start Miner

**Universal Miner:**
```bash
./gxc-miner --address=YOUR_ADDRESS --algorithm=gxhash
```

**GXHash Miner:**
```bash
./gxc-gxhash-miner --rpc-url=http://localhost:8545
```

**SHA256 Miner:**
```bash
./gxc-sha256-miner --threads=8
```

**Ethash Miner (GPU):**
```bash
./gxc-ethash-miner --rpc-url=http://localhost:8545 --gpu
```

## Installation

### Linux/macOS

1. Extract package:
   ```bash
   tar -xzf gxc-miners-cli-linux.tar.gz
   cd gxc-miners-cli-linux
   ```

2. Make executables:
   ```bash
   chmod +x gxc-*
   ```

3. Run miner (see Quick Start above)

### Windows

1. Extract package:
   - Right-click `gxc-miners-cli-windows.zip`
   - Select "Extract All"
   - Choose destination folder

2. Run miner:
   - Double-click `gxc-miner.exe` or
   - Open Command Prompt in folder and run: `gxc-miner.exe --address=YOUR_ADDRESS`

## Requirements

- GXC blockchain node running
- RPC enabled on node (port 8545)
- REST API enabled on node (port 8080)

## Documentation

For detailed usage, see:
- [MINERS_README.md](../../MINERS_README.md)
- [INSTALLATION_GUIDE.md](../../INSTALLATION_GUIDE.md)
EOF
    
    # Create installation script
    if [ "$PLATFORM" != "windows" ]; then
        cat > "$package_dir/install.sh" << 'EOF'
#!/bin/bash
echo "Installing GXC Command-Line Miners..."
echo ""

# Make all executables
chmod +x gxc-*

echo "✅ Installation complete!"
echo ""
echo "Miners are ready to use. See README.md for usage instructions."
EOF
        chmod +x "$package_dir/install.sh"
    else
        cat > "$package_dir/install.bat" << 'EOF'
@echo off
echo Installing GXC Command-Line Miners...
echo.

echo ✅ Installation complete!
echo.
echo Miners are ready to use. See README.md for usage instructions.
echo.
pause
EOF
    fi
    
    # Create archive
    cd "$PACKAGES_DIR"
    if [ "$PLATFORM" = "windows" ]; then
        zip -r "$package_name.zip" "$package_name" > /dev/null
        echo "✅ Created: $package_name.zip"
    else
        tar -czf "$package_name.tar.gz" "$package_name"
        echo "✅ Created: $package_name.tar.gz"
    fi
    
    echo ""
}

# Function to package GUI miners
package_gui_miners() {
    local package_name="gxc-miners-gui-$PLATFORM"
    local package_dir="$PACKAGES_DIR/$package_name"
    
    echo "Packaging GUI miners..."
    echo "======================="
    
    if [ -d "$package_dir" ]; then
        rm -rf "$package_dir"
    fi
    mkdir -p "$package_dir"
    
    # List of GUI applications
    local gui_apps=(
        "gxc-wallet"
        "gxc-node-gui"
        "gxc-mining-gui"
    )
    
    local found_count=0
    
    for app in "${gui_apps[@]}"; do
        local exe_path="$BUILD_DIR/$EXE_DIR${app}${EXE_EXT}"
        
        if [ -f "$exe_path" ]; then
            cp "$exe_path" "$package_dir/"
            echo "  ✅ Copied: $app"
            found_count=$((found_count + 1))
        else
            echo "  ⚠️  Not found: $app (may not be built with Qt)"
        fi
    done
    
    if [ $found_count -eq 0 ]; then
        echo "⚠️  No GUI applications found!"
        echo "   Build with Qt: cmake .. -DBUILD_GUI=ON -DBUILD_MINING_CLIENT=ON"
        rm -rf "$package_dir"
        return 1
    fi
    
    # Create README
    cat > "$package_dir/README.md" << 'EOF'
# GXC GUI Miners

## Available Applications

- **gxc-wallet** - Main wallet GUI with mining support
- **gxc-node-gui** - Node management GUI
- **gxc-mining-gui** - Dedicated mining GUI application

## Quick Start

### gxc-mining-gui (Mining GUI)

1. Start blockchain node:
   ```bash
   ./gxc-node --rpc-port=8545
   ```

2. Launch mining GUI:
   ```bash
   ./gxc-mining-gui
   ```

3. In the GUI:
   - Enter your wallet address
   - Select mining algorithm
   - Configure threads/devices
   - Click "Start Mining"

### gxc-wallet (Wallet with Mining)

1. Launch wallet:
   ```bash
   ./gxc-wallet
   ```

2. Navigate to Mining tab
3. Configure and start mining

### gxc-node-gui (Node Management)

1. Launch node GUI:
   ```bash
   ./gxc-node-gui
   ```

2. Monitor blockchain status
3. View network statistics

## Installation

### Linux

1. Extract package:
   ```bash
   tar -xzf gxc-miners-gui-linux.tar.gz
   cd gxc-miners-gui-linux
   ```

2. Make executables:
   ```bash
   chmod +x gxc-*
   ```

3. Run application:
   ```bash
   ./gxc-mining-gui
   ```

**Note:** You may need Qt libraries installed:
```bash
# Ubuntu/Debian
sudo apt-get install qt5-default libqt5widgets5

# Fedora/RHEL
sudo dnf install qt5-qtbase-devel
```

### macOS

1. Extract package:
   ```bash
   tar -xzf gxc-miners-gui-macos.tar.gz
   cd gxc-miners-gui-macos
   ```

2. Make executables:
   ```bash
   chmod +x gxc-*
   ```

3. Run application:
   ```bash
   ./gxc-mining-gui
   ```

**Note:** You may need Qt installed:
```bash
brew install qt5
```

### Windows

1. Extract package:
   - Right-click `gxc-miners-gui-windows.zip`
   - Select "Extract All"

2. Run application:
   - Double-click `gxc-mining-gui.exe`

**Note:** You may need Visual C++ Redistributables and Qt DLLs.

## Requirements

- Qt 5 or Qt 6 (for GUI applications)
- GXC blockchain node running (for mining)
- RPC enabled on node (port 8545)

## Troubleshooting

### Application Won't Start

**Linux:** Install Qt libraries (see Installation section)

**macOS:** Install Qt via Homebrew

**Windows:** Ensure Visual C++ Redistributables are installed

### GUI Not Showing

- Check Qt installation
- Verify display server is running (Linux)
- Check application logs

## Documentation

For more information:
- [MINERS_README.md](../../MINERS_README.md)
- [INSTALLATION_GUIDE.md](../../INSTALLATION_GUIDE.md)
EOF
    
    # Create installation script
    if [ "$PLATFORM" != "windows" ]; then
        cat > "$package_dir/install.sh" << 'EOF'
#!/bin/bash
echo "Installing GXC GUI Miners..."
echo ""

# Make all executables
chmod +x gxc-*

echo "✅ Installation complete!"
echo ""
echo "GUI applications are ready to use."
echo ""
echo "Note: You may need Qt libraries installed."
echo "See README.md for details."
EOF
        chmod +x "$package_dir/install.sh"
    else
        cat > "$package_dir/install.bat" << 'EOF'
@echo off
echo Installing GXC GUI Miners...
echo.

echo ✅ Installation complete!
echo.
echo GUI applications are ready to use.
echo.
echo Note: You may need Qt libraries and Visual C++ Redistributables.
echo See README.md for details.
echo.
pause
EOF
    fi
    
    # Create archive
    cd "$PACKAGES_DIR"
    if [ "$PLATFORM" = "windows" ]; then
        zip -r "$package_name.zip" "$package_name" > /dev/null
        echo "✅ Created: $package_name.zip"
    else
        tar -czf "$package_name.tar.gz" "$package_name"
        echo "✅ Created: $package_name.tar.gz"
    fi
    
    echo ""
}

# Main packaging
package_cli_miners
package_gui_miners

echo "========================================"
echo "  Packaging Complete!"
echo "========================================"
echo ""
echo "Packages created in: $PACKAGES_DIR"
echo ""
