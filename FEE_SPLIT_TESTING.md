# Testing Transaction Fee Split

## Overview

The system pool receives **15% of transaction fees** (not block rewards). This document explains how to test this functionality.

## How Fee Split Works

When a block is mined with transactions that have fees:

1. **Transaction Fee**: User pays fee (e.g., 1.0 GXC)
2. **Split Calculation**:
   - Miner receives: 85% × 1.0 = 0.85 GXC
   - Pool receives: 15% × 1.0 = 0.15 GXC
3. **Block Reward**: Miner gets full 50 GXC (no split)
4. **Total Miner Payment**: 50 + 0.85 = 50.85 GXC

## Testing Steps

### 1. Check Initial Balances

```bash
# Miner balance
curl -X POST https://your-node.com/api/getbalance \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"getbalance","params":["tGXC9fab7317231b966af85ac453e168c0932"]}'

# System pool balance
curl -X POST https://your-node.com/api/getbalance \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"getbalance","params":["tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99"]}'
```

### 2. Send Transaction with Fee

**Option A: Using Python Script**

```python
import requests

# Create transaction with fee
payload = {
    "jsonrpc": "2.0",
    "id": 1,
    "method": "sendtoaddress",
    "params": ["tGXCrecipient...", 10.0, "", "", False, 1.0]  # 1.0 GXC fee
}

response = requests.post("https://your-node.com/api/sendtoaddress", json=payload)
print(response.json())
```

**Option B: Using gxc-cli**

```bash
./gxc-cli sendtoaddress tGXCrecipient... 10.0 "" "" false 1.0
```

### 3. Mine Block

Run the miner to include the transaction in a block:

```bash
python3 gxc_miner.py
```

Wait for the block to be mined and confirmed (6 confirmations for maturity).

### 4. Check Final Balances

```bash
# Check miner balance (should increase by block reward + 85% of fees)
curl -X POST https://your-node.com/api/getbalance \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"getbalance","params":["tGXC9fab7317231b966af85ac453e168c0932"]}'

# Check pool balance (should increase by 15% of fees)
curl -X POST https://your-node.com/api/getbalance \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"getbalance","params":["tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99"]}'
```

### 5. Verify Fee Split

Calculate expected values:

```
Transaction Fee: 1.0 GXC
Expected Miner Fee Share: 1.0 × 0.85 = 0.85 GXC
Expected Pool Fee Share: 1.0 × 0.15 = 0.15 GXC

Miner Total: 50.0 (block reward) + 0.85 (fee) = 50.85 GXC
Pool Total: 0.15 GXC (from fee only)
```

## Automated Test Script

Use the provided test script:

```bash
python3 test_fee_split.py
```

This script will:
1. Check initial balances
2. Send a transaction with a 1.0 GXC fee
3. Wait for mining
4. Verify the 85/15 split

## Expected Results

### Block with 1 GXC in Fees

| Recipient | Amount | Source |
|-----------|--------|--------|
| Miner | 50.00 GXC | Block reward (100%) |
| Miner | 0.85 GXC | Transaction fees (85%) |
| System Pool | 0.15 GXC | Transaction fees (15%) |
| **Miner Total** | **50.85 GXC** | |
| **Pool Total** | **0.15 GXC** | |

### Block with 10 GXC in Fees

| Recipient | Amount | Source |
|-----------|--------|--------|
| Miner | 50.00 GXC | Block reward (100%) |
| Miner | 8.50 GXC | Transaction fees (85%) |
| System Pool | 1.50 GXC | Transaction fees (15%) |
| **Miner Total** | **58.50 GXC** | |
| **Pool Total** | **1.50 GXC** | |

## Checking Block Details

To see the fee split in a specific block:

```bash
curl -X POST https://your-node.com/api/getblock \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"getblock","params":[BLOCK_HEIGHT]}'
```

Look for:
- `transactions`: Array of transactions in the block
- Each transaction has a `fee` field
- Sum all fees and calculate 15% to verify pool deposit

## Logs to Monitor

When a block with fees is added, you should see:

```
[INFO] [BLOCKCHAIN] System Pool: Deposited X.XX GXC from transaction fee splits (15.0%)
```

## Troubleshooting

### Pool Balance is 0

**Possible causes:**
1. No transactions with fees have been mined yet
2. UTXOs not mature (need 6 confirmations)
3. Fee split not being deposited correctly

**Solution:**
- Wait for 6+ blocks after mining a block with fees
- Check logs for "System Pool: Deposited" messages
- Verify transactions actually have fees set

### Miner Not Getting Fee Share

**Possible causes:**
1. Transactions don't have fees
2. Fee calculation error

**Solution:**
- Ensure transactions specify a fee > 0
- Check block reward calculation includes fee share

## Code References

- **Fee Split Logic**: `src/Blockchain.cpp:588-610`
- **Fee Calculation**: `src/Blockchain.cpp:645-656`
- **Pool Deposit**: `src/Blockchain.cpp:596`

## Notes

- **Block rewards are NOT split** - miners get 100%
- **Only transaction fees are split** - 85/15
- Pool accumulates over time as network processes transactions
- Empty blocks (no transactions) = no pool deposits
- This is a self-sustaining model for fraud detection funding
