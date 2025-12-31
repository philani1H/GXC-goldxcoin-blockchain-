# GXC Traceability System - Complete Guide

## Overview

GXC's traceability system provides **complete transparency** for all transactions and addresses on the blockchain. Every coin can be traced back to its origin (coinbase transaction), and every address's complete history is available.

---

## Traceability Methods

### 1. traceaddress - Trace Address Activity

**Purpose:** Get complete history and current state of any address

**Endpoint:**
```bash
POST / {"method":"traceaddress","params":["address"]}
GET /api/traceaddress?address=ADDRESS  # Coming soon
```

**Example:**
```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "traceaddress",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "balance": 150.5,
    "total_received": 250.0,
    "total_sent": 99.5,
    "net_balance": 150.5,
    "pending_balance": 0.0,
    "total_transactions": 15,
    "incoming_transactions": [
      {
        "txid": "abc123...",
        "from": "tGXC9fab7317231b966af85ac453e168c0932",
        "amount": 50.0,
        "timestamp": 1767188089,
        "confirmations": 6
      }
    ],
    "outgoing_transactions": [
      {
        "txid": "def456...",
        "to": "tGXC1234567890abcdef1234567890abcdef",
        "amount": 25.0,
        "timestamp": 1767188100,
        "confirmations": 5
      }
    ],
    "unspent_outputs": [
      {
        "txid": "ghi789...",
        "vout": 0,
        "amount": 50.0,
        "confirmations": 10
      }
    ],
    "mempool_transactions": [],
    "transactions": [
      {
        "txid": "abc123...",
        "type": "received",
        "amount": 50.0,
        "timestamp": 1767188089
      },
      {
        "txid": "def456...",
        "type": "sent",
        "amount": -25.0,
        "timestamp": 1767188100
      }
    ]
  },
  "id": 1
}
```

**What You Get:**
- ✅ Current balance
- ✅ Total received (all time)
- ✅ Total sent (all time)
- ✅ Net balance
- ✅ All incoming transactions
- ✅ All outgoing transactions
- ✅ Unspent outputs (UTXOs)
- ✅ Pending mempool transactions
- ✅ Complete transaction history

---

### 2. tracetransaction - Trace Transaction Chain

**Purpose:** Trace a transaction back through its history

**Endpoint:**
```bash
POST / {"method":"tracetransaction","params":["txid"]}
```

**Example:**
```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "tracetransaction",
    "params": ["abc123def456..."],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    "abc123def456...",  // Current transaction
    "prev789abc012...",  // Previous transaction (input source)
    "orig456def789...",  // Original transaction
    "coinbase123..."     // Coinbase (origin)
  ],
  "id": 1
}
```

**What You Get:**
- ✅ Complete chain of transactions
- ✅ Traces back to coinbase (mining reward)
- ✅ Shows how coins moved through the network
- ✅ Proves coin origin

---

### 3. gettransaction - Get Transaction Details

**Purpose:** Get complete details of a specific transaction

**Endpoint:**
```bash
POST / {"method":"gettransaction","params":["txid"]}
```

**Example:**
```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gettransaction",
    "params": ["abc123def456..."],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "txid": "abc123def456...",
    "hash": "abc123def456...",
    "version": 1,
    "size": 250,
    "vsize": 250,
    "weight": 1000,
    "locktime": 0,
    "type": "NORMAL",
    "from": "tGXC9fab7317231b966af85ac453e168c0932",
    "to": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "amount": 50.0,
    "fee": 0.001,
    "timestamp": 1767188089,
    "confirmations": 6,
    "blockhash": "00000000abc123...",
    "blockheight": 1000,
    "blocktime": 1767188089,
    "inputs": [
      {
        "txid": "prev789abc012...",
        "vout": 0,
        "amount": 50.001,
        "address": "tGXC9fab7317231b966af85ac453e168c0932"
      }
    ],
    "outputs": [
      {
        "n": 0,
        "value": 50.0,
        "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
        "scriptPubKey": "OP_DUP OP_HASH160 ... OP_CHECKSIG"
      }
    ],
    "prevTxHash": "prev789abc012...",
    "referencedAmount": 50.001,
    "traceability_valid": true
  },
  "id": 1
}
```

**What You Get:**
- ✅ Complete transaction details
- ✅ Inputs and outputs
- ✅ Sender and receiver addresses
- ✅ Amount and fee
- ✅ Confirmations
- ✅ Block information
- ✅ Traceability validation status

