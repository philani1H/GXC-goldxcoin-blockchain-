# GXC Miner Pro

<div align="center">

```
   ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗ 
  ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗
  ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝
  ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗
  ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║
   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝
                                 PRO
```

**The Ultimate Mining Software for GXC Blockchain**

[![Version](https://img.shields.io/badge/version-2.0.0-green.svg)](https://github.com/gxc-network/gxc-miner-pro)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

</div>

---

## Features

### All Mining Algorithms in One

| Algorithm | Hardware | Description |
|-----------|----------|-------------|
| **SHA256** | ASIC | Double SHA256 - Bitcoin-style, ASIC optimized |
| **Ethash** | GPU | Keccak-256 based - GPU optimized |
| **GxHash** | CPU | Multi-round ASIC-resistant - Fair mining |

### Mining Modes

- **Pool Mining** - Connect to GXC mining pools via Stratum protocol
- **Solo Mining** - Direct blockchain node connection for solo miners

### Auto Device Detection

- **CPU** - Detects cores, threads, frequency
- **GPU** - NVIDIA (via nvidia-smi) and AMD (via rocm-smi)
- **ASIC** - Network scan for ASIC miners

### Professional GUI

- Real-time hashrate monitoring
- Live performance charts
- Device management
- Easy configuration

### Command Line Interface

Full CLI support for advanced users and automated setups.

---

## Installation

### Quick Install (Recommended)

**Linux/macOS:**
```bash
chmod +x install.sh
./install.sh
```

**Windows:**
```cmd
install.bat
```

### Manual Installation

**From PyPI:**
```bash
pip install gxc-miner-pro
```

**From Source:**
```bash
git clone https://github.com/gxc-network/gxc-miner-pro.git
cd gxc-miner-pro
pip install -r requirements.txt
python src/gxc_miner_pro.py
```

---

## Usage

### GUI Mode

Simply run without arguments to launch the graphical interface:

```bash
gxc-miner-pro
```

### CLI Mode - Pool Mining

```bash
# Basic pool mining
gxc-miner-pro -o stratum+tcp://pool.gxc.network:3333 -u YOUR_WALLET_ADDRESS -p x

# With worker name
gxc-miner-pro -o pool.gxc.network:3333 -u YOUR_WALLET -w rig1 -p x

# Specify algorithm
gxc-miner-pro -o pool.gxc.network:3333 -u YOUR_WALLET -a gxhash

# CPU only mining
gxc-miner-pro -o pool.gxc.network:3333 -u YOUR_WALLET --cpu-only

# GPU only mining
gxc-miner-pro -o pool.gxc.network:3333 -u YOUR_WALLET --gpu-only
```

### CLI Mode - Solo Mining

```bash
# Solo mining to local node
gxc-miner-pro --solo -n http://localhost:8332 -u YOUR_WALLET

# Solo mining with RPC auth
gxc-miner-pro --solo -n http://localhost:8332 --rpc-user user --rpc-pass pass -u YOUR_WALLET
```

### Other Commands

```bash
# List available devices
gxc-miner-pro --list-devices

# Run benchmark
gxc-miner-pro --benchmark -a sha256

# Show version
gxc-miner-pro --version

# Show help
gxc-miner-pro --help
```

---

## CLI Options

| Option | Description |
|--------|-------------|
| `-o, --pool` | Pool URL (stratum+tcp://host:port) |
| `-u, --user, --wallet` | Wallet address |
| `-p, --password` | Pool password (default: x) |
| `-w, --worker` | Worker name |
| `-a, --algorithm` | Algorithm: sha256, ethash, gxhash, auto |
| `--solo` | Enable solo mining mode |
| `-n, --node` | Node RPC URL for solo mining |
| `--rpc-user` | Node RPC username |
| `--rpc-pass` | Node RPC password |
| `--list-devices` | List available mining devices |
| `--cpu-only` | Use CPU only |
| `--gpu-only` | Use GPU only |
| `--benchmark` | Run performance benchmark |
| `-v, --verbose` | Verbose output |

---

## Mining Pools

### Official GXC Pools

| Pool | Address | Port | Fee |
|------|---------|------|-----|
| **Mainnet** | pool.gxc.network | 3333 | 1% |
| **Testnet** | testnet-pool.gxc.network | 13333 | 0% |

### Pool Mining Flow

```
1. Miner connects to pool via Stratum protocol
2. Pool sends mining jobs (work units)
3. Miner finds valid shares and submits to pool
4. Pool collects shares and finds blocks
5. Pool distributes rewards to miners proportionally
6. Pool takes a small fee (typically 1-3%)
```

---

## Hardware Requirements

### Minimum
- CPU: 2 cores, 2 GHz
- RAM: 4 GB
- Storage: 100 MB
- Internet: Stable connection

### Recommended
- CPU: 4+ cores, 3+ GHz (for GxHash)
- GPU: NVIDIA GTX 1060+ / AMD RX 580+ (for Ethash)
- RAM: 8+ GB
- Storage: SSD recommended

---

## Building from Source

### Create Standalone Executable

```bash
# Install build dependencies
pip install pyinstaller

# Build executable
cd packages/gxc-miner-pro
./build.sh
```

The executable will be created in the `dist/` directory.

---

## Troubleshooting

### Connection Issues

```
"Failed to connect to pool"
```
- Check your internet connection
- Verify pool address and port
- Try a different pool server

### No Devices Detected

```
"No mining devices found"
```
- Ensure drivers are installed for GPUs
- Run as administrator/root for full access
- Check that nvidia-smi or rocm-smi work

### Low Hashrate

- Check CPU/GPU temperatures
- Ensure adequate cooling
- Close other resource-intensive applications
- Verify power settings (high performance mode)

---

## Security Notes

1. **Keep your wallet address private** - Never share your private keys
2. **Verify pool authenticity** - Use official pools only
3. **Monitor hardware** - Watch temperatures and power usage
4. **Use secure networks** - Avoid mining on public WiFi

---

## License

MIT License - see [LICENSE](LICENSE) for details.

### Mining Disclaimer

- Mining profitability is not guaranteed
- Electricity costs may exceed rewards
- Hardware wear is the user's responsibility
- Check local regulations regarding cryptocurrency mining

---

## Support

- **Documentation**: https://docs.gxc.network/miner
- **Issues**: https://github.com/gxc-network/gxc-miner-pro/issues
- **Discord**: https://discord.gg/gxc-network
- **Telegram**: https://t.me/gxc_network

---

<div align="center">

**Built with ❤️ by the GXC Foundation**

</div>
