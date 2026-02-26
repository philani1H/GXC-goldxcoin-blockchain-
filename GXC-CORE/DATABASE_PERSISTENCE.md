# GoldXCoin Database Persistence - Complete Reference

## Overview

**EVERYTHING is saved to the database. NOTHING is lost on restart or rebuild.**

This document describes how GoldXCoin ensures complete data persistence across restarts, rebuilds, and deployments.

## Critical Data Structures - All Persisted

### 1. Blockchain Data (Per Block)

**Saved automatically when block is added:**

```cpp
- Block metadata: index, hash, prev_hash, merkle_root, timestamp, difficulty, nonce
- Block type: POW_SHA256, POW_ETHASH, POS
- Miner address and block reward
- Validator signature (for PoS blocks)
- All transactions in the block (full details)
- UTXO set updates (spent outputs deleted, new outputs added)
- Traceability records (transaction lineage)
- Address index (for balance lookups)
```

**Database Keys:**
- `blk:<block_hash>` → Block data (JSON)
- `blkh:<height>` → Block hash (for height lookups)
- `cfg:latest_block_height` → Latest height
- `cfg:latest_block_hash` → Latest hash

### 2. Transaction Data (Per Transaction)

**Saved for every transaction:**

```cpp
- Transaction hash
- Sender and receiver addresses
- Fee, timestamp, nonce
- Transaction type: NORMAL, STAKE, UNSTAKE, REWARD, COINBASE, REVERSAL
- Inputs: tx_hash, output_index, amount, signature, public_key
- Outputs: address, amount, script
- Traceability: prev_tx_hash, referenced_amount
- Gold-backed fields: pop_reference, is_gold_backed
- Metadata: memo, lock_time, work_receipt_hash, block_height
```

**Database Keys:**
- `tx:<tx_hash>` → Transaction data (JSON)
- `txb:<tx_hash>` → Transaction-to-block mapping
- `trace:<tx_hash>` → Traceability record

### 3. **NEW: Mempool Persistence** ✅

**Problem Solved:** Pending transactions were LOST on restart (in-memory only).

**Now Saved:**

```cpp
- ALL pending transactions are saved to database when added to mempool
- Automatically loaded on node startup
- Invalid transactions removed during reload
- Expired transactions (>1 hour) deleted from database
- Confirmed transactions deleted from database when block is mined
```

**Database Keys:**
- `pending:<tx_hash>` → Pending transaction (full JSON)

**Functions:**
- `Database::savePendingTransaction(tx)` - Called when tx added to mempool
- `Database::getAllPendingTransactions()` - Called on startup
- `Database::deletePendingTransaction(tx_hash)` - Called when confirmed or expired

**User Impact:**
- ✅ Submit transaction → restart node → transaction still in mempool
- ✅ Deploy to Railway → pending transactions survive deployment
- ✅ No need to resubmit transactions after restart

### 4. UTXO Set

**Saved atomically with each block:**

```cpp
- Every unspent output tracked
- Key: "utxo:<tx_hash>:<output_index>"
- Value: {address, amount, script, block_height}
- Address index: "addr:<address>:<tx_hash>:<output_index>"
```

**Ensures:**
- Balance queries work instantly after restart
- No UTXO set rebuild needed
- Double-spend protection persists

### 5. Validators

**Saved per validator:**

```cpp
- Address, public_key
- Stake amount, staking days
- Stake start time, is_active
- Blocks produced, missed blocks
- Total rewards, pending rewards
- Slashing info: is_slashed, slashed_amount
```

**Database Keys:**
- `val:<validator_address>` → Validator data (JSON)

**Ensures:**
- Validator registrations survive restart
- Stake amounts preserved
- Slashing records permanent

### 6. **NEW: Gold Reserves Persistence** ✅

**Problem Solved:** Gold-backed token reserves were in-memory only.

**Now Saved:**

```cpp
- Address → gold reserve amount mapping
- Timestamp of last update
```

**Database Keys:**
- `gold:<address>` → {address, amount, timestamp}

**Functions:**
- `Database::saveGoldReserve(address, amount)`
- `Database::getGoldReserve(address, amount)`
- `Database::getAllGoldReserves()`

### 7. **NEW: Reversal Tracking Persistence** ✅

