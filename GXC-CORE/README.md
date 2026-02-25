# GXC-CORE — GoldXCoin Blockchain Node

> **The official open-source C++ implementation of the GoldXCoin (GXC) blockchain.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Build](https://img.shields.io/badge/build-CMake%203.16%2B-brightgreen.svg)](CMakeLists.txt)

---

## What is GXC?

GoldXCoin (GXC) is a next-generation blockchain with:

- **Hybrid PoW / PoS consensus** — alternating Proof-of-Work and Proof-of-Stake blocks
- **Multi-algorithm mining** — SHA-256, Ethash, and GXHash
- **Proof of Traceability (POT)** — every transaction is cryptographically chained
- **Fraud Detection + Reversal** — on-chain taint propagation and admin-approved reversals
- **Gold-backed tokens (GXC-G)** — 100 % reserve-backed, fully auditable
- **Tokenized stock contracts** — synthetic, custodial, and issuer-authorized equity
- **Decentralized governance** — stake-weighted on-chain proposals and voting
- **Proof of Price oracle** — multi-oracle consensus for reliable price feeds
- **Cross-chain bridge** — 6-chain support (BTC, ETH, BSC, SOL, DOT, GXC)
- **AI security engine** — predictive hashrate sentinel and adaptive difficulty

---

## Quick Start

### 1. Dependencies

**Ubuntu / Debian**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git pkg-config \
    libssl-dev libleveldb-dev libsnappy-dev
```

**macOS**
```bash
brew install cmake openssl leveldb snappy
```

**Windows**
```powershell
# Install vcpkg then:
vcpkg install openssl leveldb snappy
```

### 2. Clone and Build

```bash
git clone https://github.com/GoldXCoin/GXC-CORE.git
cd GXC-CORE

# Linux / macOS
chmod +x scripts/build.sh
./scripts/build.sh Release

# Windows
scripts\build.bat Release
```

### 3. Run the Node

```bash
# Mainnet
./build/gxc-node --config config/gxc.conf.example

# Testnet
./build/gxc-node --testnet --config config/gxc-testnet.conf
```

---

## Repository Structure

```
GXC-CORE/
│
├── src/                        # C++ implementation files
│   ├── Blockchain.cpp          # Core chain logic, validation, UTXO
│   ├── block.cpp               # Block structure, PoW, PoS, work receipt
│   ├── Transaction.cpp         # Transaction model, Proof of Traceability
│   ├── Wallet.cpp              # Key generation, signing, address derivation
│   ├── Validator.cpp           # Validator stake, weight, APY
│   ├── SalectValidator.cpp     # Weighted-stake validator selection
│   ├── StakingPool.cpp         # LevelDB-backed stake tracking
│   ├── Database.cpp            # LevelDB singleton, atomic writes, all key schemas
│   ├── Crypto.cpp              # secp256k1 ECDSA
│   ├── HashUtils.cpp           # SHA-256, RIPEMD-160, Keccak-256
│   ├── Keccak256.cpp           # Keccak sponge construction
│   ├── Blake2b.cpp             # Blake2b 256-bit hash
│   ├── Argon2id.cpp            # Argon2id memory-hard KDF
│   ├── EthashAlgorithm.cpp     # Ethash DAG generation and verification
│   ├── arith_uint256.cpp       # 256-bit chainwork arithmetic
│   ├── FraudDetection.cpp      # Taint propagation (BFS), 5 detection rules
│   ├── ProofGenerator.cpp      # Proof of Feasibility (POF) generation
│   ├── ReversalExecutor.cpp    # Fund reversal execution engine
│   ├── ReversalFeePool.cpp     # Self-sustaining fee pool
│   ├── MarketMakerAdmin.cpp    # Admin roles, sessions, fraud reports
│   ├── ProofOfPrice.cpp        # Multi-oracle price consensus
│   ├── CrossChainBridge.cpp    # Multi-chain bridge (67% multi-sig)
│   ├── AddressRegistry.cpp     # Clean-zone entity registry
│   ├── Network.cpp             # P2P networking
│   ├── P2PNetwork.cpp          # Low-level TCP peer management
│   ├── PeerManager.cpp         # Peer lifecycle management
│   ├── MessageHandler.cpp      # Protocol message dispatcher
│   ├── RPCAPI.cpp              # JSON-RPC 2.0 API
│   ├── RESTServer.cpp          # HTTP REST API
│   ├── RESTServer_AdminAPIs.cpp# Admin-only REST endpoints
│   ├── WebSocketServer.cpp     # Real-time WebSocket push API
│   ├── Stratum.cpp             # Stratum mining-pool protocol
│   ├── CPUMiner.cpp            # CPU mining implementation
│   ├── Config.cpp              # Node configuration
│   ├── Logger.cpp              # Structured logging
│   ├── Utils.cpp               # Shared utilities
│   ├── node_main.cpp           # Node entry point
│   ├── main.cpp                # Alternative entry point
│   ├── governance/             # On-chain governance
│   │   ├── Governance.cpp
│   │   ├── Proposals.cpp
│   │   └── Voting.cpp
│   ├── tokens/                 # Token contracts
│   │   ├── GoldToken.cpp       # GXC-G gold-backed token
│   │   ├── StockContract.cpp   # Tokenized equity contracts
│   │   └── TokenManager.cpp
│   ├── mining/                 # Mining subsystem
│   │   ├── MiningManager.cpp   # Mining coordinator
│   │   ├── SHA256Miner.cpp
│   │   ├── EthashMiner.cpp
│   │   ├── GXHashMiner.cpp
│   │   ├── PoolManager.cpp     # Mining pool management
│   │   ├── HardwareDetector.cpp# CPU / GPU / ASIC detection
│   │   └── MiningOptimizer.cpp # System-level tuning
│   └── security/
│       └── SecurityEngine.cpp  # AI hashrate sentinel, 6-layer protection
│
├── include/                    # Public header files
│   ├── blockchain.h
│   ├── Block.h
│   ├── transaction.h
│   ├── Wallet.h
│   ├── Validator.h
│   ├── StakingPool.h
│   ├── Database.h
│   ├── Crypto.h
│   ├── HashUtils.h
│   ├── Blake2b.h
│   ├── Argon2id.h
│   ├── EthashAlgorithm.h
│   ├── FraudDetection.h
│   ├── ProofGenerator.h
│   ├── ReversalExecutor.h
│   ├── ReversalFeePool.h
│   ├── MarketMakerAdmin.h
│   ├── ProofOfPrice.h
│   ├── CrossChainBridge.h
│   ├── Governance.h
│   ├── GoldToken.h
│   ├── StockContract.h
│   ├── ExplorerAPI.h
│   ├── AddressRegistry.h
│   ├── Network.h
│   ├── RPCAPI.h
│   ├── RESTServer.h
│   ├── WebSocketServer.h
│   ├── Stratum.h
│   ├── Config.h
│   ├── Logger.h
│   ├── Utils.h
│   ├── arith_uint256.h
│   ├── mining/                 # Mining headers
│   └── security/               # Security headers
│
├── mining/                     # Miner entry-point sources
│   ├── main.cpp                # All-algorithm miner
│   ├── sha256_main.cpp
│   ├── ethash_main.cpp
│   ├── gxhash_main.cpp
│   ├── pool_proxy_main.cpp
│   └── GXCMiner.h
│
├── config/
│   ├── gxc.conf.example        # Mainnet config template
│   └── gxc-testnet.conf        # Testnet config
│
├── docs/
│   └── WHITEPAPER.md           # Full technical whitepaper (3,978 lines)
│
├── tests/                      # Test suite (cmake -DBUILD_TESTS=ON)
├── benchmarks/                 # Performance benchmarks
├── third_party/json/           # nlohmann/json (header-only)
├── scripts/
│   ├── build.sh                # Linux/macOS build helper
│   └── build.bat               # Windows build helper
│
├── CMakeLists.txt              # Main build definition
├── LICENSE                     # MIT License
├── CONTRIBUTING.md             # Contribution guidelines
├── SECURITY.md                 # Vulnerability reporting
└── .gitignore
```

---

## Network Parameters

| Parameter                | Mainnet           | Testnet          |
|--------------------------|-------------------|------------------|
| P2P Port                 | 8333              | 18333            |
| RPC Port                 | 8332              | 18332            |
| REST Port                | 8080              | 8080             |
| Block time target        | 600 s (10 min)    | 120 s (2 min)    |
| Difficulty interval      | 2 016 blocks      | 2 016 blocks     |
| Min difficulty           | 1 000.0           | 1.0              |
| Initial block reward     | 50 GXC            | 50 GXC           |
| Halving interval         | 1 051 200 blocks  | 1 051 200 blocks |
| Max supply               | 31 000 000 GXC    | 31 000 000 GXC   |
| Address prefix           | `GXC`             | `tGXC`           |
| Network magic            | `GXC\x01`         | `GXCT`           |
| Max block size           | 1 048 576 bytes   | 1 048 576 bytes  |
| Max connections          | 125               | 125              |
| Protocol version         | 70015             | 70015            |

---

## API Overview

### JSON-RPC (port 8332)

```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'
```

Key methods: `getblockcount`, `getblock`, `gettransaction`, `getbalance`,
`sendtoaddress`, `getmininginfo`, `validateaddress`, `getstakinginfo`

### REST (port 8080)

```
GET  /api/block/:hash
GET  /api/block/height/:height
GET  /api/transaction/:hash
GET  /api/balance/:address
GET  /api/validators
GET  /api/stats
POST /api/transaction
```

### WebSocket (port 8080 /ws)

Streams real-time block, transaction, and network-stats events.

---

## Mining

```bash
# SHA-256 (Bitcoin-compatible)
./build/gxc-sha256-miner --address GXCyouraddress --threads 4

# Ethash (Ethereum-compatible, GPU-optimized)
./build/gxc-ethash-miner --address GXCyouraddress --gpu 0

# GXHash (native, ASIC-resistant)
./build/gxc-gxhash-miner --address GXCyouraddress --threads 8

# Pool mining proxy (Stratum)
./build/gxc-pool-proxy --pool pool.goldxcoin.network:3333 --address GXCyouraddress
```

---

## Staking

```bash
# Create a validator and stake GXC
./build/gxc-cli stake --amount 1000 --days 180 --address GXCyouraddress

# Check staking rewards
./build/gxc-cli getstakinginfo --address GXCyouraddress
```

Minimum validator stake: **100 GXC**
Minimum staking period: **14 days**
Maximum staking period: **365 days**

---

## Key Tools

| Binary            | Purpose                                      |
|-------------------|----------------------------------------------|
| `gxc-node`        | Full blockchain node (mainnet / testnet)      |
| `gxc-miner`       | Multi-algorithm miner                         |
| `gxc-sha256-miner`| SHA-256 dedicated miner                       |
| `gxc-ethash-miner`| Ethash dedicated miner (GPU)                  |
| `gxc-gxhash-miner`| GXHash dedicated miner                        |
| `gxc-pool-proxy`  | Stratum pool proxy                            |
| `gxc-cli`         | Command-line interface                        |
| `gxc-keygen`      | Key pair and address generator                |
| `gxc-tx`          | Transaction builder and broadcaster           |
| `gxc-explorer`    | Local block explorer                          |
| `gxc-netdiag`     | Network diagnostics                           |

---

## Technical Whitepaper

The complete technical specification — all algorithms, mathematical formulas,
and implementation details — is in [`docs/WHITEPAPER.md`](docs/WHITEPAPER.md).

Covers:
- Cryptographic primitives (secp256k1, SHA-256, Keccak-256, Blake2b, Argon2id)
- Hybrid consensus mathematics
- Taint propagation algorithm: `τ(Tⱼ) = Σᵢ wᵢ · τ(Tᵢ)`
- Proof of Feasibility (POF) for fraud reversals
- Validator selection: `wᵢ = stakeᵢ × (daysᵢ/365)^0.5`
- Difficulty adjustment formulas
- Halving schedule: `R(h) = 50 / 2^⌊h/1,051,200⌋`
- Complete LevelDB schema with all 17 key namespaces
- Market maker admin pipeline and permission matrix

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Security

See [SECURITY.md](SECURITY.md) — **do not open public issues for vulnerabilities**.

## License

MIT — see [LICENSE](LICENSE).

---

*GXC-CORE is maintained by the GoldXCoin Development Team.*
*Protocol Version 70015 · Node v2.0.0 · C++17*
