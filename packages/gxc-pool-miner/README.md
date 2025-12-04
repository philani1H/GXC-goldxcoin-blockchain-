# GXC Pool Miner

## Universal Mining Management System for GXC Blockchain

```
╔═══════════════════════════════════════════════════════════════╗
║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗     ║
║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║     ║
║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║     ║
║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║     ║
║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║     ║
║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝     ║
║                                                               ║
║                    POOL MINER v2.0.0                         ║
╚═══════════════════════════════════════════════════════════════╝
```

## Features

- **🔌 Real Blockchain Connection** - Connects directly to GXC blockchain nodes
- **🌐 Pool Mining Support** - Full Stratum protocol implementation
- **💻 Multi-Device Support** - CPU, GPU (NVIDIA/AMD), and ASIC detection
- **⚡ Multiple Algorithms** - SHA256, Ethash/ProgPoW, GxHash
- **📊 Real-time Dashboard** - Live hashrate charts and statistics
- **💰 Wallet Integration** - Balance checking and earnings tracking
- **🎨 Modern Dark UI** - Professional mining interface

## Supported Hardware

| Type | Vendor | Algorithm | Status |
|------|--------|-----------|--------|
| CPU | Any | GxHash | ✅ Full Support |
| GPU | NVIDIA | Ethash | ✅ Full Support |
| GPU | AMD | Ethash | ✅ Full Support |
| ASIC | Bitmain | SHA256 | ✅ Full Support |
| ASIC | Whatsminer | SHA256 | ✅ Full Support |
| ASIC | AvalonMiner | SHA256 | ✅ Full Support |

## Installation

### Quick Install (Recommended)

```bash
# Linux/macOS
chmod +x install.sh
./install.sh

# Windows
install.bat
```

### Manual Install

```bash
# Clone or download
git clone https://github.com/gxc-blockchain/gxc-pool-miner
cd gxc-pool-miner

# Install dependencies
pip install -r requirements.txt

# Run
python -m src.gxc_pool_miner
```

### Using pip

```bash
pip install gxc-pool-miner
gxc-pool-miner
```

## Quick Start

1. **Launch the miner**
   ```bash
   gxc-pool-miner
   ```

2. **Configure Pool Settings**
   - Go to "Pool Config" tab
   - Enter your GXC wallet address
   - Select network (Testnet/Mainnet)

3. **Scan Devices**
   - Click "SCAN DEVICES" to detect your hardware

4. **Start Mining**
   - Click "START MINING" to begin

## Configuration

### Pool Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Pool Address | `pool.gxc.network:3333` | Mining pool endpoint |
| Wallet Address | - | Your GXC wallet address |
| Worker Name | `hostname` | Identifier for this miner |
| Password | `x` | Pool password (usually 'x') |

### Network Selection

| Network | Pool | RPC Port | Address Prefix |
|---------|------|----------|----------------|
| Mainnet | `pool.gxc.network` | 8332 | `GXC` |
| Testnet | `localhost` | 8332 | `tGXC` |

## Command Line Options

```bash
# Basic usage
gxc-pool-miner

# With custom config
gxc-pool-miner --config /path/to/config.json

# CPU only mode
gxc-pool-miner --cpu-only

# GPU only mode  
gxc-pool-miner --gpu-only
```

## Dashboard

The dashboard provides real-time monitoring:

- **Hashrate Monitor** - Live chart of mining performance
- **Device Status** - Per-device statistics
- **Share Counter** - Accepted/rejected shares
- **Efficiency** - Mining efficiency percentage
- **Network Stats** - Block height, difficulty, network hashrate

## Troubleshooting

### Common Issues

**Python not found**
```bash
# Install Python 3.8+
# Ubuntu/Debian
sudo apt install python3 python3-pip python3-tk

# macOS
brew install python3 python-tk

# Windows: Download from python.org
```

**GPU not detected**
```bash
# NVIDIA
nvidia-smi  # Verify drivers

# AMD
rocm-smi    # Verify ROCm installation
```

**Pool connection failed**
- Check your internet connection
- Verify pool address and port
- Try a different pool

### Log Location

```
Linux/macOS: ~/.gxc-pool-miner/logs/
Windows: %LOCALAPPDATA%\GXC-Pool-Miner\logs\
```

## Building from Source

### Create Standalone Executable

```bash
# Install PyInstaller
pip install pyinstaller

# Build
pyinstaller pyinstaller.spec

# Output in dist/
```

## API Reference

The miner connects to GXC blockchain using JSON-RPC:

```python
# Get blockchain info
gxc.call("getblockchaininfo")

# Get balance
gxc.call("getbalance", ["GXCaddress..."])

# Get block template
gxc.call("getblocktemplate")
```

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing`)
3. Commit changes (`git commit -am 'Add feature'`)
4. Push to branch (`git push origin feature/amazing`)
5. Open Pull Request

## License

MIT License - See [LICENSE](LICENSE) for details.

## Disclaimer

⚠️ **Mining involves financial risk.**

- Hardware wear and electricity costs may exceed mining rewards
- Cryptocurrency values are volatile
- You are responsible for compliance with local regulations

Use this software at your own risk.

## Support

- **Documentation**: https://docs.gxc.io/mining
- **GitHub Issues**: https://github.com/gxc-blockchain/gxc-pool-miner/issues
- **Discord**: https://discord.gg/gxc

---

**Built with ❤️ by GXC Blockchain Team**
