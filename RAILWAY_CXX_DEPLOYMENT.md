# Deploy C++ Testnet Node to Railway - Complete Guide

## Quick Deploy

```bash
# 1. Install Railway CLI
npm install -g @railway/cli

# 2. Login
railway login

# 3. Link to project (or create new)
railway link

# 4. Deploy
railway up

# 5. Get URL
railway domain
```

## What's Configured

### Files Created/Updated:

1. **`Dockerfile.testnet`** - Builds C++ node for Railway
2. **`railway.toml`** - Railway configuration (uses Dockerfile)
3. **`src/node_main.cpp`** - Updated to read Railway PORT env var
4. **`.dockerignore`** - Optimizes Docker build

### Key Changes:

- ✅ C++ node automatically uses Railway's `PORT` environment variable
- ✅ Dockerfile builds the C++ node from source
- ✅ Testnet configuration included
- ✅ Health check configured

## Deployment Process

### Step 1: Railway Setup

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login (opens browser)
railway login
```

### Step 2: Initialize Project

```bash
# Link to existing project or create new
railway link

# Or create new project
railway init
# Choose: Empty project
# Name: gxc-testnet-node
```

### Step 3: Deploy

```bash
# Deploy (builds Docker image and starts node)
railway up
```

This will:
1. Build the C++ node using `Dockerfile.testnet`
2. Create testnet configuration
3. Start the node
4. Use Railway's PORT for RPC

### Step 4: Get Public URL

```bash
# Generate public domain
railway domain

# Output example:
# https://gxc-testnet-node-production.up.railway.app
```

### Step 5: Verify Deployment

```bash
# View logs
railway logs

# Test RPC endpoint
curl -X POST https://your-railway-url.up.railway.app/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

## Configuration Details

### Railway Environment Variables

Railway automatically provides:
- `PORT` - Port for the service (C++ node uses this for RPC)

You can set in Railway dashboard:
- `GXC_NETWORK=testnet` (optional, already in railway.toml)
- `GXC_LOG_LEVEL=INFO` (optional)

### Port Configuration

The C++ node:
- Reads `PORT` environment variable (Railway provides this)
- Uses it for RPC port
- Network port: 19333 (testnet)
- REST port: 18080 (testnet)

### Data Persistence

**Important:** Railway containers are ephemeral. For persistent data:

```bash
# Create volume
railway volume create testnet_data

# Mount at: /app/testnet_data
```

Update `railway.toml`:
```toml
[[volumes]]
name = "testnet_data"
mountPath = "/app/testnet_data"
```

## Update Services to Use New Node

Once deployed, update all services:

### Update `config/railway_config.py`

```python
RAILWAY_NODE_URL = "https://your-railway-url.up.railway.app/rpc"
```

### Or Set Environment Variable

```bash
export BLOCKCHAIN_RPC_URL=https://your-railway-url.up.railway.app/rpc
```

## Build Process

The Dockerfile:
1. Installs build dependencies (cmake, g++, libssl, sqlite3)
2. Copies source files
3. Builds C++ node: `make gxc-node`
4. Creates testnet config
5. Starts node with Railway PORT

**Build time:** ~5-10 minutes (first build)
**Subsequent builds:** Faster (Docker layer caching)

## Troubleshooting

### Build Fails

**Error:** CMake not found
**Solution:** Dockerfile installs all dependencies

**Error:** Compilation errors
**Solution:** Check build logs: `railway logs`

### Node Won't Start

**Check logs:**
```bash
railway logs
```

**Common issues:**
- Port binding → Node uses Railway PORT automatically
- Missing dependencies → Dockerfile installs them
- Config errors → Check gxc-testnet.conf format

### Can't Connect from Miners

**Verify URL:**
```bash
# Test RPC
curl -X POST https://your-railway-url.up.railway.app/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

**Update miner:**
```python
RPC_URL = "https://your-railway-url.up.railway.app/rpc"
```

## Monitoring

### View Logs

```bash
# Live logs
railway logs --follow

# Recent logs
railway logs --tail 100
```

### Check Status

```bash
# Service status
railway status

# Metrics (in Railway dashboard)
railway open
```

## Migration from Python Node

### Step 1: Deploy C++ Node

```bash
railway up
```

### Step 2: Get New URL

```bash
railway domain
```

### Step 3: Update All Services

Update miners, wallets, explorers to use new URL.

### Step 4: Test

```bash
# Test mining
python3 mine_testnet.py --address tGXC5066644d2aa70a7bf8947ee25af5e7c4e
```

## Benefits of C++ Node on Railway

✅ **Performance**: Faster than Python
✅ **Full Features**: All RPC endpoints
✅ **Traceability**: Complete transaction tracking
✅ **UTXO Support**: Accurate balances
✅ **Production Ready**: All C++ features
✅ **Always Online**: Railway handles uptime

## Summary

**Deployment:**
```bash
railway login
railway link
railway up
railway domain
```

**Your C++ testnet node will be live!** 🚀

**URL Format:**
```
https://your-railway-url.up.railway.app/rpc
```

**No port number needed** - Railway handles routing automatically!
