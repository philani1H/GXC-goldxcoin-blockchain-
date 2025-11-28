# 🚀 START HERE - Deploy Your GXC Node to Railway

## ⚡ Quick Deploy (3 Options)

### Option 1: Automated Script (Easiest) ✅

```bash
./DEPLOY_NOW.sh
```

This script will:
- ✅ Check Railway CLI
- ✅ Login to Railway
- ✅ Initialize project
- ✅ Create volume
- ✅ Set environment variables
- ✅ Deploy node
- ✅ Get your URL
- ✅ Show logs

**Just run it and follow the prompts!**

---

### Option 2: Manual Commands (Step by Step)

See: `DEPLOY_MANUAL_STEPS.md`

Copy and paste these commands:

```bash
# 1. Login
railway login

# 2. Initialize
railway init

# 3. Create volume
railway volume create gxc_blockchain_data

# 4. Set variables
railway variables set GXC_NETWORK=testnet
railway variables set GXC_LOG_LEVEL=info

# 5. Deploy
railway up

# 6. Get URL
railway domain
```

---

### Option 3: Railway Dashboard (Web UI)

1. Go to https://railway.app
2. Click "New Project"
3. Choose "Deploy from GitHub"
4. Select your repository
5. Configure:
   - Add volume: `gxc_blockchain_data` → `/app/gxc_data`
   - Add variables: `GXC_NETWORK=testnet`, `GXC_LOG_LEVEL=info`
6. Click "Deploy"

---

## 📋 What You Need

### Before Starting:

- [ ] Railway account (free at https://railway.app)
- [ ] Railway CLI installed (`npm install -g @railway/cli`)
- [ ] This repository cloned locally
- [ ] Terminal access

### What Gets Deployed:

```
┌─────────────────────────────────────────────┐
│         Railway Cloud                       │
│                                             │
│   ┌─────────────────────────────────┐      │
│   │  GXC C++ Blockchain Node        │      │
│   │  - Production-ready             │      │
│   │  - All mining algorithms        │      │
│   │  - RPC + REST + WebSocket       │      │
│   │  - Persistent storage           │      │
│   │  - Public URL                   │      │
│   └─────────────────────────────────┘      │
│                                             │
└─────────────────────────────────────────────┘
```

---

## 🎯 After Deployment

### 1. Get Your URL

```bash
railway domain
```

Example output:
```
https://gxc-cpp-blockchain-node-production.up.railway.app
```

### 2. Test Your Node

```bash
python test_railway_node.py https://your-railway-url.up.railway.app
```

Expected output:
```
Testing node at: https://your-railway-url.up.railway.app
============================================================

1. Testing getblocktemplate...
   ✅ getblocktemplate works!
   Block height: 0
   Difficulty: 0.1

2. Testing getblockcount...
   ✅ getblockcount works!
   Current height: 0

3. Testing getdifficulty...
   ✅ getdifficulty works!
   Difficulty: 0.1

============================================================
✅ Node is accessible and responding!
```

### 3. Update Your Miner

Edit `mine_testnet_fixed.py`:

```python
# Change this line:
RPC_URL = "http://localhost:18332"

# To your Railway URL:
RPC_URL = "https://gxc-cpp-blockchain-node-production.up.railway.app"
```

### 4. Start Mining!

```bash
python mine_testnet_fixed.py
```

Expected output:
```
Mining to address: tGXC9fab7317231b966af85ac453e168c0932
Connected to node: https://gxc-cpp-blockchain-node-production.up.railway.app

Mining block 1...
✅ Block 1 mined successfully!
Block hash: 00000abc...
Reward: 12.5 GXC

Mining block 2...
✅ Block 2 mined successfully!
Block hash: 00000def...
Reward: 12.5 GXC
```

---

## 📊 Monitor Your Node

### View Logs

```bash
# Recent logs
railway logs

# Follow logs in real-time
railway logs --follow
```

### Open Dashboard

```bash
railway open
```

This opens the Railway dashboard where you can see:
- CPU usage
- Memory usage
- Network traffic
- Deployment status
- Logs
- Metrics

---

## 🔧 Common Commands

```bash
# Check status
railway status

# Restart node
railway restart

# Redeploy
railway up

# Get URL
railway domain

# View variables
railway variables

# Update variable
railway variables set KEY=VALUE

# View volumes
railway volume list

# SSH into container
railway shell
```

---

## 📚 Documentation

### Quick Guides:
- **START_HERE.md** ← You are here
- **DEPLOY_MANUAL_STEPS.md** - Step-by-step manual deployment
- **RAILWAY_QUICK_START.md** - 5-minute quick start

### Complete Guides:
- **RAILWAY_CPP_NODE_DEPLOYMENT.md** - Complete C++ node deployment
- **DEPLOYMENT_READY_CPP.md** - Deployment summary
- **PYTHON_VS_CPP_NODE.md** - Node comparison

### Connection Guides:
- **CONNECT_TO_RAILWAY.md** - How to connect services

### Configuration:
- **config/gxc-railway.conf** - Node configuration
- **start_gxc_node_railway.sh** - Startup script
- **railway.json** - Railway config
- **railway.toml** - Volume config

---

## 💰 Cost

### Free Tier (Hobby):
```
$5 credit/month
512MB RAM
Shared CPU

Good for: Initial testing
```

### Pro Tier ($5/month):
```
$5/month + usage
8GB RAM
Dedicated CPU

Good for: Production testnet
Estimated: $10-15/month
```

### Optimal Setup:
```
$5/month + usage
16GB RAM
4 vCPUs

Good for: Heavy production
Estimated: $20-30/month
```

---

## ⚠️ Important Notes

### 1. Testnet vs Mainnet

Currently configured for **testnet**:
- Block time: 60 seconds
- Difficulty: 0.1 (easy)
- Address prefix: tGXC
- Good for testing

To switch to **mainnet**:
```bash
railway variables set GXC_NETWORK=mainnet
railway restart
```

### 2. Data Persistence

Your blockchain data is stored in a persistent volume:
```
/app/gxc_data
```

This survives restarts and redeployments.

### 3. Security

The node is public. For production:
- Change RPC credentials in `config/gxc-railway.conf`
- Set up rate limiting
- Enable monitoring
- Configure alerts

---

## 🐛 Troubleshooting

### Issue: Railway CLI not found

```bash
npm install -g @railway/cli
```

### Issue: Login fails

```bash
# Make sure you're in a terminal with browser access
railway login

# If still fails, use Railway dashboard (Option 3)
```

### Issue: Deployment fails

```bash
# Check logs
railway logs

# Verify configuration
cat railway.json
cat start_gxc_node_railway.sh

# Restart
railway restart
```

### Issue: Can't connect to node

```bash
# Verify URL
railway domain

# Test connection
curl https://your-railway-url.up.railway.app

# Check if node is running
railway logs | grep "Node ready"
```

### Issue: Node crashes

```bash
# Check logs for errors
railway logs

# Common causes:
# - Out of memory (upgrade plan)
# - Configuration error (check config)
# - Volume not attached (check volumes)

# Fix and restart
railway restart
```

---

## 📞 Get Help

### Railway Support:
- Docs: https://docs.railway.app
- Discord: https://discord.gg/railway
- Status: https://status.railway.app

### GXC Support:
- GitHub: https://github.com/philani1H/GXC-goldxcoin-blockchain-
- Issues: https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues

---

## ✅ Deployment Checklist

### Before Deployment:
- [ ] Railway account created
- [ ] Railway CLI installed
- [ ] Repository cloned locally
- [ ] Terminal ready

### During Deployment:
- [ ] Run `./DEPLOY_NOW.sh` or manual commands
- [ ] Login to Railway
- [ ] Initialize project
- [ ] Create volume
- [ ] Set environment variables
- [ ] Deploy
- [ ] Get URL

### After Deployment:
- [ ] Test node with `test_railway_node.py`
- [ ] Update miner with Railway URL
- [ ] Start mining
- [ ] Check logs
- [ ] Monitor dashboard
- [ ] Set up alerts

---

## 🎉 Ready to Deploy?

### Choose your method:

**Easiest (Automated):**
```bash
./DEPLOY_NOW.sh
```

**Manual (Step by Step):**
```bash
railway login
railway init
railway volume create gxc_blockchain_data
railway variables set GXC_NETWORK=testnet
railway variables set GXC_LOG_LEVEL=info
railway up
railway domain
```

**Web UI:**
Go to https://railway.app and deploy from GitHub

---

## 🚀 Let's Go!

Your C++ blockchain node is ready to deploy. Choose a method above and let's get it live!

**Time to deploy: ~5 minutes**

**Cost: $10-15/month**

**Result: Production blockchain node accessible from anywhere!**

---

## 📖 Next Steps After Deployment

1. ✅ Deploy node (you're doing this now!)
2. Test node connection
3. Update miner with Railway URL
4. Start mining
5. Connect wallet
6. Connect explorer
7. Build your dApp!

**Let's deploy!** 🚀
