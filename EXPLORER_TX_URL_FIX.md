# Explorer Transaction URL - Correct Format

## Issue

User provided URL: `http://localhost:3000/tx/0e7ec5e90dc30a94e48f6493a1a91825e2fdf4896d66fe045fcf3dd23714ea99`

**Problems:**
1. ❌ Wrong port: `3000` (should be `5001` for explorer)
2. ❌ Transaction hash doesn't exist in blockchain

## Correct Format

### Explorer URL

**Correct Port:** `5001` (not 3000)

```
http://localhost:5001/tx/<transaction_hash>
```

### Example with Real Transaction

**Get a real transaction:**
```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getaddresstransactions","params":["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],"id":1}' \
  | jq '.result[0].txid'
```

**Output:**
```
"164789a73f1cf006b0ff72e3da7378379a8e04d6dc6e079ff04e7fa68c8d5b01"
```

**Correct Explorer URL:**
```
http://localhost:5001/tx/164789a73f1cf006b0ff72e3da7378379a8e04d6dc6e079ff04e7fa68c8d5b01
```

---

## Port Configuration

### GXC Services Ports

| Service | Port | URL |
|---------|------|-----|
| **Blockchain Node** | 8332 | http://localhost:8332 |
| **Explorer** | 5001 | http://localhost:5001 |
| **Admin API** | 5002 | http://localhost:5002 |
| **Forum** | 5000 | http://localhost:5000 |
| **Market Maker** | 5003 | http://localhost:5003 |

### Why Port 3000 Doesn't Work

Port 3000 is typically used for:
- React development servers
- Next.js applications
- Other frontend frameworks

The GXC Explorer runs on **port 5001** by default.

---

## How to Find Real Transactions

### Method 1: Get Transactions from Address

```bash
# Get transactions for an address
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getaddresstransactions",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": [
    {
      "txid": "164789a73f1cf006b0ff72e3da7378379a8e04d6dc6e079ff04e7fa68c8d5b01",
      "amount": 50.0,
      "blockHeight": 38,
      "confirmations": 15,
      "type": "coinbase"
    }
  ]
}
```

### Method 2: Get Transactions from Block

```bash
# Get latest block
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
  | jq '.result.blocks'

# Get block details
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getblock","params":[53],"id":1}' \
  | jq '.result.transactions'
```

### Method 3: List All Unspent Outputs

```bash
# Get all UTXOs (includes transaction hashes)
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"listunspent","params":[],"id":1}' \
  | jq '.result[0].txHash'
```

---

## Testing Transaction Page

### Step 1: Get a Real Transaction

```bash
TX_HASH=$(curl -s -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getaddresstransactions","params":["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],"id":1}' \
  | jq -r '.result[0].txid')

echo "Transaction Hash: $TX_HASH"
```

### Step 2: Test Explorer URL

```bash
# Open in browser
echo "http://localhost:5001/tx/$TX_HASH"

# Or test with curl
curl -s "http://localhost:5001/tx/$TX_HASH" | grep -o "Transaction Details" && echo "✅ Page loads"
```

### Step 3: Verify Transaction Data

The explorer page should show:
- ✅ Transaction hash
- ✅ Block number
- ✅ Amount (50 GXC)
- ✅ Confirmations
- ✅ Timestamp
- ✅ From/To addresses
- ✅ Transaction type (coinbase)
- ✅ Traceability status

---

## Explorer Routes

### All Transaction-Related Routes

| Route | Description | Example |
|-------|-------------|---------|
| `/tx/<hash>` | Transaction details | `/tx/164789a73f1cf006...` |
| `/transactions` | List all transactions | `/transactions` |
| `/address/<address>` | Address transactions | `/address/tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9` |
| `/block/<number>` | Block with transactions | `/block/38` |
| `/search?q=<query>` | Search transactions | `/search?q=164789a73f1cf006` |

---

## Common Issues

### Issue 1: Transaction Not Found

**Error:** `Transaction not found`

**Causes:**
- Transaction hash doesn't exist
- Transaction not yet mined
- Wrong network (testnet vs mainnet)

**Solution:**
```bash
# Verify transaction exists
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"gettransaction","params":["<hash>",true],"id":1}'
```

### Issue 2: Wrong Port

**Error:** `Connection refused` or `Cannot GET /tx/...`

**Cause:** Using wrong port (3000 instead of 5001)

**Solution:** Use correct port `5001`

### Issue 3: Explorer Not Running

**Error:** `Connection refused`

**Solution:**
```bash
# Start explorer
cd web
python3 blockchain_explorer.py
```

---

## Summary

### ❌ Wrong URL
```
http://localhost:3000/tx/0e7ec5e90dc30a94e48f6493a1a91825e2fdf4896d66fe045fcf3dd23714ea99
```

**Problems:**
- Wrong port (3000 → should be 5001)
- Transaction doesn't exist in blockchain

### ✅ Correct URL
```
http://localhost:5001/tx/164789a73f1cf006b0ff72e3da7378379a8e04d6dc6e079ff04e7fa68c8d5b01
```

**Features:**
- Correct port (5001)
- Real transaction from blockchain
- Shows all transaction details
- Includes traceability validation

---

## Quick Test

```bash
# 1. Get real transaction
TX=$(curl -s -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getaddresstransactions","params":["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],"id":1}' \
  | jq -r '.result[0].txid')

# 2. Open in browser
echo "Open this URL: http://localhost:5001/tx/$TX"

# 3. Or test with curl
curl -s "http://localhost:5001/tx/$TX" | head -20
```

The explorer transaction page is working correctly - just need to use the right port (5001) and a real transaction hash!
