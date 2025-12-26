# GXC Transaction Types - Automatic vs Manual

## Overview

GXC has 8 transaction types. Some are **automatic** (created by blockchain), others are **manual** (created by users).

---

## Transaction Types

### 1. NORMAL - Regular Transfers
**Type:** ❌ **MANUAL** (User-initiated)

**Who Creates:** User/Wallet

**How:**
```bash
# User sends transaction
curl -X POST http://localhost:8332 \
  -d '{"method":"sendtoaddress","params":["TO_ADDRESS",10.0],"id":1}'
```

**Process:**
1. User creates transaction
2. Signs with private key
3. Broadcasts to network
4. Blockchain validates
5. Included in block

**Automatic Validation:** ✅ Yes (traceability, signatures, balance)

---

### 2. STAKE - Locking Coins
**Type:** ❌ **MANUAL** (User-initiated)

**Who Creates:** User/Wallet

**How:**
```bash
# User stakes coins
curl -X POST http://localhost:8332 \
  -d '{"method":"stake","params":["ADDRESS",1000.0,90],"id":1}'
```

**Process:**
1. User creates stake transaction
2. Signs with private key
3. Broadcasts to network
4. Blockchain validates
5. Coins locked for specified period

**Automatic Validation:** ✅ Yes (minimum amount, lock period, balance)

---

### 3. UNSTAKE - Unlocking Coins
**Type:** ❌ **MANUAL** (User-initiated)

**Who Creates:** User/Wallet

**How:**
```bash
# User unstakes coins
curl -X POST http://localhost:8332 \
  -d '{"method":"unstake","params":["ADDRESS","STAKE_TX_HASH"],"id":1}'
```

**Process:**
1. User creates unstake transaction
2. Signs with private key
3. Broadcasts to network
4. Blockchain validates lock period
5. Coins unlocked if period expired

**Automatic Validation:** ✅ Yes (lock period, ownership, stake exists)

---

### 4. REWARD - Staking Rewards
**Type:** ✅ **AUTOMATIC** (Blockchain-created)

**Who Creates:** Blockchain (automatically)

**When:** After each PoS block is added

**Code Location:** `src/Blockchain.cpp` - `distributeStakingRewards()`

**Process:**
```cpp
// AUTOMATIC - Blockchain creates REWARD transactions
void Blockchain::distributeStakingRewards(const Block& block) {
    // 1. Get all active stakes
    std::vector<Stake> activeStakes = stakingPool->getActiveStakes();
    
    // 2. Calculate each staker's share
    for (const auto& stake : activeStakes) {
        double stakerShare = (stake.amount / totalStake) * blockReward;
        
        // 3. Create REWARD transaction (AUTOMATIC)
        Transaction rewardTx;
        rewardTx.setType(TransactionType::REWARD);
        rewardTx.setSenderAddress("STAKING_POOL");
        rewardTx.setReceiverAddress(stake.ownerAddress);
        rewardTx.setReferencedAmount(stakerShare);
        
        // 4. Link to stake transaction (traceability)
        TransactionInput rewardInput;
        rewardInput.txHash = stake.stakeTxHash;
        rewardInput.amount = stakerShare;
        rewardTx.addInput(rewardInput);
        
        // 5. Create output (NEW COINS - minting)
        TransactionOutput rewardOutput;
        rewardOutput.address = stake.ownerAddress;
        rewardOutput.amount = stakerShare;
        rewardTx.addOutput(rewardOutput);
        
        // 6. Add to pending transactions (AUTOMATIC)
        pendingTransactions.push_back(rewardTx);
    }
}
```

**Triggers:**
- PoS block added to blockchain
- Active validators/stakers exist
- Block reward distributed proportionally

**User Action Required:** ❌ None (completely automatic)

**Validation:** ✅ Automatic (blockchain validates before adding)

---

### 5. COINBASE - Mining Rewards
**Type:** ✅ **AUTOMATIC** (Miner-created, Blockchain-validated)

**Who Creates:** Miner (when mining block)

**When:** First transaction in every block

**Code Location:** `src/Blockchain.cpp` - `validateBlock()`

**Process:**
```cpp
// AUTOMATIC - Miner creates COINBASE, blockchain validates
Block block;

// 1. Miner creates coinbase transaction
Transaction coinbase;
coinbase.setType(TransactionType::COINBASE);
coinbase.setCoinbaseTransaction(true);
coinbase.setBlockHeight(blockHeight);
coinbase.setWorkReceiptHash(proofOfWork);

// 2. Add output (NEW COINS - minting)
TransactionOutput output;
output.address = minerAddress;
output.amount = blockReward + totalFees;
coinbase.addOutput(output);

// 3. Add as first transaction in block
block.addTransaction(coinbase);

// 4. Blockchain validates AUTOMATICALLY
bool valid = blockchain->validateBlock(block);
```

