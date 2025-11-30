# Testnet Safety Verification

## ✅ Confirmed: Railway Deployment is TESTNET ONLY

### Verification Checklist

#### 1. Dockerfile Configuration ✅
**File:** `Dockerfile.testnet`
- ✅ Config file: `gxc-testnet.conf` (testnet-specific)
- ✅ Testnet flag: `testnet=true`
- ✅ Testnet ports: Network 19333, RPC 18332, REST 18080
- ✅ Testnet data dir: `/app/testnet_data`
- ✅ Command: `--testnet` flag explicitly set

#### 2. Railway Configuration ✅
**File:** `railway.toml`
- ✅ Environment: `GXC_NETWORK=testnet`
- ✅ Start command: `--testnet` flag included
- ✅ Config file: `gxc-testnet.conf` (testnet config)

#### 3. Node Code ✅
**File:** `src/node_main.cpp`
- ✅ `--testnet` flag sets testnet mode
- ✅ Testnet ports: 19333 (network), 18332 (RPC), 18080 (REST)
- ✅ Separate data directory for testnet
- ✅ Testnet mode explicitly enabled

#### 4. Address Prefixes ✅
- ✅ Testnet: `tGXC` prefix
- ✅ Mainnet: `GXC` prefix
- ✅ Nodes validate addresses match their network

## Complete Separation

### Testnet Configuration (Railway)
```
Network Port: 19333
RPC Port: 18332 (or Railway PORT)
REST Port: 18080
Data Directory: /app/testnet_data
Address Prefix: tGXC
Config File: gxc-testnet.conf
Flag: --testnet
```

### Mainnet Configuration (Separate)
```
Network Port: 9333
RPC Port: 8332
REST Port: 8080
Data Directory: ./gxc_data (or /app/gxc_data)
Address Prefix: GXC
Config File: gxc.conf
Flag: (none, or --mainnet)
```

## Safety Guarantees

### ✅ 1. Different Ports
- Testnet: 19333, 18332, 18080
- Mainnet: 9333, 8332, 8080
- **No port conflicts possible**

### ✅ 2. Different Data Directories
- Testnet: `/app/testnet_data` or `./testnet_data`
- Mainnet: `./gxc_data` or `/app/gxc_data`
- **Completely separate blockchains**

### ✅ 3. Different Address Prefixes
- Testnet addresses: `tGXC...`
- Mainnet addresses: `GXC...`
- **Nodes reject wrong network addresses**

### ✅ 4. Explicit Testnet Flag
- Railway node: `--testnet` flag always set
- Config file: `testnet=true`
- Environment: `GXC_NETWORK=testnet`
- **Cannot accidentally run mainnet**

### ✅ 5. Separate Configuration Files
- Testnet: `gxc-testnet.conf`
- Mainnet: `gxc.conf`
- **Different configs, no mixing**

## What Railway Will Run

```bash
/app/gxc-node --testnet --config=/app/gxc-testnet.conf
```

This explicitly:
- ✅ Enables testnet mode
- ✅ Uses testnet configuration
- ✅ Uses testnet ports
- ✅ Uses testnet data directory
- ✅ Only accepts `tGXC` addresses

## Mainnet Protection

### Mainnet Cannot Be Affected Because:

1. **Different Ports**
   - Mainnet uses 9333/8332/8080
   - Testnet uses 19333/18332/18080
   - No overlap

2. **Different Data**
   - Mainnet: `gxc_data/`
   - Testnet: `testnet_data/`
   - Separate blockchains

3. **Network Validation**
   - Testnet node rejects `GXC` addresses
   - Mainnet node rejects `tGXC` addresses
   - Automatic protection

4. **Explicit Flags**
   - Railway always uses `--testnet`
   - Cannot accidentally run mainnet
   - Configuration enforces testnet

## Verification Commands

### Check Railway Node is Testnet

```bash
# After deployment, test:
curl -X POST https://your-railway-url.up.railway.app/rpc \
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
  "result": {
    "chain": "testnet",  // ✅ Confirms testnet
    ...
  }
}
```

### Check Address Validation

```bash
# Testnet address (should work)
curl -X POST https://your-railway-url.up.railway.app/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXC5066644d2aa70a7bf8947ee25af5e7c4e"],
    "id": 1
  }'

# Mainnet address (should be rejected or return 0)
curl -X POST https://your-railway-url.up.railway.app/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["GXC5066644d2aa70a7bf8947ee25af5e7c4e"],
    "id": 1
  }'
```

## Summary

### ✅ Railway Deployment is 100% Testnet

**Guarantees:**
- ✅ Uses `--testnet` flag
- ✅ Uses testnet ports (19333, 18332, 18080)
- ✅ Uses testnet data directory
- ✅ Uses testnet config file
- ✅ Environment variable: `GXC_NETWORK=testnet`
- ✅ Only accepts `tGXC` addresses

**Mainnet Protection:**
- ✅ Completely separate ports
- ✅ Completely separate data
- ✅ Network validation prevents mixing
- ✅ Explicit testnet configuration

**You can safely deploy to Railway - it will ONLY run testnet!** 🛡️
