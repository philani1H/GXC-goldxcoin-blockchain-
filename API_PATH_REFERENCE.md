# GXC API Path Reference - Complete Guide

## Summary

**NOT all APIs work with both `/` and `/api/` - here's the complete breakdown:**

---

## Three Ways to Call APIs

### 1. POST / (JSON-RPC) - ✅ ALL METHODS
**Works for:** ALL 100+ RPC methods
**Format:** JSON-RPC 2.0
**Best for:** Maximum compatibility, all features

```bash
curl -X POST https://gxc-node.railway.app/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "METHOD_NAME",
    "params": [],
    "id": 1
  }'
```

### 2. GET /api/method - ✅ 5 COMMON METHODS
**Works for:** Only methods we explicitly added GET support
**Format:** REST-style GET request
**Best for:** Simple queries, Railway proxy compatibility

```bash
curl https://gxc-node.railway.app/api/METHOD_NAME
```

### 3. GET /method - ✅ 5 COMMON METHODS
**Works for:** Same as /api/method (alternative path)
**Format:** REST-style GET request without /api prefix
**Best for:** Shorter URLs

```bash
curl https://gxc-node.railway.app/METHOD_NAME
```

---

## Complete Method Support Matrix

| Method | POST / | GET /api/method | GET /method | Notes |
|--------|--------|-----------------|-------------|-------|
| **getinfo** | ✅ | ✅ | ✅ | Blockchain info |
| **getblockcount** | ✅ | ✅ | ✅ | Current height |
| **getdifficulty** | ✅ | ✅ | ✅ | Current difficulty |
| **getbestblockhash** | ✅ | ✅ | ✅ | Latest block hash |
| **getblocktemplate** | ✅ | ✅ | ✅ | Mining template |
| **getblockchaininfo** | ✅ | ❌ | ❌ | POST only |
| **getmininginfo** | ✅ | ❌ | ❌ | POST only |
| **getnetworkinfo** | ✅ | ❌ | ❌ | POST only |
| **getpeerinfo** | ✅ | ❌ | ❌ | POST only |
| **getblock** | ✅ | ❌ | ❌ | POST only |
| **getnewaddress** | ✅ | ❌ | ❌ | POST only |
| **getbalance** | ✅ | ❌ | ❌ | POST only |
| **sendtoaddress** | ✅ | ❌ | ❌ | POST only |
| **submitblock** | ✅ | ❌ | ❌ | POST only |
| **listunspent** | ✅ | ❌ | ❌ | POST only |
| **getstakinginfo** | ✅ | ❌ | ❌ | POST only |
| **listvalidators** | ✅ | ❌ | ❌ | POST only |
| **getrawmempool** | ✅ | ❌ | ❌ | POST only |
| **gettransaction** | ✅ | ❌ | ❌ | POST only |
| **validatetraceability** | ✅ | ❌ | ❌ | POST only |
| **All other methods** | ✅ | ❌ | ❌ | POST only |

---

## Methods with GET Support (5 total)

These methods work with **all three** calling patterns:

### 1. getinfo
```bash
# Method 1: POST /
curl -X POST https://gxc-node.railway.app/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}'

# Method 2: GET /api/getinfo
curl https://gxc-node.railway.app/api/getinfo

# Method 3: GET /getinfo
curl https://gxc-node.railway.app/getinfo
```

### 2. getblockcount
```bash
# All three work:
POST / {"method":"getblockcount"}
GET /api/getblockcount
GET /getblockcount
```

### 3. getdifficulty
```bash
# All three work:
POST / {"method":"getdifficulty"}
GET /api/getdifficulty
GET /getdifficulty
```

### 4. getbestblockhash
```bash
# All three work:
POST / {"method":"getbestblockhash"}
GET /api/getbestblockhash
GET /getbestblockhash
```

### 5. getblocktemplate
```bash
# All three work:
POST / {"method":"getblocktemplate"}
GET /api/getblocktemplate
GET /getblocktemplate
```

---

## Methods with POST ONLY (95+ methods)

These methods **ONLY** work with `POST /`:

### Blockchain Methods
- getblockchaininfo
- getblock
- getblockhash
- getchaintips
- getmempoolinfo
- getrawmempool
- gettxout
- gettxoutsetinfo
- verifychain

### Mining Methods
- getmininginfo
- getnetworkhashps
- prioritisetransaction
- submitblock
- getwork
- generatetoaddress

### Network Methods
- getnetworkinfo
- getpeerinfo
- addnode
- disconnectnode
- getaddednodeinfo
- getnettotals
- ping

### Wallet Methods
- getnewaddress
- getbalance
- listunspent
- sendtoaddress
- sendfrom
- sendmany
- gettransaction
- listtransactions
- listaccounts
- getaccount
- getaccountaddress
- setaccount
- dumpprivkey
- importprivkey
- signmessage
- verifymessage

