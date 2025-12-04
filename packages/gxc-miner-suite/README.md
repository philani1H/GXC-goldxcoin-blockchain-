# GXC Miner Suite

## SHA256 • Ethash • GxHash - Universal Mining for GXC Blockchain

```
╔═══════════════════════════════════════════════════════════════════════════════╗
║                                                                               ║
║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗      ║
║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗     ║
║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝     ║
║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗     ║
║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║     ║
║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝     ║
║                                                                               ║
║                           MINER SUITE v2.0.0                                 ║
║                                                                               ║
╚═══════════════════════════════════════════════════════════════════════════════╝
```

## Features

- **🔌 Real Blockchain Connection** - Direct RPC to GXC nodes
- **🌐 Pool Mining** - Full Stratum protocol support
- **💻 Multi-Device** - CPU, GPU (NVIDIA/AMD), ASIC
- **⚡ 3 Algorithms** - SHA256, Ethash, GxHash
- **📊 Live Dashboard** - Real-time hashrate charts
- **💰 Wallet Integration** - Balance & earnings tracking

## Algorithms

| Algorithm | Device | Description |
|-----------|--------|-------------|
| **SHA256** | ASIC | Double SHA256 - Bitcoin-style |
| **Ethash** | GPU | Memory-hard - Ethereum-style |
| **GxHash** | CPU | Traceability-optimized - ASIC resistant |

## Quick Install

### Linux/macOS
```bash
chmod +x install.sh
./install.sh
```

### Windows
```cmd
install.bat
```

### pip
```bash
pip install -r requirements.txt
python -m src.gxc_miner
```

## Usage

1. **Launch**: `gxc-miner`
2. **Enter wallet address**
3. **Select algorithm** (GxHash/Ethash/SHA256)
4. **Click START MINING**

## Configuration

| Setting | Description |
|---------|-------------|
| Pool Address | `pool.gxc.network:3333` (mainnet) |
| Wallet | Your GXC wallet address |
| Worker | Identifier for this miner |
| Threads | CPU threads for GxHash |

## Networks

| Network | Pool | RPC |
|---------|------|-----|
| Mainnet | `pool.gxc.network:3333` | `node.gxc.network:8332` |
| Testnet | `localhost:3333` | `localhost:8332` |

## Requirements

- Python 3.8+
- customtkinter
- psutil
- requests
- matplotlib

## License

MIT License - See [LICENSE](LICENSE)

⚠️ **Mining involves financial risk. No profit guarantee.**

---

**Built by GXC Blockchain Team**
