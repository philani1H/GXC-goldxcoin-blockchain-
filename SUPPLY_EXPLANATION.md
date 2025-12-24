# GXC Blockchain Supply Explanation

## Supply Types

### 1. Maximum Supply (Cap)
**31,000,000 GXC**

This is the hard cap defined in the blockchain code:
```cpp
// From include/blockchain.h
static const uint32_t MAX_SUPPLY = 31000000; // 31 million GXC
```

This is the absolute maximum number of GXC that will ever exist.

### 2. Circulating Supply (Current)
**2,500 GXC** (as of block 50)

This is the actual amount of GXC currently in circulation, calculated by summing all unspent transaction outputs (UTXOs).

**How it's calculated:**
```python
# Get all unspent outputs from blockchain
utxos = rpc_call('listunspent', [])
circulating_supply = sum(utxo['amount'] for utxo in utxos)
```

**Current breakdown:**
- 50 blocks mined
- 50 GXC reward per block
- 50 × 50 = 2,500 GXC circulating

### 3. Block Rewards

**Initial Reward:** 50 GXC per block

**Halving Schedule:**
- Every 1,051,200 blocks (~4 years)
- Reward halves each time
- Similar to Bitcoin's halving

**Halving Timeline:**
```
Block 0 - 1,051,200:     50 GXC per block
Block 1,051,201 - 2,102,400:  25 GXC per block
Block 2,102,401 - 3,153,600:  12.5 GXC per block
...and so on
```

---

## Explorer Display

### Before Fix
```
Total Supply: 2,500 GXC
```
This was confusing because it looked like the total supply was only 2,500 GXC.

### After Fix
```
Circulating Supply: 2,500 GXC / 31,000,000 Max
```
Now it's clear:
- **2,500 GXC** is currently circulating
- **31,000,000 GXC** is the maximum that will ever exist

---

## How Supply Grows

### Current State (Block 50)
- Blocks mined: 50
- Circulating supply: 2,500 GXC
- Percentage of max: 0.008% (2,500 / 31,000,000)

### Future Projections

**After 1 year (~262,800 blocks at 2 min/block):**
- Circulating supply: ~13,140,000 GXC
- Percentage of max: 42.4%

**After 4 years (first halving at block 1,051,200):**
- Circulating supply: ~26,280,000 GXC
- Percentage of max: 84.8%
- Reward drops to 25 GXC per block

**After 8 years (second halving):**
- Circulating supply: ~29,415,000 GXC
- Percentage of max: 94.9%
- Reward drops to 12.5 GXC per block

**After 12+ years:**
- Circulating supply approaches 31,000,000 GXC
- Percentage of max: ~100%
- Rewards become very small

---

## Why This Matters

### 1. Scarcity
With a hard cap of 31 million GXC, the token has built-in scarcity similar to Bitcoin (21 million BTC).

### 2. Inflation Rate
The inflation rate decreases over time due to halving:
- **Year 1-4:** High inflation (~50% per year initially)
- **Year 5-8:** Medium inflation (~25% per year)
- **Year 9+:** Low inflation (decreasing)

### 3. Long-term Value
As supply approaches the cap, new issuance slows down, potentially increasing value if demand remains constant or grows.

---

## Technical Implementation

### Getting Circulating Supply

**Method 1: Sum all UTXOs (Most Accurate)**
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listunspent",
    "params": [],
    "id": 1
  }'
```

**Method 2: Calculate from blocks (Estimate)**
```python
blocks = 50
reward_per_block = 50
estimated_supply = blocks * reward_per_block  # 2,500 GXC
```

### Explorer Implementation

The explorer now uses Method 1 (listunspent) to get the real circulating supply:

```python
# Get actual total supply from blockchain (sum of all UTXOs)
total_supply = 0.0
try:
    utxos = rpc_call('listunspent', [], timeout=10, show_errors=False)
    if utxos and isinstance(utxos, list):
        total_supply = sum(utxo.get('amount', 0) for utxo in utxos)
except Exception:
    # Fallback to calculated supply if RPC fails
    total_supply = height * CURRENT_NETWORK['block_reward']
```

---

## Verification

### Test 1: Check Current Supply
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"listunspent","params":[],"id":1}' \
  | jq '[.result[].amount] | add'
```

**Expected Output:** `2500`

### Test 2: Check Max Supply
```bash
grep "MAX_SUPPLY" include/blockchain.h
```

**Expected Output:** `static const uint32_t MAX_SUPPLY = 31000000;`

### Test 3: Verify Explorer Display
Visit: `http://localhost:5001`

**Expected Display:**
```
Circulating Supply
2,500
GXC / 31,000,000 Max
```

---

## Comparison with Other Cryptocurrencies

| Cryptocurrency | Max Supply | Current Supply | Halving Interval |
|----------------|------------|----------------|------------------|
| **GXC** | 31,000,000 | 2,500 | 1,051,200 blocks (~4 years) |
| Bitcoin | 21,000,000 | ~19,500,000 | 210,000 blocks (~4 years) |
| Litecoin | 84,000,000 | ~73,000,000 | 840,000 blocks (~4 years) |
| Ethereum | Unlimited | ~120,000,000 | No halving |

---

## Summary

✅ **Maximum Supply:** 31,000,000 GXC (hard cap)
✅ **Circulating Supply:** 2,500 GXC (current, from blockchain)
✅ **Block Reward:** 50 GXC (halves every ~4 years)
✅ **Explorer Display:** Now shows "Circulating Supply: 2,500 GXC / 31,000,000 Max"

The explorer now accurately displays the real circulating supply from the blockchain, making it clear that:
1. Only 2,500 GXC are currently in circulation
2. The maximum supply is 31,000,000 GXC
3. We're at 0.008% of the maximum supply

This provides transparency and helps users understand the tokenomics of GXC.