**Problem Solved:** Reversal records were in-memory, allowing double-reversals after restart.

**Now Saved:**

```cpp
- Original transaction hash → reversal transaction hash
- Timestamp of reversal
```

**Database Keys:**
- `reversal:<original_tx_hash>` → {original_tx, reversal_tx, timestamp}

**Functions:**
- `Database::saveReversalRecord(originalTxHash, reversalTxHash)`
- `Database::isTransactionReversed(txHash)`
- `Database::getReversalTxHash(originalTxHash)`

**Ensures:**
- Fraudulent transactions can only be reversed ONCE
- Reversal history permanent
- No double-reversals across restarts

### 8. **NEW: Staking Pool Persistence** ✅

**Saved:**

```cpp
- Complete staking pool state (JSON serialization)
- All stakes with stake IDs
- Accumulated rewards
```

**Database Keys:**
- `staking:state` → Complete staking pool state (JSON)

**Functions:**
- `Database::saveStakingPoolState(stateJson)`
- `Database::loadStakingPoolState(stateJson)`

### 9. Governance (Proposals & Votes)

**Saved per proposal:**

```cpp
- Proposal ID, title, description
- Proposer address
- Block height created
```

**Saved per vote:**

```cpp
- Proposal ID
- Voter address
- Vote type (YES, NO, ABSTAIN)
```

**Database implementation:** Already exists in Database.cpp

### 10. Network Peers

**Saved per peer:**

```cpp
- IP address, port
- Last seen timestamp
```

**Ensures:**
- Peer connections resume after restart
- Faster network bootstrap

### 11. Traceability Index

**Saved per non-coinbase transaction:**

```cpp
- Transaction hash
- Previous transaction hash
- Referenced amount
- From/to addresses
- Block height
- Timestamp
```

**Database Keys:**
- `trace:<tx_hash>` → Traceability entry

**Ensures:**
- Transaction lineage survives restart
- Fraud detection works across restarts
- Complete audit trail preserved

## Bulletproof Database Protection

### Protection Layer 1: Ephemeral Storage Detection

```cpp
Detects: /tmp, /var/tmp, ./gxc_data, gxc_data paths
Warning: "⚠️  WARNING: Database path appears to be on EPHEMERAL storage!"
Action: Logs loud warnings, provides Railway setup instructions
```

### Protection Layer 2: Block Count Check

```cpp
if (blockCount > 0 && loadBlocksFromDatabase() failed) {
    ERROR: "Database contains X blocks but loading failed!"
    Action: REFUSE TO START - prevents silent data loss
}
```

### Protection Layer 3: Database Lock File

```cpp
File: <data_dir>/blockchain.lock
Created: When first block (genesis) is saved
Content: Creation timestamp, network, genesis hash, warnings
Check: If lock file exists but no blocks loaded → REFUSE TO START
```

**This is the BULLETPROOF protection:**
- Even if database is empty (corrupted/deleted)
- But lock file exists (database was used before)
- Node REFUSES to start fresh
- Prevents accidental data loss on Railway volume unmount

### Protection Layer 4: Pending Transaction Validation

```cpp
On startup:
- Load all pending transactions from database
- Re-validate each transaction
- If UTXOs spent (transaction no longer valid) → delete from DB
- Only valid transactions added to mempool
```

Prevents:
- Invalid/stale transactions in mempool
- Double-spend via mempool persistence

## Data Flow on Startup

```
1. Database::initialize()
   ├─ Open LevelDB at configured data_dir
   ├─ Check network isolation (testnet vs mainnet)
   └─ Load all key prefixes

2. Blockchain::initialize()
   ├─ Check for ephemeral storage → warn
   ├─ Load blocks: loadBlocksFromDatabase()
   │  ├─ Iterate blkh:* keys (block heights)
   │  ├─ For each height: load block (blk:<hash>)
   │  ├─ For each block: load transactions (tx:*)
   │  ├─ Reconstruct in-memory chain
   │  └─ Recalculate chainwork
   ├─ If load fails && blockCount > 0 → ERROR & EXIT
   ├─ If lock file exists && no blocks → ERROR & EXIT
   ├─ If chain empty: create genesis + create lock file
   ├─ Else: rebuild UTXO set from loaded blocks
   ├─ Load validators from database
   ├─ Load pending transactions from database ← NEW!
   │  ├─ Re-validate each transaction
   │  └─ Add valid ones to mempool
   ├─ Initialize traceability index
   └─ Validate chain integrity

3. Ready to accept transactions/blocks
```

