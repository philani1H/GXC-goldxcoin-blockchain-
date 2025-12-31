# GXC Mining Pool - RPC Integration Guide

**Date:** 2025-12-31
**Purpose:** Complete guide for integrating mining pools with GXC blockchain node
**Node Version:** GXC v1.0+

---

## 📋 Table of Contents

1. [Connection Setup](#connection-setup)
2. [Required RPC Endpoints](#required-rpc-endpoints)
3. [Work Submission Flow](#work-submission-flow)
4. [Validator Integration](#validator-integration)
5. [Payment Processing](#payment-processing)
6. [Proof of Receipt](#proof-of-receipt)
7. [Error Handling](#error-handling)
8. [Testing](#testing)

---

## 🔌 Connection Setup

### Node Configuration

**Default Ports:**
- **Mainnet RPC:** `8332`
- **Testnet RPC:** `18332`
- **Content-Type:** `application/json`
- **Protocol:** `JSON-RPC 2.0`

### Start the Node

```bash
# Mainnet
./gxc-node --rpc-port=8332 --datadir=./gxc_data

# Testnet (recommended for pool testing)
./gxc-node --testnet --rpc-port=18332 --datadir=./gxc_test_data
```

### Health Check

**Endpoint:** `GET /health`

```bash
curl http://localhost:8332/health
```

**Response:**
```json
{
  "status": "ok",
  "service": "GXC Blockchain Node",
  "network": "testnet",
  "height": 150,
  "difficulty": 0.1,
  "timestamp": 1735646400
}
```

---

## 🎯 Required RPC Endpoints

### 1. Get Node Info

**Method:** `getinfo`
**Purpose:** Get basic node information
**Parameters:** None

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getinfo",
  "params": [],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "blocks": 150,
    "difficulty": 0.1,
    "chain": "test",
    "connections": 3
  },
  "id": 1
}
```

**Example:**
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}'
```

---

### 2. Get Balance

**Method:** `getbalance`
**Purpose:** Get wallet balance for an address
**Parameters:** `[address]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getbalance",
  "params": ["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": 250.5,
  "id": 1
}
```

**Example:**
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],
    "id": 1
  }'
```

---

### 3. Get Block Template

**Method:** `getblocktemplate`
**Purpose:** Get mining work template
**Parameters:** `[{"minerAddress": "address"}]` (optional)

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getblocktemplate",
  "params": [
    {
      "minerAddress": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
    }
  ],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "version": 1,
    "previousblockhash": "0000a1b2c3d4e5f6...",
    "height": 151,
    "transactions": [
      {
        "hash": "abc123...",
        "txid": "abc123...",
        "type": "COINBASE",
        "from": "",
        "to": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
        "amount": 50.0,
        "fee": 0.0
      }
    ],
    "coinbasevalue": 50.0,
    "target": "00000000ffff0000000000000000000000000000000000000000000000000000",
    "mintime": 1735646400,
    "mutable": ["time", "transactions", "prevblock"],
    "noncerange": "00000000ffffffff",
    "capabilities": ["proposal"],
    "bits": "1e0ffff0",
    "curtime": 1735646450,
    "difficulty": 0.1
  },
  "id": 1
}
```

**Key Fields:**
- `previousblockhash` - Hash of previous block
- `height` - Next block height
- `transactions` - Array of transactions to include (first is always coinbase)
- `coinbasevalue` - Mining reward (50 GXC)
- `target` - Difficulty target hash
- `difficulty` - Current difficulty
- `curtime` - Current timestamp

---

### 4. Submit Block

**Method:** `submitblock`
**Purpose:** Submit mined block to blockchain
**Parameters:** `[blockData]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "submitblock",
  "params": [
    {
      "height": 151,
      "hash": "00000abc123...",
      "previousblockhash": "0000a1b2c3d4e5f6...",
      "nonce": 123456789,
      "miner": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
      "timestamp": 1735646500,
      "difficulty": 0.1,
      "transactions": [...]
    }
  ],
  "id": 1
}
```

**Success Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "status": "accepted",
    "hash": "00000abc123...",
    "height": 151
  },
  "id": 1
}
```

**Error Response:**
```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -25,
    "message": "Block already exists or is invalid"
  },
  "id": 1
}
```

---

### 5. List Unspent Outputs (UTXOs)

**Method:** `listunspent`
**Purpose:** Get UTXOs for an address
**Parameters:** `[address]` or `[minConf, maxConf, [addresses]]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "listunspent",
  "params": ["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "txid": "abc123...",
      "txHash": "abc123...",
      "vout": 0,
      "outputIndex": 0,
      "address": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
      "amount": 50.0,
      "confirmations": 100,
      "spendable": true,
      "solvable": true
    }
  ],
  "id": 1
}
```

---

### 6. Send Payment

**Method:** `sendtoaddress`
**Purpose:** Send coins to an address
**Parameters:** `[toAddress, amount, fromAddress (optional)]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "sendtoaddress",
  "params": [
    "tGXC9fab7317231b966af85ac453e168c0932",
    10.5,
    "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
  ],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": "def456...",
  "id": 1
}
```

---

## 🔄 Work Submission Flow

### Step-by-Step Mining Flow

```
1. Pool starts up
   ↓