### Staking Methods
- getstakinginfo
- listvalidators
- stake
- unstake
- getvalidatorinfo

### Governance Methods
- createproposal
- voteproposal
- listproposals
- getproposal

### Utility Methods
- validateaddress
- validatetraceability
- estimatefee
- estimatepriority

---

## Why Only 5 Methods Have GET Support?

We added GET support for the **5 most commonly used methods** to:
1. ✅ Bypass Railway proxy POST body issues
2. ✅ Enable simple browser testing
3. ✅ Provide REST-style convenience
4. ✅ Support mining pools (getblocktemplate)
5. ✅ Enable health monitoring (getinfo)

**All other methods still work perfectly via POST /**

---

## Recommendations by Use Case

### For Mining Pools
```bash
# Get template (GET for Railway compatibility)
GET /api/getblocktemplate

# Submit block (POST required)
POST / {"method":"submitblock","params":[block_data]}

# Check status (GET for convenience)
GET /api/getinfo
```

### For Wallets
```bash
# All wallet operations require POST /
POST / {"method":"getnewaddress"}
POST / {"method":"getbalance","params":[address]}
POST / {"method":"sendtoaddress","params":[to, amount]}
```

### For Explorers
```bash
# Mix of GET and POST
GET /api/getblockcount          # Quick queries
GET /api/getbestblockhash       # Quick queries
POST / {"method":"getblock"}    # Detailed data
POST / {"method":"gettransaction"}  # Detailed data
```

### For Monitoring
```bash
# Use GET for simple monitoring
GET /health                     # Health check
GET /api/getinfo               # Blockchain state
GET /api/getdifficulty         # Network difficulty
```

---

## Railway Deployment Specifics

### What Works on Railway

✅ **GET endpoints** - Always work (no proxy issues)
```bash
GET /health
GET /api/getinfo
GET /api/getblockcount
GET /api/getdifficulty
GET /api/getbestblockhash
GET /api/getblocktemplate
```

✅ **POST endpoints** - Work after our fixes
```bash
POST / with any JSON-RPC method
```

### Railway URL
```
https://gxc-chain112-blockchain-node-production.up.railway.app
```

### Testing on Railway
```bash
# Test GET (always works)
curl https://gxc-chain112-blockchain-node-production.up.railway.app/health

# Test POST (works after our fixes)
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'
```

---

## Quick Reference

### Health Check
```bash
GET /health          # ✅ Works
GET /ping            # ✅ Works
GET /                # ✅ Works
```

### Most Common Operations
```bash
# Get blockchain info
GET /api/getinfo                                    # ✅ Works
POST / {"method":"getblockchaininfo"}              # ✅ Works

# Get block count
GET /api/getblockcount                             # ✅ Works
POST / {"method":"getblockcount"}                  # ✅ Works

# Get mining template
GET /api/getblocktemplate                          # ✅ Works
POST / {"method":"getblocktemplate"}               # ✅ Works

# Submit block
POST / {"method":"submitblock","params":[data]}    # ✅ Works (POST only)

# Create address
POST / {"method":"getnewaddress"}                  # ✅ Works (POST only)

# Send transaction
POST / {"method":"sendtoaddress","params":[to,amt]} # ✅ Works (POST only)
```

---

## Error Handling

### 404 Not Found
```json
{
  "error": "Not found",
  "path": "/api/somemethod",
  "hint": "Try /health, /api/getinfo, /api/getblockcount, /api/getblocktemplate"
}
```

**Cause:** Trying to use GET on a POST-only method

**Solution:** Use POST / with JSON-RPC format

### Empty POST Body
```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32700,
    "message": "Parse error: Empty request body. Use GET endpoints instead: /api/getinfo, /api/getblockcount, /api/getblocktemplate"
  },
  "id": null
}
```

**Cause:** Railway proxy stripped POST body

**Solution:** Use GET endpoints or wait for Railway deployment

---

## Summary

### ✅ Works with All Three Paths (5 methods)
- getinfo
- getblockcount
- getdifficulty
- getbestblockhash
- getblocktemplate

### ✅ Works with POST / Only (95+ methods)
- All other RPC methods
- Wallet operations
- Transaction operations
- Staking operations
- Governance operations

### 🎯 Best Practice
**Use POST / for everything** - it works for all methods and is the standard JSON-RPC 2.0 protocol.

**Use GET /api/method** only for:
- Quick queries
- Browser testing
- Railway proxy compatibility
- Simple monitoring

---

**Last Updated:** December 31, 2025
**Node Version:** GXC 2.0.0
**Railway URL:** https://gxc-chain112-blockchain-node-production.up.railway.app