## Data Flow on Block Addition

```
Blockchain::addBlock(block)
├─ Validate block (POW/POS, transactions, rewards)
├─ Add block to in-memory chain
├─ Update UTXO set (in-memory)
├─ Update traceability index (in-memory)
├─ Save to database: Database::saveBlock(block)
│  ├─ START WriteBatch (atomic)
│  ├─ Save block: blk:<hash>
│  ├─ Save height index: blkh:<height>
│  ├─ Update config: latest_block_height, latest_block_hash
│  ├─ For each transaction:
│  │  ├─ Save tx: tx:<tx_hash>
│  │  ├─ Save tx-to-block mapping: txb:<tx_hash>
│  │  ├─ Delete spent UTXOs: utxo:<hash>:<index>
│  │  ├─ Add new UTXOs: utxo:<hash>:<index>
│  │  ├─ Add address index: addr:<address>:<hash>:<index>
│  │  └─ Save traceability: trace:<tx_hash>
│  └─ COMMIT WriteBatch (all or nothing)
├─ Update transaction pool: updateTransactionPool(block)
│  └─ For each confirmed transaction:
│     ├─ Remove from in-memory pendingTransactions
│     └─ Delete from database: pending:<tx_hash> ← NEW!
└─ Success
```

## Data Flow on Transaction Submission

```
Blockchain::addTransaction(tx)
├─ Validate transaction
├─ Check if already in pool
├─ Add to in-memory pendingTransactions
├─ Save to database: Database::savePendingTransaction(tx) ← NEW!
│  └─ Save: pending:<tx_hash>
└─ Success
```

## Data Flow on Mempool Cleanup

```
Blockchain::processTransactions()
├─ Lock transaction mutex
├─ Get current time
├─ For each pending transaction:
│  ├─ If timestamp > 1 hour old:
│  │  ├─ Delete from database: pending:<tx_hash> ← NEW!
│  │  └─ Remove from in-memory pendingTransactions
│  └─ Else: keep in pool
└─ Done
```

## Verification Checklist

After restart, verify:

```bash
# 1. Check database path (must be persistent!)
$ echo $GXC_DATA_DIR
/data/blockchain  # ✅ Good (Railway volume)
./gxc_data        # ❌ Bad (ephemeral)

# 2. Check lock file exists
$ ls -la /data/blockchain/blockchain.lock
-rw-r--r-- 1 user user 245 Feb 26 12:34 blockchain.lock  # ✅ Good

# 3. Check logs for successful load
✅ Successfully loaded 1234 blocks from persistent database
✅ Restored 5 pending transactions from database (mempool persisted)

# 4. Check block count matches
Before restart: 1234 blocks
After restart:  1234 blocks  # ✅ All blocks persisted

# 5. Check pending transactions
Before restart: 5 pending
After restart:  5 pending (or less if some expired)  # ✅ Mempool persisted

# 6. Check balance
Before restart: 100.5 GXC
After restart:  100.5 GXC  # ✅ UTXO set persisted
```

## Common Issues

### Issue: "ALL BLOCKCHAIN DATA WILL BE LOST ON RESTART!"

**Cause:** Database on ephemeral storage (./gxc_data, /tmp, /var/tmp)

**Fix:**
1. Add Railway persistent volume at /data
2. Set `GXC_DATA_DIR=/data/blockchain`
3. Restart node
4. Fresh genesis will be created ON PERSISTENT STORAGE
5. From now on, all data persists

### Issue: "Database contains 1234 blocks but loading failed!"

**Cause:** Database corruption or incomplete write

**Fix:**
```bash
# Option 1: Restore from backup
cp -r /backups/blockchain-20260226/* /data/blockchain/

# Option 2: Force fresh start (LOSES ALL DATA)
rm /data/blockchain/blockchain.lock
# Then restart node
```

### Issue: Pending transactions lost after restart

**Cause:** Old version without mempool persistence

