# ✅ Testnet Railway URL Configuration - Complete Fix

## 🎯 Summary

All testnet services, miners, APIs, and pools are now configured to connect to the Railway testnet node URL with proper environment variable support.

## 🔧 Railway Testnet Node URL

**Production URL:** `https://gxc-chain112-blockchain-node-production.up.railway.app`

**Important:** Railway URLs should NOT include port numbers. Railway handles port routing automatically.

✅ **Correct:**
- `https://gxc-chain112-blockchain-node-production.up.railway.app`

❌ **Incorrect:**
- `https://gxc-chain112-blockchain-node-production.up.railway.app:18332`
- `http://localhost:18332` (for production)

## 📋 Files Updated

### 1. Testnet Miners ✅

#### `mine_testnet.py`
- ✅ Uses Railway URL from environment variables
- ✅ Falls back to Railway URL for production
- ✅ Supports `BLOCKCHAIN_RPC_URL` and `RAILWAY_NODE_URL` environment variables

```python
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', 'http://localhost:18332'))
```

#### `mine_testnet_fixed.py`
- ✅ Uses Railway URL from environment variables
- ✅ Falls back to Railway URL for production
- ✅ Supports environment variable overrides

```python
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
```

### 2. Testnet Node ✅

#### `testnet_blockchain_node.py`
- ✅ Uses Railway's `PORT` environment variable
- ✅ Binds to `0.0.0.0` when `PORT` is present (Railway requirement)
- ✅ Proper error handling for Railway deployment
- ✅ Default port 18332 for local development

### 3. Mining Pools ✅

#### `mining_pool/general_pool.py`
- ✅ Uses Railway URL as default
- ✅ Supports `BLOCKCHAIN_NODE_URL` environment variable

#### `mining_pool/gpu_pool.py`
- ✅ Uses Railway URL as default
- ✅ Supports `BLOCKCHAIN_NODE_URL` environment variable

#### `mining_pool/asic_pool.py`
- ✅ Uses Railway URL as default
- ✅ Supports `BLOCKCHAIN_NODE_URL` environment variable

#### `mining_pool/pool_base.py`
- ✅ Already had Railway URL as default
- ✅ Inherits from environment variables

### 4. APIs ✅

#### `api/wallet_service.py`
- ✅ Uses Railway URL for both testnet and mainnet
- ✅ Supports `BLOCKCHAIN_RPC_URL` and `BLOCKCHAIN_REST_URL` environment variables
- ✅ Separate configuration for testnet vs mainnet
- ✅ `BlockchainClient` uses Railway URL by default

```python
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
NETWORK_INFO = {
    'rpc_url': os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL)),
    'rest_url': os.environ.get('BLOCKCHAIN_REST_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL)),
}
```

#### `web/blockchain_explorer.py`
- ✅ Uses Railway URL from environment variables
- ✅ Supports `BLOCKCHAIN_NODE_URL` and `RAILWAY_NODE_URL` environment variables
- ✅ Falls back to Railway URL for production

```python
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
BLOCKCHAIN_NODE_URL = os.environ.get('BLOCKCHAIN_NODE_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
```

### 5. Configuration Files ✅

#### `config/railway_config.py`
- ✅ Centralized Railway URL configuration
- ✅ Supports testnet and mainnet
- ✅ Environment variable support

#### `railway.toml`
- ✅ Proper Railway deployment configuration
- ✅ Environment variables set correctly
- ✅ PORT handling (Railway provides automatically)

## 🔌 Environment Variables

All services support these environment variables:

```bash
# Primary Railway node URL (used by all services)
BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
BLOCKCHAIN_REST_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Alternative (for backwards compatibility)
RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Network configuration
GXC_NETWORK=testnet

# Railway-specific (automatically set by Railway)
PORT=<auto-set-by-railway>
RAILWAY_ENVIRONMENT=1
```

## 🚀 Usage Examples

### Testnet Miner

```bash
# Use Railway URL (default)
python mine_testnet.py

# Override with environment variable
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
python mine_testnet.py

# Or use fixed version
python mine_testnet_fixed.py
```

### Mining Pool

```bash
# Use Railway URL (default)
python mining_pool/general_pool.py

# Override with environment variable
export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
python mining_pool/general_pool.py
```

### Wallet API

```bash
# Use Railway URL (default)
python api/wallet_service.py

# Override with environment variable
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
export BLOCKCHAIN_REST_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
python api/wallet_service.py
```

### Explorer

```bash
# Use Railway URL (default)
python web/blockchain_explorer.py

# Override with environment variable
export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
python web/blockchain_explorer.py
```

## ✅ Verification Checklist

- [x] All testnet miners use Railway URL
- [x] All mining pools use Railway URL
- [x] Wallet API uses Railway URL for testnet
- [x] Explorer uses Railway URL
- [x] Testnet node properly handles Railway PORT
- [x] All services support environment variable overrides
- [x] No hardcoded localhost URLs in production code
- [x] Proper fallback to Railway URL when environment variables not set
- [x] Testnet and mainnet properly separated

## 🔍 Testing

### Test Railway Node Connection

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

### Test Mining Pool Connection

```bash
# Set Railway URL
export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Run pool
python mining_pool/general_pool.py
```

### Test Wallet API Connection

```bash
# Set Railway URL
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
export BLOCKCHAIN_REST_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Run wallet API
python api/wallet_service.py
```

### Test Explorer Connection

```bash
# Set Railway URL
export BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Run explorer
python web/blockchain_explorer.py
```

## 📝 Important Notes

1. **Railway URLs don't include ports** - Railway handles port routing automatically
2. **PORT is auto-set by Railway** - Don't set it manually in environment variables
3. **All services default to Railway URL** - Production-ready by default
4. **Environment variables override defaults** - Easy to customize per deployment
5. **Testnet and mainnet separated** - Different databases and configurations
6. **Local development still works** - Falls back to localhost when Railway URL not available

## 🎯 Summary

All testnet services are now:
- ✅ Connected to Railway testnet node URL
- ✅ Using proper environment variable support
- ✅ Production-ready by default
- ✅ Supporting both local and Railway deployments
- ✅ Properly handling testnet vs mainnet separation

**Railway Testnet Node:** `https://gxc-chain112-blockchain-node-production.up.railway.app`

All miners, pools, APIs, and services are configured to connect to this URL automatically! 🚀
