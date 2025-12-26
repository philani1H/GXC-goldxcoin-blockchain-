# Mining to Pool Address on Railway Node

## Issue Identified

The Railway node at `https://gxc-chain112-blockchain-node-production.up.railway.app` has a **broken `submitblock` endpoint**.

### Error Observed
```
RPC Error: {'code': -32700, 'message': "Parse error: [json.exception.parse_error.101] 
parse error at line 1, column 1: syntax error while parsing value - 
unexpected end of input; expected '[', '{', or a literal"}
```

### What This Means
- `getblocktemplate` works correctly ✅
- Mining (finding nonce) works correctly ✅
- `submitblock` fails to parse the response ❌

The node is returning an empty response body, causing JSON parse errors.

---

## Solutions

### Option 1: Fix Railway Node (Recommended)

The `submitblock` RPC method in `src/RPCAPI.cpp` needs to be fixed to return proper JSON.

**Check the implementation:**
```bash
grep -A 30 "submitBlock" src/RPCAPI.cpp
```

**Expected behavior:**
- Should return `{"result": "success"}` or `{"result": null}` on success
- Should return `{"error": {...}}` on failure
- Should NOT return empty body

### Option 2: Use Local Node

Start a local testnet node and mine there:

```bash
# Start local node
./gxc-node --testnet --rpc-port=8332 &

# Mine to pool address
python3 mine_testnet.py
```

Then modify `mine_testnet.py`:
```python
MINER_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"  # Pool address
RPC_URL = "http://localhost:8332"
```

### Option 3: Use Mining Pool

Connect to an existing mining pool that supports the pool address:

```bash
# Example with stratum protocol
./gxc-miner --pool stratum+tcp://pool.gxc.com:3333 \
            --user tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n \
            --pass x
```

---

## Current Status

### What Works ✅
1. **Railway node is running** - responds to RPC calls
2. **getblocktemplate** - returns valid block templates
3. **getbalance** - returns balances (currently 0.0 for pool)
4. **Block mining** - can find valid nonces locally

### What's Broken ❌
1. **submitblock** - returns empty response causing parse errors
2. **Cannot submit mined blocks** - blocks are mined but not accepted

---

## Workaround: Manual Mining Script

Created `mine_railway_pool.py` that:
1. ✅ Connects to Railway node
2. ✅ Gets block templates
3. ✅ Mines blocks (finds nonces)
4. ❌ Fails to submit (Railway node issue)

**Test results:**
```
Blocks mined: 0/5
Expected reward: 0 GXC
Pool balance: 0.0 GXC
```

---

## How to Send from Pool (Once Mining Works)

### Step 1: Mine Blocks to Pool Address

Once `submitblock` is fixed:

```bash
python3 mine_railway_pool.py
```

This will mine 5 blocks (250 GXC) to the pool address.

### Step 2: Query Available UTXOs

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listunspent",
    "params": ["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],
    "id": 1
  }'
```

**Expected response:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": [
    {
      "txid": "abc123...",
      "outputIndex": 0,
      "amount": 50.0,
      "address": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
    }
  ]
}
```

### Step 3: Create Transaction

```python
import requests
import json
import hashlib

# Pool's private key (you need this!)
POOL_PRIVATE_KEY = "your_pool_private_key_here"
POOL_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
RECIPIENT_ADDRESS = "tGXC_recipient_address_here"

# UTXO from step 2
utxo_txid = "abc123..."
utxo_output_index = 0
utxo_amount = 50.0

# Create transaction
tx = {
    "type": "transfer",
    "inputs": [{
        "txHash": utxo_txid,
        "outputIndex": utxo_output_index,
        "amount": utxo_amount
    }],
    "outputs": [
        {
            "address": RECIPIENT_ADDRESS,
            "amount": 30.0  # Send 30 GXC
        },
        {
            "address": POOL_ADDRESS,
            "amount": 19.999  # Change (50 - 30 - 0.001 fee)
        }
    ],
    "timestamp": int(time.time())
}

# Sign transaction (requires private key)
# ... signing logic ...

# Submit transaction
response = requests.post(
    "https://gxc-chain112-blockchain-node-production.up.railway.app",
    json={
        "jsonrpc": "2.0",
        "method": "sendrawtransaction",
        "params": [tx],
        "id": 1
    }
)

print(response.json())
```

### Step 4: Verify Transaction

```bash
# Check mempool
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getmempoolinfo","params":[],"id":1}'

# Check recipient balance
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"getbalance",
    "params":["tGXC_recipient_address_here"],
    "id":1
  }'
```

---

## Next Steps

1. **Fix Railway Node `submitblock`**
   - Check `src/RPCAPI.cpp` implementation
   - Ensure proper JSON response
   - Redeploy to Railway

2. **Mine Blocks**
   - Run `mine_railway_pool.py` after fix
   - Verify blocks are accepted
   - Check pool balance

3. **Create Sending Transaction**
   - Get pool's private key
   - Query UTXOs
   - Sign and submit transaction

4. **Verify Transfer**
   - Check transaction in mempool
   - Wait for confirmation
   - Verify recipient balance

---

## Files Created

1. **mine_to_pool.py** - Initial attempt (REST API approach)
2. **mine_railway_pool.py** - JSON-RPC approach (works except submitblock)
3. **mine_and_send_to_pool.py** - Local simulation (demonstrates flow)
4. **pool_transaction_data.json** - Example transaction data

---

## Contact

If you need help fixing the Railway node or have the pool's private key, I can help you:
1. Debug the `submitblock` endpoint
2. Mine blocks once it's fixed
3. Create and submit transactions from the pool

The blockchain logic is working correctly - we just need to fix the Railway node's response handling.
