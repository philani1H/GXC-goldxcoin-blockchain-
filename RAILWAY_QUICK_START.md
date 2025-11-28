# 🚀 Railway Quick Start - Deploy in 5 Minutes

## 📋 What You're Deploying

```
ONLY the blockchain node → Railway (cloud)
Everything else → Runs locally, connects via URL
```

---

## ⚡ 5-Minute Deployment

### 1️⃣ Install Railway CLI (1 min)

```bash
npm install -g @railway/cli
```

### 2️⃣ Login (30 sec)

```bash
railway login
```

Browser will open → Click "Authorize"

### 3️⃣ Initialize (30 sec)

```bash
cd /workspaces/GXC-goldxcoin-blockchain-
railway init
```

When prompted:
- Project name: `gxc-blockchain-node` (or any name)
- Start from: `Empty project`

### 4️⃣ Deploy (2 min)

```bash
railway up
```

Wait for:
```
✓ Build successful
✓ Deployment successful
```

### 5️⃣ Get Your URL (30 sec)

```bash
railway domain
```

Copy the URL, something like:
```
https://gxc-blockchain-node-production.up.railway.app
```

### 6️⃣ Test It (30 sec)

```bash
python test_railway_node.py https://gxc-blockchain-node-production.up.railway.app
```

Should show:
```
✅ getblocktemplate works!
✅ getblockcount works!
✅ getdifficulty works!
✅ Node is accessible and responding!
```

---

## 🎯 Done! Now Connect Your Services

### Update Miner

Edit `mine_testnet_fixed.py` line 8:

```python
# Change this:
RPC_URL = "http://localhost:18332"

# To this (use YOUR Railway URL):
RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"
```

Save and run:
```bash
python mine_testnet_fixed.py
```

Should mine successfully! 🎉

---

## 📊 What Just Happened?

```
Before:
┌──────────────────────────┐
│   Your Computer          │
│                          │
│  ┌────────────────────┐  │
│  │ Blockchain Node    │  │
│  │ (localhost:18332)  │  │
│  └────────────────────┘  │
│           ↑              │
│           │              │
│  ┌────────┴────────┐     │
│  │ Miner           │     │
│  └─────────────────┘     │
└──────────────────────────┘

After:
┌──────────────────────────┐
│   Railway (Cloud)        │
│                          │
│  ┌────────────────────┐  │
│  │ Blockchain Node    │  │
│  │ (public URL)       │  │
│  └────────────────────┘  │
└──────────────────────────┘
           ↑
           │ Internet
           │
┌──────────┴───────────────┐
│   Your Computer          │
│                          │
│  ┌─────────────────┐     │
│  │ Miner           │     │
│  │ (connects via   │     │
│  │  Railway URL)   │     │
│  └─────────────────┘     │
└──────────────────────────┘
```

---

## ✅ Benefits

- ✅ Node always online (24/7)
- ✅ Accessible from anywhere
- ✅ No need to run node locally
- ✅ Easy to share with others
- ✅ Professional setup
- ✅ Low cost ($0-10/month)

---

## 🔧 Useful Commands

```bash
# View logs
railway logs

# Restart service
railway restart

# Check status
railway status

# Open dashboard
railway open

# Get URL again
railway domain

# Deploy updates
git push origin master
railway up
```

---

## 💡 Tips

### Tip 1: Save Your URL

Create a file `RAILWAY_URL.txt`:
```
https://gxc-blockchain-node-production.up.railway.app
```

### Tip 2: Use Environment Variables

Create `.env`:
```bash
BLOCKCHAIN_RPC_URL=https://gxc-blockchain-node-production.up.railway.app
```

Load it:
```bash
export $(cat .env | xargs)
python mine_testnet_fixed.py
```

### Tip 3: Monitor Your Node

```bash
# Watch logs in real-time
railway logs --follow
```

---

## 🐛 Troubleshooting

### Problem: "railway: command not found"

**Solution:**
```bash
# Install Railway CLI
npm install -g @railway/cli

# Or use curl
curl -fsSL https://railway.app/install.sh | sh
```

### Problem: "Failed to connect"

**Solution:**
```bash
# Check if node is running
railway logs

# Restart if needed
railway restart
```

### Problem: "Connection refused"

**Solution:**
Make sure you're using HTTPS, not HTTP:
```
✅ https://your-app.up.railway.app
❌ http://your-app.up.railway.app
```

---

## 📚 More Info

- **Complete Guide**: See `RAILWAY_NODE_DEPLOYMENT.md`
- **Connection Guide**: See `CONNECT_TO_RAILWAY.md`
- **Detailed Info**: See `RAILWAY_DEPLOYMENT_GUIDE.md`

---

## 🎉 That's It!

You now have:
- ✅ Blockchain node running on Railway
- ✅ Public URL for access
- ✅ 24/7 availability
- ✅ Professional setup

**Start mining and building!** 🚀

---

## 📞 Need Help?

- Railway Docs: https://docs.railway.app
- Railway Discord: https://discord.gg/railway
- GXC GitHub Issues: https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues

---

## 🔄 Next Steps

1. ✅ Deploy node to Railway (DONE!)
2. Update miner with Railway URL
3. Update wallet with Railway URL
4. Update explorer with Railway URL
5. Start mining!
6. Build your dApp!

**Happy mining!** ⛏️💰
