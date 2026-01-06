# Work Receipt System - Complete Documentation

## Overview

The Work Receipt system is a cryptographic proof mechanism that ensures mining rewards are traceable to actual proof-of-work. It prevents reward theft and enables complete transaction traceability.

## How Work Receipts Work

### 1. Work Receipt Calculation

A work receipt is computed as:

```
work_receipt = SHA256(prev_hash + merkle_root + nonce + miner_pubkey + difficulty + timestamp)
```

**Components:**
- `prev_hash`: Hash of the previous block (proves chain continuity)
- `merkle_root`: Root of transaction merkle tree (proves transaction inclusion)
- `nonce`: The nonce that solved the mining puzzle (proves work was done)
- `miner_pubkey`: Miner's public key (proves identity)
- `difficulty`: Mining difficulty (proves work difficulty)
- `timestamp`: Block timestamp (proves when work was done)

### 2. Work Receipt Storage

Work receipts are stored in two places:

1. **Block Level**: `block.workReceiptHash`
   - Proves the entire block was mined correctly
   - Used for block validation

2. **Coinbase Transaction**: `coinbase.workReceiptHash`
   - Links the mining reward to the work
   - Ensures reward traceability

### 3. Work Receipt Validation

When a block is submitted, the node:

1. Recalculates the work receipt from block data
2. Compares with the submitted work receipt
3. Verifies coinbase transaction has matching work receipt
4. Rejects block if any mismatch

## Genesis Block

### Automatic Creation

The genesis block is automatically created when the node starts with an empty blockchain:

```cpp
if (chain.empty()) {
    LOG_BLOCKCHAIN(LogLevel::INFO, "Blockchain is empty. Creating genesis block...");
    createGenesisBlock();
}
```

### Genesis Block Details

**Testnet:**
- Address: `tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf`
- Reward: 50 GXC
- Difficulty: 1.0 (2 leading zeros)
- Work Receipt: Automatically generated

**Mainnet:**
- Address: `GXC9fab7317231b966af85ac453e168c0932` (to be updated before launch)
- Reward: 50 GXC
- Difficulty: 1000.0 (4 leading zeros)
- Work Receipt: Automatically generated

### Genesis UTXO

The genesis block creates a UTXO:

```
UTXO Key: {genesis_tx_hash}_0
Address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
Amount: 50 GXC
```

## Mining Process

### 1. Get Block Template

```python
template = rpc_call("getblocktemplate", [miner_address])
```

Returns:
- `height`: Next block height
- `previousblockhash`: Hash to extend
- `difficulty`: Current difficulty
- `block_reward`: Reward amount
- `transactions`: Transactions to include (including coinbase)

### 2. Mine the Block

```python
for nonce in range(max_attempts):
    block_data = f"{height}{prev_hash}{miner_address}{timestamp}{nonce}"
    hash_result = hashlib.sha256(block_data.encode()).hexdigest()
    
    if hash_result.startswith('0' * int(difficulty)):
        # Block mined!
        break
```

### 3. Submit Block

```python
block = {
    "height": height,
    "hash": hash_result,
    "previousblockhash": prev_hash,
    "nonce": nonce,
    "timestamp": timestamp,
    "difficulty": difficulty,
    "miner": miner_address,
    "transactions": template['transactions']
}

result = rpc_call("submitblock", [block])
```

### 4. Node Processing

The node:
1. Validates proof-of-work
2. Adds transactions to block
3. Calculates merkle root
4. **Computes work receipt**
5. **Sets work receipt on block and coinbase**
6. Validates block
7. Adds to blockchain
8. Updates UTXO set

## Coinbase Maturity

Coinbase rewards require confirmations before they're spendable:

- **Testnet**: 6 confirmations
- **Mainnet**: 100 confirmations

This prevents spending rewards from orphaned blocks.

## Current Implementation Status

### ✅ Working

1. **Genesis Block Creation**
   - Automatically created on startup
   - Correct address assignment
   - Work receipt generated
   - UTXO created

2. **Block Template Generation**
   - Includes coinbase transaction
   - Correct reward calculation
   - Transaction inclusion

3. **Work Receipt Computation**
   - Calculated in `submitBlock`
   - Set on block level

### ⚠️ Known Issues

1. **Coinbase Work Receipt**
   - Work receipt is computed but not properly set on coinbase transaction
   - Transactions are const references, can't be modified after adding to block
   - **Fix needed**: Modify Block class to allow updating transaction work receipts

