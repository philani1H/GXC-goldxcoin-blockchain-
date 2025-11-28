# 🚂 Railway Deployment Guide for GXC Blockchain

## ⏱️ Mining Time Comparison

### Testnet:
```
Block Time: 60 seconds (1 minute)
Difficulty: 0.1 (very easy)
Reward: 12.5 GXC
Mining Speed: ~2 seconds per block (tested)
```

### Mainnet (C++ Node):
```
Block Time: 600 seconds (10 minutes) ⚠️
Difficulty: Higher (adjusts automatically)
Reward: 50 GXC
Mining Speed: Depends on difficulty and hashrate
```

**⚠️ IMPORTANT:** Mainnet blocks take ~10 minutes each (like Bitcoin), NOT 2 seconds!

---

## 🔧 Mainnet Configuration Status

### Current Configuration:
```cpp
// From src/Config.cpp
target_block_time = 600 seconds (10 minutes)
difficulty_adjustment_interval = 10 blocks
initial_difficulty = (needs to be set)
block_reward = 50 GXC (standard)
```

### What This Means:
- First block: Could take 10+ minutes
- Difficulty adjusts every 10 blocks
- Mining is MUCH harder than testnet
- Need proper mining hardware for mainnet

---

## 🚂 Railway Deployment Steps

### Step 1: Prepare Railway Project

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login to Railway
railway login

# Create new project
railway init
```

### Step 2: Create Railway Configuration

Create `railway.json`:
```json
{
  "build": {
    "builder": "NIXPACKS",
    "buildCommand": "echo 'Using pre-built binaries'"
  },
  "deploy": {
    "startCommand": "./start_mainnet.sh",
    "restartPolicyType": "ON_FAILURE",
    "restartPolicyMaxRetries": 10
  }
}
```

### Step 3: Create Startup Script

Create `start_mainnet.sh`:
```bash
#!/bin/bash

echo "Starting GXC Mainnet Node on Railway"

# Set environment variables
export GXC_NETWORK=mainnet
export GXC_DATA_DIR=./gxc_data
export GXC_RPC_PORT=${PORT:-8332}
export GXC_REST_PORT=8080
export GXC_MINER_ADDRESS=${MINER_ADDRESS:-GXC21d83ca607604df4f86bd60761e3c4ee30}

# Create data directory
mkdir -p $GXC_DATA_DIR

# Start blockchain node
./packages/gxc-miners-cli-linux/gxc-node \
  --datadir=$GXC_DATA_DIR \
  --rpc-port=$GXC_RPC_PORT \
  --rest-port=$GXC_REST_PORT \
  --verbose

# If node fails, start Python testnet for testing
if [ $? -ne 0 ]; then
    echo "C++ node failed, starting Python testnet node"
    python3 testnet_blockchain_node.py
fi
```

### Step 4: Create Procfile

Create `Procfile`:
```
web: ./start_mainnet.sh
```

### Step 5: Set Railway Environment Variables

```bash
# Set via Railway CLI
railway variables set MINER_ADDRESS=GXC21d83ca607604df4f86bd60761e3c4ee30
railway variables set GXC_NETWORK=mainnet
railway variables set NODE_ENV=production

# Or via Railway Dashboard:
# Settings → Variables → Add Variable
```

### Step 6: Deploy

```bash
# Add files
git add railway.json start_mainnet.sh Procfile

# Commit
git commit -m "Add Railway deployment configuration"

# Push to Railway
railway up

# Or link to GitHub and auto-deploy
railway link
git push origin master
```

---

## ⚠️ Railway Limitations

### 1. Persistent Storage
```
Problem: Railway restarts can lose data
Solution: Use Railway Volumes

# Add volume in railway.toml
[volumes]
  [volumes.blockchain_data]
    mountPath = "/app/gxc_data"
```

### 2. Port Configuration
```
Problem: Railway assigns random PORT
Solution: Use $PORT environment variable

# In your code
port = os.environ.get('PORT', 8332)
```

### 3. Memory Limits
```
Problem: Free tier has 512MB RAM limit
Solution: Upgrade to Pro ($5/month) for 8GB RAM

