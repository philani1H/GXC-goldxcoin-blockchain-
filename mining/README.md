# GXC GXHash Miner - Python

Cross-platform Python implementation of the GXC GXHash miner with GUI.

## Quick Installation

### Option 1: Pip Install (Recommended)

```bash
pip install -e .
```

After installation, run:
```bash
gxc-gxhash-miner
```

### Option 2: Direct Run

```bash
pip install -r gxhash_miner_requirements.txt
python gxhash_miner.py
```

### Option 3: Standalone Package

Download pre-built package from `packages/` directory and follow installation instructions.

## Features

- ✅ **Cross-Platform**: Works on Windows, Linux, and macOS
- ✅ **GUI Interface**: User-friendly graphical interface
- ✅ **Auto-Connect**: Automatically connects to blockchain node
- ✅ **Real-Time Stats**: Live hash rate, balance, and transaction tracking
- ✅ **Multi-Threaded**: Utilizes all CPU cores for maximum performance
- ✅ **No Configuration**: Works out of the box with default settings

## Requirements

- Python 3.7 or higher
- GXC blockchain node running
- RPC enabled on node (port 8545)
- REST API enabled on node (port 8080)

## Usage

### Basic Usage

1. Start GXC blockchain node:
   ```bash
   ./gxc-node --rpc-port=8545 --rest-port=8080
   ```

2. Start miner:
   ```bash
   gxc-gxhash-miner
   ```

3. In the GUI:
   - Enter your GXC wallet address
   - Adjust thread count (optional)
   - Click "START MINING"

### Command Line Options

```bash
python gxhash_miner.py [OPTIONS]
```

Options:
- `--rpc-url URL` - Blockchain RPC URL (default: http://localhost:8545)
- `--rest-url URL` - Blockchain REST URL (default: http://localhost:8080)
- `--threads N` - Number of mining threads (0 = auto-detect CPU cores)
- `--address ADDRESS` - Miner address for coinbase transactions
- `--daemon` - Run in background/daemon mode (no GUI)

### Example

```bash
python gxhash_miner.py --rpc-url http://localhost:8545 --threads 4 --address GXC1234567890abcdef
```

## How It Works

1. **Connection**: Connects to GXC blockchain node via RPC
2. **Work Retrieval**: Gets block template from blockchain
3. **Mining**: Mines blocks using GXHash algorithm
4. **Submission**: Submits valid blocks to blockchain
5. **Rewards**: Rewards automatically included in coinbase transaction

## Installation Methods

### Method 1: Development Installation

```bash
cd mining
pip install -e .
```

### Method 2: System Installation

```bash
cd mining
pip install .
```

### Method 3: User Installation

```bash
cd mining
pip install --user -e .
```

### Method 4: Standalone Executable

Create standalone executable using PyInstaller:

```bash
pip install pyinstaller
pyinstaller pyinstaller.spec
```

Executable will be in `dist/` directory.

## Troubleshooting

### Python Not Found

Install Python 3.7+ from https://www.python.org/downloads/

### Dependencies Missing

```bash
pip install -r gxhash_miner_requirements.txt
```

### Cannot Connect to Node

1. Ensure blockchain node is running
2. Check RPC port (default: 8545)
3. Verify firewall settings
4. Check node logs for errors

### GUI Not Showing

On Linux, install tkinter:
```bash
# Ubuntu/Debian
sudo apt-get install python3-tk

# Fedora/RHEL
sudo dnf install python3-tkinter

# macOS (usually pre-installed)
# If missing: brew install python-tk
```

## Building Installable Packages

To create installable packages for distribution:

```bash
# From project root
bash scripts/create-standalone-packages.sh
bash scripts/build-installers.sh
```

Packages will be created in `packages/` directory.

## Support

For more information:
- [INSTALLATION_GUIDE.md](../INSTALLATION_GUIDE.md) - Complete installation guide
- [MINERS_README.md](../MINERS_README.md) - All miners documentation
- [BUILD_INSTRUCTIONS.md](../BUILD_INSTRUCTIONS.md) - Build instructions

## License

See LICENSE file in project root.
