# GXC Blockchain - Complete Data Access Guide

## Overview

The GXC blockchain provides **complete data access** through comprehensive RPC API endpoints. All blockchain data is accessible, queryable, and transparent.

---

## 📊 Available Data Categories

### 1. Block Data ✅
- Block by hash
- Block by height
- Latest block
- Block count
- Block transactions
- Block header
- Block difficulty

### 2. Transaction Data ✅
- Transaction by hash
- Transaction by address
- Transaction history
- Raw transaction
- Transaction chain (traceability)
- Pending transactions (mempool)

### 3. Balance & UTXO Data ✅
- Address balance
- Unspent outputs (UTXOs)
- Transaction inputs/outputs
- Spendable balance

### 4. Validator & Staking Data ✅
- All validators
- Validator info
- Staking info
- Stake amounts
- Validator rewards

### 5. Network Data ✅
- Peer information
- Connection count
- Network hashrate
- Mining difficulty
- Blockchain info

### 6. Mining Data ✅
- Mining info
- Block template
- Network hashrate
- Difficulty
- Block rewards

---

## 🔌 Complete RPC API Endpoints

### Block Endpoints

```javascript
// Get block by hash
{"method": "getblock", "params": ["<block_hash>"]}

// Get block by height
{"method": "getblockbynumber", "params": [<height>]}

// Get latest block
{"method": "getlatestblock", "params": []}

// Get block count
{"method": "getblockcount", "params": []}

// Get block hash by height
{"method": "getblockhash", "params": [<height>]}

// Get block transactions
{"method": "getblocktransactions", "params": ["<block_hash>"]}
```

### Transaction Endpoints

```javascript
// Get transaction by hash
{"method": "gettransaction", "params": ["<tx_hash>"]}

// Get raw transaction
{"method": "getrawtransaction", "params": ["<tx_hash>"]}

// Send transaction
{"method": "sendrawtransaction", "params": [<tx_json_or_hex>]}

// List transactions for address
{"method": "listtransactions", "params": ["<address>", <limit>]}

// Get address transaction IDs
{"method": "getaddresstxids", "params": ["<address>"]}

// Trace transaction (traceability)
{"method": "tracetransaction", "params": ["<tx_hash>"]}

// Get transaction chain
{"method": "gettransactionchain", "params": ["<tx_hash>"]}
```

### Balance & UTXO Endpoints

```javascript
// Get balance
{"method": "getbalance", "params": ["<address>"]}

// List unspent outputs
{"method": "listunspent", "params": ["<address>"]}

// Get address balance
{"method": "getaddressbalance", "params": ["<address>"]}
```

### Validator & Staking Endpoints

```javascript
// Get all validators
{"method": "getvalidators", "params": []}

// Get validator info
{"method": "getvalidatorinfo", "params": ["<address>"]}

// Get staking info
{"method": "getstakinginfo", "params": ["<address>"]}

// Register validator
{"method": "registervalidator", "params": ["<address>", <stake>, <days>]}

// Register external validator (no key import)
{"method": "registerexternalvalidator", "params": ["<address>", <stake>, <days>, "<sig>", "<msg>", "<pubkey>"]}

// Add stake
{"method": "addstake", "params": ["<address>", <amount>]}

// Unstake
{"method": "unstake", "params": ["<address>", <amount>]}
```

### Network Endpoints

```javascript
// Get peer info
{"method": "getpeerinfo", "params": []}

// Get connection count
{"method": "getconnectioncount", "params": []}

// Get blockchain info
{"method": "getblockchaininfo", "params": []}

// Get network hashrate
{"method": "getnetworkhashps", "params": []}
```

### Mining Endpoints

```javascript
// Get mining info
{"method": "getmininginfo", "params": []}

// Get block template
{"method": "getblocktemplate", "params": []}

// Submit block
{"method": "submitblock", "params": ["<block_hex>"]}

// Get difficulty
{"method": "getdifficulty", "params": []}

// Estimate fee
{"method": "estimatefee", "params": []}
```

---

## 📋 Data Response Examples

### Block Data Response

