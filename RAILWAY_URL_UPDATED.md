# ✅ Railway URL Updated in All Files

## 🎯 Railway Node URL

```
https://gxc-chain112-blockchain-node-production.up.railway.app
```

---

## 📁 Files Updated

### 1. Mining Scripts

✅ **mine_testnet_fixed.py**
```python
RPC_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
```

### 2. Wallet Service

✅ **api/wallet_service.py**
```python
NETWORK_INFO = {
    'rpc_url': 'https://gxc-chain112-blockchain-node-production.up.railway.app',
    'rest_url': 'https://gxc-chain112-blockchain-node-production.up.railway.app:8080',
}
```

### 3. Explorer Service

✅ **web/blockchain_explorer.py**
```python
BLOCKCHAIN_NODE_URL = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
```

### 4. Mining Pool

✅ **mining_pool/pool_base.py**
```python
self.rpc_url = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
```

### 5. GXHash Miner

✅ **mining/gxhash_miner.py**
```python
def __init__(self, rpc_url: str = "https://gxc-chain112-blockchain-node-production.up.railway.app"):
```

### 6. Configuration File

✅ **config/railway_config.py** (NEW)
- Central configuration for all services
- Easy to update in one place

---

## 🔧 Configuration File

Created `config/railway_config.py` for centralized configuration:

```python
# Railway Node URL
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# API Endpoints
RPC_URL = RAILWAY_NODE_URL
REST_URL = f"{RAILWAY_NODE_URL}:8080"
WEBSOCKET_URL = f"{RAILWAY_NODE_URL}:8081"

# Network Configuration
NETWORK = 'testnet'  # or 'mainnet'

# Mining Configuration
DEFAULT_MINER_ADDRESS = 'tGXC9fab7317231b966af85ac453e168c0932'
```

Usage:
```python
from config.railway_config import get_rpc_url, get_miner_address

rpc_url = get_rpc_url()
miner_address = get_miner_address()
```

---

## 🛠️ URL Update Script

Created `update_railway_url.py` to update all files at once:

```bash
# Update to default Railway URL
python update_railway_url.py

# Update to custom URL
python update_railway_url.py https://your-new-url.up.railway.app
```

This script automatically updates:
- All mining scripts
- Wallet service
- Explorer service
- Mining pools
- Configuration files

---

## 🧪 Test Your Connection

### 1. Test Railway Node

```bash
python test_railway_node.py https://gxc-chain112-blockchain-node-production.up.railway.app
```

Expected output:
```
Testing node at: https://gxc-chain112-blockchain-node-production.up.railway.app
============================================================

1. Testing getblocktemplate...
   ✅ getblocktemplate works!

2. Testing getblockcount...
   ✅ getblockcount works!

3. Testing getdifficulty...
   ✅ getdifficulty works!

============================================================
✅ Node is accessible and responding!
```

### 2. Test Mining

```bash
python mine_testnet_fixed.py
```

Expected output:
```
Mining to address: tGXC9fab7317231b966af85ac453e168c0932
Connected to node: https://gxc-chain112-blockchain-node-production.up.railway.app

Mining block 1...
✅ Block 1 mined successfully!
Reward: 12.5 GXC
```

### 3. Test Wallet Service

```bash
python api/wallet_service.py
```

Then open: http://localhost:5000

### 4. Test Explorer

```bash
python web/blockchain_explorer.py
```

Then open: http://localhost:3000

---

## 📊 Service Endpoints

### Railway Node (Production)

```
RPC:       https://gxc-chain112-blockchain-node-production.up.railway.app
REST:      https://gxc-chain112-blockchain-node-production.up.railway.app:8080
WebSocket: https://gxc-chain112-blockchain-node-production.up.railway.app:8081
```

### Local Services

```
Wallet:    http://localhost:5000
Explorer:  http://localhost:3000
```

---

## 🔗 Architecture

```
┌─────────────────────────────────────────────┐
│         Railway Cloud                       │
│                                             │
│   ┌─────────────────────────────────┐      │
│   │  GXC Blockchain Node            │      │
│   │  https://gxc-chain112-...       │      │
│   │                                 │      │
│   │  ✅ RPC API                     │      │
│   │  ✅ REST API                    │      │
│   │  ✅ WebSocket API               │      │
│   └─────────────────────────────────┘      │
│                                             │
└─────────────────────────────────────────────┘
                    ↑
                    │ HTTPS
                    │
    ┌───────────────┼───────────────┐
    │               │               │
    ↓               ↓               ↓
┌────────┐    ┌─────────┐    ┌──────────┐
│ Miner  │    │ Wallet  │    │ Explorer │
│(Local) │    │(Local)  │    │(Local)   │
└────────┘    └─────────┘    └──────────┘

All services connect to Railway node!
```

---

## 🚀 Quick Start

### 1. Verify Connection

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"1","method":"getblockcount","params":[]}'
```

### 2. Start Mining

```bash
python mine_testnet_fixed.py
```

### 3. Check Balance

```bash
python check_balance.py tGXC9fab7317231b966af85ac453e168c0932
```

### 4. Start Wallet

```bash
python api/wallet_service.py
```

### 5. Start Explorer

```bash
python web/blockchain_explorer.py
```

---

## 📝 Environment Variables

You can also use environment variables instead of hardcoded URLs:

```bash
# Set Railway URL
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
export BLOCKCHAIN_REST_URL=https://gxc-chain112-blockchain-node-production.up.railway.app:8080
export BLOCKCHAIN_WS_URL=https://gxc-chain112-blockchain-node-production.up.railway.app:8081

# Set miner address
export MINER_ADDRESS=tGXC9fab7317231b966af85ac453e168c0932

# Set network
export GXC_NETWORK=testnet

# Run services
python mine_testnet_fixed.py
python api/wallet_service.py
python web/blockchain_explorer.py
```

---

## 🔄 Update URL in Future

If you deploy a new Railway instance or change the URL:

### Option 1: Use Update Script

```bash
python update_railway_url.py https://your-new-url.up.railway.app
```

### Option 2: Update Config File

Edit `config/railway_config.py`:
```python
RAILWAY_NODE_URL = "https://your-new-url.up.railway.app"
```

### Option 3: Use Environment Variables

```bash
export BLOCKCHAIN_RPC_URL=https://your-new-url.up.railway.app
```

---

## ✅ Summary

```
╔════════════════════════════════════════════════════════╗
║                                                        ║
║   ✅ ALL FILES UPDATED                                 ║
║                                                        ║
║   Railway URL:                                         ║
║   https://gxc-chain112-blockchain-node-                ║
║   production.up.railway.app                            ║
║                                                        ║
║   Updated Files:                                       ║
║   ✅ mine_testnet_fixed.py                            ║
║   ✅ api/wallet_service.py                            ║
║   ✅ web/blockchain_explorer.py                       ║
║   ✅ mining_pool/pool_base.py                         ║
║   ✅ mining/gxhash_miner.py                           ║
║                                                        ║
║   New Files:                                           ║
║   ✅ config/railway_config.py                         ║
║   ✅ update_railway_url.py                            ║
║                                                        ║
║   Ready to:                                            ║
║   ✅ Mine blocks                                      ║
║   ✅ Create wallets                                   ║
║   ✅ Explore blockchain                               ║
║   ✅ Run mining pools                                 ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

---

## 🎉 All Set!

Your GXC blockchain is now fully configured to use the Railway node!

**Start mining:**
```bash
python mine_testnet_fixed.py
```

**Happy mining!** ⛏️💰
