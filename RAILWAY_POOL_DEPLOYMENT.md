# 🚂 Deploy Pool Server (Stratum Server) to Railway

## ✅ Files Ready for Deployment

All pool server files are configured and ready:
- ✅ `mining_pool/general_pool.py` - Main pool server
- ✅ `mining_pool/railway.toml` - Railway configuration
- ✅ `mining_pool/Procfile` - Process file
- ✅ `mining_pool/requirements.txt` - Dependencies
- ✅ `mining_pool/deploy-railway.ps1` - Deployment script

## 🚀 Quick Deployment Steps

### Step 1: Login to Railway

```powershell
cd mining_pool
railway login
```

This will open your browser to authenticate. Click "Yes" when prompted.

### Step 2: Initialize Railway Project

```powershell
railway init
```

Choose:
- Project name: `gxc-pool-server` (or your preferred name)
- Start from: `Empty project`

### Step 3: Link to Project

```powershell
railway link
```

Select your project if prompted.

### Step 4: Set Environment Variables

```powershell
railway variables set RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set GXC_NETWORK=testnet
railway variables set POOL_TYPE=general
railway variables set DATABASE_PATH=/tmp/pool.db
railway variables set GENERAL_POOL_STRATUM_PORT=3333
railway variables set GENERAL_POOL_DASHBOARD_PORT=6000
railway variables set GENERAL_POOL_HOST=0.0.0.0
```

### Step 5: Deploy

```powershell
railway up
```

This will:
1. Upload your code
2. Install dependencies from `requirements.txt`
3. Start the pool server using `general_pool.py`
4. Give you a public URL

### Step 6: Get Your Public Domain

```powershell
railway domain
```

This will generate a public domain like:
```
https://gxc-pool-server-production.up.railway.app
```

### Step 7: Update Stratum URL

After getting your domain, update the environment variables:

```powershell
railway variables set GENERAL_POOL_STRATUM_URL=stratum+tcp://your-domain.up.railway.app:3333
railway variables set GENERAL_POOL_DASHBOARD_URL=https://your-domain.up.railway.app
```

## 📋 Complete Deployment Command Sequence

```powershell
# Navigate to pool directory
cd mining_pool

# Login (will open browser)
railway login

# Initialize project
railway init
# Choose: gxc-pool-server, Empty project

# Link to project
railway link

# Set environment variables
railway variables set RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set GXC_NETWORK=testnet
railway variables set POOL_TYPE=general
railway variables set DATABASE_PATH=/tmp/pool.db
railway variables set GENERAL_POOL_STRATUM_PORT=3333
railway variables set GENERAL_POOL_DASHBOARD_PORT=6000
railway variables set GENERAL_POOL_HOST=0.0.0.0

# Deploy
railway up

# Get domain
railway domain

# Update URLs (replace YOUR-DOMAIN with actual domain)
railway variables set GENERAL_POOL_STRATUM_URL=stratum+tcp://YOUR-DOMAIN.up.railway.app:3333
railway variables set GENERAL_POOL_DASHBOARD_URL=https://YOUR-DOMAIN.up.railway.app
```

## 🔍 Verify Deployment

### Check Logs

```powershell
railway logs
```

You should see:
```
GXC General Mining Pool (All Algorithms)
============================================================
Public Stratum Endpoint: stratum+tcp://...
Public Dashboard: https://...
Stratum Port: 3333
Dashboard Port: 6000
Algorithms: SHA-256, Ethash, GXHash
Blockchain Node: https://gxc-chain112-blockchain-node-production.up.railway.app
✅ Pool is PUBLIC - Third-party miners can connect!
```

### Check Status

```powershell
railway status
```

### Test Dashboard

Open your dashboard URL in browser:
```
https://your-domain.up.railway.app
```

You should see the pool dashboard.

## 🔗 Connect Miners

Once deployed, miners can connect using:

```
stratum+tcp://your-domain.up.railway.app:3333
```

### Example Miner Connection

```python
# Python miner
python mining/gxhash_miner.py \
  --pool stratum+tcp://your-domain.up.railway.app:3333 \
  --user YOUR_GXC_ADDRESS
```

## 📊 Configuration Files

### railway.toml (Already Configured)

```toml
[build]
builder = "NIXPACKS"
buildCommand = "pip install -r requirements.txt"

[deploy]
startCommand = "python3 general_pool.py"
restartPolicyType = "ON_FAILURE"
restartPolicyMaxRetries = 10

[deploy.env]
RAILWAY_ENVIRONMENT = "1"
GXC_NETWORK = "testnet"
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
BLOCKCHAIN_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
POOL_TYPE = "general"
DATABASE_PATH = "/tmp/pool.db"
GENERAL_POOL_STRATUM_PORT = "3333"
GENERAL_POOL_DASHBOARD_PORT = "6000"
GENERAL_POOL_HOST = "0.0.0.0"
```

### Procfile (Already Configured)

```
web: python3 general_pool.py
```

## ⚠️ Important Notes

1. **Stratum Port**: Railway may need special configuration for TCP port 3333. The dashboard will work on the HTTP port automatically assigned by Railway.

2. **Database**: Uses `/tmp/pool.db` which is temporary. For production, consider using Railway Postgres or external database.

3. **Blockchain Node**: Make sure your blockchain node is running at the configured URL.

4. **Public Domain**: Railway will assign a domain automatically. You can also add a custom domain in Railway dashboard.

## 🎯 What Gets Deployed

- ✅ Stratum Server (TCP port 3333) - For miners to connect
- ✅ Web Dashboard (HTTP port) - For users to view pool stats
- ✅ Pool Management - Miner registration, share tracking, block submission
- ✅ Real-time Statistics - Live pool stats and miner information

## 📝 Next Steps After Deployment

1. ✅ Get your public domain
2. ✅ Update environment variables with domain
3. ✅ Test dashboard in browser
4. ✅ Connect a test miner
5. ✅ Monitor logs for activity
6. ✅ Share Stratum URL with miners

## 🔧 Troubleshooting

### Issue: Port 3333 Not Accessible

Railway may need TCP proxy configuration. Check Railway dashboard → Networking → TCP Proxy.

### Issue: Pool Not Starting

Check logs:
```powershell
railway logs
```

Common issues:
- Missing dependencies (check `requirements.txt`)
- Database path issues (ensure `/tmp` is writable)
- Blockchain node connection (verify URL is correct)

### Issue: Miners Can't Connect

1. Verify Stratum URL is correct
2. Check Railway logs for connection attempts
3. Ensure port 3333 is exposed
4. Test with a simple Python connection script

## ✅ Deployment Checklist

- [ ] Railway CLI installed
- [ ] Logged in to Railway
- [ ] Project initialized
- [ ] Environment variables set
- [ ] Code deployed (`railway up`)
- [ ] Public domain generated
- [ ] Stratum URL updated
- [ ] Dashboard URL updated
- [ ] Logs checked
- [ ] Dashboard tested in browser
- [ ] Test miner connected

---

**Ready to deploy!** Run the commands above to deploy your pool server to Railway. 🚀

