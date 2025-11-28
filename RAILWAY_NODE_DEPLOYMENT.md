# 🚂 Railway Blockchain Node Deployment Guide

## 📋 Overview

This guide shows you how to deploy ONLY the GXC blockchain node to Railway. Everything else (wallet, miner, explorer) will connect to it via URL.

```
┌─────────────────────────────────────────────┐
│                                             │
│         Railway (Cloud)                     │
│                                             │
│   ┌─────────────────────────────────┐      │
│   │  GXC Blockchain Node            │      │
│   │  - Stores blocks                │      │
│   │  - Validates transactions       │      │
│   │  - Provides RPC API             │      │
│   │  - Public URL                   │      │
│   └─────────────────────────────────┘      │
│                                             │
└─────────────────────────────────────────────┘
                    ↑
                    │ HTTP/RPC
                    │
    ┌───────────────┼───────────────┐
    │               │               │
    ↓               ↓               ↓
┌────────┐    ┌─────────┐    ┌──────────┐
│ Wallet │    │  Miner  │    │ Explorer │
│ (Local)│    │ (Local) │    │ (Local)  │
└────────┘    └─────────┘    └──────────┘

All connect to Railway node via URL!
```

---

## ✅ What's Already Done

I've already configured:
- ✅ `railway.json` - Railway configuration
- ✅ `Procfile` - Process definition
- ✅ `testnet_blockchain_node.py` - Updated to use PORT env variable
- ✅ `testnet_blockchain_node.py` - Updated to bind to 0.0.0.0 for Railway

---

## 🚀 Quick Deploy to Railway

### Step 1: Install Railway CLI

```bash
# Install Railway CLI
npm install -g @railway/cli

# Or use curl
curl -fsSL https://railway.app/install.sh | sh
```

### Step 2: Login to Railway

```bash
railway login
```

This will open your browser to authenticate.

### Step 3: Initialize Project

```bash
cd /workspaces/GXC-goldxcoin-blockchain-

# Initialize Railway project
railway init
```

Choose:
- Project name: `gxc-blockchain-node`
- Start from: `Empty project`

### Step 4: Link to GitHub (Optional but Recommended)

```bash
# Link to your GitHub repo
railway link

# Or deploy directly
railway up
```

### Step 5: Deploy

```bash
# Deploy the blockchain node
railway up

# This will:
# 1. Upload your code
# 2. Install dependencies (requirements.txt)
# 3. Start testnet_blockchain_node.py
# 4. Give you a public URL
```

### Step 6: Get Your Node URL

```bash
# Generate a public domain
railway domain

# You'll get something like:
# https://gxc-blockchain-node-production.up.railway.app
```

---

## 🔧 Configuration Files

### railway.json (Already Created)

```json
{
  "$schema": "https://railway.app/railway.schema.json",
  "build": {
    "builder": "NIXPACKS",
    "buildCommand": "pip install -r requirements.txt"
  },
  "deploy": {
    "startCommand": "python testnet_blockchain_node.py",
    "restartPolicyType": "ON_FAILURE",
    "restartPolicyMaxRetries": 10,
    "healthcheckPath": "/health",
    "healthcheckTimeout": 100
  }
}
```

### Procfile (Already Created)

```
web: python testnet_blockchain_node.py
```

### Environment Variables (Set These)

```bash
# Optional: Set custom variables
railway variables set NETWORK=testnet
railway variables set LOG_LEVEL=INFO
```

---

## 📡 Connect Your Local Tools to Railway Node

Once deployed, you'll get a URL like:
```
https://gxc-blockchain-node-production.up.railway.app
```

### Connect Miner

Edit `mine_testnet_fixed.py`:

```python
# Change this line:
RPC_URL = "http://localhost:18332"

# To your Railway URL:
RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"
```

Then run:
```bash
python mine_testnet_fixed.py
```

### Connect Wallet

Edit `api/wallet_service.py` or set environment variable:

