# 🔗 Blockchain Explorer - Network Configuration Guide

## 📋 How Network Differentiation Works

The blockchain explorer can connect to **both mainnet and testnet** networks. Here's how it differentiates between them:

### 🔧 Network Detection

The explorer determines which network to use based on the `GXC_NETWORK` environment variable:

```bash
# For Testnet (default)
export GXC_NETWORK=testnet

# For Mainnet
export GXC_NETWORK=mainnet
```

If not set, it defaults to **testnet**.

### 🌐 Network Configuration

Each network has different settings:

| Setting | Testnet | Mainnet |
|---------|---------|---------|
| **Address Prefix** | `tGXC` | `GXC` |
| **Block Time** | 60 seconds | 600 seconds (10 minutes) |
| **Block Reward** | 12.5 GXC | 50.0 GXC |
| **Difficulty** | 0.1 | 1.0 |
| **RPC Port (local)** | 18332 | 8332 |
| **Railway URL** | `https://gxc-chain112-blockchain-node-production.up.railway.app` | (Set when deployed) |

### 💾 Database Separation

**Important:** The explorer uses **separate databases** for each network to avoid mixing data:

- **Testnet Database:** `gxc_explorer_testnet.db`
- **Mainnet Database:** `gxc_explorer_mainnet.db`

This means:
- ✅ **Testnet data** is stored separately from **mainnet data**
- ✅ You can switch between networks without data conflicts
- ✅ Each network maintains its own block history, transactions, and addresses

### 📊 What Data You Will See

The explorer shows **ALL data from the connected blockchain node**:

#### ✅ Blocks Data
- Block number, hash, previous hash
- Timestamp
- Miner address
- Difficulty, nonce
- Transaction count
- Block reward

#### ✅ Transactions Data
- Transaction hash
- From/to addresses
- Amount/value
- Fees
- Block number
- Status
- Traceability information

#### ✅ Address Data
- Balance
- Transaction history
- Sent/received amounts

#### ✅ Network Statistics
- Total blocks
- Total transactions
- Total addresses
- Average block time
- Current difficulty
- Total supply

### 🔄 How to Switch Networks

#### Option 1: Environment Variable (Recommended)

```bash
# For Testnet
export GXC_NETWORK=testnet
python web/blockchain_explorer.py

# For Mainnet
export GXC_NETWORK=mainnet
python web/blockchain_explorer.py
```

#### Option 2: Railway/Vercel Environment Variable

Set in your deployment platform:
- `GXC_NETWORK=testnet` or `GXC_NETWORK=mainnet`

#### Option 3: Direct Node URL Override

You can override the node URL directly:

```bash
export BLOCKCHAIN_NODE_URL=https://your-mainnet-node-url.com
export GXC_NETWORK=mainnet
```

### 🎯 Current Setup

**Default Configuration:**
- **Network:** Testnet
- **Node URL:** `https://gxc-chain112-blockchain-node-production.up.railway.app`
- **Database:** `gxc_explorer_testnet.db`
- **Address Prefix:** `tGXC`

### ⚠️ Important Notes

1. **Separate Databases:** Mainnet and testnet data are stored in separate database files
2. **Network-Specific Addresses:** Testnet addresses start with `tGXC`, mainnet with `GXC`
3. **Different Block Rewards:** Testnet rewards 12.5 GXC per block, mainnet rewards 50 GXC
4. **Automatic Detection:** The explorer automatically detects the network based on the node URL and environment variable

### 📡 API Endpoints

Get current network information:

```bash
GET /api/network-info
```

Returns:
```json
{
  "network": "TESTNET",
  "address_prefix": "tGXC",
  "block_reward": 12.5,
  "block_time": 60,
  "difficulty": 0.1,
  "is_testnet": true,
  "is_mainnet": false,
  "database_path": "gxc_explorer_testnet.db"
}
```

### 🔍 Verifying Network

The explorer displays network information in:
- UI header/banner showing "TESTNET" or "MAINNET"
- Block reward amounts (12.5 for testnet, 50 for mainnet)
- Address prefixes in addresses
- Network info API endpoint

### 🚀 Deployment Examples

#### Railway Deployment (Testnet)

```toml
[build]
builder = "nixpacks"

[deploy]
startCommand = "python web/blockchain_explorer.py"

[env]
GXC_NETWORK = "testnet"
BLOCKCHAIN_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
```

#### Railway Deployment (Mainnet)

```toml
[env]
GXC_NETWORK = "mainnet"
BLOCKCHAIN_NODE_URL = "https://your-mainnet-node-url.com"
```

### ✅ Summary

**Yes, you will see ALL data from the blockchain node you're connected to!**

- The explorer fetches and displays **all blocks** from the connected node
- It stores **all transactions** from those blocks
- It tracks **all addresses** that appear in transactions
- It shows **real-time network statistics**

The network differentiation ensures:
- ✅ Data is organized by network (testnet vs mainnet)
- ✅ No data mixing between networks
- ✅ Easy switching between networks
- ✅ Clear indication of which network you're viewing

