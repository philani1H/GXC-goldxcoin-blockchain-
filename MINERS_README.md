# GXC Mining Software - Download & Usage Guide

## Overview

This guide covers all GXC mining software, including how to build, download, and use each miner.

---

## Available Miners

### 1. **GXC Node** (`gxc-node`)
**Required for all mining operations**

The blockchain node that provides:
- RPC API (port 8545)
- REST API (port 8080)
- Network connectivity
- Block validation

**Build:**
```bash
.\build-blockchain.ps1
# Or manually:
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target gxc-node
```

**Run:**
```bash
.\build\Release\gxc-node.exe --rpc-port=8545 --rest-port=8080
```

---

### 2. **GXHash CPU Miner** (Python)

**Location:** `mining/gxhash_miner.py`

**Features:**
- ✅ Connects directly to blockchain node
- ✅ Gets real block templates from blockchain
- ✅ Submits mined blocks to blockchain
- ✅ No manual endpoints - everything automatic
- ✅ No auto-transfers - rewards come from blockchain
- ✅ Background/daemon mode support
- ✅ Multi-threaded CPU mining

**Installation:**
```bash
cd mining
pip install -r gxhash_miner_requirements.txt
```

**Usage:**
```bash
# Basic mining
python gxhash_miner.py

# With custom RPC URL
python gxhash_miner.py --rpc-url http://localhost:8545

# With thread count
python gxhash_miner.py --threads 4

# With miner address (for coinbase rewards)
python gxhash_miner.py --address GXC...

# Background/daemon mode
python gxhash_miner.py --daemon
```

