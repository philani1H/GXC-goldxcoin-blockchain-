# GXC Miner v2.0.0

**Professional Pool Mining Software for GXC Blockchain**

```
╔═══════════════════════════════════════════════════════════════════════════════╗
║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗      ║
║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗     ║
║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝     ║
║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗     ║
║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║     ║
║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝     ║
╚═══════════════════════════════════════════════════════════════════════════════╝
```

## Features

- **🔌 Real Pool Mining** - Full Stratum protocol support
- **⚡ 3 Algorithms** - SHA256, Ethash, GxHash
- **🚀 Standalone** - No Python/compiler required
- **💻 Cross-Platform** - Linux, Windows, macOS
- **📊 Live Stats** - Hashrate, shares, uptime

## Quick Start

### Installation

**Linux:**
```bash
chmod +x install-linux.sh
./install-linux.sh
```

**Windows:**
```cmd
install-windows.bat
```

**macOS:**
```bash
chmod +x install-macos.sh
./install-macos.sh
```

### Usage

```bash
gxc-miner -o stratum+tcp://pool.gxc.io:3333 -u YOUR_WALLET -p x
```

## Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `-o, --url` | Pool URL (required) | - |
| `-u, --user` | Wallet address (required) | - |
| `-p, --pass` | Pool password | x |
| `-a, --algo` | Algorithm: gxhash, sha256, ethash | gxhash |
| `-t, --threads` | Number of threads | CPU cores - 1 |
| `-w, --worker` | Worker name | hostname |
| `-v, --verbose` | Verbose output | false |

## Examples

**GxHash CPU Mining (Recommended):**
```bash
gxc-miner -o pool.gxc.io:3333 -u GXC1abc123... -p x
```

**SHA256 with 8 threads:**
```bash
gxc-miner -o pool.gxc.io:3333 -u GXC1abc123... -a sha256 -t 8
```

**Ethash GPU Mining:**
```bash
gxc-miner -o pool.gxc.io:3333 -u GXC1abc123... -a ethash
```

**Testnet Mining:**
```bash
gxc-miner -o localhost:3333 -u tGXC1abc123... -p x -v
```

## Algorithms

| Algorithm | Device | Description |
|-----------|--------|-------------|
| **gxhash** | CPU | ASIC-resistant, traceability optimized |
| **sha256** | ASIC/CPU | Double SHA256, Bitcoin-style |
| **ethash** | GPU | Keccak256-based, Ethereum-style |

## Pool Payment Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   MINER     │────▶│    POOL     │────▶│  BLOCKCHAIN │
│             │     │             │     │             │
│ Submits     │     │ Validates   │     │ Block       │
│ Shares      │     │ Shares      │     │ Reward      │
└─────────────┘     └──────┬──────┘     └─────────────┘
                          │
                          ▼
                   ┌─────────────┐
                   │  PAYOUT     │
                   │             │
                   │ Pool pays   │
                   │ miners based│
                   │ on shares   │
                   └─────────────┘
```

1. **Miner → Pool**: Submit shares (proof of work)
2. **Pool → Blockchain**: Submit blocks when found
3. **Blockchain → Pool**: Block reward (50 GXC)
4. **Pool → Miner**: Payment proportional to shares

Pool typically takes 1-2% fee from block rewards.

## Mining Pools

| Pool | URL | Port | Fee |
|------|-----|------|-----|
| **GXC Main Pool** | pool.gxc.io | 3333 | 1% |
| **Testnet Pool** | localhost | 3333 | 0% |

## System Requirements

- **CPU**: Any x64/ARM processor
- **RAM**: 512 MB minimum
- **Storage**: 50 MB
- **Network**: Stable internet connection

## Building from Source

```bash
# Clone
git clone https://github.com/gxc-blockchain/gxc-miner

# Build
cd gxc-miner-standalone/scripts
./build-standalone.sh

# Run
./bin/gxc-miner --help
```

## License

MIT License - See [LICENSE](LICENSE)

⚠️ **Mining involves financial risk. No profit guarantee.**

---

**Built by GXC Blockchain Team**

Website: https://gxc.network
