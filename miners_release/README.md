# GXC Blockchain Miners - Release Package

Pre-built GXC blockchain miners ready to install and run.

## Contents

```
miners_release/
├── linux-x64/                  # Pre-built Linux binaries (64-bit)
│   ├── gxc-node               # Blockchain node (required)
│   ├── gxc-miner              # Universal miner (all algorithms)
│   ├── gxc-gxhash-miner       # CPU miner (GXHash algorithm)
│   ├── gxc-sha256-miner       # ASIC miner (SHA-256)
│   ├── gxc-ethash-miner       # GPU miner (Ethash)
│   ├── gxc-pool-proxy         # Mining pool proxy
│   ├── gxc-cli                # Command-line interface
│   ├── gxc-keygen             # Key/wallet generator
│   ├── gxc-tx                 # Transaction builder
│   ├── gxc-explorer           # Block explorer CLI
│   └── gxc-netdiag            # Network diagnostics
├── gxhash-miner-python/        # Python GXHash miner with GUI
│   ├── gxhash_miner.py        # Main miner script
│   ├── setup.py               # Pip installation
│   ├── requirements.txt       # Python dependencies
│   ├── install.sh             # Quick installer
│   └── run_miner.sh           # Quick start script
├── install.sh                  # Main installation script
└── README.md                   # This file
```

## Quick Installation

### C++ Miners (Linux)

```bash
# System-wide installation (requires sudo)
sudo ./install.sh

# User installation (no sudo required)
./install.sh ~/.local
```

### Python Miner

```bash
cd gxhash-miner-python
./install.sh
```

Or manually:
```bash
cd gxhash-miner-python
pip install -r requirements.txt
pip install -e .
```

## Quick Start

### 1. Start the Blockchain Node

```bash
# Start in testnet mode
gxc-node --testnet

# Or with custom ports
gxc-node --rpc-port=8545 --rest-port=8080
```

### 2. Run a Miner

**Option A: GXHash CPU Miner (C++)**
```bash
gxc-gxhash-miner --rpc-url http://localhost:8545
```

**Option B: GXHash Miner with GUI (Python)**
```bash
cd gxhash-miner-python
python3 gxhash_miner.py
```

**Option C: Universal Miner (all algorithms)**
```bash
gxc-miner --rpc-url http://localhost:8545 --algorithm gxhash
```

**Option D: SHA-256 Miner (ASIC)**
```bash
gxc-sha256-miner --rpc-url http://localhost:8545
```

**Option E: Ethash GPU Miner**
```bash
gxc-ethash-miner --rpc-url http://localhost:8545 --gpu
```

## Mining Algorithms

| Algorithm | Hardware | Miner | Best For |
|-----------|----------|-------|----------|
| GXHash | CPU | gxc-gxhash-miner | Beginners, CPUs |
| SHA-256 | ASIC | gxc-sha256-miner | ASIC farms |
| Ethash | GPU | gxc-ethash-miner | GPU rigs |
| All | Any | gxc-miner | Flexibility |

## Command-Line Tools

### Generate a Wallet
```bash
gxc-keygen --output wallet.json
```

### Check Balance
```bash
gxc-cli getbalance GXC...
```

### Send Transaction
```bash
gxc-tx --from wallet.json --to GXCxxx --amount 10
```

### Network Diagnostics
```bash
gxc-netdiag --peers
```

## Configuration

### Node Configuration
Create `~/.gxc/gxc.conf`:
```ini
# Network
port=9333
rpc-port=8545
rest-port=8080

# Mining
mining=true
mining-threads=4

# Logging
log-level=info
```

### Miner Configuration
Most miners accept command-line arguments:
- `--rpc-url URL` - Node RPC endpoint
- `--threads N` - Number of mining threads
- `--address ADDR` - Wallet address for rewards
- `--algorithm ALG` - Mining algorithm (gxhash/sha256/ethash)

## Pool Mining

### Connect to Pool
```bash
gxc-miner --pool stratum+tcp://pool.gxc.network:3333 --user GXC...
```

### Run Pool Proxy
```bash
gxc-pool-proxy --pool pool.gxc.network:3333 --local-port 3334
```

## System Requirements

### Minimum
- CPU: 2 cores
- RAM: 4 GB
- Disk: 10 GB SSD
- Network: 10 Mbps

### Recommended
- CPU: 8+ cores
- RAM: 16 GB
- Disk: 100 GB NVMe SSD
- Network: 100 Mbps

### GPU Mining (Ethash)
- NVIDIA GPU with CUDA 11+ OR
- AMD GPU with OpenCL 1.2+
- 6+ GB VRAM

## Troubleshooting

### Cannot Connect to Node
1. Ensure node is running: `gxc-node --testnet`
2. Check RPC port is open: `curl http://localhost:8545`
3. Verify firewall settings

### No Blocks Found
- Normal for solo mining with low hash rate
- Consider joining a mining pool
- Check difficulty: `gxc-cli getmininginfo`

### High CPU Usage
- Reduce threads: `--threads 2`
- Set CPU affinity in config

### Python GUI Not Working
```bash
# Install tkinter
sudo apt-get install python3-tk  # Ubuntu/Debian
sudo dnf install python3-tkinter  # Fedora
```

## Support

- Documentation: See project docs/ directory
- Issues: Report on GitHub
- Community: Join Discord/Telegram

## License

MIT License - See LICENSE file
