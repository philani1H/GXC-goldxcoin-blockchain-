# 🚀 Manual Deployment Steps

## Railway requires browser authentication, so follow these steps:

### Step 1: Open Terminal

Open a terminal on your local machine (not in the dev container).

### Step 2: Navigate to Project

```bash
cd /path/to/GXC-goldxcoin-blockchain-
```

### Step 3: Run Deployment Script

```bash
./DEPLOY_NOW.sh
```

Or run commands manually:

---

## Manual Commands (Copy & Paste)

### 1. Login to Railway

```bash
railway login
```

This will open your browser. Click "Authorize" to login.

### 2. Initialize Project

```bash
railway init
```

When prompted:
- Project name: `gxc-cpp-blockchain-node` (or any name you like)
- Start from: `Empty project`

### 3. Create Persistent Volume

```bash
railway volume create gxc_blockchain_data
```

### 4. Set Environment Variables

```bash
railway variables set GXC_NETWORK=testnet
railway variables set GXC_LOG_LEVEL=info
railway variables set GXC_DATA_DIR=/app/gxc_data
```

### 5. Deploy

```bash
railway up
```

Wait for:
```
✓ Build successful
✓ Deployment successful
```

### 6. Get Your URL

```bash
railway domain
```

Copy the URL (something like):
```
https://gxc-cpp-blockchain-node-production.up.railway.app
```

### 7. Test Your Node

```bash
# Replace with your actual URL
python test_railway_node.py https://gxc-cpp-blockchain-node-production.up.railway.app
```

### 8. Check Logs

```bash
railway logs
```

Should show:
```
==========================================
  GXC Blockchain Node - Railway Startup
==========================================

RPC Port: XXXX
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

[INFO] Starting RPC server on 0.0.0.0:XXXX
[INFO] Node ready to accept connections
```

---

## ✅ Deployment Complete!

Your C++ blockchain node is now live on Railway!

### Next Steps:

1. **Update your miner** with the Railway URL
2. **Start mining** to test the node
3. **Monitor logs** with `railway logs --follow`
4. **Open dashboard** with `railway open`

---

## 🔧 Useful Commands

```bash
# View logs
railway logs

# Follow logs in real-time
railway logs --follow

# Restart service
railway restart

# Check status
railway status

# Open dashboard
railway open

# Get URL again
railway domain

# List environment variables
railway variables

# Update environment variable
railway variables set KEY=VALUE
```

---

## 🐛 Troubleshooting

### If deployment fails:

```bash
# Check logs
railway logs

# Restart
railway restart

# Redeploy
railway up
```

### If node won't start:

```bash
# Check environment variables
railway variables

# Verify volume is attached
railway volume list

# Check startup script
cat start_gxc_node_railway.sh
```

### If can't connect:

```bash
# Verify URL
railway domain

# Test connection
curl https://your-railway-url.up.railway.app

# Check if node is running
railway logs | grep "Node ready"
```

---

## 📞 Need Help?

- Railway Docs: https://docs.railway.app
- Railway Discord: https://discord.gg/railway
- GXC GitHub: https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues

---

## 🎉 That's It!

Your production C++ blockchain node is now running on Railway!

**Start mining and building!** 🚀
