# 🔗 Connect to Stratum Server - Quick Commands

## ✅ Safe Deployment Confirmation

**The Stratum server deployment will NOT interfere with your existing blockchain node!**

- **Existing Node**: `https://gxc-chain112-blockchain-node-production.up.railway.app` (HTTP/RPC)
- **New Stratum Server**: `stratum+tcp://your-pool.up.railway.app:3333` (TCP)

They are **completely separate services** on different ports and protocols! ✅

## 📍 Your Stratum Server URL

After deploying, your Stratum URL will be:

```
stratum+tcp://gxc-pool-production.up.railway.app:3333
```

*(Replace `gxc-pool-production.up.railway.app` with your actual Railway domain)*

## 🚀 Commands to Connect

### Command 1: GXHash Python Miner

```bash
python3 mining/gxhash_miner.py \
  --pool stratum+tcp://gxc-pool-production.up.railway.app:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932
```

### Command 2: Using Environment Variables

```bash
export POOL_STRATUM_URL="stratum+tcp://gxc-pool-production.up.railway.app:3333"
export MINER_ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"
python3 mining/gxhash_miner.py
```

### Command 3: Testnet Miner (Updated)

```bash
# Edit mine_testnet_fixed.py to add:
POOL_STRATUM_URL = "stratum+tcp://gxc-pool-production.up.railway.app:3333"

# Then run:
python3 mine_testnet_fixed.py
```

### Command 4: Direct Python Connection

```python
import socket
import json

# Connect
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('gxc-pool-production.up.railway.app', 3333))

# Subscribe
sock.send((json.dumps({"id": 1, "method": "mining.subscribe", "params": []}) + "\n").encode())

# Authorize
sock.send((json.dumps({"id": 2, "method": "mining.authorize", "params": ["tGXC9fab7317231b966af85ac453e168c0932", "x"]}) + "\n").encode())
```

## 📋 Paths

### Stratum Server Code
```
Path: mining_pool/general_pool.py
Config: mining_pool/railway.toml
Deploy from: mining_pool/
```

### Miner Code
```
Path: mining/gxhash_miner.py
Testnet miner: mine_testnet_fixed.py
```

## 🔧 Quick Setup

### 1. Deploy Stratum Server

```bash
cd mining_pool
railway up
```

### 2. Get Your URL

After deployment, Railway gives you a domain like:
```
gxc-pool-production.up.railway.app
```

### 3. Connect Miner

```bash
python3 mining/gxhash_miner.py \
  --pool stratum+tcp://gxc-pool-production.up.railway.app:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932
```

## ✅ Summary

**Stratum URL Format:**
```
stratum+tcp://YOUR-RAILWAY-DOMAIN.up.railway.app:3333
```

**Miner Command:**
```bash
python3 mining/gxhash_miner.py --pool <STRATUM_URL> --user <WALLET_ADDRESS>
```

**Path:**
- Deploy from: `mining_pool/`
- Code: `mining_pool/general_pool.py`
- Config: `mining_pool/railway.toml`

**100% Safe - Won't touch existing node!** ✅