Blockchain needs:
- Minimum: 1GB RAM
- Recommended: 4GB RAM
```

### 4. CPU Limits
```
Problem: Shared CPU on free tier
Solution: Upgrade for dedicated CPU

Mining needs:
- Minimum: 1 vCPU
- Recommended: 2+ vCPUs
```

---

## 🔧 Mainnet vs Testnet Configuration

### Create Unified Node Script

Create `blockchain_node_unified.py`:
```python
#!/usr/bin/env python3
"""
Unified GXC Blockchain Node - Supports both testnet and mainnet
"""

import os
import sys

# Get network from environment
NETWORK = os.environ.get('GXC_NETWORK', 'testnet')

if NETWORK == 'mainnet':
    # Mainnet configuration
    RPC_PORT = int(os.environ.get('PORT', 8332))
    REST_PORT = 8080
    P2P_PORT = 9333
    DATA_DIR = './gxc_data'
    BLOCK_TIME = 600  # 10 minutes
    INITIAL_DIFFICULTY = 1.0  # Higher difficulty
    BLOCK_REWARD = 50.0
    ADDRESS_PREFIX = 'GXC'
    NETWORK_ID = 'mainnet'
else:
    # Testnet configuration
    RPC_PORT = int(os.environ.get('PORT', 18332))
    REST_PORT = 18080
    P2P_PORT = 19333
    DATA_DIR = './gxc_testnet_data'
    BLOCK_TIME = 60  # 1 minute
    INITIAL_DIFFICULTY = 0.1  # Easy
    BLOCK_REWARD = 12.5
    ADDRESS_PREFIX = 'tGXC'
    NETWORK_ID = 'testnet'

print(f"Starting GXC Node - {NETWORK_ID.upper()}")
print(f"RPC Port: {RPC_PORT}")
print(f"Block Time: {BLOCK_TIME} seconds")
print(f"Block Reward: {BLOCK_REWARD} GXC")
print(f"Difficulty: {INITIAL_DIFFICULTY}")

# Import and start node
# (Use your existing testnet_blockchain_node.py code here)
# Just replace the constants with the variables above
```

---

## 📋 Railway Deployment Checklist

### Before Deployment:
- [ ] Railway account created
- [ ] Railway CLI installed
- [ ] Project initialized
- [ ] Environment variables set
- [ ] Startup scripts created
- [ ] Volume configured (for data persistence)
- [ ] Upgraded to Pro tier (recommended)

### Configuration Files Needed:
- [ ] `railway.json` - Railway config
- [ ] `Procfile` - Process definition
- [ ] `start_mainnet.sh` - Startup script
- [ ] `railway.toml` - Volume configuration
- [ ] `.railwayignore` - Ignore unnecessary files

### After Deployment:
- [ ] Check logs: `railway logs`
- [ ] Verify node started
- [ ] Check RPC endpoint
- [ ] Test mining
- [ ] Monitor performance
- [ ] Set up alerts

---

## 🎯 Recommended Railway Setup

### Option 1: Testnet First (Recommended)
```bash
# Deploy testnet to Railway
railway variables set GXC_NETWORK=testnet
railway up

# Test everything works
# Mine a few blocks
# Verify all features

# Then switch to mainnet
railway variables set GXC_NETWORK=mainnet
railway restart
```

### Option 2: Mainnet Only
```bash
# Deploy mainnet directly
railway variables set GXC_NETWORK=mainnet
railway variables set MINER_ADDRESS=GXC21d83ca607604df4f86bd60761e3c4ee30
railway up

# Start mining (will take ~10 minutes per block)
```

### Option 3: Both Networks
```bash
# Deploy two separate Railway services
# Service 1: Testnet
railway init testnet-gxc
railway variables set GXC_NETWORK=testnet

# Service 2: Mainnet
railway init mainnet-gxc
railway variables set GXC_NETWORK=mainnet
```

---

## ⏱️ Expected Mining Times

### Testnet (Tested):
```
First block: ~2 seconds ✅
Subsequent blocks: ~2 seconds each ✅
10 blocks: ~20 seconds ✅
100 blocks: ~3 minutes ✅
```

### Mainnet (Estimated):
```
First block: ~10 minutes ⚠️
Subsequent blocks: ~10 minutes each
10 blocks: ~100 minutes (~1.5 hours)
100 blocks: ~1000 minutes (~16 hours)

