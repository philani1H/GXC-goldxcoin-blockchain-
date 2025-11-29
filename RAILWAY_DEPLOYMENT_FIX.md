# Railway Deployment Fix - Complete Guide

## ✅ Fixed Issues

### 1. Railway Node Crash Fix
- **Fixed PORT handling**: Node now properly uses Railway's `PORT` environment variable
- **Added error handling**: Better error messages and graceful shutdown
- **Fixed binding**: Automatically binds to `0.0.0.0` when `PORT` env var is present
- **Data directory**: Uses environment variable for data directory (Railway volumes)

### 2. All Miners Updated
- ✅ `mine_testnet.py` - Uses Railway URL from environment
- ✅ `mining/gxhash_miner.py` - Uses Railway URL from environment
- ✅ `packages/gxc-gxhash-miner-linux/gxhash_miner.py` - Uses Railway URL from environment
- ✅ `packages/gxc-gxhash-miner-windows/gxhash_miner.py` - Uses Railway URL from environment
- ✅ `packages/gxc-gxhash-miner-macos/gxhash_miner.py` - Uses Railway URL from environment

### 3. All Mining Pools Updated
- ✅ `mining_pool/general_pool.py` - Uses Railway URL as default
- ✅ `mining_pool/gpu_pool.py` - Uses Railway URL as default
- ✅ `mining_pool/asic_pool.py` - Uses Railway URL as default
- ✅ `mining_pool/pool_base.py` - Already had Railway URL as default

### 4. All APIs Updated
- ✅ `api/wallet_service.py` - Already uses Railway URL as default
- ✅ `web/blockchain_explorer.py` - Already uses Railway URL as default

## 🔧 Configuration

### Railway Node URL
**Production URL:** `https://gxc-chain112-blockchain-node-production.up.railway.app`

### Environment Variables

All services now support these environment variables:

```bash
# Primary Railway node URL
BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
BLOCKCHAIN_REST_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Alternative (for backwards compatibility)
RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
```

### URL Format

**Important:** Railway URLs should NOT include port numbers. Railway handles port routing automatically.

✅ **Correct:**
- `https://gxc-chain112-blockchain-node-production.up.railway.app`

❌ **Incorrect:**
- `https://gxc-chain112-blockchain-node-production.up.railway.app:18332`
- `https://gxc-chain112-blockchain-node-production.up.railway.app:8080`

## 🚀 Deployment Steps

### 1. Deploy to Railway

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login
railway login

# Deploy
railway up

# Get your URL
railway domain
```

### 2. Set Environment Variables in Railway

In Railway dashboard, set:
- `RAILWAY_ENVIRONMENT=1`
- `GXC_NETWORK=testnet`
- `GXC_LOG_LEVEL=info`

**Note:** `PORT` is automatically set by Railway - don't set it manually!

### 3. Connect Miners

All miners now automatically connect to Railway URL. You can override with:

```bash
# Set environment variable
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Or pass as argument (if miner supports it)
python gxhash_miner.py --rpc-url https://gxc-chain112-blockchain-node-production.up.railway.app
```

### 4. Connect Mining Pools

Pools automatically use Railway URL. Override if needed:

```bash
export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
python mining_pool/general_pool.py
```

### 5. Connect APIs

APIs automatically use Railway URL. Override if needed:

```bash
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
python api/wallet_service.py
```

## 🔍 Testing

### Test Railway Node

```bash
# Test RPC endpoint
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": "test",
    "method": "getblockchaininfo",
    "params": []
  }'
```

### Test Miner Connection

```bash
# Set Railway URL
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Run miner
python mine_testnet.py
```

### Test Mining Pool

```bash
# Set Railway URL
export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Run pool
python mining_pool/general_pool.py
```

## 📋 Checklist

- [x] Railway node uses PORT environment variable
- [x] Railway node binds to 0.0.0.0 for Railway
- [x] All miners use Railway URL from environment
- [x] All mining pools use Railway URL from environment
- [x] All APIs use Railway URL from environment
- [x] URLs don't include port numbers (Railway handles routing)
- [x] Error handling added to Railway node
- [x] Environment variable support added everywhere

## 🐛 Troubleshooting

### Node Crashes on Railway

1. **Check logs:**
   ```bash
   railway logs
   ```

2. **Verify PORT is set:**
   Railway automatically sets `PORT` - don't set it manually

3. **Check data directory:**
   Use Railway volumes for persistent storage:
   ```bash
   railway volume create blockchain_data
   ```

### Miners Can't Connect

1. **Verify Railway URL:**
   ```bash
   curl https://gxc-chain112-blockchain-node-production.up.railway.app
   ```

2. **Check environment variable:**
   ```bash
   echo $BLOCKCHAIN_RPC_URL
   ```

3. **Test RPC endpoint:**
   Use the test command above

### Pools Can't Connect

1. **Set environment variable:**
   ```bash
   export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
   ```

2. **Check pool logs:**
   Look for connection errors

## 📝 Notes

- Railway automatically handles HTTPS and port routing
- No need to specify ports in Railway URLs
- All services fall back to Railway URL if environment variable not set
- Local development still works with localhost URLs
- Production-ready configuration is now the default

## 🎯 Summary

All services are now configured to:
1. ✅ Use Railway URL by default
2. ✅ Support environment variable overrides
3. ✅ Work in both local and production environments
4. ✅ Handle Railway's PORT environment variable correctly
5. ✅ Use proper URL format (no port numbers for Railway)

Your Railway deployment should now work correctly! 🚀