**Fix:** Upgrade to this version (commit 20c0327 or later)

### Issue: Balance shows zero after restart

**Cause:**
1. Database not on persistent volume (wiped on restart)
2. Database lock file missing → fresh genesis created

**Fix:** See Railway deployment guide

## Database Schema Reference

### All Key Prefixes (17 total + 4 new)

| Prefix | Description | Example Key | Example Value |
|--------|-------------|-------------|---------------|
| `blk:` | Block by hash | `blk:00000abc...` | Block JSON |
| `blkh:` | Block hash by height | `blkh:1234` | `00000abc...` |
| `tx:` | Transaction by hash | `tx:abcd1234...` | Transaction JSON |
| `txb:` | Tx-to-block mapping | `txb:abcd1234...` | `{block_hash, block_height}` |
| `utxo:` | Unspent output | `utxo:abc:0` | `{address, amount, script, height}` |
| `addr:` | Address index | `addr:GXC123...:abc:0` | UTXO JSON |
| `trace:` | Traceability | `trace:abc123...` | Lineage JSON |
| `val:` | Validator | `val:GXC123...` | Validator JSON |
| `peer:` | Network peer | `peer:1.2.3.4:8333` | Last seen timestamp |
| `cfg:` | Configuration | `cfg:latest_block_height` | `1234` |
| **`pending:`** ✅ | **Mempool tx** | `pending:abcd...` | Transaction JSON |
| **`gold:`** ✅ | **Gold reserve** | `gold:GXC123...` | `{address, amount, timestamp}` |
| **`reversal:`** ✅ | **Reversal record** | `reversal:abc123...` | `{original_tx, reversal_tx, timestamp}` |
| **`staking:`** ✅ | **Staking pool** | `staking:state` | Staking pool JSON |

## Performance Notes

- **LevelDB** uses Bloom filters for fast key lookups
- **WriteBatch** ensures atomic writes (all or nothing)
- **Block cache** (16MB) speeds up repeated reads
- **Async writes** (`sync=false`) for performance
- **Single atomic batch** per block (all data saved together)

## Backup Strategy

### Automatic Backup (Recommended)

Add to deployment script:

```bash
#!/bin/bash
# Pre-deployment backup
BACKUP_DIR="/data/backups"
TIMESTAMP=$(date +%Y%m%d-%H%M%S)

if [ -d "/data/blockchain" ]; then
    mkdir -p $BACKUP_DIR
    tar -czf $BACKUP_DIR/blockchain-$TIMESTAMP.tar.gz /data/blockchain

    # Keep only last 7 backups
    ls -t $BACKUP_DIR/blockchain-*.tar.gz | tail -n +8 | xargs rm -f

    echo "✅ Backup created: blockchain-$TIMESTAMP.tar.gz"
fi

# Start node
./gxc-node --data-dir=/data/blockchain
```

### Manual Backup

```bash
# Backup
tar -czf blockchain-backup-$(date +%Y%m%d).tar.gz /data/blockchain

# Restore
tar -xzf blockchain-backup-20260226.tar.gz -C /data
```

## Summary

✅ **Blocks** - Fully persisted (metadata + transactions)
✅ **Transactions** - Fully persisted (all fields including type)
✅ **UTXO Set** - Fully persisted (atomic with blocks)
✅ **Validators** - Fully persisted
✅ **Mempool** - ✨ **NEW:** Fully persisted
✅ **Gold Reserves** - ✨ **NEW:** Fully persisted
✅ **Reversal Tracking** - ✨ **NEW:** Fully persisted
✅ **Staking Pool** - ✨ **NEW:** Fully persisted
✅ **Traceability Index** - Fully persisted
✅ **Governance** - Fully persisted
✅ **Network Peers** - Fully persisted

✅ **Database Lock** - Prevents accidental rebuilds
✅ **Ephemeral Storage Detection** - Warns about data loss risk
✅ **Block Count Validation** - Refuses to start on corruption
✅ **Transaction Type Serialization** - Fixed (STAKE/UNSTAKE/REWARD preserved)
✅ **Timestamp Precision** - Fixed (explicit uint64_t casting)

**NOTHING is lost. EVERYTHING is saved. Even on rebuild.**
