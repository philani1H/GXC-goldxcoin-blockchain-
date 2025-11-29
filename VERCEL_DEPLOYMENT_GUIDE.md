# 🚀 Vercel Deployment Guide - All Services Use Railway URL

## ✅ Yes! Everything Uses Railway URL (Not Localhost)

When you deploy to Vercel, **ALL services will use the Railway testnet URL**, not localhost:

- ✅ **Explorer** → Railway URL
- ✅ **Mining Pools** → Railway URL  
- ✅ **Wallet API** → Railway URL
- ✅ **All RPC calls** → Railway URL

## 📋 Configuration Summary

### 1. **Explorer** (`web/blockchain_explorer.py`)

```python
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
BLOCKCHAIN_NODE_URL = os.environ.get('BLOCKCHAIN_NODE_URL', 
                                     os.environ.get('RAILWAY_NODE_URL', 
                                                   RAILWAY_NODE_URL))
```

**Vercel Config** (`web/vercel.json`):
```json
{
  "env": {
    "RAILWAY_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app",
    "BLOCKCHAIN_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app"
  }
}
```

### 2. **Mining Pools** (`mining_pool/pool_base.py`)

```python
self.rpc_url = rpc_url or os.environ.get(
    'BLOCKCHAIN_NODE_URL', 
    'https://gxc-chain112-blockchain-node-production.up.railway.app'
)
```

**Vercel Config** (`mining_pool/vercel.json`):
```json
{
  "env": {
    "BLOCKCHAIN_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app",
    "RAILWAY_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app"
  }
}
```

### 3. **Wallet API** (`api/wallet_service.py`)

```python
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
NETWORK_INFO = {
    'rpc_url': os.environ.get('BLOCKCHAIN_RPC_URL', 
                              os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL)),
    'rest_url': os.environ.get('BLOCKCHAIN_REST_URL', 
                               os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL)),
}
```

## 🚀 Deploy to Vercel

### Deploy Explorer

```bash
cd web
vercel --prod
```

**Environment Variables** (already in `vercel.json`):
- `RAILWAY_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
- `BLOCKCHAIN_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
- `VERCEL` = `1`

### Deploy Mining Pool

```bash
cd mining_pool
vercel --prod
```

**Environment Variables** (already in `vercel.json`):
- `BLOCKCHAIN_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
- `RAILWAY_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
- `VERCEL` = `1`
- `POOL_TYPE` = `general`
- `DATABASE_PATH` = `/tmp/pool.db`

### Deploy Wallet API

```bash
cd api
vercel --prod
```

**Environment Variables** (set in Vercel dashboard):
- `BLOCKCHAIN_RPC_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
- `RAILWAY_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
- `VERCEL` = `1`

## ⚠️ Important Notes

### Stratum Protocol (Mining Pools)

**Stratum cannot run on Vercel** (serverless doesn't support socket binding).

**Solution:**
- **Dashboard/Web UI** → Deploy to Vercel ✅
- **Stratum Server** → Deploy to Railway or dedicated server (needs persistent socket)

**For Stratum, you need:**
1. Deploy pool dashboard to Vercel (web interface)
2. Deploy Stratum server separately to Railway or a VPS
3. Set `GENERAL_POOL_STRATUM_URL` environment variable to your Stratum server URL

### What Works on Vercel

✅ **Explorer** - Full functionality (HTTP routes)
✅ **Pool Dashboard** - Full functionality (HTTP routes)
✅ **Wallet API** - Full functionality (HTTP routes)
✅ **RPC Calls** - All use Railway URL automatically

❌ **Stratum Server** - Needs persistent socket (use Railway/VPS)

## 🔍 Verify Deployment

### Check Explorer

1. Visit your deployed explorer URL
2. Open browser console (F12)
3. Check Network tab - all RPC calls should go to Railway URL
4. Verify blocks are loading from testnet

### Check Mining Pool

1. Visit your deployed pool dashboard URL
2. Check "Network Status" page
3. Verify "Blockchain RPC" shows Railway URL
4. Verify connection status is "Online"

### Check Wallet API

1. Visit your deployed wallet API URL
2. Create a test wallet
3. Check network info - should show Railway URL

## 📝 Summary

**YES - Everything uses Railway URL when deployed!**

- ✅ Explorer → Railway URL
- ✅ Pools → Railway URL
- ✅ Wallet API → Railway URL
- ✅ All RPC → Railway URL
- ✅ All services → Testnet (Railway)

**No localhost references in production!** 🎉