2. Call getinfo to verify node is accessible
   ↓
3. Call getbalance to get pool wallet balance
   ↓
4. Call getblocktemplate with pool's miner address
   ↓
5. Distribute work to connected miners
   ↓
6. Miner finds valid hash (nonce)
   ↓
7. Pool validates hash locally
   ↓
8. Pool calls submitblock with complete block data
   ↓
9. Node validates and adds block to chain
   ↓
10. Pool receives confirmation
   ↓
11. Pool updates shares and prepares payments
```

### Example: Complete Mining Cycle

```bash
# 1. Get block template
TEMPLATE=$(curl -s -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblocktemplate",
    "params": [{"minerAddress": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"}],
    "id": 1
  }')

echo "Template: $TEMPLATE"

# 2. Mine block (find nonce)
# ... mining happens here ...

# 3. Submit mined block
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "submitblock",
    "params": [{
      "height": 151,
      "hash": "00000abc123...",
      "previousblockhash": "0000a1b2c3d4e5f6...",
      "nonce": 123456789,
      "miner": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
      "timestamp": 1735646500,
      "difficulty": 0.1,
      "transactions": [...]
    }],
    "id": 1
  }'
```

---

## 👥 Validator Integration

### Register as Validator

**Method:** `registervalidator`
**Purpose:** Register pool address as validator for PoS mining
**Parameters:** `[address, stakeAmount, lockPeriodDays]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "registervalidator",
  "params": [
    "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
    10000.0,
    90
  ],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
    "stake_amount": 10000.0,
    "weighted_stake": 10000.0,
    "stake_tx": "xyz789...",
    "lock_period_days": 90
  },
  "id": 1
}
```

### Get Validators List

**Method:** `getvalidators`
**Purpose:** Get all active validators
**Parameters:** None

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getvalidators",
  "params": [],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "address": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
      "stake_amount": 10000.0,
      "weighted_stake": 10000.0,
      "is_active": true,
      "blocks_produced": 15,
      "total_rewards": 750.0
    }
  ],
  "id": 1
}
```

### Get Staking Info

**Method:** `getstakinginfo`
**Purpose:** Get staking information for address
**Parameters:** `[address]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getstakinginfo",
  "params": ["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n",
    "spendable_balance": 100.0,
    "staked_balance": 10000.0,
    "total_balance": 10100.0,
    "is_validator": true,
    "pending_rewards": 50.0
  },
  "id": 1
}
```

---

## 💰 Payment Processing

### Distribute Mining Rewards to Miners

```python
# Example: Pay miners their share
def distribute_payments(miners_shares, pool_address):
    for miner_address, amount in miners_shares.items():
        if amount < 0.001:  # Minimum payment threshold
            continue

        # Send payment
        response = rpc_call("sendtoaddress", [
            miner_address,      # Recipient
            amount,             # Amount in GXC
            pool_address        # From address (pool wallet)
        ])

        if "result" in response:
            tx_hash = response["result"]
            print(f"Paid {amount} GXC to {miner_address}: {tx_hash}")
        else:
            print(f"Payment failed: {response.get('error')}")
```

