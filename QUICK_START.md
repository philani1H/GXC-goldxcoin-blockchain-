# GXC Mining - Quick Start Guide

## ✅ Packages Ready for Download!

All miners are packaged and ready to download from the `packages/` directory.

---

## Step 1: Download Miners

### Python Miner (GUI)
- **Windows**: `packages/gxc-gxhash-miner-windows.zip`
- **Linux**: `packages/gxc-gxhash-miner-linux.tar.gz`
- **macOS**: `packages/gxc-gxhash-miner-macos.tar.gz`

### C++ Miners (CLI)
- **Windows**: `packages/gxc-miners-cli-windows.zip`
- **Linux**: `packages/gxc-miners-cli-linux.tar.gz`
- **macOS**: `packages/gxc-miners-cli-macos.tar.gz`

### C++ Miners (GUI)
- **Windows**: `packages/gxc-miners-gui-windows.zip`
- **Linux**: `packages/gxc-miners-gui-linux.tar.gz`
- **macOS**: `packages/gxc-miners-gui-macos.tar.gz`

---

## Step 2: Install Miners

### Python Miner

**Windows:**
1. Extract `gxc-gxhash-miner-windows.zip`
2. Open Command Prompt in extracted folder
3. Run: `pip install -r requirements.txt`
4. Run: `python gxhash_miner.py`

**Linux/macOS:**
1. Extract `gxc-gxhash-miner-linux.tar.gz`
2. Run: `bash install.sh`
3. Run: `python3 gxhash_miner.py`

### C++ Miners

**Windows:**
1. Extract `gxc-miners-cli-windows.zip` or `gxc-miners-gui-windows.zip`
2. Run `install.bat`
3. Run miner executable

**Linux/macOS:**
1. Extract package
2. Run: `bash install.sh`
3. Run: `./gxc-miner` or `./gxc-miner-gui`

---

## Step 3: Deploy Blockchain Node

### Quick Deployment

```bash
# Run deployment script
bash scripts/deploy-blockchain.sh
```

This will:
- ✅ Install all dependencies
- ✅ Build blockchain node
- ✅ Create configuration
- ✅ Set up systemd service (Linux)
- ✅ Configure firewall

### Manual Deployment

```bash
# Build node
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target gxc-node

# Start node
./gxc-node --rpc-port=8545 --rest-port=8080
```

---

## Step 4: Start Mining!

### Using Python GUI Miner

1. Start blockchain node:
   ```bash
   ./build/gxc-node --rpc-port=8545 --rest-port=8080
   ```

2. Start Python miner:
   ```bash
   python gxhash_miner.py
   ```

3. In GUI:
   - Enter wallet address
   - Click "START MINING"

### Using C++ GUI Miner

1. Start blockchain node
2. Run: `./gxc-miner-gui` or `./gxc-gxhash-miner-gui`
3. Configure and start mining

### Using C++ CLI Miner

```bash
./gxc-miner --address=YOUR_ADDRESS --algorithm=gxhash
```

---

## All Miners Available

### CLI Miners
- `gxc-miner` - Universal miner
- `gxc-gxhash-miner` - GXHash CPU miner
- `gxc-sha256-miner` - SHA-256 ASIC miner
- `gxc-ethash-miner` - Ethash GPU miner
- `gxc-pool-proxy` - Pool proxy

### GUI Miners
- `gxc-miner-gui` - Universal miner GUI
- `gxc-gxhash-miner-gui` - GXHash miner GUI
- `gxc-sha256-miner-gui` - SHA256 miner GUI
- `gxc-ethash-miner-gui` - Ethash miner GUI
- `gxc-pool-proxy-gui` - Pool proxy GUI
- `gxc-mining-gui` - Main mining GUI
- `gxc-wallet` - Wallet with mining
- `gxc-node-gui` - Node management

### Python Miner
- `gxhash_miner.py` - Python GUI miner

---

## Quick Commands

### Start Blockchain
```bash
./build/gxc-node --rpc-port=8545 --rest-port=8080
```

### Start Mining (Python)
```bash
python gxhash_miner.py
```

### Start Mining (C++ GUI)
```bash
./gxc-miner-gui
```

### Start Mining (C++ CLI)
```bash
./gxc-miner --address=YOUR_ADDRESS
```

---

## All Packages Ready! 🎉

Download from `packages/` directory and start mining immediately!
