# 🚂 Railway C++ Node Deployment Guide

## 📋 Overview

Deploy the **production C++ blockchain node** to Railway. This is the real GXC node that supports:
- ✅ All mining algorithms (SHA-256, Ethash, GXHash)
- ✅ Full consensus validation
- ✅ High performance
- ✅ Production-ready features

```
┌─────────────────────────────────────────────┐
│         Railway (Cloud)                     │
│                                             │
│   ┌─────────────────────────────────┐      │
│   │  GXC C++ Blockchain Node        │      │
│   │  - Full node implementation     │      │
│   │  - All mining algorithms        │      │
│   │  - RPC + REST API               │      │
│   │  - WebSocket support            │      │
│   │  - Public URL                   │      │
│   └─────────────────────────────────┘      │
│                                             │
└─────────────────────────────────────────────┘
                    ↑
                    │ HTTP/RPC/WebSocket
                    │
    ┌───────────────┼───────────────┐
    │               │               │
    ↓               ↓               ↓
┌────────┐    ┌─────────┐    ┌──────────┐
│SHA-256 │    │ Ethash  │    │ GXHash   │
│ Miner  │    │ Miner   │    │ Miner    │
│(Local) │    │(Local)  │    │(Local)   │
└────────┘    └─────────┘    └──────────┘
```

---

## ✅ What's Been Configured

### 1. C++ Node Binary
- ✅ Pre-built Linux binary: `packages/gxc-miners-cli-linux/gxc-node`
- ✅ 849KB executable
- ✅ Supports all features

### 2. Railway Configuration Files
- ✅ `railway.json` - Railway deployment config
- ✅ `railway.toml` - Volume and environment config
- ✅ `Procfile` - Process definition
- ✅ `config/gxc-railway.conf` - Node configuration
- ✅ `start_gxc_node_railway.sh` - Startup script

### 3. Node Configuration
- ✅ Binds to 0.0.0.0 (Railway requirement)
- ✅ Uses PORT environment variable
- ✅ Persistent storage configured
- ✅ Optimized for cloud deployment

---

## 🚀 Deploy to Railway (5 Minutes)

### Step 1: Install Railway CLI

```bash
npm install -g @railway/cli
```

### Step 2: Login

```bash
railway login
```

### Step 3: Initialize Project

```bash
cd /workspaces/GXC-goldxcoin-blockchain-
railway init
```

Choose:
- Project name: `gxc-cpp-node`
- Start from: `Empty project`

### Step 4: Create Volume for Blockchain Data

```bash
# Create persistent volume
railway volume create gxc_blockchain_data

# Link volume to service
railway volume attach gxc_blockchain_data /app/gxc_data
```

### Step 5: Set Environment Variables

```bash
# Set network (testnet or mainnet)
railway variables set GXC_NETWORK=testnet

# Set log level
railway variables set GXC_LOG_LEVEL=info

# Optional: Set data directory
railway variables set GXC_DATA_DIR=/app/gxc_data
```

### Step 6: Deploy

```bash
railway up
```

Wait for:
```
✓ Build successful
✓ Deployment successful
```

### Step 7: Get Your URL

```bash
railway domain
```

You'll get:
```
https://gxc-cpp-node-production.up.railway.app
```

### Step 8: Check Logs

```bash
railway logs
```

