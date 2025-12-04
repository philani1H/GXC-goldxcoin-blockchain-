# GXC Blockchain Miners - Standalone Release

**Version 2.0.0** | **No Dependencies Required** | **Just Download and Run!**

Like VSCode or Discord - download, install, and you're ready to mine!

## 📦 What's Included

### Miners
| Binary | Description | Algorithm | Hardware |
|--------|-------------|-----------|----------|
| `gxc-miner` | Universal miner | All | CPU/GPU/ASIC |
| `gxc-gxhash-miner` | GXHash miner | GXHash | CPU |
| `gxc-sha256-miner` | SHA256 miner | SHA-256 | CPU/ASIC |
| `gxc-ethash-miner` | Ethash miner | Ethash | GPU |
| `gxc-pool-proxy` | Pool proxy | N/A | N/A |

### Blockchain
| Binary | Description |
|--------|-------------|
| `gxc-node` | Full blockchain node |
| `gxc-cli` | Command-line interface |

### Tools
| Binary | Description |
|--------|-------------|
| `gxc-keygen` | Generate wallets/keys |
| `gxc-tx` | Build transactions |
| `gxc-explorer` | Block explorer |
| `gxc-netdiag` | Network diagnostics |

## 🚀 Quick Installation

### Linux/macOS
```bash
# Download and extract
tar -xzf gxc-miners-linux-x64.tar.gz
cd miners_release

# Run installer
./install.sh
```

### Windows
```powershell
# Extract the zip file
# Run install.bat (coming soon)
# Or copy files to your preferred location
```

## 💻 Quick Start

### 1. Start the Blockchain Node
```bash
gxc-node --testnet
```

### 2. Start Mining
```bash
# Universal miner
gxc-miner -a YOUR_WALLET_ADDRESS --algorithm=gxhash

# Or specialized miners
gxc-gxhash-miner --help
gxc-sha256-miner --threads=8
```

### 3. Generate a Wallet
```bash
gxc-keygen --output wallet.json
```

## 📋 Miner Commands

### Universal Miner (gxc-miner)
```bash
gxc-miner [options]

Options:
  --address, -a ADDR   Miner address (required)
  --algorithm ALGO     Mining algorithm (sha256|ethash|gxhash)
  --pool URL           Mining pool URL
  --threads N          Number of mining threads
  --verbose            Verbose logging

Examples:
  gxc-miner -a GXC123... --algorithm=gxhash
  gxc-miner -a GXC123... --pool=stratum+tcp://pool.gxc.io:4444
```

### SHA256 Miner (gxc-sha256-miner)
```bash
gxc-sha256-miner [options]

Options:
  --threads, -t N      Mining threads (default: auto)
  --server URL         Pool server URL
  --user USER          Pool username
  --benchmark          Run benchmark

Examples:
  gxc-sha256-miner --threads=8
  gxc-sha256-miner --benchmark
```

### GXHash Miner (gxc-gxhash-miner)
```bash
gxc-gxhash-miner [options]

Options:
  --help               Show help
  (More options available)
```

## 🔧 Node Commands

### Start Node
```bash
# Testnet
gxc-node --testnet

# Mainnet
gxc-node

# With custom ports
gxc-node --rpc-port=8545 --rest-port=8080
```

### Node Options
```
  --testnet           Use testnet network
  --datadir DIR       Data directory path
  --port PORT         Network port (default: 9333)
  --rpc-port PORT     RPC port (default: 8332)
  --rest-port PORT    REST API port (default: 8080)
  --daemon            Run as daemon
  --verbose           Verbose logging
```

## 📊 API Endpoints

When the node is running:

- **RPC API**: `http://localhost:8332` (or 18332 for testnet)
- **REST API**: `http://localhost:8080` (or 18080 for testnet)

### Example RPC Calls
```bash
# Get blockchain info
curl -X POST -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
  http://localhost:18332

# Get balance
curl -X POST -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["GXC..."],"id":1}' \
  http://localhost:18332
```

## 🔒 Security Notes

- All binaries are statically linked - no external dependencies needed
- Data is stored in LevelDB for optimal performance
- Separate databases for testnet and mainnet (no data mixing)
- Full traceability support built-in

## 📁 Data Directories

| Network | Default Location |
|---------|-----------------|
| Mainnet | `~/.gxc/` or `./gxc_data/` |
| Testnet | `~/.gxc/testnet/` or `./gxc_data/testnet/` |

## 🌐 Mining Algorithms

### GXHash (Recommended for CPU)
- Custom algorithm optimized for GXC
- ASIC-resistant
- Best for beginners

### SHA-256 (For ASIC)
- Bitcoin-compatible
- High performance with ASIC hardware
- Pool mining recommended

### Ethash (For GPU)
- Ethereum-compatible
- GPU mining
- Requires OpenCL/CUDA

## 🔗 Pool Mining

```bash
# Connect to pool
gxc-miner -a YOUR_ADDRESS \
  --pool=stratum+tcp://pool.gxc.network:3333

# With worker name
gxc-miner -a YOUR_ADDRESS \
  --pool=stratum+tcp://pool.gxc.network:3333 \
  --username=worker1
```

## 📈 System Requirements

### Minimum
- CPU: 2 cores
- RAM: 4 GB
- Disk: 10 GB SSD
- Network: 10 Mbps

### Recommended
- CPU: 8+ cores (for mining)
- RAM: 16 GB
- Disk: 100 GB NVMe SSD
- Network: 100 Mbps

### GPU Mining (Ethash)
- NVIDIA with CUDA 11+ or
- AMD with OpenCL 1.2+
- 6+ GB VRAM

## 🐛 Troubleshooting

### Cannot start node
1. Check if ports are in use: `netstat -tlnp | grep 8332`
2. Check logs in `./gxc_data/gxc.log`
3. Try with `--verbose` flag

### Mining not finding blocks
- Normal for solo mining
- Consider joining a pool
- Check difficulty with `gxc-cli getmininginfo`

### Permission denied
```bash
chmod +x gxc-*
```

## 📞 Support

- Documentation: [docs.gxc.network](https://docs.gxc.network)
- Issues: GitHub Issues
- Community: Discord/Telegram

## 📜 License

MIT License - See LICENSE file

---

**Happy Mining! 🎉**
