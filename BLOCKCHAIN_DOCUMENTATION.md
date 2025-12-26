# GXC Blockchain - Complete Documentation

## Overview

GXC (GoldXCoin) is a production-ready blockchain with:
- Hybrid PoW/PoS consensus
- Multiple mining algorithms (SHA256, GXHASH, ETHASH)
- Validator system with staking
- Complete transaction traceability
- AI-powered security
- Stock token support
- Cross-chain bridge

---

## Core Components

### 1. Blockchain Node
**File:** `src/Blockchain.cpp`

**Features:**
- Block creation and validation
- Transaction processing
- UTXO management
- Consensus enforcement
- Database persistence

**Key Methods:**
```cpp
bool addBlock(const Block& block);
Block getLatestBlock();
bool validateBlock(const Block& block);
Transaction createCoinbaseTransaction(address, amount);
```

### 2. Consensus System

**Hybrid PoW/PoS:**
- Even blocks: Proof of Work
- Odd blocks: Proof of Stake
- Automatic switching based on block height

**Mining Algorithms:**
1. SHA256 - Bitcoin compatible
2. GXHASH - ASIC resistant
3. ETHASH - GPU optimized

### 3. Validator System
**File:** `src/Validator.cpp`

**Features:**
- Validator registration
- Stake-weighted selection
- Performance tracking
- Reward distribution

**Validator Selection:**
```
Weight = Stake × sqrt(Days/365)
Probability = Weight / TotalWeight
```

### 4. Transaction System
**File:** `src/Transaction.cpp`

**Transaction Types:**
- NORMAL - Regular transfers
- STAKE - Lock coins for staking
- UNSTAKE - Unlock staked coins
- COINBASE - Mining rewards
- REWARD - Staking rewards

### 5. RPC API
**File:** `src/RPCAPI.cpp`

**110+ RPC Methods:**
- Blockchain queries
- Transaction submission
- Wallet operations
- Mining operations
- Staking operations
- Validator management

---

## Key Features

### Complete Traceability
Every transaction links to previous:
```
Transaction.inputs[0].txHash = PreviousTransaction.hash
Transaction.inputs[0].amount = PreviousTransaction.outputs[n].amount
```

### AI Security Engine
**File:** `src/security/AIHashrateSentinel.cpp`

**Protections:**
- Hashrate spike detection
- Difficulty manipulation prevention
- Stake attack detection
- Fee manipulation detection

### Dynamic Fees
**File:** `src/Transaction.cpp`

Fees adjust based on:
- Network congestion
- Transaction size
- Priority level

### Stock Tokens
**File:** `src/StockContractModels.cpp`

Synthetic stock tokens:
- Real-time price feeds
- Trading on blockchain
- Collateralized positions

---

## Network Specifications

### Mainnet
- Address prefix: `GXC`
- Block time: 600 seconds (10 min)
- Difficulty: 1.0
- RPC port: 8332
- P2P port: 8333

### Testnet
- Address prefix: `tGXC`
- Block time: 120 seconds (2 min)
- Difficulty: 0.1
- RPC port: 8545
- P2P port: 8546

---

## Building

### Prerequisites
```bash
sudo apt-get install build-essential cmake
sudo apt-get install libleveldb-dev libsqlite3-dev
```

### Compile
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run Node
```bash
./gxc-node --testnet
```

---

## Mining

### Start Mining
```bash
# SHA256
./gxc-miner --address YOUR_ADDRESS --algorithm sha256

# GXHASH
./gxc-gxhash-miner --address YOUR_ADDRESS

# ETHASH
./gxc-ethash-miner --address YOUR_ADDRESS
```

### Mining Pools
```bash
# Connect to pool
./gxc-miner --pool stratum+tcp://pool.gxc.com:3333 --user YOUR_ADDRESS
```

---

## Staking

### Stake Coins
```bash
curl -X POST http://localhost:8332 \
  -d '{"method":"stake","params":["ADDRESS",1000,90],"id":1}'
```

### Become Validator
```bash
curl -X POST http://localhost:8332 \
  -d '{"method":"registervalidator","params":["ADDRESS",10000,365],"id":1}'
```

---

## API Reference

### Get Blockchain Info
```
POST /
{"method":"getblockchaininfo","params":[],"id":1}
```

### Get Block
```
POST /
{"method":"getblock","params":[HEIGHT],"id":1}
```

### Send Transaction
```
POST /
{"method":"sendtoaddress","params":["TO_ADDRESS",AMOUNT],"id":1}
```

### List Validators
```
POST /
{"method":"listvalidators","params":[],"id":1}
```

---

## Database Structure

### Blockchain DB
- Blocks table
- Transactions table
- UTXO set
- Chain state

### Staking Pool DB
- Validators table
- Stakes table
- Rewards table

### Wallet DB
- Addresses table
- Keys table
- Transaction history

---

## Security

### Attack Prevention
1. 51% attack - Hybrid consensus
2. Double spend - UTXO validation
3. Stake grinding - Weighted selection
4. Fee manipulation - Dynamic fees
5. Inflation - Fixed supply cap

### Cryptography
- SHA256 for hashing
- ECDSA for signatures
- Keccak256 for addresses

---

## Testing

### Run Tests
```bash
./test_complete_functionality.py
./test_staking_complete.py
./test_mining_complete.sh
```

### Stress Tests
```bash
./stress_test_60000_miners.py
./stress_test_validators.py
./stress_test_billions_tps.py
```

---

## Deployment

### Production Node
```bash
./gxc-node --rpc-port=8332 --p2p-port=8333
```

### Docker
```bash
docker-compose up -d
```

### Railway
```bash
railway up
```

---

## Documentation Files

See these files for details:
- `README.md` - Main documentation
- `QUICK_START.md` - Getting started
- `API_COMPLETE_REFERENCE.md` - Full API docs
- `MINING_GUIDE.md` - Mining instructions
- `STAKING_SYSTEM_COMPLETE.md` - Staking guide
- `VALIDATOR_REGISTRATION_COMPLETE.md` - Validator guide
- `WALLET_GUIDE.md` - Wallet usage
- `DEPLOYMENT_GUIDE.md` - Deployment instructions

---

## Support

- GitHub: https://github.com/philani1H/GXC-goldxcoin-blockchain-
- Issues: https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues
