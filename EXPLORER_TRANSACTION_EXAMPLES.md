# GXC Explorer - Transaction Display Examples

This document shows how transactions appear in the GXC Block Explorer, with examples from the node API.

## Table of Contents
1. [Transaction Types](#transaction-types)
2. [API Response Format](#api-response-format)
3. [Explorer Display](#explorer-display)
4. [Traceability Verification](#traceability-verification)

---

## Transaction Types

### 1. Coinbase Transaction (Mining Reward)
**What it is:** First transaction in a block, creates new coins as mining reward.

**API Response:**
```json
{
  "txid": "a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456",
  "version": 1,
  "time": 1703462400,
  "locktime": 0,
  "confirmations": 150,
  "blockhash": "00000000000abc123def456789...",
  "blockheight": 1000,
  "type": "coinbase",
  "vin": [],
  "vout": [
    {
      "value": 50.0,
      "n": 0,
      "scriptPubKey": {
        "address": "gxc1qminer123address456789abcdef"
      }
    }
  ],
  "prev_tx_hash": null,
  "referenced_amount": 0,
  "fee": 0,
  "is_traceable": false,
  "traceability_status": "genesis"
}
```

**Explorer Display:**
```
┌─────────────────────────────────────────────────────────────┐
│ Transaction Details                                          │
├─────────────────────────────────────────────────────────────┤
│ Transaction Hash:                                            │
│ a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef  │
│                                                              │
│ Status: ✅ Confirmed  🔗 Genesis/Coinbase                   │
│                                                              │
│ Block Height: 1000                                           │
│ Confirmations: 150                                           │
│ Timestamp: Dec 24, 2023 12:00:00 PM                         │
│                                                              │
│ Previous TX Hash: None (Genesis/Coinbase)                   │
│ Referenced Amount: 0 GXC                                     │
│ Transaction Fee: 0 GXC                                       │
├─────────────────────────────────────────────────────────────┤
│ 📊 Traceability Formula Verification                        │
│                                                              │
│ Formula 1: Ti.Inputs[0].txHash == Ti.PrevTxHash             │
│ Status: ⚪ N/A (Genesis transaction)                        │
│                                                              │
│ Formula 2: Ti.Inputs[0].amount == Ti.ReferencedAmount       │
│ Status: ⚪ N/A (Genesis transaction)                        │
│                                                              │
│ ℹ️ Coinbase transactions create new coins and don't have    │
│    previous transactions to trace.                          │
├─────────────────────────────────────────────────────────────┤
│ ➡️ INPUTS (0)                                               │
│ No inputs (Coinbase transaction)                            │
│                                                              │
│ ⬅️ OUTPUTS (1)                                              │
│ Output #0                                    50.0 GXC       │
│ Address: gxc1qminer123address456789abcdef                   │
└─────────────────────────────────────────────────────────────┘
```

---

### 2. Regular Send Transaction
**What it is:** Standard transfer of GXC from one address to another.

**API Response:**
```json
{
  "txid": "b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef1234567",
  "version": 1,
  "time": 1703462700,
  "locktime": 0,
  "confirmations": 50,
  "blockhash": "00000000000def456789abc123...",
  "blockheight": 1100,
  "type": "send",
  "vin": [
    {
      "txid": "a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456",
      "vout": 0,
      "amount": 50.0,
      "address": "gxc1qminer123address456789abcdef"
    }
  ],
  "vout": [
    {
      "value": 45.0,
      "n": 0,
      "scriptPubKey": {
        "address": "gxc1qrecipient456address789abcdef"
      }
    },
    {
      "value": 4.99,
      "n": 1,
      "scriptPubKey": {
        "address": "gxc1qminer123address456789abcdef"
      }
    }
  ],
  "prev_tx_hash": "a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456",
  "referenced_amount": 50.0,
  "fee": 0.01,
  "is_traceable": true,
  "traceability_status": "valid"
}
```

**Explorer Display:**
```
┌─────────────────────────────────────────────────────────────┐
│ Transaction Details                                          │
├─────────────────────────────────────────────────────────────┤
│ Transaction Hash:                                            │
│ b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef    │
│                                                              │
│ Status: ✅ Confirmed  🔗 Traceable                          │
│                                                              │
│ Block Height: 1100                                           │
│ Confirmations: 50                                            │
│ Timestamp: Dec 24, 2023 12:05:00 PM                         │
│                                                              │
│ Previous TX Hash:                                            │
│ a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef  │
│ Referenced Amount: 50.0 GXC                                  │
│ Transaction Fee: 0.01 GXC                                    │
├─────────────────────────────────────────────────────────────┤
│ 📊 Traceability Formula Verification                        │
│                                                              │
│ Formula 1: Ti.Inputs[0].txHash == Ti.PrevTxHash             │
│ Status: ✅ VALID                                            │
│ ✓ a1b2c3d4e5f6... == a1b2c3d4e5f6...                       │
│                                                              │
│ Formula 2: Ti.Inputs[0].amount == Ti.ReferencedAmount       │
│ Status: ✅ VALID                                            │
│ ✓ 50.0 GXC == 50.0 GXC                                      │
│                                                              │
│ ℹ️ This transaction maintains the mathematical breadcrumb   │
│    trail required for full traceability.                    │
├─────────────────────────────────────────────────────────────┤
│ ➡️ INPUTS (1)                                               │
│                                                              │
│ Input #0                                     50.0 GXC       │
│ Previous TX:                                                 │
│ a1b2c3d4e5f6789012345678901234567890abcdef123456789...      │
│ Output Index: 0                                              │
│ From: gxc1qminer123address456789abcdef                      │
│                                                              │
│ ⬅️ OUTPUTS (2)                                              │
│                                                              │
│ Output #0                                    45.0 GXC       │
│ To: gxc1qrecipient456address789abcdef                       │
│                                                              │
│ Output #1 (Change)                           4.99 GXC       │
│ To: gxc1qminer123address456789abcdef                        │
└─────────────────────────────────────────────────────────────┘
```

---

### 3. Stake Transaction
**What it is:** Locks coins for staking/validation.

**API Response:**
```json
{
  "txid": "c3d4e5f6789012345678901234567890abcdef1234567890abcdef12345678901",
  "version": 1,
  "time": 1703463000,
  "locktime": 0,
  "confirmations": 25,
  "blockhash": "00000000000ghi789abc123def...",
  "blockheight": 1125,
  "type": "stake",
  "vin": [
    {
      "txid": "b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef1234567",
      "vout": 1,
      "amount": 4.99,
      "address": "gxc1qminer123address456789abcdef"
    }
  ],
  "vout": [
    {
      "value": 4.98,
      "n": 0,
      "scriptPubKey": {
        "address": "gxc1qminer123address456789abcdef",
        "type": "stake"
      }
    }
  ],
  "prev_tx_hash": "b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef1234567",
  "referenced_amount": 4.99,
  "fee": 0.01,
  "is_traceable": true,
  "traceability_status": "valid",
  "stake_info": {
    "amount": 4.98,
    "validator_address": "gxc1qminer123address456789abcdef",
    "lock_time": 86400
  }
}
```

**Explorer Display:**
```
┌─────────────────────────────────────────────────────────────┐
│ Stake Transaction Details                                    │
├─────────────────────────────────────────────────────────────┤
│ Transaction Hash:                                            │
│ c3d4e5f6789012345678901234567890abcdef1234567890abcdef      │
│                                                              │
│ Status: ✅ Confirmed  🔗 Traceable  🔒 Staked              │
│                                                              │
│ Block Height: 1125                                           │
│ Confirmations: 25                                            │
│ Timestamp: Dec 24, 2023 12:10:00 PM                         │
│                                                              │
│ Previous TX Hash:                                            │
│ b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef    │
│ Referenced Amount: 4.99 GXC                                  │
│ Transaction Fee: 0.01 GXC                                    │
├─────────────────────────────────────────────────────────────┤
│ 🔒 Stake Information                                        │
│                                                              │
│ Staked Amount: 4.98 GXC                                      │
│ Validator: gxc1qminer123address456789abcdef                 │
│ Lock Time: 24 hours                                          │
│ Status: Active                                               │
├─────────────────────────────────────────────────────────────┤
│ 📊 Traceability Formula Verification                        │
│                                                              │
│ Formula 1: Ti.Inputs[0].txHash == Ti.PrevTxHash             │
│ Status: ✅ VALID                                            │
│                                                              │
│ Formula 2: Ti.Inputs[0].amount == Ti.ReferencedAmount       │
│ Status: ✅ VALID                                            │
│                                                              │
│ ℹ️ Stake transactions maintain full traceability while      │
│    locking coins for network validation.                    │
├─────────────────────────────────────────────────────────────┤
│ ➡️ INPUTS (1)                                               │
│                                                              │
│ Input #0                                     4.99 GXC       │
│ Previous TX: b2c3d4e5f6...                                  │
│ Output Index: 1                                              │
│                                                              │
│ ⬅️ OUTPUTS (1)                                              │
│                                                              │
│ Output #0 (Staked)                           4.98 GXC       │
│ Validator: gxc1qminer123address456789abcdef                 │
│ Type: Stake Lock                                             │
└─────────────────────────────────────────────────────────────┘
```

---

### 4. Unstake Transaction
**What it is:** Unlocks previously staked coins.

**API Response:**
```json
{
  "txid": "d4e5f6789012345678901234567890abcdef1234567890abcdef123456789012",
  "version": 1,
  "time": 1703549400,
  "locktime": 0,
  "confirmations": 10,
  "blockhash": "00000000000jkl123abc456def...",
  "blockheight": 1200,
  "type": "unstake",
  "vin": [
    {
      "txid": "c3d4e5f6789012345678901234567890abcdef1234567890abcdef12345678901",
      "vout": 0,
      "amount": 4.98,
      "address": "gxc1qminer123address456789abcdef"
    }
  ],
  "vout": [
    {
      "value": 5.47,
      "n": 0,
      "scriptPubKey": {
        "address": "gxc1qminer123address456789abcdef"
      }
    }
  ],
  "prev_tx_hash": "c3d4e5f6789012345678901234567890abcdef1234567890abcdef12345678901",
  "referenced_amount": 4.98,
  "fee": 0.01,
  "is_traceable": true,
  "traceability_status": "valid",
  "unstake_info": {
    "original_stake": 4.98,
    "rewards_earned": 0.50,
    "total_returned": 5.47
  }
}
```

**Explorer Display:**
```
┌─────────────────────────────────────────────────────────────┐
│ Unstake Transaction Details                                  │
├─────────────────────────────────────────────────────────────┤
│ Transaction Hash:                                            │
│ d4e5f6789012345678901234567890abcdef1234567890abcdef        │
│                                                              │
│ Status: ✅ Confirmed  🔗 Traceable  🔓 Unstaked            │
│                                                              │
│ Block Height: 1200                                           │
│ Confirmations: 10                                            │
│ Timestamp: Dec 25, 2023 12:10:00 PM                         │
│                                                              │
│ Previous TX Hash:                                            │
│ c3d4e5f6789012345678901234567890abcdef1234567890abcdef      │
│ Referenced Amount: 4.98 GXC                                  │
│ Transaction Fee: 0.01 GXC                                    │
├─────────────────────────────────────────────────────────────┤
│ 🔓 Unstake Information                                      │
│                                                              │
│ Original Stake: 4.98 GXC                                     │
│ Rewards Earned: 0.50 GXC                                     │
│ Total Returned: 5.47 GXC                                     │
│ Stake Duration: 24 hours                                     │
├─────────────────────────────────────────────────────────────┤
│ 📊 Traceability Formula Verification                        │
│                                                              │
│ Formula 1: Ti.Inputs[0].txHash == Ti.PrevTxHash             │
│ Status: ✅ VALID                                            │
│                                                              │
│ Formula 2: Ti.Inputs[0].amount == Ti.ReferencedAmount       │
│ Status: ✅ VALID                                            │
│                                                              │
│ ℹ️ Unstake transactions maintain traceability while         │
│    returning staked coins plus rewards.                     │
├─────────────────────────────────────────────────────────────┤
│ ➡️ INPUTS (1)                                               │
│                                                              │
│ Input #0                                     4.98 GXC       │
│ Previous TX: c3d4e5f6...                                    │
│ Output Index: 0                                              │
│ Type: Staked Output                                          │
│                                                              │
│ ⬅️ OUTPUTS (1)                                              │
│                                                              │
│ Output #0                                    5.47 GXC       │
│ To: gxc1qminer123address456789abcdef                        │
│ (Original 4.98 + Rewards 0.50 - Fee 0.01)                   │
└─────────────────────────────────────────────────────────────┘
```

---

## Transaction Lineage Trace

When you click "Trace Lineage" in the explorer, it shows the full transaction history:

```
┌─────────────────────────────────────────────────────────────┐
│ 🔍 Transaction Lineage Trace                                │
│ Showing 4 transactions in chain                              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│ ● Transaction #1 (Genesis)                                  │
│   Hash: a1b2c3d4e5f6...                                     │
│   Type: Coinbase                                             │
│   Amount: 50.0 GXC                                           │
│   Block: 1000                                                │
│   Time: Dec 24, 2023 12:00:00 PM                            │
│   │                                                          │
│   ↓ (Spent in next transaction)                             │
│   │                                                          │
│ ● Transaction #2 (Send)                                     │
│   Hash: b2c3d4e5f6...                                       │
│   Type: Send                                                 │
│   Amount: 50.0 → 45.0 + 4.99 GXC                            │
│   Block: 1100                                                │
│   Time: Dec 24, 2023 12:05:00 PM                            │
│   ✅ Formula 1: Valid (input matches prev hash)             │
│   ✅ Formula 2: Valid (amount matches)                      │
│   │                                                          │
│   ↓ (Change output spent)                                   │
│   │                                                          │
│ ● Transaction #3 (Stake)                                    │
│   Hash: c3d4e5f6...                                         │
│   Type: Stake                                                │
│   Amount: 4.99 → 4.98 GXC (staked)                          │
│   Block: 1125                                                │
│   Time: Dec 24, 2023 12:10:00 PM                            │
│   ✅ Formula 1: Valid                                       │
│   ✅ Formula 2: Valid                                       │
│   │                                                          │
│   ↓ (Unstaked after 24 hours)                               │
│   │                                                          │
│ ● Transaction #4 (Unstake) ← CURRENT                       │
│   Hash: d4e5f6...                                           │
│   Type: Unstake                                              │
│   Amount: 4.98 → 5.47 GXC (with rewards)                    │
│   Block: 1200                                                │
│   Time: Dec 25, 2023 12:10:00 PM                            │
│   ✅ Formula 1: Valid                                       │
│   ✅ Formula 2: Valid                                       │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## API Endpoints for Explorer

### Get Transaction Details
```bash
GET /api/v1/transaction/{txid}
```

**Example:**
```bash
curl http://localhost:8080/api/v1/transaction/b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef1234567
```

### Get Transaction Lineage
```bash
GET /api/v1/transaction/{txid}/trace
```

**Example:**
```bash
curl http://localhost:8080/api/v1/transaction/d4e5f6789012345678901234567890abcdef1234567890abcdef123456789012/trace
```

**Response:**
```json
{
  "current_tx": "d4e5f6...",
  "trace_length": 4,
  "transactions": [
    {
      "txid": "a1b2c3d4e5f6...",
      "type": "coinbase",
      "amount": 50.0,
      "block": 1000,
      "is_genesis": true
    },
    {
      "txid": "b2c3d4e5f6...",
      "type": "send",
      "amount": 50.0,
      "block": 1100,
      "formula1_valid": true,
      "formula2_valid": true
    },
    {
      "txid": "c3d4e5f6...",
      "type": "stake",
      "amount": 4.99,
      "block": 1125,
      "formula1_valid": true,
      "formula2_valid": true
    },
    {
      "txid": "d4e5f6...",
      "type": "unstake",
      "amount": 4.98,
      "block": 1200,
      "formula1_valid": true,
      "formula2_valid": true
    }
  ]
}
```

---

## Web Interface Features

### 1. Real-Time Updates
- WebSocket connection for live transaction updates
- Auto-refresh when new blocks are mined
- Notification badges for new confirmations

### 2. Search Functionality
```
Search by:
- Transaction Hash
- Block Height
- Address
- Amount Range
- Date Range
- Transaction Type
```

### 3. Filtering Options
```
Filter transactions by:
☐ Coinbase
☐ Send
☐ Stake
☐ Unstake
☐ Confirmed Only
☐ Traceable Only
```

### 4. Export Options
```
Export transaction data as:
- JSON
- CSV
- PDF Report
```

---

## Traceability Indicators

### Visual Indicators in Explorer

**✅ Fully Traceable Transaction:**
```
┌────────────────────────────────────┐
│ ✅ Confirmed                       │
│ 🔗 Traceable                       │
│ ✅ Formula 1: VALID                │
│ ✅ Formula 2: VALID                │
└────────────────────────────────────┘
```

**⚪ Genesis/Coinbase Transaction:**
```
┌────────────────────────────────────┐
│ ✅ Confirmed                       │
│ 🔗 Genesis/Coinbase                │
│ ⚪ Formula 1: N/A                  │
│ ⚪ Formula 2: N/A                  │
└────────────────────────────────────┘
```

**⏳ Pending Transaction:**
```
┌────────────────────────────────────┐
│ ⏳ Pending (0 confirmations)       │
│ 🔗 Traceable                       │
│ ⏳ Awaiting confirmation           │
└────────────────────────────────────┘
```

---

## Summary

The GXC Explorer provides:

1. **Complete Transaction Details** - All transaction data in readable format
2. **Traceability Verification** - Visual confirmation of formula validation
3. **Transaction Lineage** - Full history trace with visual timeline
4. **Real-Time Updates** - Live data from blockchain node
5. **Search & Filter** - Find transactions by multiple criteria
6. **Export Options** - Download transaction data in various formats

All transactions maintain the mathematical breadcrumb trail:
- **Formula 1:** `Ti.Inputs[0].txHash == Ti.PrevTxHash`
- **Formula 2:** `Ti.Inputs[0].amount == Ti.ReferencedAmount`

This ensures complete traceability from genesis to current transaction.