### Get Transaction Confirmation

**Method:** `gettransaction`
**Purpose:** Check transaction status
**Parameters:** `[txHash]`

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "gettransaction",
  "params": ["abc123..."],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "txid": "abc123...",
    "confirmations": 15,
    "blockhash": "00000def...",
    "blockheight": 152,
    "amount": 10.5,
    "fee": 0.001
  },
  "id": 1
}
```

---

## 📜 Proof of Receipt

### Work Receipt Hash

The coinbase transaction in each block contains a **work receipt hash** that links the block to the proof-of-work:

```json
{
  "coinbase_transaction": {
    "hash": "abc123...",
    "outputs": [{
      "address": "miner_address",
      "amount": 50.0,
      "script": "..."
    }],
    "work_receipt_hash": "sha256(previous_hash + nonce + timestamp)"
  }
}
```

### Verify Work Receipt

```python
import hashlib

def verify_work_receipt(block):
    """Verify that the work receipt hash is correct"""
    prev_hash = block["previousblockhash"]
    nonce = block["nonce"]
    timestamp = block["timestamp"]

    # Calculate expected receipt hash
    receipt_data = f"{prev_hash}{nonce}{timestamp}"
    expected_hash = hashlib.sha256(receipt_data.encode()).hexdigest()

    # Get actual receipt from coinbase
    coinbase = block["transactions"][0]
    actual_receipt = coinbase.get("work_receipt_hash", "")

    return expected_hash == actual_receipt
```

### Include Receipt in Coinbase

When submitting a block, ensure the coinbase transaction includes the work receipt:

```json
{
  "transactions": [
    {
      "type": "COINBASE",
      "outputs": [{
        "address": "miner_address",
        "amount": 50.0
      }],
      "metadata": {
        "work_receipt_hash": "calculated_hash",
        "pool_name": "GXC Mining Pool",
        "pool_version": "1.0.0"
      }
    }
  ]
}
```

---

## ⚠️ Error Handling

### Common RPC Errors

| Error Code | Message | Cause | Solution |
|------------|---------|-------|----------|
| `-32700` | Parse error | Invalid JSON | Check JSON syntax |
| `-32600` | Invalid Request | Missing jsonrpc/method | Add required fields |
| `-32601` | Method not found | Wrong method name | Check spelling |
| `-32602` | Invalid params | Wrong parameter types | Verify parameter format |
| `-32603` | Internal error | Node exception | Check node logs |
| `-8` | Invalid parameter | Parameter validation failed | Check parameter values |
| `-25` | Verify rejected | Block validation failed | Check block format/hash |
| `-6` | Insufficient funds | Not enough balance | Add funds to wallet |

### Error Response Format

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32700,
    "message": "Parse error: [json.exception.parse_error.101] parse error at line 1, column 1: syntax error while parsing value - unexpected end of input"
  },
  "id": null
}
```

### Handling Empty Responses

If you receive an empty response (causing "unexpected end of input"):

**Possible Causes:**
1. Node is not running
2. Wrong port (check 8332 vs 18332)
3. Network connection issue
4. Node crashed during request processing

**Debug Steps:**
```bash
# 1. Check if node is running
ps aux | grep gxc-node

# 2. Test with health endpoint
curl http://localhost:8332/health

# 3. Test with simple RPC call
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}'

# 4. Check node logs
tail -f ./gxc_data/debug.log
```

---

## 🧪 Testing

### Test Script

```bash
#!/bin/bash

NODE_URL="http://localhost:8332"
POOL_ADDRESS="tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"

echo "=== Testing GXC Node RPC ==="

# Test 1: Node info
echo "Test 1: getinfo"
curl -s -X POST $NODE_URL \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}' \
  | jq '.'

# Test 2: Get balance
echo -e "\nTest 2: getbalance"
curl -s -X POST $NODE_URL \
  -H "Content-Type: application/json" \
  -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$POOL_ADDRESS\"],\"id\":1}" \
  | jq '.'

# Test 3: Get block template
echo -e "\nTest 3: getblocktemplate"
curl -s -X POST $NODE_URL \
  -H "Content-Type: application/json" \
  -d "{\"jsonrpc\":\"2.0\",\"method\":\"getblocktemplate\",\"params\":[{\"minerAddress\":\"$POOL_ADDRESS\"}],\"id\":1}" \
  | jq '.result | {height, difficulty, coinbasevalue}'

# Test 4: Get validators
echo -e "\nTest 4: getvalidators"
curl -s -X POST $NODE_URL \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getvalidators","params":[],"id":1}' \
  | jq '.'

echo -e "\n=== Tests Complete ==="
```