```bash
export BLOCKCHAIN_RPC_URL=https://gxc-blockchain-node-production.up.railway.app
python api/wallet_service.py
```

### Connect Explorer

Edit explorer configuration:

```bash
export BLOCKCHAIN_NODE_URL=https://gxc-blockchain-node-production.up.railway.app
python api/explorer_service.py
```

---

## 🔍 Verify Deployment

### Check Logs

```bash
# View live logs
railway logs

# You should see:
# [BLOCKCHAIN] RPC server started on http://0.0.0.0:XXXX
# [BLOCKCHAIN] Current height: 0
# Ready to accept RPC requests from mining pools!
```

### Test RPC Endpoint

```bash
# Get your Railway URL
RAILWAY_URL=$(railway domain)

# Test getblocktemplate
curl -X POST $RAILWAY_URL \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": "test",
    "method": "getblocktemplate",
    "params": []
  }'

# Should return block template
```

### Test from Python

```python
import requests

RAILWAY_URL = "https://gxc-blockchain-node-production.up.railway.app"

response = requests.post(RAILWAY_URL, json={
    "jsonrpc": "2.0",
    "id": "test",
    "method": "getblocktemplate",
    "params": []
})

print(response.json())
```

---

## 💾 Data Persistence

### Problem: Railway Restarts Lose Data

Railway can restart your service, which would lose blockchain data.

### Solution: Use Railway Volumes

Create `railway.toml`:

```toml
[build]
builder = "NIXPACKS"

[deploy]
startCommand = "python testnet_blockchain_node.py"
restartPolicyType = "ON_FAILURE"
restartPolicyMaxRetries = 10

[[volumes]]
name = "blockchain_data"
mountPath = "/app/gxc_testnet_data"
```

Then:
```bash
railway volume create blockchain_data
```

---

## 💰 Railway Costs

### Free Tier (Hobby Plan)
```
✅ $5 credit/month
✅ 512MB RAM
✅ Shared CPU
✅ 1GB storage
✅ Good for: Testing
```

### Pro Tier ($5/month)
```
✅ $5/month + usage
✅ 8GB RAM
✅ Dedicated CPU
✅ 100GB storage
✅ Good for: Production testnet
```

### Estimated Costs for GXC Node

**Testnet (Light Usage):**
- Free tier: $0-5/month ✅
- Pro tier: $5-10/month

**Mainnet (Heavy Usage):**
- Pro tier: $10-20/month
- Recommended: Dedicated server

---

## 🔧 Troubleshooting

### Issue: Port Binding Error

```
Error: Address already in use
```

**Solution:** Railway automatically sets PORT variable. Code already handles this.

### Issue: Database Locked

```
sqlite3.OperationalError: database is locked
```

**Solution:** Use Railway Volumes for persistent storage.

### Issue: Connection Refused

```
Connection refused when connecting from local miner
```

**Solution:** Make sure you're using the Railway URL, not localhost.

### Issue: Slow Performance

```
Node is slow or timing out
```

**Solution:** Upgrade to Pro tier for dedicated CPU.

---

## 📊 Monitoring

### View Metrics

```bash
# Open Railway dashboard
railway open

# View metrics:
# - CPU usage
# - Memory usage
# - Network traffic
# - Request count
```

### Set Up Alerts

In Railway dashboard:
1. Go to Settings
2. Click Notifications
3. Add webhook or email alerts
4. Set thresholds (CPU > 80%, Memory > 90%)

---

## 🔄 Updates and Redeployment

### Update Code

```bash
# Make changes to code
git add .
git commit -m "Update blockchain node"
git push origin master

# Railway auto-deploys if linked to GitHub
# Or manually deploy:
railway up
```

### Restart Service

```bash
# Restart without redeploying
railway restart

# View status
railway status
```

---

## 🌐 Custom Domain (Optional)

### Add Custom Domain