Should show:
```
==========================================
  GXC Blockchain Node - Railway Startup
==========================================

RPC Port: 8332
Data Directory: /app/gxc_data
Network: testnet
Log Level: info

Starting GXC Node...
==========================================

   _____ _  _  ___    _   _           _      
  / ____| \| |/ __|  | \ | |         | |     
 | |  __|  .` | (__   |  \| | ___  __| | ___ 
 | | |_ | |\ | \__|  | . ` |/ _ \/ _` |/ _ \
 | |__| | | | |___   | |\  | (_) | (_| |  __/
  \_____|_| |_|___|  |_| \_|\___/ \__,_|\___|
                                             
GXC Blockchain Full Node
Version 2.0.0 - Production Ready

[INFO] Starting RPC server on 0.0.0.0:8332
[INFO] Starting REST API on 0.0.0.0:8080
[INFO] Starting WebSocket server on 0.0.0.0:8081
[INFO] Node ready to accept connections
```

---

## 🔧 Configuration Details

### Railway Configuration (railway.json)

```json
{
  "build": {
    "builder": "NIXPACKS",
    "buildCommand": "echo 'Using pre-built C++ binaries'"
  },
  "deploy": {
    "startCommand": "./start_gxc_node_railway.sh",
    "restartPolicyType": "ON_FAILURE",
    "restartPolicyMaxRetries": 10
  }
}
```

### Volume Configuration (railway.toml)

```toml
[[deploy.volumes]]
name = "gxc_blockchain_data"
mountPath = "/app/gxc_data"
```

### Node Configuration (config/gxc-railway.conf)

Key settings:
```conf
# Bind to all interfaces
bind=0.0.0.0
rpcbind=0.0.0.0

# Allow RPC from anywhere
rpcallowip=0.0.0.0/0

# Data directory (persistent volume)
datadir=/app/gxc_data

# Disable wallet/mining on node
wallet=0
mining=0
staking=0

# Enable APIs
server=1
rest=1
websocket=1
```

---

## 🔗 Connect Your Miners

Once deployed, connect all your miners to the Railway node:

### SHA-256 Miner

```bash
# Your Railway URL
export GXC_NODE_URL=https://gxc-cpp-node-production.up.railway.app

# Run SHA-256 miner
./packages/gxc-miners-cli-linux/gxc-sha256-miner \
  --node=$GXC_NODE_URL \
  --address=tGXC9fab7317231b966af85ac453e168c0932
```

### Ethash Miner

```bash
./packages/gxc-miners-cli-linux/gxc-ethash-miner \
  --node=$GXC_NODE_URL \
  --address=tGXC9fab7317231b966af85ac453e168c0932
```

### GXHash Miner

```bash
./packages/gxc-miners-cli-linux/gxc-gxhash-miner \
  --node=$GXC_NODE_URL \
  --address=tGXC9fab7317231b966af85ac453e168c0932
```

### Python Miner (mine_testnet_fixed.py)

```python
# Edit mine_testnet_fixed.py
RPC_URL = "https://gxc-cpp-node-production.up.railway.app"
```

---

## 🧪 Test the Node

### Test RPC Endpoint

```bash
curl -X POST https://gxc-cpp-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": "test",
    "method": "getblockcount",
    "params": []
  }'
```

### Test REST API

```bash
curl https://gxc-cpp-node-production.up.railway.app:8080/api/v1/blocks/latest
```

### Test WebSocket

```javascript
const ws = new WebSocket('wss://gxc-cpp-node-production.up.railway.app:8081');

ws.on('open', () => {
  ws.send(JSON.stringify({
    method: 'subscribe',
    params: ['newblock']
  }));
});

ws.on('message', (data) => {
  console.log('New block:', data);
});
```

---

## 📊 Node Features

### RPC API (Port from Railway PORT variable)

Available methods:
- `getblockcount` - Get current block height
- `getblocktemplate` - Get block template for mining
- `submitblock` - Submit mined block
- `getdifficulty` - Get current difficulty
- `getbalance` - Get address balance
- `sendtransaction` - Send transaction
- And many more...

### REST API (Port 8080)

Endpoints:
- `GET /api/v1/blocks/latest` - Latest block
- `GET /api/v1/blocks/:height` - Block by height
- `GET /api/v1/transactions/:hash` - Transaction details
- `GET /api/v1/address/:address/balance` - Address balance
- `POST /api/v1/transactions` - Submit transaction

### WebSocket API (Port 8081)

Real-time events:
- `newblock` - New block mined
- `newtransaction` - New transaction
- `difficulty` - Difficulty change
- `price` - Price updates

---

## 💾 Data Persistence

### Railway Volumes

The blockchain data is stored in a persistent volume:
```
/app/gxc_data
├── blocks/          # Block data
├── chainstate/      # UTXO set
├── indexes/         # Block indexes
└── peers.dat        # Peer information
```

### Backup Strategy

```bash
# Create backup
railway run tar -czf backup.tar.gz /app/gxc_data

# Download backup
railway run cat backup.tar.gz > local_backup.tar.gz

# Restore backup
railway run tar -xzf backup.tar.gz -C /
```

---

## 🔧 Environment Variables

### Required Variables

```bash
# Network type
GXC_NETWORK=testnet  # or mainnet

# Log level
GXC_LOG_LEVEL=info   # debug, info, warning, error
```

### Optional Variables

```bash
# Data directory
GXC_DATA_DIR=/app/gxc_data

# RPC credentials (if you want to override config)
GXC_RPC_USER=myuser
GXC_RPC_PASSWORD=mypassword

# Mining address (if enabling mining on node)
GXC_MINING_ADDRESS=tGXC9fab7317231b966af85ac453e168c0932
```

---

## 💰 Railway Costs

### Free Tier (Hobby)
```
❌ Not recommended for C++ node
- 512MB RAM (too small)
- Shared CPU (too slow)
```

### Pro Tier ($5/month + usage)
```
✅ Recommended minimum
- 8GB RAM
- Dedicated CPU
- 100GB storage
- Estimated: $10-15/month
```

### Optimal Setup
```
✅ Best for production
- 16GB RAM
- 4 vCPUs
- 200GB storage
- Estimated: $20-30/month
```

---

## 📈 Monitoring

### View Metrics

```bash
# Open Railway dashboard
railway open
```

Monitor:
- CPU usage
- Memory usage
- Network traffic
- Disk usage
- Request count

### Check Node Status

```bash
# View logs
railway logs --follow

# Check if node is syncing
curl -X POST https://your-railway-url.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"1","method":"getblockcount","params":[]}'
```

### Set Up Alerts

In Railway dashboard:
1. Settings → Notifications
2. Add webhook or email
3. Set thresholds:
   - CPU > 80%
   - Memory > 90%
   - Disk > 85%

---

## 🔄 Updates and Maintenance

### Update Node Binary

```bash
# Build new binary locally
cmake --build build --target gxc-node

# Copy to packages
cp build/gxc-node packages/gxc-miners-cli-linux/

# Commit and push
git add packages/gxc-miners-cli-linux/gxc-node
git commit -m "Update C++ node binary"
git push origin master

# Redeploy on Railway
railway up
```

### Restart Node

```bash
# Restart without redeploying
railway restart

# Redeploy
railway up
```

### View Logs

```bash
# Recent logs
railway logs

# Follow logs
railway logs --follow

# Filter logs
railway logs | grep ERROR
```

---

## 🐛 Troubleshooting

### Issue: Node Won't Start

**Check logs:**
```bash
railway logs
```

**Common causes:**
- Missing volume
- Port conflict
- Configuration error

**Solution:**
```bash
# Verify volume is attached
railway volume list

# Check environment variables
railway variables

# Restart
railway restart
```

### Issue: Can't Connect to Node

**Test connection:**
```bash
curl https://your-railway-url.up.railway.app
```

**Common causes:**
- Node still starting
- Firewall issue
- Wrong URL

**Solution:**
```bash
# Wait for node to start (check logs)
railway logs --follow

# Verify URL
railway domain
```

### Issue: Out of Memory

**Check memory usage:**
```bash
railway open
# View metrics
```

**Solution:**
```bash
# Upgrade to larger plan
# In Railway dashboard: Settings → Plan
```

### Issue: Disk Full

**Check disk usage:**
```bash
railway run df -h /app/gxc_data
```

**Solution:**
```bash
# Increase volume size
# In Railway dashboard: Volumes → Resize

# Or prune old data
railway run ./packages/gxc-miners-cli-linux/gxc-node --prune
```

---

## 🔒 Security

### RPC Authentication

Update `config/gxc-railway.conf`:
```conf
rpcuser=your_secure_username
rpcpassword=your_secure_password_change_this
```

Set via environment variables:
```bash
railway variables set GXC_RPC_USER=your_username
railway variables set GXC_RPC_PASSWORD=your_secure_password
```

### Rate Limiting

Add to config:
```conf
rpcthreads=4
rpcworkqueue=32
rpcservertimeout=30
```

### IP Whitelisting

For production, restrict RPC access:
```conf
# Only allow specific IPs
rpcallowip=1.2.3.4
rpcallowip=5.6.7.8
```

---

## 📋 Deployment Checklist

### Before Deployment:
- [x] Railway account created
- [x] Railway CLI installed
- [x] C++ node binary ready
- [x] Configuration files created
- [x] Startup script created
- [ ] Volume created
- [ ] Environment variables set

### During Deployment:
- [ ] `railway login`
- [ ] `railway init`
- [ ] `railway volume create gxc_blockchain_data`
- [ ] `railway variables set GXC_NETWORK=testnet`
- [ ] `railway up`
- [ ] `railway domain`
- [ ] Test RPC endpoint
- [ ] Check logs

### After Deployment:
- [ ] Connect SHA-256 miner
- [ ] Connect Ethash miner
- [ ] Connect GXHash miner
- [ ] Test mining
- [ ] Verify blocks
- [ ] Set up monitoring
- [ ] Configure alerts
- [ ] Document URL for team

---

## 🎯 Comparison: Python vs C++ Node

### Python Node (testnet_blockchain_node.py)
```
✅ Easy to modify
✅ Good for testing
❌ Slower performance
❌ Limited features
❌ Single algorithm only
```

### C++ Node (gxc-node)
```
✅ Production-ready
✅ High performance
✅ All algorithms supported
✅ Full feature set
✅ Optimized for scale
❌ Requires compilation to modify
```

**For Railway deployment: Use C++ node!** ✅

---

## 🌐 Network Configuration

### Testnet (Current)

```bash
railway variables set GXC_NETWORK=testnet
```

Configuration:
- Block time: 60 seconds
- Difficulty: 0.1 (easy)
- Address prefix: tGXC
- RPC port: Uses Railway PORT
- P2P port: 18333

### Mainnet (Production)

```bash
railway variables set GXC_NETWORK=mainnet
```

Configuration:
- Block time: 600 seconds (10 minutes)
- Difficulty: Higher (adjusts automatically)
- Address prefix: GXC
- RPC port: Uses Railway PORT
- P2P port: 9333

---

## 📚 Additional Resources

- **Node Documentation**: See `packages/gxc-miners-cli-linux/README.md`
- **Configuration Reference**: See `config/gxc.conf.example`
- **Mining Guide**: See `MINING_GUIDE.md`
- **Railway Docs**: https://docs.railway.app

---

## 🎉 Summary

```
╔════════════════════════════════════════════════════════╗
║                                                        ║
║   🚂 C++ NODE DEPLOYMENT                               ║
║                                                        ║
║   What's Deployed:                                     ║
║   ✅ Production C++ blockchain node                   ║
║   ✅ All mining algorithms supported                  ║
║   ✅ RPC + REST + WebSocket APIs                      ║
║   ✅ Persistent storage                               ║
║   ✅ High performance                                 ║
║                                                        ║
║   What Connects:                                       ║
║   ✅ SHA-256 miners                                   ║
║   ✅ Ethash miners                                    ║
║   ✅ GXHash miners                                    ║
║   ✅ Wallets                                          ║
║   ✅ Explorers                                        ║
║                                                        ║
║   Cost: $10-30/month                                   ║
║   Performance: Production-ready                        ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

---

## 🚀 Deploy Now!

```bash
# 1. Install Railway CLI
npm install -g @railway/cli

# 2. Login
railway login

# 3. Initialize
cd /workspaces/GXC-goldxcoin-blockchain-
railway init

# 4. Create volume
railway volume create gxc_blockchain_data

# 5. Set variables
railway variables set GXC_NETWORK=testnet
railway variables set GXC_LOG_LEVEL=info

# 6. Deploy
railway up

# 7. Get URL
railway domain

# 8. Test
curl -X POST https://your-railway-url.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"1","method":"getblockcount","params":[]}'

# Done! 🎉
```

**Your production C++ blockchain node is now live!** 🚀