Note: Times depend on:
- Your hashrate
- Network difficulty
- Number of miners
```

---

## 🔧 Create Mainnet Miner

Create `mine_mainnet.py`:
```python
#!/usr/bin/env python3
"""
GXC Mainnet Miner
"""

import requests
import hashlib
import time

MINER_ADDRESS = "GXC21d83ca607604df4f86bd60761e3c4ee30"
RPC_URL = "http://localhost:8332"  # or Railway URL

# Same code as mine_testnet_fixed.py
# But with mainnet address and RPC port

# Note: Mining will be MUCH slower on mainnet!
# Expect 10+ minutes per block
```

---

## 📊 Railway Costs

### Free Tier:
```
- 512MB RAM
- Shared CPU
- $5 credit/month
- Good for: Testing only
```

### Pro Tier ($5/month):
```
- 8GB RAM
- Dedicated CPU
- Unlimited projects
- Good for: Mainnet node
```

### Recommended for GXC:
```
Pro Tier + Additional Resources
- 4GB RAM minimum
- 2 vCPUs
- 20GB storage
Total: ~$10-15/month
```

---

## ⚠️ Important Warnings

### 1. Mining Time
```
❌ Mainnet is NOT like testnet!
❌ Blocks take 10 minutes, not 2 seconds
❌ Need proper mining hardware
❌ Railway CPU may be too slow

✅ Consider:
- Dedicated mining server
- GPU mining
- Mining pool
```

### 2. Data Persistence
```
❌ Railway can restart and lose data
✅ MUST use Railway Volumes
✅ MUST backup blockchain data
✅ MUST backup wallet keys
```

### 3. Costs
```
❌ Free tier insufficient for mainnet
✅ Need Pro tier minimum
✅ Budget $10-20/month
✅ Monitor usage
```

---

## 🎯 My Recommendation

### For Railway Deployment:

**Option A: Testnet on Railway (Best for Testing)**
```
✅ Deploy testnet to Railway
✅ Test all features
✅ Low cost
✅ Fast mining
✅ Perfect for development
```

**Option B: Mainnet on Dedicated Server (Best for Production)**
```
✅ Deploy mainnet to VPS (DigitalOcean, AWS, etc.)
✅ More control
✅ Better performance
✅ Persistent storage
✅ Suitable for mining
```

**Option C: Hybrid**
```
✅ Testnet on Railway (testing)
✅ Mainnet on dedicated server (production)
✅ Best of both worlds
```

---

## 🚀 Quick Start for Railway

```bash
# 1. Install Railway CLI
npm install -g @railway/cli

# 2. Login
railway login

# 3. Initialize project
cd /path/to/GXC-goldxcoin-blockchain-
railway init

# 4. Set variables
railway variables set GXC_NETWORK=testnet
railway variables set MINER_ADDRESS=tGXC9fab7317231b966af85ac453e168c0932

# 5. Deploy
railway up

# 6. Check logs
railway logs

# 7. Get URL
railway domain
```

---

## 📋 Summary

```
╔════════════════════════════════════════════════════════╗
║                                                        ║
║   ⏱️  MINING TIMES                                     ║
║                                                        ║
║   Testnet:  ~2 seconds per block ✅                   ║
║   Mainnet:  ~10 minutes per block ⚠️                  ║
║                                                        ║
║   🚂 RAILWAY DEPLOYMENT                                ║
║                                                        ║
║   Testnet:  ✅ Works great                            ║
║   Mainnet:  ⚠️  Need Pro tier + resources             ║
║                                                        ║
║   💡 RECOMMENDATION                                    ║
║                                                        ║
║   Testnet → Railway ✅                                ║
║   Mainnet → Dedicated Server ✅                       ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

**Mainnet mining is MUCH slower than testnet!** ⚠️

**Railway works, but dedicated server is better for mainnet!** ✅

---

## 📁 Files to Create

1. `railway.json` - Railway configuration
2. `Procfile` - Process definition
3. `start_mainnet.sh` - Startup script
4. `railway.toml` - Volume configuration
5. `blockchain_node_unified.py` - Unified node
6. `mine_mainnet.py` - Mainnet miner

Want me to create these files for you?
