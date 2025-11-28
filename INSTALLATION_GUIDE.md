# GXC Miners Installation Guide

Complete guide for installing all GXC miners on Windows, Linux, and macOS.

## Quick Installation

### Option 1: Python Package (Recommended - Cross-Platform)

The Python GXHash miner can be installed via pip:

```bash
# Install from source
cd mining
pip install -e .

# Or install directly
pip install gxc-gxhash-miner
```

After installation, run:
```bash
gxc-gxhash-miner
```

### Option 2: Standalone Packages

Download pre-built packages from the `packages/` directory:

- **Windows**: `gxc-gxhash-miner-windows.zip`
- **Linux**: `gxc-gxhash-miner-linux.tar.gz`
- **macOS**: `gxc-gxhash-miner-macos.tar.gz`

Extract and run the installation script:
- Windows: Double-click `install.bat`
- Linux/macOS: Run `bash install.sh`

### Option 3: Standalone Executables

For systems without Python installed, download standalone executables:

- **Windows**: `gxc-gxhash-miner-windows-standalone.zip`
- **Linux**: `gxc-gxhash-miner-linux-standalone.tar.gz`
- **macOS**: `gxc-gxhash-miner-macos-standalone.tar.gz`

These are self-contained executables that don't require Python.

---

## Platform-Specific Installation

### Windows

#### Method 1: Automated Installation Script

```powershell
.\scripts\install-miners.ps1
```

This will:
- Detect Python installation
- Install Python GXHash miner
- Set up all dependencies
- Provide instructions for C++ miners

#### Method 2: Manual Installation

1. **Install Python** (if not already installed):
   - Download from https://www.python.org/downloads/
   - Version 3.7 or higher required
   - Check "Add Python to PATH" during installation

2. **Install Python Miner**:
   ```powershell
   cd mining
   pip install -e .
   ```

3. **Run Miner**:
   ```powershell
   gxc-gxhash-miner
   ```

#### Method 3: C++ Miners

1. **Build Miners**:
   ```powershell
   .\build-miners.ps1
   ```

2. **Run Miner**:
   ```powershell
   .\build\Release\gxc-gxhash-miner.exe --rpc-url=http://localhost:8545
   ```

---

### Linux

#### Method 1: Automated Installation Script

```bash
bash scripts/install-miners.sh
```

#### Method 2: Manual Installation

1. **Install Python** (if needed):
   ```bash
   sudo apt-get update
   sudo apt-get install python3 python3-pip
   ```

2. **Install Python Miner**:
   ```bash
   cd mining
   pip3 install -e . --user
   ```

3. **Run Miner**:
   ```bash
   gxc-gxhash-miner
   ```

#### Method 3: C++ Miners

1. **Install Dependencies**:
   ```bash
   sudo apt-get install build-essential cmake libssl-dev libsqlite3-dev
   ```

2. **Build Miners**:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON
   cmake --build . --target gxc-gxhash-miner -j$(nproc)
   ```

3. **Run Miner**:
   ```bash
   ./gxc-gxhash-miner --rpc-url=http://localhost:8545
   ```

---

### macOS

#### Method 1: Automated Installation Script

```bash
bash scripts/install-miners.sh
```

#### Method 2: Manual Installation

1. **Install Python** (if needed):
   ```bash
   brew install python3
   ```

2. **Install Python Miner**:
   ```bash
   cd mining
   pip3 install -e . --user
   ```

3. **Run Miner**:
   ```bash
   gxc-gxhash-miner
   ```

#### Method 3: C++ Miners

1. **Install Dependencies**:
   ```bash
   brew install cmake openssl sqlite
   ```

2. **Build Miners**:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON
   cmake --build . --target gxc-gxhash-miner
   ```

3. **Run Miner**:
   ```bash
   ./gxc-gxhash-miner --rpc-url=http://localhost:8545
   ```

---

## Creating Installable Packages

To create installable packages for distribution:

### All Platforms

```bash
# Linux/macOS
bash scripts/create-standalone-packages.sh
bash scripts/build-installers.sh

# Windows
.\scripts\create-standalone-packages.ps1
.\scripts\build-installers.ps1
```

This creates:
- Python source packages (zip/tar.gz)
- Standalone executables (PyInstaller)
- C++ miner packages

All packages are created in the `packages/` directory.

---

## Creating Standalone Executables (PyInstaller)

For Python miners, you can create standalone executables that don't require Python:

### Install PyInstaller

```bash
pip install pyinstaller
```

### Build Executable

```bash
cd mining
pyinstaller --onefile --windowed --name gxc-gxhash-miner gxhash_miner.py
```

Or use the spec file:

```bash
pyinstaller pyinstaller.spec
```

The executable will be in `dist/` directory.

---

## Verification

After installation, verify everything works:

1. **Check Python Miner**:
   ```bash
   gxc-gxhash-miner --help
   ```

2. **Check C++ Miners**:
   ```bash
   ./gxc-gxhash-miner --help
   ```

3. **Test Connection**:
   - Start blockchain node: `./gxc-node --rpc-port=8545`
   - Start miner: `gxc-gxhash-miner`

---

## Troubleshooting

### Python Not Found

**Windows**: Add Python to PATH or reinstall with "Add to PATH" checked.

**Linux/macOS**: Install Python 3.7+:
```bash
# Linux
sudo apt-get install python3 python3-pip

# macOS
brew install python3
```

### Dependencies Missing

Install required Python packages:
```bash
pip install -r mining/gxhash_miner_requirements.txt
```

### C++ Build Errors

Ensure all dependencies are installed:
- CMake 3.16+
- C++ compiler (GCC/Clang/MSVC)
- OpenSSL development libraries
- SQLite3 development libraries

### Permission Errors

Use `--user` flag for pip installs:
```bash
pip install -e . --user
```

---

## Next Steps

1. **Start Blockchain Node**:
   ```bash
   ./gxc-node --rpc-port=8545 --rest-port=8080
   ```

2. **Start Mining**:
   ```bash
   gxc-gxhash-miner
   ```

3. **Configure Miner**:
   - Enter your GXC wallet address
   - Adjust thread count
   - Click "START MINING"

For more information, see:
- [MINERS_README.md](MINERS_README.md) - Detailed miner documentation
- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Build instructions
- [MINING_GUIDE.md](docs/MINING.md) - Mining guide

---

## Support

For issues or questions:
- Check documentation in `docs/` directory
- Open an issue on GitHub
- Contact support: support@gxc.network
