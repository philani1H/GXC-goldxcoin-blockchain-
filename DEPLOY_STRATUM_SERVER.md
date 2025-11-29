# 🚀 Deploy Stratum Server to Railway

## ✅ Quick Deployment Guide

The Stratum server needs to run on a platform that supports persistent TCP connections (not Vercel). Railway is perfect for this!

## 📋 Step-by-Step Deployment

### Option 1: Deploy to Railway (Recommended)

#### Step 1: Prepare Files

The `mining_pool/railway.toml` file is already configured with:
- ✅ Railway Node URL
- ✅ Environment variables
- ✅ Port configuration

#### Step 2: Deploy via Railway CLI

```bash
# Install Railway CLI (if not installed)
npm i -g @railway/cli

# Login to Railway
railway login

# Navigate to mining pool directory
cd mining_pool

# Initialize Railway project
railway init

# Link to existing project (or create new)
railway link

# Deploy
railway up
```

#### Step 3: Configure Environment Variables

In Railway Dashboard:
1. Go to your project
2. Click on the service
3. Go to "Variables" tab
4. Add/verify these variables:

```
RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
GXC_NETWORK=testnet
POOL_TYPE=general
DATABASE_PATH=/tmp/pool.db
GENERAL_POOL_STRATUM_PORT=3333
GENERAL_POOL_DASHBOARD_PORT=6000
GENERAL_POOL_HOST=0.0.0.0
```

#### Step 4: Configure Public Domain

1. In Railway Dashboard → Your Service → Settings
2. Generate a public domain (e.g., `gxc-pool-production.up.railway.app`)
3. Note the domain - you'll use it for Stratum URL

#### Step 5: Configure Ports

**Important:** Railway provides one `PORT` environment variable. For Stratum, you need:

1. **Stratum Port (3333)** - For miners to connect
2. **Dashboard Port (6000)** - For web interface

**Solution:** Use Railway's port forwarding or expose both ports:

```bash
# In Railway, you can:
# 1. Use PORT for dashboard (default)
# 2. Expose Stratum port separately via Railway's networking
```

Or use Railway's TCP proxy feature to expose port 3333.

#### Step 6: Get Your Public URLs

After deployment, your URLs will be:

```
Stratum URL: stratum+tcp://gxc-pool-production.up.railway.app:3333
Dashboard URL: https://gxc-pool-production.up.railway.app
```

### Option 2: Deploy via Railway Dashboard

1. **Create New Project**
   - Go to [Railway Dashboard](https://railway.app)
   - Click "New Project"
   - Select "Deploy from GitHub repo"

2. **Configure Service**
   - Select your repository
   - Set root directory to `mining_pool`
   - Railway will auto-detect `railway.toml`

3. **Set Environment Variables**
   - Go to Variables tab
   - Add all variables from `railway.toml`

4. **Generate Public Domain**
   - Settings → Generate Domain
   - Copy the domain

5. **Configure Ports**
   - Railway exposes one port by default
   - For Stratum, you may need to:
     - Use Railway's TCP proxy
     - Or run Stratum on a separate service

### Option 3: Deploy Stratum and Dashboard Separately

Since Railway provides one PORT, you can:

1. **Deploy Stratum Server** (separate service)
   ```bash
   # Create railway.toml for Stratum only
   startCommand = "python3 -c 'from pool_base import MiningPool; pool = MiningPool(\"pool\", \"all\", 3333); pool.start_stratum_server(); import time; time.sleep(999999)'"
   ```

2. **Deploy Dashboard** (separate service)
   ```bash
   # Use existing general_pool.py
   # Dashboard runs on Railway's PORT
   ```

## 🔧 Configuration

### Environment Variables

All these are set in `railway.toml`:

```toml
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
BLOCKCHAIN_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
GXC_NETWORK = "testnet"
POOL_TYPE = "general"
DATABASE_PATH = "/tmp/pool.db"
GENERAL_POOL_STRATUM_PORT = "3333"
GENERAL_POOL_DASHBOARD_PORT = "6000"
```

### Public URLs

After deployment, update these in Railway:

```
GENERAL_POOL_STRATUM_URL = "stratum+tcp://your-pool.up.railway.app:3333"
GENERAL_POOL_DASHBOARD_URL = "https://your-pool-dashboard.up.railway.app"
```

## 🧪 Test Deployment

### 1. Check Dashboard

Visit your dashboard URL:
```
https://your-pool-dashboard.up.railway.app
```

You should see:
- ✅ Pool statistics
- ✅ Network status (connected to Railway node)
- ✅ Miner registration

### 2. Test Stratum Connection

Use a miner to connect:
```bash
# Example with gxhash miner
python3 gxhash_miner.py \
  --pool stratum+tcp://your-pool.up.railway.app:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932
```

### 3. Verify Connection

Check Railway logs:
```bash
railway logs
```

You should see:
```
[pool] Stratum server listening on port 3333
[pool] Miner connected: 192.168.1.1:12345
[pool] Share accepted from miner_id
```

## 📝 Important Notes

### Port Configuration

Railway provides one `PORT` environment variable. For Stratum:

- **Option A:** Use Railway's TCP proxy to expose port 3333
- **Option B:** Deploy Stratum and Dashboard as separate services
- **Option C:** Use Railway's networking features to expose both ports

### Database Persistence

Railway's `/tmp` directory is ephemeral. For production:

1. **Use Railway Postgres** (recommended)
   ```bash
   railway add postgres
   # Update DATABASE_PATH to use Postgres connection string
   ```

2. **Or use Railway Volume**
   ```bash
   railway volume create pool-data
   # Mount to /data
   ```

### Public Domain

After generating public domain in Railway:
- Update `GENERAL_POOL_STRATUM_URL` environment variable
- Update `GENERAL_POOL_DASHBOARD_URL` environment variable
- Miners will use the Stratum URL to connect

## 🚀 Quick Start Commands

```bash
# Deploy to Railway
cd mining_pool
railway up

# View logs
railway logs

# Check status
railway status

# Update environment variables
railway variables set RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
```

## ✅ Verification Checklist

- [ ] Stratum server deployed to Railway
- [ ] Environment variables set (RAILWAY_NODE_URL, etc.)
- [ ] Public domain generated
- [ ] Stratum port (3333) accessible
- [ ] Dashboard accessible
- [ ] Connected to Railway blockchain node
- [ ] Miners can connect via Stratum URL
- [ ] Shares are being accepted
- [ ] Rewards are being distributed

## 🎯 Summary

**Stratum Server Deployment:**
- ✅ Deploy to Railway (not Vercel)
- ✅ Uses Railway URL automatically
- ✅ Configure public domain
- ✅ Expose Stratum port (3333)
- ✅ Dashboard on Railway PORT

**Your Stratum URL will be:**
```
stratum+tcp://your-pool.up.railway.app:3333
```

**Everything connects to Railway testnet automatically!** 🎉
