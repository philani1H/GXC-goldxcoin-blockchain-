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