**Options:**
- `--rpc-url URL` - Blockchain RPC URL (default: http://localhost:8545)
- `--rest-url URL` - Blockchain REST URL (default: http://localhost:8080)
- `--threads N` - Number of mining threads (0 = auto-detect CPU cores)
- `--address ADDRESS` - Miner address for coinbase transactions (optional)
- `--daemon` - Run in background/daemon mode

**How It Works:**
1. Connects to blockchain node via RPC
2. Gets block template from blockchain
3. Mines blocks using GXHash algorithm
4. Submits valid blocks to blockchain
5. Rewards are automatically included in coinbase transaction
6. No manual transfers needed - everything handled by blockchain

---

### 3. **GXHash CPU Miner** (C++)

**Location:** Built as `gxc-gxhash-miner`

**Build:**
```bash
.\build-miners.ps1
# Or manually:
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON
cmake --build . --target gxc-gxhash-miner
```

**Usage:**
```bash
.\build\Release\gxc-gxhash-miner.exe --rpc-url=http://localhost:8545
```

---

### 4. **Ethash GPU Miner** (C++)

**Location:** Built as `gxc-ethash-miner`

**Requirements:**
- NVIDIA GPU with CUDA support, OR
- AMD GPU with OpenCL support

**Build:**
```bash
.\build-miners.ps1
# Or manually:
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON
cmake --build . --target gxc-ethash-miner
```

**Usage:**
```bash
.\build\Release\gxc-ethash-miner.exe --rpc-url=http://localhost:8545 --gpu
```

---

### 5. **SHA-256 ASIC Miner** (C++)

**Location:** Built as `gxc-sha256-miner`

**Requirements:**
- ASIC mining hardware (Antminer, WhatsMiner, etc.)

**Build:**
```bash
.\build-miners.ps1
# Or manually:
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON
cmake --build . --target gxc-sha256-miner
```

**Usage:**
```bash
.\build\Release\gxc-sha256-miner.exe --rpc-url=http://localhost:8545 --device=0
```

---

### 6. **Universal Miner** (C++)

**Location:** Built as `gxc-miner`

**Features:**
- Supports all algorithms (GXHash, Ethash, SHA-256)
- Auto-detects best algorithm for hardware
- Pool mining support

**Build:**
```bash
.\build-miners.ps1
# Or manually:
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON
cmake --build . --target gxc-miner
```

**Usage:**
```bash
.\build\Release\gxc-miner.exe --rpc-url=http://localhost:8545 --algorithm=gxhash
```

---

## Building All Miners

### Quick Build (All Miners)
```powershell
.\build-miners.ps1
```

This will:
1. Build all C++ miners
2. Package Python miner
3. Create release directory with all miners
4. Generate README files

### Output
All miners will be in `miners_release/` directory:
```
miners_release/
├── gxc-node.exe
├── gxc-miner.exe
├── gxc-gxhash-miner.exe
├── gxc-ethash-miner.exe
├── gxc-sha256-miner.exe
├── gxhash-miner-python/
│   ├── gxhash_miner.py
│   ├── requirements.txt
│   ├── README.md
│   ├── run_miner.bat
│   └── run_miner.sh
└── README.md
```

---

## Python GXHash Miner - Detailed Guide

### Architecture

The Python GXHash miner is designed to work seamlessly with the blockchain:

1. **Connection Phase:**
   - Connects to blockchain node via RPC (port 8545)
   - Verifies connection by getting blockchain info
   - Gets mining information (difficulty, etc.)

2. **Mining Phase:**
   - Requests block template from blockchain
   - Extracts block data (prev_hash, merkle_root, difficulty, etc.)
   - Mines using GXHash algorithm
   - Checks if hash meets difficulty target

3. **Submission Phase:**
   - When valid block found, constructs complete block
   - Adds coinbase transaction (if miner address provided)
   - Submits block to blockchain via RPC
   - Blockchain validates and adds block
   - Rewards automatically included in coinbase

4. **Monitoring Phase:**
   - Updates work every 30 seconds
   - Prints statistics every 10 seconds
   - Handles errors gracefully

### Key Features

✅ **No Manual Endpoints:**
- All communication via standard RPC/REST APIs
- No custom endpoints needed
- Works with any GXC node

✅ **No Auto-Transfers:**
- Rewards come from blockchain coinbase transactions
- No separate transfer mechanism
- Everything handled by blockchain consensus

✅ **Background Mode:**
- Can run as daemon/service
- Logs to file (if configured)
- Graceful shutdown on signals

✅ **Real Blockchain Integration:**
- Gets real block templates
- Submits real blocks
- All data from blockchain

### Example Output

```
Connecting to GXC blockchain node...
✅ Connected to blockchain
   Chain: main
   Blocks: 12345
   Difficulty: 4.5

⚡ Starting GXHash miner...
   Threads: 4
   Miner Address: GXC...
   RPC URL: http://localhost:8545
   REST URL: http://localhost:8080

✅ Mining started! Press Ctrl+C to stop.

Thread 0 started (nonce range: 0 - 1073741824)
Thread 1 started (nonce range: 1073741824 - 2147483648)
Thread 2 started (nonce range: 2147483648 - 3221225472)
Thread 3 started (nonce range: 3221225472 - 4294967296)

[14:30:15] Hash Rate: 125.50 KH/s | Total Hashes: 1,255,000 | Blocks: 0 | Accepted: 0 | Rejected: 0
[14:30:25] Hash Rate: 128.30 KH/s | Total Hashes: 2,538,000 | Blocks: 0 | Accepted: 0 | Rejected: 0

🎉 Block found by thread 2!
   Hash: 0000abc123def456...
   Nonce: 2147485000
✅ Block submitted and accepted!

[14:30:35] Hash Rate: 130.20 KH/s | Total Hashes: 3,840,000 | Blocks: 1 | Accepted: 1 | Rejected: 0
```

---

## Troubleshooting

### Python Miner Can't Connect
- Ensure blockchain node is running
- Check RPC port (default: 8545)
- Verify firewall settings
- Check node logs for errors

### No Blocks Found
- Normal for solo mining with low hash rate
- Consider joining a mining pool
- Check difficulty is reasonable
- Verify miner is actually hashing (check stats)

### Build Errors
- Ensure CMake and C++ compiler installed
- Check dependencies (OpenSSL, etc.)
- See BUILD_INSTRUCTIONS.md for details

---

## Download Links

After building, all miners are available in:
- **Windows:** `miners_release/` directory
- **Linux:** `miners_release/` directory  
- **macOS:** `miners_release/` directory

To create downloadable packages:
```powershell
Compress-Archive -Path miners_release -DestinationPath gxc-miners-release.zip
```

---

## Support

For more information:
- **Mining Guide:** See `MINING_GUIDE.md`
- **Build Instructions:** See `BUILD_INSTRUCTIONS.md`
- **Blockchain Deployment:** See `BLOCKCHAIN_DEPLOYMENT_GUIDE.md`