### Expected Results

```
Test 1: getinfo
{
  "jsonrpc": "2.0",
  "result": {
    "blocks": 150,
    "difficulty": 0.1,
    "chain": "test"
  },
  "id": 1
}

Test 2: getbalance
{
  "jsonrpc": "2.0",
  "result": 250.5,
  "id": 1
}

Test 3: getblocktemplate
{
  "height": 151,
  "difficulty": 0.1,
  "coinbasevalue": 50.0
}

Test 4: getvalidators
{
  "jsonrpc": "2.0",
  "result": [],
  "id": 1
}
```

---

## 📊 Summary

### Required Endpoints for Mining Pool

| Endpoint | Purpose | Priority |
|----------|---------|----------|
| `getinfo` | Node status | ✅ Critical |
| `getbalance` | Pool balance | ✅ Critical |
| `getblocktemplate` | Get mining work | ✅ Critical |
| `submitblock` | Submit mined block | ✅ Critical |
| `listunspent` | Get UTXOs | ✅ Critical |
| `sendtoaddress` | Pay miners | ✅ Critical |
| `registervalidator` | PoS validator | ⚡ Optional |
| `getvalidators` | Validator list | ⚡ Optional |
| `getstakinginfo` | Staking status | ⚡ Optional |
| `gettransaction` | TX confirmation | 📝 Useful |

### JSON-RPC 2.0 Request Format

**All requests must include:**
- `jsonrpc`: "2.0"
- `method`: RPC method name
- `params`: Array of parameters
- `id`: Request ID (number or string)

**Example:**
```json
{
  "jsonrpc": "2.0",
  "method": "methodname",
  "params": ["param1", "param2"],
  "id": 1
}
```

---

## ✅ Checklist for Pool Integration

- [ ] Node is running and accessible on port 8332 (or 18332 for testnet)
- [ ] Health endpoint responds: `GET /health`
- [ ] `getinfo` returns valid blockchain data
- [ ] `getbalance` works with pool address
- [ ] `getblocktemplate` returns valid mining template
- [ ] `submitblock` accepts properly formatted blocks
- [ ] `sendtoaddress` successfully sends payments
- [ ] Error responses are properly formatted JSON
- [ ] All requests use `Content-Type: application/json`
- [ ] All requests follow JSON-RPC 2.0 format
- [ ] Pool handles empty/error responses gracefully

---

## 🔧 Troubleshooting

### Pool Getting Empty Responses

**Symptom:** `Parse error: unexpected end of input`

**Cause:** Node is not responding or returning empty body

**Fix:**
1. Verify node is running: `ps aux | grep gxc-node`
2. Check node is listening: `netstat -tlnp | grep 8332`
3. Test with curl: `curl http://localhost:8332/health`
4. Check node logs for errors

### Pool Cannot Connect

**Symptom:** Connection refused

**Fix:**
1. Check firewall allows port 8332
2. Verify node is bound to correct interface (0.0.0.0 vs 127.0.0.1)
3. Use correct URL (http://localhost:8332 vs http://127.0.0.1:8332)

### Invalid Block Submission

**Symptom:** Error -25 "Verify rejected"

**Fix:**
1. Ensure hash meets difficulty target
2. Verify previous hash is correct
3. Check block height matches template
4. Include all required fields in block data
5. Ensure coinbase transaction is first in transactions array

---

## 📞 Support

For issues or questions:
- Check node logs: `./gxc_data/debug.log`
- Review this guide
- Test with provided test script
- Verify JSON formatting with `jq` or online validator

**End of Mining Pool RPC Integration Guide**
