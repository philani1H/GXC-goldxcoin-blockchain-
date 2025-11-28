# C++ Miners Installation Guide

Complete guide for installing C++ miners (command-line and GUI) on all platforms.

## Overview

GXC provides two types of C++ miners:

1. **Command-Line Miners** - Terminal-based miners
   - `gxc-node` - Blockchain node
   - `gxc-miner` - Universal miner
   - `gxc-gxhash-miner` - GXHash CPU miner
   - `gxc-ethash-miner` - Ethash GPU miner
   - `gxc-sha256-miner` - SHA-256 ASIC miner
   - `gxc-pool-proxy` - Mining pool proxy

2. **GUI Miners** - Qt-based graphical applications
   - `gxc-mining-gui` - **Dedicated mining GUI** ⭐
   - `gxc-wallet` - Wallet with mining support
   - `gxc-node-gui` - Node management GUI

---

## Quick Installation

### Option 1: Pre-Built Packages (Recommended)

Download pre-built packages from `packages/` directory:

**Command-Line Miners:**
- Windows: `gxc-miners-cli-windows.zip`
- Linux: `gxc-miners-cli-linux.tar.gz`
- macOS: `gxc-miners-cli-macos.tar.gz`

**GUI Miners:**
- Windows: `gxc-miners-gui-windows.zip`
- Linux: `gxc-miners-gui-linux.tar.gz`
- macOS: `gxc-miners-gui-macos.tar.gz`

**Installation:**
1. Extract package
2. Run `install.bat` (Windows) or `bash install.sh` (Linux/macOS)
3. Start mining!

---

## Platform-Specific Installation

### Windows

#### Command-Line Miners

1. **Download Package:**
   - Get `gxc-miners-cli-windows.zip` from `packages/` directory

2. **Extract:**
   - Right-click → "Extract All"
   - Choose destination folder

3. **Install:**
   - Double-click `install.bat`
   - Or manually: Extract and run executables directly

4. **Run Miner:**
   ```powershell
   .\gxc-miner.exe --address=YOUR_ADDRESS --algorithm=gxhash
   ```

#### GUI Miners

1. **Download Package:**
   - Get `gxc-miners-gui-windows.zip` from `packages/` directory

2. **Extract and Install:**
   - Extract package
   - Run `install.bat`
   - Launch `gxc-mining-gui.exe`

3. **Requirements:**
   - Visual C++ Redistributables (usually pre-installed)
   - Qt DLLs (if not statically linked)

---

### Linux

#### Command-Line Miners

1. **Download Package:**
   ```bash
   # Get from packages directory
   tar -xzf packages/gxc-miners-cli-linux.tar.gz
   cd gxc-miners-cli-linux
   ```

2. **Install:**
   ```bash
   bash install.sh
   # Or manually:
   chmod +x gxc-*
   ```

3. **Run Miner:**
   ```bash
   ./gxc-miner --address=YOUR_ADDRESS --algorithm=gxhash
   ```

#### GUI Miners

1. **Install Qt Dependencies:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt5-default libqt5widgets5 libqt5core5a
   
   # Fedora/RHEL
   sudo dnf install qt5-qtbase qt5-qtbase-devel
   ```

2. **Download and Extract:**
   ```bash
   tar -xzf packages/gxc-miners-gui-linux.tar.gz
   cd gxc-miners-gui-linux
   bash install.sh
   ```

3. **Run GUI:**
   ```bash
   ./gxc-mining-gui
   ```

---

### macOS

#### Command-Line Miners

1. **Download Package:**
   ```bash
   tar -xzf packages/gxc-miners-cli-macos.tar.gz
   cd gxc-miners-cli-macos
   ```

2. **Install:**
   ```bash
   bash install.sh
   chmod +x gxc-*
   ```

3. **Run Miner:**
   ```bash
   ./gxc-miner --address=YOUR_ADDRESS --algorithm=gxhash
   ```

#### GUI Miners

1. **Install Qt:**
   ```bash
   brew install qt5
   ```

2. **Download and Extract:**
   ```bash
   tar -xzf packages/gxc-miners-gui-macos.tar.gz
   cd gxc-miners-gui-macos
   bash install.sh
   ```

3. **Run GUI:**
   ```bash
   ./gxc-mining-gui
   ```

---

## Building from Source

### Command-Line Miners

```bash
# Configure build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON

# Build
cmake --build . --target gxc-miner -j$(nproc)

# Or build all miners
cmake --build . --target gxc-gxhash-miner gxc-ethash-miner gxc-sha256-miner
```

### GUI Miners

```bash
# Install Qt first
# Ubuntu/Debian: sudo apt-get install qt5-default
# macOS: brew install qt5

# Configure build with GUI
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON -DBUILD_MINING_CLIENT=ON

# Build GUI miners
cmake --build . --target gxc-mining-gui
```

---

## Creating Installable Packages

To create installable packages for distribution:

### Linux/macOS:
```bash
bash scripts/package-cpp-miners.sh
```

### Windows:
```powershell
.\scripts\package-cpp-miners.ps1
```

Packages will be created in `packages/` directory.

---

## Usage Examples

### Command-Line Miners

**Universal Miner:**
```bash
./gxc-miner --address=GXC123... --algorithm=gxhash --threads=4
```

**GXHash Miner:**
```bash
./gxc-gxhash-miner --rpc-url=http://localhost:8545
```

**SHA256 Miner:**
```bash
./gxc-sha256-miner --threads=8 --benchmark
```

**Ethash GPU Miner:**
```bash
./gxc-ethash-miner --rpc-url=http://localhost:8545 --gpu
```

### GUI Miners

**Mining GUI:**
```bash
./gxc-mining-gui
```

Then in the GUI:
1. Enter wallet address
2. Select algorithm
3. Configure threads/devices
4. Click "Start Mining"

---

## Which Miner Has GUI?

### ✅ GUI Miners:
- **gxc-mining-gui** - Dedicated mining GUI application (Qt-based)
- **gxc-wallet** - Wallet application with mining tab
- **gxc-node-gui** - Node management GUI

### ❌ Command-Line Only:
- **gxc-miner** - Universal miner (CLI)
- **gxc-gxhash-miner** - GXHash miner (CLI)
- **gxc-ethash-miner** - Ethash miner (CLI)
- **gxc-sha256-miner** - SHA256 miner (CLI)
- **gxc-pool-proxy** - Pool proxy (CLI)

---

## Requirements

### Command-Line Miners
- No dependencies (standalone executables)
- Or build from source (requires CMake, compiler, OpenSSL, SQLite)

### GUI Miners
- Qt 5 or Qt 6
- Visual C++ Redistributables (Windows)
- Qt libraries (Linux/macOS)

---

## Troubleshooting

### Executable Not Found
- Ensure miners are built: `.\build-miners.ps1` or `bash scripts/build-installers.sh`
- Check build directory: `build/Release/` (Windows) or `build/` (Linux/macOS)

### GUI Won't Start
- Install Qt libraries (see platform-specific instructions)
- Check Qt installation: `qmake --version`
- Verify display server (Linux)

### Missing DLLs (Windows)
- Install Visual C++ Redistributables
- Copy Qt DLLs to executable directory (if not statically linked)

### Permission Denied (Linux/macOS)
- Make executable: `chmod +x gxc-*`
- Or run with sudo (not recommended)

---

## Comparison: Python vs C++ Miners

| Feature | Python Miner | C++ Miners |
|---------|-------------|------------|
| **GUI** | ✅ Tkinter GUI | ✅ Qt GUI (gxc-mining-gui) |
| **Installation** | ✅ pip install | ✅ Pre-built packages |
| **Performance** | Good | Excellent |
| **Dependencies** | Python 3.7+ | Standalone or Qt |
| **Cross-Platform** | ✅ Yes | ✅ Yes |
| **Standalone Executable** | ✅ PyInstaller | ✅ Native binary |

---

## Next Steps

1. **Start Blockchain Node:**
   ```bash
   ./gxc-node --rpc-port=8545 --rest-port=8080
   ```

2. **Choose Your Miner:**
   - **GUI**: `./gxc-mining-gui`
   - **CLI**: `./gxc-miner --address=YOUR_ADDRESS`

3. **Start Mining!**

For more information:
- [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) - Complete installation guide
- [MINERS_README.md](MINERS_README.md) - All miners documentation
- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Build from source

---

## Support

For issues or questions:
- Check documentation in `docs/` directory
- Open an issue on GitHub
- Contact support: support@gxc.network
