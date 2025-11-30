# Deploy C++ Testnet Node to Railway

## Overview

This guide shows how to deploy the **C++ testnet node** to Railway, replacing the Python node.

## Quick Deploy

### Step 1: Update Railway Configuration

The `railway.toml` is already configured to use the C++ node via Dockerfile.

### Step 2: Deploy to Railway

```bash
# Install Railway CLI (if not installed)
npm install -g @railway/cli

# Login to Railway
railway login

# Link to your project (or create new)
railway link

# Deploy
railway up
```

### Step 3: Get Your URL

```bash
# Generate public domain
railway domain

# You'll get something like:
# https://gxc-testnet-node-production.up.railway.app
```

## What Changed

### Before (Python Node)
- `railway.toml` → Used Python/Nixpacks
- Started: `python3 testnet_blockchain_node.py`
- Dockerfile → Python-based

### After (C++ Node)
- `railway.toml` → Uses Dockerfile
- Dockerfile: `Dockerfile.testnet` → Builds C++ node
- Started: `./gxc-node --testnet --config=gxc-testnet.conf`

## Configuration Files

### `railway.toml`
```toml
[build]
builder = "DOCKERFILE"
dockerfilePath = "Dockerfile.testnet"

[deploy]
startCommand = "sh -c 'PORT=${PORT:-18332} && sed -i \"s/rpc_port=.*/rpc_port=$PORT/\" /app/gxc-testnet.conf && /app/gxc-node --testnet --config=/app/gxc-testnet.conf --rpc-port=$PORT'"
restartPolicyType = "ON_FAILURE"
restartPolicyMaxRetries = 10

[deploy.env]
RAILWAY_ENVIRONMENT = "1"
GXC_NETWORK = "testnet"
GXC_LOG_LEVEL = "INFO"
```

### `Dockerfile.testnet`
- Builds C++ node from source
- Creates testnet configuration
- Uses Railway's PORT environment variable
- Exposes RPC port

## Build Process

The Dockerfile will:
1. Install build dependencies (cmake, g++, libssl, sqlite3)
2. Copy source files
3. Build the C++ node (`gxc-node`)
4. Create testnet configuration
5. Start the node on Railway's PORT

## Environment Variables

Railway automatically provides:
- `PORT` - Port for the service (Railway sets this)

You can set in Railway dashboard:
- `GXC_NETWORK=testnet` (already set)
- `GXC_LOG_LEVEL=INFO` (already set)

## Data Persistence

### Create Volume for Blockchain Data

```bash
# Create volume in Railway
railway volume create testnet_data

# Mount at: /app/testnet_data
```

Update `railway.toml` to include volume:
```toml
[[volumes]]
name = "testnet_data"
mountPath = "/app/testnet_data"
```

## Verification

### Check Deployment

```bash
# View logs
railway logs

# Check status
railway status
```

### Test RPC Endpoint

```bash
# Get your Railway URL
RAILWAY_URL=$(railway domain)

# Test getblockchaininfo
curl -X POST $RAILWAY_URL/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "testnet",
    "blocks": 0,
    "height": 0,
    "block_reward": 50.0,
    "difficulty": 1000.0,
    ...
  },
  "id": 1
}
```

## Update Miners to Use New URL

Once deployed, update your miners:

```bash
# Set environment variable
export BLOCKCHAIN_RPC_URL=https://your-railway-url.up.railway.app/rpc

# Or update config/railway_config.py
RAILWAY_NODE_URL = "https://your-railway-url.up.railway.app/rpc"
```

## Troubleshooting

### Build Fails

**Error:** CMake or compiler not found
**Solution:** Dockerfile installs all dependencies automatically

### Port Binding Error

**Error:** Address already in use
**Solution:** Railway sets PORT automatically - code handles this

### Node Won't Start

**Check logs:**
```bash
railway logs
```

**Common issues:**
- Missing dependencies → Dockerfile installs them
- Permission errors → Files are owned by root in container
- Config errors → Check gxc-testnet.conf format

### Slow Build

**Solution:** Railway caches Docker layers. First build is slow, subsequent builds are faster.

## Migration from Python Node

### Step 1: Backup Data (if needed)

```bash
# Download blockchain data from old Python node
railway logs > old_node_logs.txt
```

### Step 2: Deploy C++ Node

```bash
# Deploy new C++ node
railway up
```

### Step 3: Verify

```bash
# Test new node
curl -X POST $(railway domain)/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

### Step 4: Update All Services

Update miners, wallets, and explorers to use new Railway URL.

## Benefits of C++ Node

✅ **Performance**: Faster than Python
✅ **Full Features**: All RPC endpoints implemented
✅ **Traceability**: Complete transaction tracking
✅ **UTXO Support**: Accurate balance calculation
✅ **Production Ready**: All features from C++ implementation

## Summary

**Deployment Steps:**
1. ✅ `railway.toml` configured for C++ node
2. ✅ `Dockerfile.testnet` builds C++ node
3. ✅ Run `railway up` to deploy
4. ✅ Get URL with `railway domain`
5. ✅ Update miners to use new URL

**Your C++ testnet node will be live on Railway!** 🚀
