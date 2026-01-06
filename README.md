# 🪙 GXC Blockchain - Complete Documentation

[![Version](https://img.shields.io/badge/version-2.1.0-blue.svg)](https://github.com/philani1H/GXC-goldxcoin-blockchain-)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/philani1H/GXC-goldxcoin-blockchain-)
[![Production Ready](https://img.shields.io/badge/production-ready-success.svg)](PRODUCTION_READY.md)

**GXC (GoldXCoin)** is a production-ready blockchain with hybrid PoW/PoS consensus, validator profiles, AI-powered security, and complete traceability.

---

## 📋 Table of Contents

- [Quick Start](#-quick-start)
- [Core Features](#-core-features)
- [Blockchain Specifications](#-blockchain-specifications)
- [Consensus Mechanisms](#-consensus-mechanisms)
- [Validator System](#-validator-system)
- [Transaction Types](#-transaction-types)
- [Mining](#-mining)
- [Staking](#-staking)
- [Wallet Features](#-wallet-features)
- [RPC API](#-rpc-api)
- [Security Features](#-security-features)
- [Network](#-network)
- [Building from Source](#-building-from-source)
- [Testing](#-testing)
- [Deployment](#-deployment)

---

## 🚀 Quick Start

### Build and Run Node

```bash
# Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Build
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

# Run node (testnet)
./gxc-node --testnet --rpc-port=8545

# Run node (mainnet)
./gxc-node --rpc-port=8332
```

### Mine Blocks

```bash
# SHA-256 mining
./gxc-miner --address=YOUR_ADDRESS --algorithm=sha256 --node=http://localhost:8545

# Ethash mining
./gxc-ethash-miner --address=YOUR_ADDRESS --node=http://localhost:8545

# GXHash mining (ASIC-resistant)
./gxc-gxhash-miner --address=YOUR_ADDRESS --node=http://localhost:8545
```

### Create Wallet

```bash
# Generate new wallet
./gxc-keygen

# Check balance
curl -X POST http://localhost:8545 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["YOUR_ADDRESS"],"id":1}'
```

---

## ✨ Core Features

### Hybrid Consensus
- **PoW (Proof-of-Work)**: SHA-256, Ethash, GXHash algorithms
- **PoS (Proof-of-Stake)**: Validator-based block production
- **Alternating Blocks**: Even blocks PoW, odd blocks PoS
- **Weighted Selection**: Validators selected by stake weight

### Validator Profiles
- **Custom Names**: Validators can set display names
- **Branding**: Logo, bio, website, contact information
- **Commission Rates**: Configurable 0-20% commission
- **Performance Tracking**: Uptime, blocks produced/missed
- **APY Calculation**: Realistic 15-20% annual yields
- **Reputation System**: Score based on performance

### Complete Traceability
- **Transaction Chaining**: Every transaction links to previous
- **UTXO Tracking**: Full input/output traceability
- **Formula**: `Ti.Inputs[0].txHash == Ti.PrevTxHash`
- **Amount Verification**: `Ti.Inputs[0].amount == Ti.ReferencedAmount`
- **Blockchain Audit**: Complete transaction history

### AI-Powered Security
- **Hashrate Sentinel**: Detects mining attacks
- **Difficulty Guard**: Prevents manipulation
- **Staker Balance Modifier**: Protects against stake attacks
- **Emission Guard**: Prevents inflation attacks
- **Fee Surge Protection**: Detects fee manipulation
- **Hybrid Penalty Logic**: Automatic attack response

---

## 📊 Blockchain Specifications

| Parameter | Value |
|-----------|-------|
| **Block Time** | 10 seconds |
| **Block Reward** | 50 GXC (halving every 1,051,200 blocks) |
| **Max Supply** | 31,000,000 GXC |
| **Min Validator Stake** | 100 GXC |
| **Min Staking Period** | 14 days |
| **Max Staking Period** | 365 days |
| **Transaction Fee** | 0.001 GXC (dynamic) |
| **Coinbase Maturity** | 100 blocks |
| **Difficulty Adjustment** | Every 2,016 blocks |

---

## 🔄 Consensus Mechanisms

### Proof of Work (PoW)

**Supported Algorithms:**

1. **SHA-256**
   - Bitcoin-compatible
   - ASIC-friendly
   - High security

2. **Ethash**
   - Ethereum-compatible
   - Memory-hard
   - GPU-optimized

3. **GXHash**
   - GXC-specific
   - ASIC-resistant
   - CPU/GPU balanced

**Mining Difficulty:**
- Adjusts every 2,016 blocks
- Target: 10-second block time
- Retarget algorithm prevents manipulation

### Proof of Stake (PoS)

**Validator Selection:**
- Weighted by stake amount and time
- Formula: `W = S × (days/365)^0.5`
- Deterministic selection using block height seed
- No randomness manipulation possible

**Block Production:**
- Validators sign blocks with private key
- Signature verification required
- Missed blocks tracked for reputation

---

## 👥 Validator System

### Registration

**RPC Method:** `registerValidator`

**Parameters:**
```json
{
  "address": "GXC_address",
  "stakeAmount": 1000.0,
  "stakingDays": 90,
  "name": "My Validator",           // Optional
  "logo": "https://logo.png",       // Optional
  "bio": "Professional validator",  // Optional
  "website": "https://site.com",    // Optional
  "contact": "email@example.com",   // Optional
  "commission": 0.10                // Optional (default 10%)
}
```

**Example:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"registerValidator",
  "params":["GXC_address", 1000.0, 90, "Acme Validator", "https://logo.png", "Professional service", "https://acme.com", "contact@acme.com", 0.10],
  "id":1
}'
```

### Query Validators

**List All Validators:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"getvalidators",
  "params":[],
  "id":1
}'
```

**Response:**
```json
{
  "result": [
    {
      "address": "GXC_address",
      "stake_amount": 1000.0,
      "staking_days": 90,
      "weighted_stake": 486.8,
      "is_active": true,
      "blocks_produced": 150,
      "missed_blocks": 5,
      "uptime": 0.968,
      "total_rewards": 75.5,
      "apy": 18.5,
      "name": "Acme Validator",
      "logo": "https://logo.png",
      "bio": "Professional service",
      "website": "https://acme.com",
      "contact": "contact@acme.com",
      "commission_rate": 0.10
    }
  ]
}
```

### Validator Info

**Get Specific Validator:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"getvalidatorinfo",
  "params":["GXC_address"],
  "id":1
}'
```

### Performance Metrics

**Tracked Automatically:**
- Blocks produced
- Blocks missed
- Uptime percentage
- Total rewards earned
- Current APY
- Reputation score
- Slashing events

---

## 💸 Transaction Types

### 1. NORMAL (Transfer)
Standard coin transfer between addresses.

**Create Transaction:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"createtransaction",
  "params":[{
    "from": "sender_address",
    "to": "receiver_address",
    "amount": 10.0,
    "fee": 0.001
  }],
  "id":1
}'
```

### 2. STAKE
Lock coins to become a validator.

**Create Stake:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"registervalidator",
  "params":["address", 100.0, 30],
  "id":1
}'
```

### 3. UNSTAKE
Release staked coins after lock period.

**Unstake:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"unstake",
  "params":["address"],
  "id":1
}'
```

### 4. REWARD
Staking rewards (mints new coins).

**Automatic:** Created by blockchain when validator produces blocks.

### 5. COINBASE
Mining rewards (mints new coins).

**Automatic:** Created by miners when solving PoW blocks.

---

## ⛏️ Mining

### Mining Algorithms

**1. SHA-256 Mining**
```bash
./gxc-miner --address=YOUR_ADDRESS --algorithm=sha256 --node=http://localhost:8545 --threads=4
```

**2. Ethash Mining**
```bash
./gxc-ethash-miner --address=YOUR_ADDRESS --node=http://localhost:8545
```

**3. GXHash Mining**
```bash
./gxc-gxhash-miner --address=YOUR_ADDRESS --node=http://localhost:8545
```

### Mining Pools

**Pool Server:**
```bash
./gxc-pool-server --port=3333 --node=http://localhost:8545
```

**Connect to Pool:**
```bash
./gxc-miner --pool=stratum+tcp://pool.example.com:3333 --user=YOUR_ADDRESS
```

### Mining Rewards

- **Block Reward:** 50 GXC
- **Halving:** Every 1,051,200 blocks (~4 years)
- **Maturity:** 100 blocks before spendable
- **Fee Rewards:** Included in block reward

---

## 🔒 Staking

### Staking Requirements

- **Minimum Stake:** 100 GXC
- **Minimum Period:** 14 days
- **Maximum Period:** 365 days
- **Lock Period:** Coins locked until period ends

### Staking Process

1. **Acquire GXC:** Mine or receive coins
2. **Register as Validator:** Stake minimum 100 GXC
3. **Wait for Activation:** Stake transaction must confirm
4. **Produce Blocks:** Selected based on weighted stake
5. **Earn Rewards:** Receive staking rewards

### Staking Rewards

**Reward Calculation:**
- **Staking Reward per Block:** 0.0001 GXC
- **Annual Blocks:** 3,155,760 (10-second blocks)
- **Estimated APY:** 15-20%
- **Commission:** Validators earn commission from delegators

**APY Formula:**
```
APY = (reward_per_block × blocks_per_year / stake) × 100
```

### Check Staking Info

```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"getstakinginfo",
  "params":["YOUR_ADDRESS"],
  "id":1
}'
```

---

## 💼 Wallet Features

### Key Generation

**Generate New Wallet:**
```bash
./gxc-keygen
```

**Output:**
```
Private Key: 5KQw...
Public Key: 04a3b...
Address: GXC9fab7317231b966af85ac453e168c0932
```

### Wallet Operations

**Check Balance:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"getbalance",
  "params":["YOUR_ADDRESS"],
  "id":1
}'
```

**Get New Address:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"getnewaddress",
  "params":[],
  "id":1
}'
```

**Send Transaction:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"sendtoaddress",
  "params":["RECIPIENT_ADDRESS", 10.0],
  "id":1
}'
```

### Transaction History

```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"listtransactions",
  "params":["YOUR_ADDRESS"],
  "id":1
}'
```

### Import/Export

**Import Private Key:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"importprivkey",
  "params":["PRIVATE_KEY"],
  "id":1
}'
```

---

## 🔌 RPC API

### Connection

**Mainnet:** `http://localhost:8332`
**Testnet:** `http://localhost:8545`

### GET Endpoints (Railway Compatible)

For Railway deployments where POST bodies may be stripped, use GET endpoints with query parameters:

```bash
# Get blockchain info
curl https://your-node.railway.app/api/getinfo

# Get block count
curl https://your-node.railway.app/api/getblockcount

# Get balance for address
curl https://your-node.railway.app/api/getbalance?address=YOUR_ADDRESS

# List unspent outputs
curl https://your-node.railway.app/api/listunspent?address=YOUR_ADDRESS

# Get block by hash
curl https://your-node.railway.app/api/getblock?hash=BLOCK_HASH

# Get transaction by ID
curl https://your-node.railway.app/api/gettransaction?txid=TX_ID
```

### Available Methods (101 total)

#### Blockchain Info
- `getblockchaininfo` - Get blockchain status
- `getblockcount` - Get current block height
- `getdifficulty` - Get current difficulty
- `getbestblockhash` - Get latest block hash
- `getinfo` - Get node information

#### Blocks
- `getblock` - Get block by hash
- `getblockbynumber` - Get block by height
- `getblockhash` - Get block hash by height
- `getblocktemplate` - Get block template for mining
- `getblocktransactions` - Get transactions in block

#### Transactions
- `gettransaction` - Get transaction by hash
- `getrawtransaction` - Get raw transaction
- `sendrawtransaction` - Broadcast transaction
- `createtransaction` - Create new transaction
- `listtransactions` - List transactions for address

#### Wallet
- `getbalance` - Get address balance
- `getpendingbalance` - Get unconfirmed balance
- `getnewaddress` - Generate new address
- `sendtoaddress` - Send coins
- `importprivkey` - Import private key

#### Validators
- `registervalidator` - Register as validator
- `getvalidators` - List all validators
- `getvalidatorinfo` - Get validator details
- `addstake` - Add more stake
- `unstake` - Remove stake

#### Staking
- `getstakinginfo` - Get staking information
- `createposblock` - Create PoS block

#### Network
- `getpeerinfo` - Get connected peers
- `getconnectioncount` - Get peer count
- `addnode` - Add peer node
- `disconnectnode` - Disconnect peer

#### Mining
- `getmininginfo` - Get mining statistics
- `getnetworkhashps` - Get network hashrate
- `estimatefee` - Estimate transaction fee

### Example RPC Call

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"getblockchaininfo",
    "params":[],
    "id":1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "testnet",
    "blocks": 12345,
    "headers": 12345,
    "bestblockhash": "0000000000...",
    "difficulty": 1000.0,
    "mediantime": 1640995200,
    "verificationprogress": 1.0,
    "chainwork": "0000000000...",
    "pruned": false
  },
  "id": 1
}
```

---

## 🛡️ Security Features

### AI-Powered Protection

**1. Hashrate Sentinel**
- Monitors network hashrate
- Detects sudden spikes (>50% increase)
- Triggers difficulty adjustment
- Prevents 51% attacks

**2. Difficulty Guard**
- Prevents difficulty manipulation
- Validates difficulty changes
- Enforces minimum difficulty
- Protects against time-warp attacks

**3. Staker Balance Modifier**
- Verifies staker balances
- Prevents fake stake attacks
- Validates UTXO ownership
- Ensures stake maturity

**4. Emission Guard**
- Monitors coin creation
- Validates block rewards
- Prevents inflation attacks
- Enforces halving schedule

**5. Fee Surge Protection**
- Detects abnormal fees
- Prevents fee manipulation
- Protects users from overpaying
- Maintains network stability

**6. Hybrid Penalty Logic**
- Automatic attack response
- Increases difficulty on attack
- Slashes malicious validators
- Bans attacking nodes

### Cryptographic Security

- **Hashing:** SHA-256, Keccak-256, Blake2b
- **Signatures:** ECDSA (secp256k1)
- **Key Derivation:** Argon2id
- **Address Format:** Base58Check encoding

---

## 🌐 Network

### Network Ports

| Network | P2P Port | RPC Port | REST Port |
|---------|----------|----------|-----------|
| Mainnet | 8333 | 8332 | 8080 |
| Testnet | 18333 | 8545 | 18080 |

### P2P Protocol

- **Message Types:** Block, Transaction, Peer, Ping
- **Peer Discovery:** DNS seeds, manual addnode
- **Block Propagation:** Compact blocks
- **Transaction Relay:** Mempool synchronization

### Network Commands

**Add Peer:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"addnode",
  "params":["peer.example.com:8333", "add"],
  "id":1
}'
```

**Get Peers:**
```bash
curl -X POST http://localhost:8545 -H "Content-Type: application/json" -d '{
  "jsonrpc":"2.0",
  "method":"getpeerinfo",
  "params":[],
  "id":1
}'
```

---

## 🔨 Building from Source

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
  libssl-dev libleveldb-dev libboost-all-dev \
  libsodium-dev libcurl4-openssl-dev

# macOS
brew install cmake openssl leveldb boost libsodium curl
```

### Build Steps

```bash
# Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Create build directory
mkdir -p build && cd build

# Configure
cmake ..

# Build (use all CPU cores)
make -j$(nproc)

# Install (optional)
sudo make install
```

### Build Targets

- `gxc-node` - Full node
- `gxc-miner` - Multi-algorithm miner
- `gxc-ethash-miner` - Ethash miner
- `gxc-gxhash-miner` - GXHash miner
- `gxc-sha256-miner` - SHA-256 miner
- `gxc-keygen` - Key generator
- `gxc-cli` - Command-line interface
- `gxc-tx` - Transaction tool
- `gxc-explorer` - Block explorer

---

## 🧪 Testing

### Unit Tests

```bash
# Build tests
cd build
make test_staking_unit
make test_comprehensive

# Run tests
./test_staking_unit
./test_comprehensive
```

### Integration Tests

```bash
# Test complete flow
python3 test_complete_flow.py

# Test staking
python3 test_complete_staking.py

# Test validator registration
python3 test_validator_registration.py

# Test mining
python3 test_mining_with_consensus.py
```

### Testnet Testing

```bash
# Start testnet node
./gxc-node --testnet --rpc-port=8545

# Run testnet tests
bash test_testnet.sh
```

---

## 🚀 Deployment

### Production Node

```bash
# Run as daemon
./gxc-node --daemon --datadir=/var/lib/gxc

# With custom config
./gxc-node --config=/etc/gxc/gxc.conf

# Systemd service
sudo systemctl start gxc-node
sudo systemctl enable gxc-node
```

### Configuration File

**Location:** `~/.gxc/gxc.conf`

```ini
# Network
testnet=0
port=8333
rpcport=8332

# Mining
gen=0
genproclimit=4

# Connections
maxconnections=125
addnode=seed1.gxc.network
addnode=seed2.gxc.network

# RPC
rpcuser=gxcuser
rpcpassword=secure_password
rpcallowip=127.0.0.1
```

### Docker Deployment

```bash
# Build image
docker build -t gxc-node .

# Run container
docker run -d \
  --name gxc-node \
  -p 8333:8333 \
  -p 8332:8332 \
  -v gxc-data:/var/lib/gxc \
  gxc-node
```

---

## 📚 Additional Documentation

- [API Usage Guide](API_USAGE_GUIDE.md)
- [Mining Guide](MINING_ANALYSIS.md)
- [Staking System Design](STAKING_SYSTEM_DESIGN.md)
- [Traceability Design](TRACEABILITY_DESIGN.md)
- [Security Features](PRODUCTION_READY.md)
- [Validator Registration](VALIDATOR_REGISTRATION_COMPLETE.md)

---

## 🤝 Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

## 🔗 Links

- **GitHub:** https://github.com/philani1H/GXC-goldxcoin-blockchain-
- **Website:** https://goldxcoin.com
- **Explorer:** https://explorer.goldxcoin.com
- **Discord:** https://discord.gg/gxc

---

## 📞 Support

- **Issues:** https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues
- **Email:** support@goldxcoin.com
- **Discord:** https://discord.gg/gxc

---

**Built with ❤️ by the GXC Community**