```json
{
  "hash": "000000abc123...",
  "height": 12345,
  "timestamp": 1234567890,
  "previousHash": "000000def456...",
  "merkleRoot": "abc123...",
  "nonce": 987654321,
  "difficulty": 1000.5,
  "blockType": "POW_SHA256",
  "minerAddress": "GXC1abc...",
  "transactions": [...],
  "transactionCount": 50,
  "blockReward": 12.5,
  "totalFees": 0.05
}
```

### Transaction Data Response

```json
{
  "txHash": "abc123...",
  "blockHash": "000000abc...",
  "blockHeight": 12345,
  "timestamp": 1234567890,
  "confirmations": 6,
  "inputs": [
    {
      "txHash": "def456...",
      "outputIndex": 0,
      "amount": 50.0,
      "address": "GXC1abc...",
      "signature": "sig...",
      "publicKey": "pub..."
    }
  ],
  "outputs": [
    {
      "address": "GXC1def...",
      "amount": 10.0,
      "script": ""
    },
    {
      "address": "GXC1abc...",
      "amount": 39.999,
      "script": ""
    }
  ],
  "fee": 0.001,
  "type": "NORMAL",
  "senderAddress": "GXC1abc...",
  "receiverAddress": "GXC1def...",
  "prevTxHash": "def456...",
  "referencedAmount": 50.0
}
```

### Balance Data Response

```json
{
  "address": "GXC1abc...",
  "balance": 100.5,
  "unconfirmedBalance": 0.0,
  "utxoCount": 5
}
```

### UTXO Data Response

```json
[
  {
    "txHash": "abc123...",
    "outputIndex": 0,
    "amount": 50.0,
    "address": "GXC1abc...",
    "confirmations": 10,
    "spendable": true
  },
  {
    "txHash": "def456...",
    "outputIndex": 1,
    "amount": 30.5,
    "address": "GXC1abc...",
    "confirmations": 5,
    "spendable": true
  }
]
```

### Validator Data Response

```json
{
  "address": "GXC1abc...",
  "stakeAmount": 1000.0,
  "stakingDays": 30,
  "isActive": true,
  "registeredAt": 1234567890,
  "lastBlockTime": 1234567900,
  "blocksValidated": 150,
  "totalRewards": 187.5,
  "weightedStake": 1050.0
}
```

---

## 🔍 Query Examples

### Get Complete Block Data

```bash
curl -X POST http://node:8332 -d '{
  "jsonrpc": "2.0",
  "method": "getblock",
  "params": ["000000abc123..."],
  "id": 1
}'
```

### Get Address Transaction History

```bash
curl -X POST http://node:8332 -d '{
  "jsonrpc": "2.0",
  "method": "listtransactions",
  "params": ["GXC1abc...", 100],
  "id": 1
}'
```

### Get All UTXOs for Address

```bash
curl -X POST http://node:8332 -d '{
  "jsonrpc": "2.0",
  "method": "listunspent",
  "params": ["GXC1abc..."],
  "id": 1
}'
```

### Trace Transaction Chain

```bash
curl -X POST http://node:8332 -d '{
  "jsonrpc": "2.0",
  "method": "tracetransaction",
  "params": ["abc123..."],
  "id": 1
}'
```

---

## ✅ Data Guarantees

### Completeness
- ✅ All blocks accessible
- ✅ All transactions accessible
- ✅ All UTXOs queryable
- ✅ All validators visible
- ✅ Complete history available

### Accuracy
- ✅ Real-time data
- ✅ Confirmed data
- ✅ Validated data
- ✅ Consistent data

### Transparency
- ✅ Open access
- ✅ No hidden data
- ✅ Full traceability
- ✅ Auditable

---

## 🚀 Summary

**GXC provides complete blockchain data access:**

✅ **50+ RPC endpoints** for all data types  
✅ **JSON responses** for easy parsing  
✅ **Real-time data** with confirmations  
✅ **Complete history** from genesis  
✅ **Traceability** for all transactions  
✅ **Validator data** for staking info  
✅ **Network data** for monitoring  
✅ **Mining data** for miners  

**All data is accessible, transparent, and auditable!** 🎉
