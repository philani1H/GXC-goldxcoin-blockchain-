# GXC Explorer & Block Rewards System

## Overview

The GXC blockchain explorer provides **complete transparency** for all blocks, transactions, and rewards. Miners and validators can see their rewards in real-time.

---

## ✅ Block Reward System

### How Rewards Work

**For Miners (PoW Blocks):**
```
Miner solves puzzle → Submits block → Block validated → Miner receives reward
```

**For Validators (PoS Blocks):**
```
Validator selected → Creates block → Signs block → Block validated → Validator receives reward
```

### Reward Structure

**Base Reward:** 12.5 GXC per block

**Block Types:**
- **PoW Blocks (Even heights):** Miner receives 12.5 GXC
- **PoS Blocks (Odd heights):** Validator receives 12.5 GXC

**Reward Calculation:**
```cpp
double baseReward = 12.5;  // GXC per block

// Security engine may adjust based on block time
if (blockTime < targetTime) {
    // Fast block = reduced reward (anti-gaming)
    reward = baseReward * (blockTime / targetTime);
} else {
    // Normal or slow block = full reward
    reward = baseReward;
}
```

### Reward Distribution

**Coinbase Transaction:**
Every block contains a coinbase transaction that creates new coins:

```json
{
  "type": "COINBASE",
  "outputs": [
    {
      "address": "GXC1miner_or_validator_address",
      "amount": 12.5,
      "script": ""
    }
  ],
  "fee": 0.0
}
```

**UTXO Creation:**
When block is accepted:
1. Coinbase transaction validated
2. New UTXO created for miner/validator
3. Balance increases immediately
4. Reward is spendable after confirmation

---

## 🔍 Explorer Features

### 1. Block Explorer

**URL:** `/block/<block_hash_or_height>`

**Displays:**
- ✅ Block hash
- ✅ Block height
- ✅ Timestamp
- ✅ Block type (PoW or PoS)
- ✅ Miner/Validator address
- ✅ **Block reward amount**
- ✅ Difficulty
- ✅ Nonce (for PoW)
- ✅ Validator signature (for PoS)
- ✅ All transactions
- ✅ Total fees collected

**Example:**
```json
{
  "height": 12345,
  "hash": "000000abc123...",
  "type": "POW_SHA256",
  "miner": "GXC1abc...",
  "reward": 12.5,
  "transactions": 50,
  "totalFees": 0.05,
  "timestamp": "2024-12-06 10:30:00"
}
```

### 2. Address Explorer

**URL:** `/address/<address>`

**Displays:**
- ✅ Current balance
- ✅ Total received
- ✅ Total sent
- ✅ Transaction count
- ✅ **All rewards received** (from mining/validating)
- ✅ Transaction history
- ✅ UTXO list

**Reward Tracking:**
```sql
SELECT SUM(amount) as total_rewards
FROM transactions t
JOIN transaction_outputs o ON t.tx_hash = o.tx_hash
WHERE o.address = '<miner_address>'
  AND t.type = 'COINBASE'
```

### 3. Transaction Explorer

**URL:** `/tx/<tx_hash>`

**Displays:**
- ✅ Transaction hash
- ✅ Block height
- ✅ Confirmations
- ✅ Type (NORMAL, STAKE, UNSTAKE, COINBASE)
- ✅ Inputs (with amounts)
- ✅ Outputs (with amounts)
- ✅ Fee
- ✅ Timestamp

**Coinbase Transaction Display:**
```json
{
  "type": "COINBASE",
  "block": 12345,
  "outputs": [
    {
      "address": "GXC1miner...",
      "amount": 12.5,
      "type": "Block Reward"
    }
  ],
  "confirmations": 6
}
```

### 4. Latest Blocks

**URL:** `/`

**Displays:**
- ✅ Latest 20 blocks
- ✅ Block height
- ✅ Block type (PoW/PoS)
- ✅ Miner/Validator
- ✅ **Reward amount**
- ✅ Transaction count
- ✅ Time ago

**Real-time Updates:**
- WebSocket connection
- New blocks appear automatically
- Rewards shown immediately

### 5. Miner/Validator Stats

**URL:** `/stats/miners` or `/stats/validators`

**Displays:**
- ✅ Top miners by blocks found
- ✅ Top validators by blocks validated
- ✅ **Total rewards earned**
- ✅ Average reward per block
- ✅ Success rate
- ✅ Last block time

**Example:**
```json
{
  "miners": [
    {
      "address": "GXC1abc...",
      "blocks_mined": 150,
      "total_rewards": 1875.0,
      "avg_reward": 12.5,
      "last_block": 12345
    }
  ],
  "validators": [
    {
      "address": "GXC1def...",
      "blocks_validated": 145,
      "total_rewards": 1812.5,
      "avg_reward": 12.5,
      "last_block": 12344
    }
  ]
}
```

---

## 📊 Reward Verification

### How to Verify Your Rewards

**Step 1: Check Block Explorer**
```
Visit: https://explorer.gxc.network/block/<your_block_height>
Verify: Your address is listed as miner/validator
Verify: Reward amount is shown (12.5 GXC)
```

**Step 2: Check Address Balance**
```
Visit: https://explorer.gxc.network/address/<your_address>
Verify: Balance increased by reward amount
Verify: Coinbase transaction appears in history
```