**Validation Rules (AUTOMATIC):**
```cpp
// Blockchain enforces these rules automatically:
✅ Must be first transaction (index 0)
✅ Only one coinbase per block
✅ Reward must not exceed: blockReward + totalFees
✅ Must reference proof-of-work
✅ Must have correct block height
✅ Cannot be spent for 100 blocks (maturity)
```

**User Action Required:** ❌ None (miner creates, blockchain validates)

**Validation:** ✅ Automatic (strict consensus rules)

---

### 6. Stock Tokens - Synthetic Stock Trading
**Type:** ❌ **MANUAL** (User-initiated)

**Who Creates:** User/Wallet

**How:**
```bash
# User buys stock token
curl -X POST http://localhost:8332 \
  -d '{"method":"buystocktoken","params":["AAPL",1],"id":1}'
```

**Process:**
1. User creates buy/sell transaction
2. Signs with private key
3. Broadcasts to network
4. Blockchain validates (price, collateral)
5. Stock contract updated

**Automatic Validation:** ✅ Yes (price, collateral, stock exists)

**Automatic Price Updates:** ✅ Yes (Proof-of-Price oracle)

---

### 7. Gold Tokens - Gold-Backed Transfers
**Type:** ❌ **MANUAL** (User-initiated)

**Who Creates:** User/Wallet

**How:**
```bash
# User creates/redeems gold token
curl -X POST http://localhost:8332 \
  -d '{"method":"creategoldtoken","params":[1000.0],"id":1}'
```

**Process:**
1. User creates gold token transaction
2. Signs with private key
3. Broadcasts to network
4. Blockchain validates (reserves, price)
5. Gold token created/redeemed

**Automatic Validation:** ✅ Yes (reserves, Proof-of-Price, collateral)

**Automatic Price Updates:** ✅ Yes (Proof-of-Price oracle)

---

### 8. Cross-Chain - Bridge Transactions
**Type:** ❌ **MANUAL** (User-initiated)

**Who Creates:** User/Wallet

**How:**
```bash
# User bridges to another chain
curl -X POST http://localhost:8332 \
  -d '{"method":"bridgetransfer","params":["ETH","0x123...",100.0],"id":1}'
```

**Process:**
1. User creates bridge transaction
2. Signs with private key
3. Broadcasts to network
4. Blockchain validates (bridge active, minimum)
5. Coins locked in bridge contract
6. Bridge validators relay to destination chain

**Automatic Validation:** ✅ Yes (bridge active, destination valid, amount)

**Automatic Bridge Processing:** ✅ Yes (validators relay automatically)

---

## Summary Table

| Transaction Type | Automatic? | Who Creates | When | User Action |
|-----------------|------------|-------------|------|-------------|
| **NORMAL** | ❌ Manual | User | Anytime | Create & Sign |
| **STAKE** | ❌ Manual | User | Anytime | Create & Sign |
| **UNSTAKE** | ❌ Manual | User | After lock period | Create & Sign |
| **REWARD** | ✅ **Automatic** | **Blockchain** | **After PoS block** | **None** |
| **COINBASE** | ✅ **Automatic** | **Miner** | **Every block** | **None** |
| **Stock Tokens** | ❌ Manual | User | Anytime | Create & Sign |
| **Gold Tokens** | ❌ Manual | User | Anytime | Create & Sign |
| **Cross-Chain** | ❌ Manual | User | Anytime | Create & Sign |

---

## Automatic Validation

### All Transactions Validated Automatically ✅

**Every transaction** (manual or automatic) is validated by the blockchain:

```cpp
bool Blockchain::validateTransaction(const Transaction& tx) {
    // 1. Traceability validation (AUTOMATIC)
    if (!tx.verifyTraceabilityFormula()) {
        return false;  // REJECTED
    }
    
    // 2. Signature validation (AUTOMATIC)
    if (!tx.validateSignatures()) {
        return false;  // REJECTED
    }
    
    // 3. Balance validation (AUTOMATIC)
    if (!tx.validateInputOutputBalance()) {
        return false;  // REJECTED
    }
    
    // 4. Type-specific validation (AUTOMATIC)
    switch (tx.getType()) {
        case STAKE:
            return validateStakeTransaction(tx);
        case UNSTAKE:
            return validateUnstakeTransaction(tx);
        case REWARD:
            return validateRewardTransaction(tx);
        case COINBASE:
            return validateCoinbaseTransaction(tx);
        // ... etc
    }
    
    return true;  // ACCEPTED
}
```

**Result:**
- ✅ Valid transactions accepted
- ❌ Invalid transactions rejected
- 🔒 No manual intervention needed
- 🤖 Completely automatic

---

## What "Automatic" Means

### For REWARD Transactions:
1. ✅ **Created automatically** by blockchain after PoS blocks
2. ✅ **Distributed automatically** to all active stakers
3. ✅ **Calculated automatically** based on stake amount
4. ✅ **Validated automatically** before adding to blockchain
5. ❌ **No user action required** - happens in background

### For COINBASE Transactions:
1. ✅ **Created automatically** by miner when mining block
2. ✅ **Validated automatically** by blockchain (strict rules)
3. ✅ **Reward calculated automatically** (block reward + fees)
4. ✅ **Maturity enforced automatically** (100 blocks)
5. ❌ **No user action required** - part of mining process

### For Manual Transactions:
1. ❌ **User must create** transaction
2. ❌ **User must sign** with private key
3. ❌ **User must broadcast** to network
4. ✅ **Blockchain validates automatically** (traceability, signatures, balance)
5. ✅ **Blockchain processes automatically** (adds to block, updates UTXO)

---

## Examples

### Example 1: REWARD (Automatic)

**Scenario:** Alice staked 1000 GXC. A PoS block is mined.

**What Happens Automatically:**
```
1. PoS block added to blockchain
2. Blockchain calls distributeStakingRewards()
3. Blockchain calculates Alice's share: 5 GXC
4. Blockchain creates REWARD transaction:
   - Type: REWARD
   - From: STAKING_POOL
   - To: Alice
   - Amount: 5 GXC
   - Links to Alice's stake transaction
5. Blockchain validates transaction
6. Blockchain adds to pending transactions
7. REWARD included in next block
8. Alice's balance increases by 5 GXC
```

**Alice's Action:** ❌ None (completely automatic)

### Example 2: COINBASE (Automatic)

**Scenario:** Miner finds a block.

**What Happens Automatically:**
```
1. Miner creates block with coinbase transaction:
   - Type: COINBASE
   - To: Miner address
   - Amount: 50 GXC (block reward) + 0.5 GXC (fees)
   - Proof-of-work: block hash
2. Miner submits block to blockchain
3. Blockchain validates AUTOMATICALLY:
   ✅ Coinbase is first transaction
   ✅ Only one coinbase
   ✅ Reward = 50 + 0.5 = 50.5 GXC (valid)
   ✅ Proof-of-work valid
   ✅ Block height correct
4. Block accepted
5. Miner's balance increases by 50.5 GXC
6. Coins locked for 100 blocks (maturity)
```

**Miner's Action:** ❌ None (mining software handles it)

### Example 3: NORMAL (Manual)

**Scenario:** Alice wants to send 10 GXC to Bob.

**What Alice Must Do:**
```
1. Alice creates transaction:
   - Type: NORMAL
   - From: Alice
   - To: Bob
   - Amount: 10 GXC
   - prevTxHash: (where Alice got coins)
   - referencedAmount: (amount from previous tx)
2. Alice signs with private key
3. Alice broadcasts to network
```

**What Happens Automatically:**
```
4. Blockchain validates AUTOMATICALLY:
   ✅ Traceability formula valid
   ✅ Signature valid
   ✅ Alice has 10 GXC
   ✅ Balance correct
5. Transaction accepted
6. Added to mempool
7. Included in next block
8. Alice's balance decreases by 10 GXC
9. Bob's balance increases by 10 GXC
```

**Alice's Action:** ✅ Required (create, sign, broadcast)

---

## Conclusion

### Automatic Transactions (2):
1. ✅ **REWARD** - Blockchain creates after PoS blocks
2. ✅ **COINBASE** - Miner creates, blockchain validates

### Manual Transactions (6):
1. ❌ **NORMAL** - User creates and signs
2. ❌ **STAKE** - User creates and signs
3. ❌ **UNSTAKE** - User creates and signs
4. ❌ **Stock Tokens** - User creates and signs
5. ❌ **Gold Tokens** - User creates and signs
6. ❌ **Cross-Chain** - User creates and signs

### Automatic Validation (ALL):
✅ **Every transaction** is validated automatically by blockchain
✅ **Traceability** enforced automatically
✅ **Signatures** verified automatically
✅ **Balance** checked automatically
✅ **Type-specific rules** enforced automatically

**No manual intervention needed for validation!** 🤖