2. **UTXO Balance Lookup**
   - UTXO is created correctly
   - Balance lookup may have address matching issues
   - **Fix needed**: Debug address comparison in getBalance()

## Required Fixes

### 1. Fix Coinbase Work Receipt

**Problem**: Coinbase transaction doesn't have work receipt set

**Solution**: Add method to Block class:

```cpp
// In Block.h
void updateTransactionWorkReceipts(const std::string& workReceipt);

// In block.cpp
void Block::updateTransactionWorkReceipts(const std::string& workReceipt) {
    for (auto& tx : transactions) {
        if (tx.isCoinbaseTransaction()) {
            tx.setWorkReceiptHash(workReceipt);
        }
    }
}
```

Then in `submitBlock`:

```cpp
// Compute work receipt
std::string workReceipt = newBlock.computeWorkReceipt();
newBlock.setWorkReceiptHash(workReceipt);

// Update coinbase transaction
newBlock.updateTransactionWorkReceipts(workReceipt);
```

### 2. Fix Balance Lookup

**Problem**: getBalance() returns 0 even though UTXO exists

**Possible causes**:
- Address string comparison issue
- UTXO key format mismatch
- Coinbase maturity check too strict

**Debug steps**:
1. Add logging to show all UTXOs in set
2. Log address comparison results
3. Verify UTXO key format matches

## Testing

### Test Script

Use `test_work_receipt.py` to test:

```bash
python3 test_work_receipt.py
```

This will:
1. Examine genesis block
2. Get block template
3. Mine a block
4. Submit block
5. Examine mined block
6. Check balance

### Expected Output

```
Genesis Block (Height 0):
  Hash: [64-char hex]
  Miner: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
  Reward: 50.0 GXC
  Work Receipt: [64-char hex]

Block Mined:
  Height: 1
  Nonce: [number]
  Hash: [64-char hex with leading zeros]
  
Block Accepted: YES
Balance: 0 GXC (needs 6 confirmations)
```

## Third-Party Pool Integration

### Pool Setup

```python
import requests

RPC_URL = "http://node-ip:8332"
POOL_ADDRESS = "your_pool_address"

def get_work():
    """Get mining work from node"""
    response = requests.post(RPC_URL, json={
        "jsonrpc": "2.0",
        "method": "getblocktemplate",
        "params": [POOL_ADDRESS],
        "id": 1
    })
    return response.json()['result']

def submit_work(block_data):
    """Submit mined block"""
    response = requests.post(RPC_URL, json={
        "jsonrpc": "2.0",
        "method": "submitblock",
        "params": [block_data],
        "id": 1
    })
    return response.json()
```

### Mining Flow

1. Pool gets block template from node
2. Pool distributes work to miners
3. Miner finds valid nonce
4. Pool submits block to node
5. Node validates and adds block
6. **Work receipt proves pool did the work**
7. Reward goes to pool address
8. Pool distributes to miners based on shares

## Security

### Work Receipt Benefits

1. **Prevents Reward Theft**
   - Can't claim someone else's mining reward
   - Work receipt must match block data

2. **Enables Traceability**
   - Every reward traceable to specific work
   - Audit trail for all mining rewards

3. **Prevents Double-Spending**
   - Coinbase must have valid work receipt
   - Can't create fake coinbase transactions

### Attack Prevention

1. **Fake Block Attack**
   - Attacker submits block without doing work
   - Work receipt validation fails
   - Block rejected

2. **Reward Hijacking**
   - Attacker tries to change coinbase address
   - Work receipt mismatch
   - Block rejected

3. **Replay Attack**
   - Attacker resubmits old block
   - Previous hash doesn't match
   - Block rejected

## Files Modified

1. `src/Blockchain.cpp`
   - Genesis block creation
   - UTXO format fix
   - Coinbase maturity adjustment

2. `src/RPCAPI.cpp`
   - Work receipt computation in submitBlock
   - Coinbase work receipt handling (needs fix)

3. `config/railway_config.py`
   - Default miner address updated

4. `mine_testnet_complete.py`
   - Default miner address updated

## Next Steps

1. Fix coinbase work receipt setting
2. Debug balance lookup issue
3. Test with 6+ blocks to verify maturity
4. Test with third-party pool
5. Document pool integration guide

---

**Status**: Genesis working, mining working, work receipt computed, coinbase update needed
**Date**: 2026-01-06
**Network**: Testnet
