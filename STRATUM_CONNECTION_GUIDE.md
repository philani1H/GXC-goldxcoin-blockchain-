# 🔗 Stratum Server Connection Guide

## ✅ Safe Deployment - Won't Interfere with Existing Node

**The Stratum server is a SEPARATE service** - it will NOT interfere with your already deployed blockchain node!

- **Blockchain Node**: `https://gxc-chain112-blockchain-node-production.up.railway.app` (RPC/HTTP)
- **Stratum Server**: `stratum+tcp://your-pool.up.railway.app:3333` (TCP for miners)

They are **completely separate** services! ✅

## 📍 Stratum Server URLs

After deploying the Stratum server to Railway, you'll get a URL like:

```
stratum+tcp://gxc-pool-production.up.railway.app:3333
```

**Format:**
- Protocol: `stratum+tcp://`
- Domain: `your-pool-name.up.railway.app`
- Port: `3333`

## 🚀 Commands to Connect Miners

### 1. GXHash Python Miner

```bash
python3 mining/gxhash_miner.py \
  --pool stratum+tcp://gxc-pool-production.up.railway.app:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932
```

**Or set environment variable:**
```bash
export POOL_STRATUM_URL="stratum+tcp://gxc-pool-production.up.railway.app:3333"
export MINER_ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"
python3 mining/gxhash_miner.py
```

### 2. Testnet Miner (Simple)

```bash
python3 mine_testnet_fixed.py
```

**Update the file to use Stratum:**
```python
POOL_STRATUM_URL = "stratum+tcp://gxc-pool-production.up.railway.app:3333"
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
```

### 3. Using External Miners (CGMiner, BFGMiner, etc.)

```bash
# CGMiner
cgminer --stratum-url stratum+tcp://gxc-pool-production.up.railway.app:3333 \
        --user tGXC9fab7317231b966af85ac453e168c0932 \
        --pass x

# BFGMiner
bfgminer --stratum-url stratum+tcp://gxc-pool-production.up.railway.app:3333 \
         --user tGXC9fab7317231b966af85ac453e168c0932 \
         --pass x
```

### 4. Using Python Requests (Direct Connection)

```python
import socket
import json

# Connect to Stratum server
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('gxc-pool-production.up.railway.app', 3333))

# Subscribe
subscribe = {
    "id": 1,
    "method": "mining.subscribe",
    "params": []
}
sock.send((json.dumps(subscribe) + "\n").encode())

# Authorize
authorize = {
    "id": 2,
    "method": "mining.authorize",
    "params": ["tGXC9fab7317231b966af85ac453e168c0932", "x"]
}
sock.send((json.dumps(authorize) + "\n").encode())
```

## 📋 Paths and URLs Summary

### Blockchain Node (Already Deployed)
```
URL: https://gxc-chain112-blockchain-node-production.up.railway.app
Port: 443 (HTTPS) or Railway's PORT
Type: HTTP/RPC server
Purpose: Blockchain node, RPC calls, block submission
```

### Stratum Server (To Deploy)
```
URL: stratum+tcp://gxc-pool-production.up.railway.app:3333
Port: 3333 (TCP)
Type: TCP/Stratum server
Purpose: Miner connections, share submission
Path: mining_pool/general_pool.py
```

### Pool Dashboard (Web Interface)
```
URL: https://gxc-pool-production.up.railway.app
Port: Railway's PORT (auto-provided)
Type: HTTP/Web server
Purpose: Web dashboard, statistics, miner registration
Path: mining_pool/general_pool.py (same file, different port)
```

## 🔧 Environment Variables for Miners

Set these before running miners:

```bash
# Stratum Pool URL
export POOL_STRATUM_URL="stratum+tcp://gxc-pool-production.up.railway.app:3333"

# Your wallet address (username for Stratum)
export MINER_ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

# Blockchain node (for direct mining, not needed for pool)
export RAILWAY_NODE_URL="https://gxc-chain112-blockchain-node-production.up.railway.app"
export BLOCKCHAIN_RPC_URL="https://gxc-chain112-blockchain-node-production.up.railway.app"
```

## 📝 Quick Connection Test

### Test 1: Check if Stratum server is running

```bash
# Test TCP connection
nc -zv gxc-pool-production.up.railway.app 3333

# Or use telnet
telnet gxc-pool-production.up.railway.app 3333
```

### Test 2: Connect with Python

```python
import socket
import json

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    sock.connect(('gxc-pool-production.up.railway.app', 3333))
    print("✅ Connected to Stratum server!")
    
    # Send subscribe
    subscribe = {"id": 1, "method": "mining.subscribe", "params": []}
    sock.send((json.dumps(subscribe) + "\n").encode())
    
    # Receive response
    response = sock.recv(1024).decode()
    print(f"Response: {response}")
    sock.close()
except Exception as e:
    print(f"❌ Connection failed: {e}")
```

## 🎯 Complete Example

### Step 1: Deploy Stratum Server

```bash
cd mining_pool
railway up
```

### Step 2: Get Your Public URL

After deployment, Railway will give you a URL like:
```
gxc-pool-production.up.railway.app
```

### Step 3: Connect Miner

```bash
# Set environment variables
export POOL_STRATUM_URL="stratum+tcp://gxc-pool-production.up.railway.app:3333"
export MINER_ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

# Run miner
python3 mining/gxhash_miner.py
```

### Step 4: Verify Connection

Check Railway logs:
```bash
railway logs
```

You should see:
```
[pool] Stratum server listening on port 3333
[pool] Miner connected: <IP>:<PORT>
[pool] Share accepted from tGXC9fab7317231b966af85ac453e168c0932
```

## ⚠️ Important Notes

1. **Different Services**: Stratum server and blockchain node are separate
2. **Different Ports**: 
   - Blockchain node: HTTP (443/HTTPS)
   - Stratum server: TCP (3333)
3. **Different Protocols**:
   - Blockchain node: HTTP/RPC (JSON-RPC)
   - Stratum server: Stratum protocol (TCP)
4. **No Interference**: They don't conflict - completely separate!

## 📊 Connection Flow

```
Miner → Stratum Server (port 3333) → Blockchain Node (HTTPS)
         ↓
    Pool Dashboard (Railway PORT)
```

## ✅ Summary

**Stratum Server URL:**
```
stratum+tcp://gxc-pool-production.up.railway.app:3333
```

**Miner Command:**
```bash
python3 mining/gxhash_miner.py \
  --pool stratum+tcp://gxc-pool-production.up.railway.app:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932
```

**Path:**
- Stratum server code: `mining_pool/general_pool.py`
- Deployment config: `mining_pool/railway.toml`
- Deploy from: `mining_pool/` directory

**Safe to deploy - won't affect existing node!** ✅