---

### 4. listtransactions - List All Transactions

**Purpose:** List all transactions in the wallet

**Endpoint:**
```bash
POST / {"method":"listtransactions"}
GET /api/listtransactions  # Coming soon
```

**Example:**
```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listtransactions",
    "params": [],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "txid": "abc123...",
      "category": "receive",
      "amount": 50.0,
      "confirmations": 6,
      "time": 1767188089,
      "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
    },
    {
      "txid": "def456...",
      "category": "send",
      "amount": -25.0,
      "fee": -0.001,
      "confirmations": 5,
      "time": 1767188100,
      "address": "tGXC1234567890abcdef1234567890abcdef"
    }
  ],
  "id": 1
}
```

---

### 5. listunspent - List Unspent Outputs (UTXOs)

**Purpose:** List all unspent transaction outputs

**Endpoint:**
```bash
POST / {"method":"listunspent"}
GET /api/listunspent  # Coming soon
```

**Example:**
```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listunspent",
    "params": [],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "txid": "abc123...",
      "vout": 0,
      "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      "amount": 50.0,
      "confirmations": 10,
      "spendable": true,
      "solvable": true,
      "safe": true
    },
    {
      "txid": "def456...",
      "vout": 1,
      "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      "amount": 25.5,
      "confirmations": 5,
      "spendable": true,
      "solvable": true,
      "safe": true
    }
  ],
  "id": 1
}
```

---

### 6. validateaddress - Validate Address

**Purpose:** Validate address format and get address info

**Endpoint:**
```bash
POST / {"method":"validateaddress","params":["address"]}
GET /api/validateaddress?address=ADDRESS  # Coming soon
```

**Example:**
```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "validateaddress",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "isvalid": true,
    "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "ismine": false,
    "iswatchonly": false,
    "isscript": false
  },
  "id": 1
}
```

---

## Traceability Formula

Every transaction (except coinbase) must satisfy the traceability formula:

```
Ti.inputs[0].txHash == Ti.prevTxHash
Ti.inputs[0].amount == Ti.referencedAmount
```

**This ensures:**
1. ✅ Every coin can be traced to its origin
2. ✅ No coins can be created out of thin air
3. ✅ Complete transparency
4. ✅ Audit trail for regulators

---

## Use Cases

### 1. Audit Trail for Regulators

```bash
# Trace where coins came from
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "tracetransaction",
    "params": ["suspicious_tx_hash"],
    "id": 1
  }'

# Result: Complete chain back to coinbase
# Proves: Coins are legitimate mining rewards
```

### 2. Address Monitoring

```bash
# Monitor an address for activity
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "traceaddress",
    "params": ["address_to_monitor"],
    "id": 1
  }'

# Result: All transactions, balance, UTXOs
# Use case: Exchange deposit monitoring
```

### 3. Transaction Verification

```bash
# Verify a transaction is valid
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gettransaction",
    "params": ["tx_hash"],
    "id": 1
  }'

# Check: traceability_valid field
# Result: true = valid, false = invalid
```

### 4. Balance Verification

```bash
# Verify address balance
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "traceaddress",
    "params": ["address"],
    "id": 1
  }'

# Result: balance, total_received, total_sent
# Verify: balance = total_received - total_sent
```

---

## Complete Example: Transaction Chain

### Scenario
1. Miner mines block → receives 50 GXC (coinbase)
2. Miner sends 10 GXC to Alice
3. Alice sends 5 GXC to Bob
4. Bob sends 2 GXC to Charlie

### Trace the Chain

```bash
# 1. Get Charlie's transaction
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gettransaction",
    "params": ["charlie_tx_hash"],
    "id": 1
  }'

# Result:
# - From: Bob
# - To: Charlie
# - Amount: 2 GXC
# - prevTxHash: bob_tx_hash

# 2. Trace back to origin
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "tracetransaction",
    "params": ["charlie_tx_hash"],
    "id": 1
  }'

# Result:
# [
#   "charlie_tx_hash",  // Bob → Charlie (2 GXC)
#   "bob_tx_hash",      // Alice → Bob (5 GXC)
#   "alice_tx_hash",    // Miner → Alice (10 GXC)
#   "coinbase_hash"     // Mining reward (50 GXC)
# ]

# 3. Get complete address history
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "traceaddress",
    "params": ["charlie_address"],
    "id": 1
  }'

# Result:
# - balance: 2.0 GXC
# - total_received: 2.0 GXC
# - total_sent: 0.0 GXC
# - incoming_transactions: [bob_tx]
# - outgoing_transactions: []
```