**Step 3: Check Transaction**
```
Visit: https://explorer.gxc.network/tx/<coinbase_tx_hash>
Verify: Type is "COINBASE"
Verify: Output to your address
Verify: Amount is 12.5 GXC
```

**Step 4: Query via RPC**
```bash
# Get balance
curl -X POST http://node:8332 -d '{
  "method": "getbalance",
  "params": ["<your_address>"],
  "id": 1
}'

# Get block
curl -X POST http://node:8332 -d '{
  "method": "getblock",
  "params": [<block_height>],
  "id": 1
}'
```

---

## 🎯 Reward Guarantees

### For Miners

**When you mine a PoW block:**
1. ✅ Block submitted to network
2. ✅ Block validated by nodes
3. ✅ Coinbase transaction created
4. ✅ UTXO created for your address
5. ✅ Balance increases by 12.5 GXC
6. ✅ Reward visible in explorer
7. ✅ Spendable after 1 confirmation

**Verification:**
- Block appears in explorer with your address
- Coinbase transaction shows your address
- Balance query shows increased amount
- UTXO list shows new unspent output

### For Validators

**When you validate a PoS block:**
1. ✅ Selected by weighted stake
2. ✅ Create and sign block
3. ✅ Block validated by network
4. ✅ Coinbase transaction created
5. ✅ UTXO created for your address
6. ✅ Balance increases by 12.5 GXC
7. ✅ Reward visible in explorer
8. ✅ Spendable after 1 confirmation

**Verification:**
- Block appears in explorer with your address
- Block type shows "PoS"
- Validator signature present
- Coinbase transaction shows your address
- Balance query shows increased amount

---

## 🔒 Security & Transparency

### Reward Tracking

**All rewards are tracked:**
- ✅ In blockchain (immutable)
- ✅ In explorer database (queryable)
- ✅ Via RPC API (programmatic access)
- ✅ In real-time (WebSocket updates)

**No rewards can be:**
- ❌ Hidden
- ❌ Manipulated
- ❌ Lost
- ❌ Duplicated

### Audit Trail

**Every reward has:**
- Block hash (proof of work/stake)
- Transaction hash (coinbase tx)
- Block height (position in chain)
- Timestamp (when created)
- Miner/Validator address (recipient)
- Amount (12.5 GXC)
- Confirmations (security level)

**Complete transparency:**
```
Block 12345 → Coinbase TX abc123 → Output to GXC1abc → 12.5 GXC
```

---

## 📱 Explorer API Endpoints

### Get Block with Rewards

```bash
GET /api/block/<height_or_hash>

Response:
{
  "height": 12345,
  "hash": "000000abc...",
  "type": "POW_SHA256",
  "miner": "GXC1abc...",
  "reward": 12.5,
  "transactions": [...],
  "coinbase": {
    "txHash": "abc123...",
    "outputs": [
      {
        "address": "GXC1abc...",
        "amount": 12.5
      }
    ]
  }
}
```

### Get Address Rewards

```bash
GET /api/address/<address>/rewards

Response:
{
  "address": "GXC1abc...",
  "total_rewards": 1875.0,
  "blocks_found": 150,
  "rewards": [
    {
      "block": 12345,
      "type": "POW",
      "amount": 12.5,
      "timestamp": "2024-12-06 10:30:00"
    },
    ...
  ]
}
```

### Get Miner Stats

```bash
GET /api/stats/miners

Response:
{
  "miners": [
    {
      "address": "GXC1abc...",
      "blocks": 150,
      "rewards": 1875.0,
      "last_block": 12345
    }
  ]
}
```

### Get Validator Stats

```bash
GET /api/stats/validators

Response:
{
  "validators": [
    {
      "address": "GXC1def...",
      "blocks": 145,
      "rewards": 1812.5,
      "stake": 1000.0,
      "last_block": 12344
    }
  ]
}
```

---

## ✅ Verification Checklist

### For Miners

After mining a block, verify:
- [ ] Block appears in explorer
- [ ] Your address shown as miner
- [ ] Reward amount is 12.5 GXC
- [ ] Coinbase transaction present
- [ ] Balance increased
- [ ] UTXO created
- [ ] Spendable after confirmation

### For Validators

After validating a block, verify:
- [ ] Block appears in explorer
- [ ] Your address shown as validator
- [ ] Block type is "PoS"
- [ ] Validator signature present
- [ ] Reward amount is 12.5 GXC
- [ ] Coinbase transaction present
- [ ] Balance increased
- [ ] UTXO created
- [ ] Spendable after confirmation

---

## 🚀 Summary

**The GXC explorer provides complete transparency:**

✅ **All blocks visible** - Every block displayed with full details  
✅ **All rewards tracked** - Every reward recorded and queryable  
✅ **Real-time updates** - New blocks appear immediately  
✅ **Complete audit trail** - Full transaction history  
✅ **Miner stats** - Track your mining performance  
✅ **Validator stats** - Track your validation performance  
✅ **API access** - Programmatic data access  
✅ **WebSocket updates** - Real-time notifications  

**Miners and validators get rewarded:**
- ✅ 12.5 GXC per block
- ✅ Immediate UTXO creation
- ✅ Spendable after confirmation
- ✅ Fully transparent
- ✅ Completely auditable

**Everything is verifiable:**
- ✅ Via explorer web interface
- ✅ Via RPC API
- ✅ Via blockchain data
- ✅ Via UTXO queries

**No rewards are lost or hidden!** 🎉
