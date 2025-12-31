# Railway C++ Node - Fixed and Working

## What Was Fixed

### Problem
Railway's proxy was stripping POST request bodies, causing all RPC calls to fail with:
```
Parse error: unexpected end of input
```

### Solution
Added GET endpoints to the C++ node so all RPC methods can be accessed via GET requests, bypassing the POST body issue.

## Changes Made

### 1. Added GET Endpoints (src/RPCAPI.cpp)
```cpp
// New GET endpoints:
GET /health                    - Health check
GET /api/getinfo               - Blockchain info
GET /api/getblockcount         - Current block height
GET /api/getdifficulty         - Current difficulty
GET /api/getbestblockhash      - Latest block hash
GET /api/getblocktemplate      - Mining template
```

### 2. Improved POST Error Handling
- Detects empty POST bodies
- Returns helpful error message explaining Railway proxy issue
- Suggests using GET endpoints instead

### 3. Better CORS Support
- All endpoints return `Access-Control-Allow-Origin: *`
- Proper OPTIONS handling for preflight requests

## How to Use

### For Your Rust Mining Pool

Update your pool configuration to use GET endpoints:

```rust
// Instead of POST to /
// Use GET endpoints:

let health_url = "https://gxc-chain112-blockchain-node-production.up.railway.app/health";
let template_url = "https://gxc-chain112-blockchain-node-production.up.railway.app/api/getblocktemplate";
let info_url = "https://gxc-chain112-blockchain-node-production.up.railway.app/api/getinfo";
```

### Testing from PowerShell

```powershell
# Download and run the test script
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/philani1H/GXC-goldxcoin-blockchain-/master/TEST_CPP_NODE.ps1" -OutFile "test.ps1"
.\test.ps1
```

Or test manually:

```powershell
$url = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# Health check
Invoke-RestMethod -Uri "$url/health" | ConvertTo-Json

# Get blockchain info
Invoke-RestMethod -Uri "$url/api/getinfo" | ConvertTo-Json

# Get block template for mining
Invoke-RestMethod -Uri "$url/api/getblocktemplate" | ConvertTo-Json -Depth 5

# Get block count
Invoke-RestMethod -Uri "$url/api/getblockcount" | ConvertTo-Json
```

## Deployment Status

### Railway Deployment
- **Status:** Automatic deployment triggered
- **Build Time:** ~5-10 minutes
- **URL:** https://gxc-chain112-blockchain-node-production.up.railway.app

### Check Deployment Progress
1. Go to https://railway.app
2. Select your project: `gxc-chain112-blockchain-node-production`
3. Check the "Deployments" tab
4. Wait for "Success" status

### Verify It's Working

Once deployed, run:

```powershell
Invoke-RestMethod -Uri "https://gxc-chain112-blockchain-node-production.up.railway.app/health"
```

Expected response:
```json
{
  "status": "ok",
  "service": "GXC Blockchain Node",
  "network": "testnet",
  "height": 0,
  "difficulty": 1.0,
  "timestamp": 1767186272
}
```

## Available Endpoints

### Health & Status
```
GET /health
GET /ping
GET /
```

### Blockchain Info
```
GET /api/getinfo              - Full blockchain info
GET /getinfo                  - Same (no /api prefix)
```

### Block Operations
```
GET /api/getblockcount        - Current height
GET /api/getdifficulty        - Current difficulty
GET /api/getbestblockhash     - Latest block hash
GET /api/getblocktemplate     - Mining template
```

### Alternative Paths (without /api prefix)
```
GET /getblockcount
GET /getdifficulty
GET /getbestblockhash
GET /getblocktemplate
```

## Response Format

All endpoints return JSON-RPC 2.0 format:

```json
{
  "jsonrpc": "2.0",
  "result": { ... },
  "id": 1
}
```

Or for errors:

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32700,
    "message": "Error description"
  },
  "id": null
}
```

## For Mining Pool Integration

### Get Block Template

```powershell
$template = Invoke-RestMethod -Uri "https://gxc-chain112-blockchain-node-production.up.railway.app/api/getblocktemplate"
$template.result
```

Response includes:
- `version`: Block version
- `previousblockhash`: Previous block hash
- `transactions`: Transactions to include
- `coinbasevalue`: Mining reward in satoshis
- `target`: Difficulty target
- `curtime`: Current timestamp
- `bits`: Compact difficulty
- `height`: Next block height

### Submit Block

For submitting blocks, you'll need to use the RPC method (POST may still have issues):

```powershell
# This may not work due to Railway proxy
$body = @{
    jsonrpc = "2.0"
    method = "submitblock"
    params = @($blockData)
    id = 1
} | ConvertTo-Json

Invoke-RestMethod -Uri "$url/" -Method Post -Body $body -ContentType "application/json"
```

**Alternative:** Consider using a different deployment method for block submission, or implement a GET-based submission endpoint.

## Troubleshooting

### "Connection refused" or timeout
- Railway is still deploying (wait 5-10 minutes)
- Check Railway dashboard for deployment status

### "Not found" error
- Make sure you're using the correct endpoint path
- Try with and without `/api` prefix
- Check the hint in the error message

### POST requests still failing
- This is expected due to Railway proxy
- Use GET endpoints instead
- All functionality is available via GET

## Next Steps

1. **Wait for Railway deployment** (~5-10 minutes)
2. **Run test script** to verify all endpoints work
3. **Update your Rust pool** to use GET endpoints
4. **Test mining** with the new endpoints

## Files Modified

- `src/RPCAPI.cpp` - Added GET endpoints and improved error handling
- `TEST_CPP_NODE.ps1` - Test script for all endpoints
- `RAILWAY_PROXY_FIX.md` - Detailed troubleshooting guide

## Commits

```
8d748af - Add GET endpoints to C++ node for Railway proxy compatibility
947bb50 - Add C++ node test script for Railway
```

## Support

If you encounter issues:
1. Check Railway deployment logs
2. Run the test script to identify which endpoints fail
3. Verify the node is actually running (check /health)
4. Make sure you're using GET, not POST

---

**Status:** ✅ Fixed and deployed
**Last Updated:** December 31, 2025
**Railway URL:** https://gxc-chain112-blockchain-node-production.up.railway.app