---

## Integration Examples

### Python

```python
import requests

def trace_address(address):
    """Trace complete address history"""
    response = requests.post(
        "http://localhost:18332/",
        json={
            "jsonrpc": "2.0",
            "method": "traceaddress",
            "params": [address],
            "id": 1
        }
    )
    return response.json()['result']

def trace_transaction(txid):
    """Trace transaction back to origin"""
    response = requests.post(
        "http://localhost:18332/",
        json={
            "jsonrpc": "2.0",
            "method": "tracetransaction",
            "params": [txid],
            "id": 1
        }
    )
    return response.json()['result']

# Usage
address_history = trace_address("tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9")
print(f"Balance: {address_history['balance']} GXC")
print(f"Total transactions: {address_history['total_transactions']}")

tx_chain = trace_transaction("abc123...")
print(f"Transaction chain: {' → '.join(tx_chain)}")
```

### Rust

```rust
use reqwest;
use serde_json::json;

async fn trace_address(address: &str) -> Result<serde_json::Value, Box<dyn std::error::Error>> {
    let client = reqwest::Client::new();
    let response = client
        .post("http://localhost:18332/")
        .json(&json!({
            "jsonrpc": "2.0",
            "method": "traceaddress",
            "params": [address],
            "id": 1
        }))
        .send()
        .await?;
    
    let data: serde_json::Value = response.json().await?;
    Ok(data["result"].clone())
}

async fn trace_transaction(txid: &str) -> Result<Vec<String>, Box<dyn std::error::Error>> {
    let client = reqwest::Client::new();
    let response = client
        .post("http://localhost:18332/")
        .json(&json!({
            "jsonrpc": "2.0",
            "method": "tracetransaction",
            "params": [txid],
            "id": 1
        }))
        .send()
        .await?;
    
    let data: serde_json::Value = response.json().await?;
    let chain: Vec<String> = serde_json::from_value(data["result"].clone())?;
    Ok(chain)
}
```

### JavaScript

```javascript
const axios = require('axios');

async function traceAddress(address) {
    const response = await axios.post('http://localhost:18332/', {
        jsonrpc: '2.0',
        method: 'traceaddress',
        params: [address],
        id: 1
    });
    return response.data.result;
}

async function traceTransaction(txid) {
    const response = await axios.post('http://localhost:18332/', {
        jsonrpc: '2.0',
        method: 'tracetransaction',
        params: [txid],
        id: 1
    });
    return response.data.result;
}

// Usage
const history = await traceAddress('tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9');
console.log(`Balance: ${history.balance} GXC`);
console.log(`Transactions: ${history.total_transactions}`);

const chain = await traceTransaction('abc123...');
console.log(`Chain: ${chain.join(' → ')}`);
```

---

## Summary

### Available Traceability Methods

| Method | Purpose | GET Support | POST Support |
|--------|---------|-------------|--------------|
| `traceaddress` | Complete address history | ❌ | ✅ |
| `tracetransaction` | Trace transaction chain | ❌ | ✅ |
| `gettransaction` | Transaction details | ❌ | ✅ |
| `listtransactions` | List all transactions | ❌ | ✅ |
| `listunspent` | List UTXOs | ❌ | ✅ |
| `validateaddress` | Validate address | ❌ | ✅ |

### Key Features

✅ **Complete Transparency** - Every transaction traceable
✅ **Audit Trail** - Regulators can verify coin origin
✅ **Address Monitoring** - Track any address activity
✅ **UTXO Tracking** - See all unspent outputs
✅ **Balance Verification** - Cryptographic proof of balance
✅ **Transaction Validation** - Verify traceability formula

### Use Cases

- 🏦 **Exchanges** - Monitor deposits and withdrawals
- 👮 **Regulators** - Audit coin origins
- 🔍 **Explorers** - Display transaction history
- 💼 **Wallets** - Show complete transaction history
- ⛏️ **Mining Pools** - Track reward distribution

---

**All traceability methods tested and working!**

**Railway URL:** https://gxc-chain112-blockchain-node-production.up.railway.app