```bash
# Add your domain
railway domain add blockchain.yourdomain.com

# Railway will provide DNS records to add
```

Then update your DNS:
```
CNAME blockchain.yourdomain.com -> your-railway-app.up.railway.app
```

---

## 📋 Complete Deployment Checklist

### Before Deployment:
- [x] Railway account created
- [x] Railway CLI installed
- [x] Code pushed to GitHub (optional)
- [x] `railway.json` configured
- [x] `Procfile` created
- [x] Node updated to use PORT variable
- [x] Node updated to bind to 0.0.0.0

### During Deployment:
- [ ] Run `railway login`
- [ ] Run `railway init`
- [ ] Run `railway up`
- [ ] Run `railway domain` to get URL
- [ ] Test RPC endpoint
- [ ] Check logs with `railway logs`

### After Deployment:
- [ ] Update miner with Railway URL
- [ ] Update wallet with Railway URL
- [ ] Update explorer with Railway URL
- [ ] Test mining from local machine
- [ ] Set up volume for data persistence
- [ ] Configure monitoring/alerts
- [ ] Document Railway URL for team

---

## 🎯 Example: Complete Deployment

```bash
# 1. Install Railway CLI
npm install -g @railway/cli

# 2. Login
railway login

# 3. Initialize
cd /workspaces/GXC-goldxcoin-blockchain-
railway init
# Choose: gxc-blockchain-node

# 4. Deploy
railway up

# 5. Get URL
railway domain
# Output: https://gxc-blockchain-node-production.up.railway.app

# 6. Test
curl -X POST https://gxc-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"test","method":"getblocktemplate","params":[]}'

# 7. Update miner
# Edit mine_testnet_fixed.py:
# RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"

# 8. Mine!
python mine_testnet_fixed.py
```

---

## 🔗 Connect Everything

Once your node is deployed, update all services:

### 1. Miner Configuration

```python
# mine_testnet_fixed.py
RPC_URL = "https://your-railway-url.up.railway.app"
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
```

### 2. Wallet Configuration

```python
# api/wallet_service.py or environment variable
BLOCKCHAIN_RPC_URL = "https://your-railway-url.up.railway.app"
```

### 3. Explorer Configuration

```python
# api/explorer_service.py
BLOCKCHAIN_NODE_URL = "https://your-railway-url.up.railway.app"
```

### 4. Any Other Service

Just use the Railway URL instead of `http://localhost:18332`!

---

## 📝 Summary

```
╔════════════════════════════════════════════════════════╗
║                                                        ║
║   🚂 RAILWAY DEPLOYMENT                                ║
║                                                        ║
║   What's Deployed:                                     ║
║   ✅ Blockchain Node ONLY                             ║
║                                                        ║
║   What Connects to It:                                 ║
║   ✅ Miner (local) → Railway URL                      ║
║   ✅ Wallet (local) → Railway URL                     ║
║   ✅ Explorer (local) → Railway URL                   ║
║                                                        ║
║   Benefits:                                            ║
║   ✅ Node always online                               ║
║   ✅ Public URL for access                            ║
║   ✅ No local node needed                             ║
║   ✅ Easy to scale                                    ║
║                                                        ║
║   Cost:                                                ║
║   ✅ Free tier: $0-5/month                            ║
║   ✅ Pro tier: $5-10/month                            ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

---

## 🚀 Ready to Deploy?

Run these commands:

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login
railway login

# Deploy
cd /workspaces/GXC-goldxcoin-blockchain-
railway init
railway up
railway domain

# Done! 🎉
```

Your blockchain node will be live at:
```
https://gxc-blockchain-node-production.up.railway.app
```

Then just update your miner, wallet, and explorer to use this URL!

---

## ❓ Questions?

- Railway Docs: https://docs.railway.app
- Railway Discord: https://discord.gg/railway
- GXC Issues: https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues

---

**That's it! Your blockchain node is now deployed and accessible from anywhere!** 🎉
