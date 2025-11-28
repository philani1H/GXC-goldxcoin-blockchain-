# GXC Blockchain Build Instructions

## Quick Start

### Windows

1. **Install Prerequisites:**
   ```powershell
   # Option 1: Using Chocolatey (recommended)
   choco install cmake visualstudio2022community openssl
   
   # Option 2: Manual installation
   # - CMake: https://cmake.org/download/
   # - Visual Studio 2022: https://visualstudio.microsoft.com/downloads/
   #   (Install "Desktop development with C++" workload)
   # - OpenSSL: https://slproweb.com/products/Win32OpenSSL.html
   ```

2. **Run Build Script:**
   ```powershell
   .\build-blockchain.ps1
   ```

3. **Or Build Manually:**
   ```powershell
   # Open Developer Command Prompt for VS 2022
   # Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
   
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release --target gxc-node -j 4
   ```

### Linux

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev libsqlite3-dev

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node
```

### macOS

```bash
# Install dependencies
brew install cmake openssl sqlite3

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu) gxc-node
```

## Build Options

### CMake Configuration Options

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \      # Release or Debug
  -DBUILD_MINING_CLIENT=ON \        # Build mining software
  -DBUILD_GUI=ON \                  # Build Qt GUI (requires Qt6)
  -DBUILD_TESTS=OFF \               # Skip tests
  -DENABLE_LTO=OFF                  # Link Time Optimization
```

### Build Targets

- `gxc-node` - Main blockchain node (required)
- `gxc-cli` - Command-line interface
- `gxc-keygen` - Key generation tool
- `gxc-miner` - Universal miner
- `gxc-gxhash-miner` - CPU miner
- `gxc-ethash-miner` - GPU miner
- `gxc-sha256-miner` - ASIC miner

## Troubleshooting

### CMake Not Found
- Install CMake from https://cmake.org/download/
- Or use package manager: `choco install cmake` (Windows), `apt install cmake` (Linux)

### Compiler Not Found
- **Windows**: Install Visual Studio 2022 with C++ workload
- **Linux**: `sudo apt-get install build-essential`
- **macOS**: Install Xcode Command Line Tools: `xcode-select --install`

### OpenSSL Not Found
- **Windows**: Download from https://slproweb.com/products/Win32OpenSSL.html
- **Linux**: `sudo apt-get install libssl-dev`
- **macOS**: `brew install openssl`

### SQLite Not Found
- **Windows**: Usually included with Visual Studio
- **Linux**: `sudo apt-get install libsqlite3-dev`
- **macOS**: `brew install sqlite3`

### Build Errors
1. Check all dependencies are installed
2. Ensure you're using a supported compiler (GCC 9+, Clang 10+, MSVC 2019+)
3. Try a clean build: `rm -rf build && mkdir build`
4. Check CMake output for specific missing dependencies

## Running the Node

After building, run:

```bash
# Windows
.\build\Release\gxc-node.exe

# Linux/macOS
./build/gxc-node
```

### Command Line Options

```bash
gxc-node [options]

Options:
  --help, -h                 Show help message
  --config=FILE, -c FILE     Configuration file path
  --datadir=DIR, -d DIR      Data directory path
  --port=PORT, -p PORT       Network port (default: 9333)
  --rpc-port=PORT           RPC port (default: 8332)
  --rest-port=PORT           REST API port (default: 8080)
  --testnet                  Use testnet configuration
  --daemon                   Run as daemon
  --verbose, -v              Verbose logging
  --quiet, -q                Quiet mode (errors only)
```

## Next Steps

After building:
1. Configure the node: See `config/gxc.conf.example`
2. Run the node: `./build/gxc-node --testnet`
3. Check logs: `logs/gxc.log`
4. Connect to RPC: `http://localhost:8545`

For deployment, see `BLOCKCHAIN_DEPLOYMENT_GUIDE.md`

