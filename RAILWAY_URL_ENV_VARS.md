# 🔧 Railway URL Environment Variables Configuration

## ✅ All Services Now Use `RAILWAY_NODE_URL` from Environment Variables

All services are configured to use `RAILWAY_NODE_URL` from environment variables, ensuring they connect to the correct Railway testnet URL when deployed.

## 📋 Configuration Files

### 1. **Explorer** (`web/vercel.json`)

```json
{
  "env": {
    "VERCEL": "1",
    "RAILWAY_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app",
    "BLOCKCHAIN_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app"
  }
}
```

**Code** (`web/blockchain_explorer.py`):
```python
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
BLOCKCHAIN_NODE_URL = os.environ.get('BLOCKCHAIN_NODE_URL', 
                                     os.environ.get('RAILWAY_NODE_URL', 
                                                   RAILWAY_NODE_URL))
```

### 2. **Mining Pools** (`mining_pool/vercel.json`)

```json
{
  "env": {
    "VERCEL": "1",
    "POOL_TYPE": "general",
    "DATABASE_PATH": "/tmp/pool.db",
    "RAILWAY_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app",
    "BLOCKCHAIN_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app"
  }
}
```

**Code** (`mining_pool/pool_config.py`):
```python
# Railway Node URL (Testnet) - Default for all services
RAILWAY_NODE_URL = os.environ.get(
    'RAILWAY_NODE_URL',
    'https://gxc-chain112-blockchain-node-production.up.railway.app'
)

# Blockchain Node (Railway Testnet) - Use RAILWAY_NODE_URL
BLOCKCHAIN_NODE_URL = os.environ.get(
    'BLOCKCHAIN_NODE_URL',
    os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL)
)
```

**Code** (`mining_pool/pool_base.py`):
```python
# Use RAILWAY_NODE_URL from environment, fallback to Railway URL
railway_url = os.environ.get('RAILWAY_NODE_URL', 
                             'https://gxc-chain112-blockchain-node-production.up.railway.app')
self.rpc_url = rpc_url or os.environ.get('BLOCKCHAIN_NODE_URL', railway_url)
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

## 🚀 Deployment

### Set Environment Variables in Vercel

When deploying to Vercel, set these environment variables:

```bash
# For Explorer
vercel env add RAILWAY_NODE_URL production
# Enter: https://gxc-chain112-blockchain-node-production.up.railway.app

vercel env add BLOCKCHAIN_NODE_URL production
# Enter: https://gxc-chain112-blockchain-node-production.up.railway.app

# For Mining Pools
cd mining_pool
vercel env add RAILWAY_NODE_URL production
# Enter: https://gxc-chain112-blockchain-node-production.up.railway.app

vercel env add BLOCKCHAIN_NODE_URL production
# Enter: https://gxc-chain112-blockchain-node-production.up.railway.app
```

Or set them in Vercel Dashboard:
1. Go to your project settings
2. Navigate to "Environment Variables"
3. Add:
   - `RAILWAY_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
   - `BLOCKCHAIN_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`

## ✅ What's Fixed

1. ✅ **All services use `RAILWAY_NODE_URL` from environment variables**
2. ✅ **No localhost references in production code**
3. ✅ **Stratum pools use Railway URL for blockchain connection**
4. ✅ **All RPC calls go to Railway testnet**
5. ✅ **Environment variables are set in `vercel.json` files**

## 🔍 Verification

After deployment, verify:

1. **Explorer**: Check browser console → Network tab → All RPC calls should go to Railway URL
2. **Mining Pool**: Check "Network Status" page → Should show Railway URL and "Online" status
3. **Wallet API**: Create wallet → Check network info → Should show Railway URL

## 📝 Summary

**All services now use `RAILWAY_NODE_URL` from environment variables!**

- ✅ Explorer → Uses `RAILWAY_NODE_URL` env var
- ✅ Mining Pools → Uses `RAILWAY_NODE_URL` env var
- ✅ Wallet API → Uses `RAILWAY_NODE_URL` env var
- ✅ All RPC → Railway testnet URL
- ✅ No localhost in production
