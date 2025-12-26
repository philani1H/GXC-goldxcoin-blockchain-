# GXC Blockchain - Complete Traceability Design

## Overview

Every transaction in the GXC blockchain maintains a complete audit trail through traceability fields. This prevents fraud, enables feasibility checking, and provides full transparency.

---

## Core Traceability Formula

```
Ti.Inputs[0].txHash == Ti.PrevTxHash && 
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

**This ensures:**
1. Every transaction links to its predecessor
2. Amounts are verifiable
3. Complete chain of custody
4. Fraud prevention (can't claim coins you don't own)

---

## Transaction Types & Traceability

### 1. Regular Transfer Transaction

```
User A sends 100 GXC to User B

Transaction:
  Type: TRANSFER
  Inputs:
    - txHash: [previous transaction where A received coins]
    - outputIndex: 0
    - amount: 100 GXC
    - signature: [A's signature]
    - publicKey: [A's public key]
  Outputs:
    - address: B's address
    - amount: 100 GXC
  PrevTxHash: [same as input txHash]
  ReferencedAmount: 100 GXC
  
Traceability Chain:
  Mining → A receives → A sends to B → B receives
```

---

### 2. Staking Transaction

```
User stakes 500 GXC with Validator V

Transaction:
  Type: STAKE
  Inputs:
    - txHash: [previous transaction where user received 500 GXC]
    - outputIndex: 0
    - amount: 500 GXC
    - signature: [user's signature]
    - publicKey: [user's public key]
  Outputs: NONE (coins go to staking pool)
  PrevTxHash: [input txHash]
  ReferencedAmount: 500 GXC
  Metadata:
    - validatorAddress: V
    - lockPeriodDays: 30
    - stakeId: stake_abc123
    
Staking Pool Entry:
  stakeId: stake_abc123
  ownerAddress: user
  validatorAddress: V
  amount: 500 GXC
  stakeTxHash: [hash of this stake transaction]
  previousTxHash: [input txHash - where coins came from]
  referencedAmount: 500 GXC
  inputTxHashes: [all UTXOs consumed]
  
Traceability Chain:
  Mining → User receives → User stakes → Pool reserves
```

**Key Points:**
- ✅ Coins are RESERVED in pool (not destroyed)
- ✅ Complete history tracked (where coins came from)
- ✅ Can verify user actually owned these coins
- ✅ Prevents staking coins you don't have

---

### 3. Reward Transaction

```
Validator V produces block, user earns 15 GXC reward

Transaction:
  Type: REWARD
  Inputs:
    - txHash: [stake transaction hash]
    - outputIndex: 0
    - amount: 15 GXC
    - signature: [validator signature]
    - publicKey: [validator public key]
  Outputs:
    - address: user's address
    - amount: 15 GXC (NEW COINS)
  PrevTxHash: [stake transaction hash]
  ReferencedAmount: 15 GXC
  Metadata:
    - blockHash: [block that generated reward]
    - stakeTxHash: [stake that earned reward]
    - rewardType: "staking"
    - validatorAddress: V
    
Feasibility Check:
  1. Verify stake exists: stakeTxHash in staking pool
  2. Verify stake is active: not yet unstaked
  3. Calculate eligible reward:
     reward = (user_stake / total_validator_stake) * block_reward
  4. Verify amount matches calculation
  5. Verify validator actually produced the block
  
Traceability Chain:
  Stake → Block production → Reward minted → User receives
```

**Key Points:**
- ✅ NEW coins minted (controlled inflation)
- ✅ Linked to stake transaction (proves legitimacy)
- ✅ Feasibility checked (can't claim fake rewards)
- ✅ Separate from original stake (stake remains locked)

---

### 4. Unstake Transaction

```
User unstakes 500 GXC after lock period

Transaction:
  Type: UNSTAKE
  Inputs:
    - txHash: [stake transaction hash]
    - outputIndex: 0
    - amount: 500 GXC
    - signature: [user's signature]
    - publicKey: [user's public key]
  Outputs:
    - address: user's address
    - amount: 500 GXC (RETURNED from pool)
  PrevTxHash: [stake transaction hash]
  ReferencedAmount: 500 GXC
  Metadata:
    - stakeId: stake_abc123
    
Feasibility Check:
  1. Verify stake exists: stakeId in staking pool
  2. Verify ownership: user owns this stake
  3. Verify maturity: currentTime >= lockStartTime + lockPeriod
  4. Verify amount matches: unstakeAmount <= stakeAmount
  5. Verify not already unstaked: stake.isLocked == true
  
Staking Pool Update:
  - Remove stake entry
  - Update validator pool totals
  - Create UTXO for returned coins
  
Traceability Chain:
  Original coins → Stake → Pool reserves → Unstake → User receives (SAME COINS)
```

**Key Points:**
- ✅ RETURNS original coins (doesn't mint)
- ✅ Links to stake transaction (proves ownership)
- ✅ Feasibility checked (can't unstake what you didn't stake)
- ✅ Complete audit trail maintained

---

## Complete Staking Flow with Traceability

### Step 1: User Receives Coins from Mining

```
Block 100 mined by user

Coinbase Transaction:
  Type: COINBASE
  Inputs: NONE (new coins)
  Outputs:
    - address: user
    - amount: 50 GXC
  TxHash: tx_mining_100
  
Supply: +50 GXC (inflation)
```

### Step 2: User Stakes Coins

```
User stakes 50 GXC with Validator V

Stake Transaction:
  Type: STAKE
  Inputs:
    - txHash: tx_mining_100
    - amount: 50 GXC
  Outputs: NONE
  PrevTxHash: tx_mining_100
  ReferencedAmount: 50 GXC
  TxHash: tx_stake_200
  
Staking Pool:
  + stake_abc123:
      amount: 50 GXC
      stakeTxHash: tx_stake_200
      previousTxHash: tx_mining_100
      
UTXO Set:
  - Remove: tx_mining_100_0
  
Supply: UNCHANGED (coins reserved, not destroyed)
```

### Step 3: Validator Produces Block, User Earns Reward

```
Validator V produces block 300

Reward Transaction:
  Type: REWARD
  Inputs:
    - txHash: tx_stake_200 (link to stake)
    - amount: 5 GXC
  Outputs:
    - address: user
    - amount: 5 GXC
  PrevTxHash: tx_stake_200
  ReferencedAmount: 5 GXC
  TxHash: tx_reward_300
  Metadata:
    - blockHash: block_300
    - stakeTxHash: tx_stake_200
    
UTXO Set:
  + Add: tx_reward_300_0 (5 GXC to user)
  
Supply: +5 GXC (NEW coins minted)
```

### Step 4: User Unstakes After Lock Period

```
Lock period expires, user unstakes

Unstake Transaction:
  Type: UNSTAKE
  Inputs:
    - txHash: tx_stake_200 (link to original stake)
    - amount: 50 GXC
  Outputs:
    - address: user
    - amount: 50 GXC
  PrevTxHash: tx_stake_200
  ReferencedAmount: 50 GXC
  TxHash: tx_unstake_400
  Metadata:
    - stakeId: stake_abc123
    
Staking Pool:
  - Remove: stake_abc123
  
UTXO Set:
  + Add: tx_unstake_400_0 (50 GXC to user)
  
Supply: UNCHANGED (coins returned, not minted)
```

### Final State

```
User's UTXOs:
  - tx_reward_300_0: 5 GXC (reward)
  - tx_unstake_400_0: 50 GXC (returned stake)
  Total: 55 GXC

Supply Changes:
  Block 100: +50 GXC (mining)
  Block 300: +5 GXC (reward)
  Total: +55 GXC

Traceability Chain:
  tx_mining_100 → tx_stake_200 → tx_reward_300 (NEW)
                              → tx_unstake_400 (RETURNED)
```

---

## Feasibility Checking

### For Staking

```cpp
bool validateStakeTransaction(const Transaction& tx) {
    // 1. Verify inputs are valid UTXOs
    for (const auto& input : tx.getInputs()) {
        if (!utxoExists(input.txHash, input.outputIndex)) {
            return false;  // Can't stake coins you don't have
        }
    }
    
    // 2. Verify amounts match
    double inputTotal = sumInputs(tx);
    if (inputTotal != tx.getReferencedAmount()) {
        return false;  // Amount mismatch
    }
    
    // 3. Verify signature
    if (!verifySignature(tx)) {
        return false;  // Not the owner
    }
    
    // 4. Verify validator exists
    if (!validatorExists(tx.getMetadata("validatorAddress"))) {
        return false;  // Invalid validator
    }
    
    return true;
}
```

### For Rewards

```cpp
bool validateRewardTransaction(const Transaction& tx) {
    std::string stakeTxHash = tx.getInputs()[0].txHash;
    std::string blockHash = tx.getMetadata("blockHash");
    double rewardAmount = tx.getReferencedAmount();
    
    // 1. Verify stake exists
    if (!stakingPool->stakeExists(stakeTxHash)) {
        return false;  // Fake stake
    }
    
    // 2. Verify stake is still active
    auto stake = stakingPool->getStake(stakeTxHash);
    if (!stake.isLocked) {
        return false;  // Already unstaked
    }
    
    // 3. Verify block was produced by validator
    Block block = getBlock(blockHash);
    if (block.getValidatorAddress() != stake.validatorAddress) {
        return false;  // Wrong validator
    }
    
    // 4. Verify reward amount is correct
    double expectedReward = calculateReward(stake, block);
    if (abs(rewardAmount - expectedReward) > 0.0001) {
        return false;  // Wrong amount
    }
    
    // 5. Verify reward not already distributed
    if (rewardAlreadyPaid(stakeTxHash, blockHash)) {
        return false;  // Double reward
    }
    
    return true;
}
```

### For Unstaking

```cpp
bool validateUnstakeTransaction(const Transaction& tx) {
    std::string stakeTxHash = tx.getInputs()[0].txHash;
    std::string stakeId = tx.getMetadata("stakeId");
    double unstakeAmount = tx.getReferencedAmount();
    
    // 1. Verify stake exists
    if (!stakingPool->stakeExists(stakeId)) {
        return false;  // Fake stake
    }
    
    // 2. Verify ownership
    auto stake = stakingPool->getStake(stakeId);
    if (stake.ownerAddress != tx.getSenderAddress()) {
        return false;  // Not the owner
    }
    
    // 3. Verify maturity
    if (!stakingPool->isStakeMature(stakeId)) {
        return false;  // Lock period not expired
    }
    
    // 4. Verify amount
    if (unstakeAmount > stake.amount) {
        return false;  // Can't unstake more than staked
    }
    
    // 5. Verify not already unstaked
    if (!stake.isLocked) {
        return false;  // Already unstaked
    }
    
    // 6. Verify signature
    if (!verifySignature(tx)) {
        return false;  // Not the owner
    }
    
    return true;
}
```

---

## Supply Integrity Verification

```cpp
struct SupplyAudit {
    double totalMined;           // From coinbase transactions
    double totalRewards;         // From reward transactions
    double totalBurned;          // From slashing/fees
    double circulatingSupply;    // In UTXOs
    double stakedSupply;         // In staking pool
    double expectedTotal;        // totalMined + totalRewards - totalBurned
    double actualTotal;          // circulatingSupply + stakedSupply
};

SupplyAudit auditSupply() {
    SupplyAudit audit;
    
    // Count all minted coins
    audit.totalMined = countCoinbaseTransactions();
    audit.totalRewards = countRewardTransactions();
    audit.totalBurned = countBurnedCoins();
    
    // Count all existing coins
    audit.circulatingSupply = sumAllUTXOs();
    audit.stakedSupply = stakingPool->getTotalStaked();
    
    // Calculate expected vs actual
    audit.expectedTotal = audit.totalMined + audit.totalRewards - audit.totalBurned;
    audit.actualTotal = audit.circulatingSupply + audit.stakedSupply;
    
    // Verify integrity
    if (abs(audit.expectedTotal - audit.actualTotal) > 0.0001) {
        LOG_ERROR("Supply integrity violation!");
        LOG_ERROR("Expected: " + std::to_string(audit.expectedTotal));
        LOG_ERROR("Actual: " + std::to_string(audit.actualTotal));
    }
    
    return audit;
}
```

---

## Benefits of Traceability

### 1. Fraud Prevention
- ✅ Can't stake coins you don't own
- ✅ Can't claim rewards you didn't earn
- ✅ Can't unstake more than you staked
- ✅ Can't double-spend staked coins

### 2. Audit Trail
- ✅ Complete history of every coin
- ✅ Transparent reward distribution
- ✅ Verifiable validator performance
- ✅ Supply integrity maintained

### 3. Feasibility Checking
- ✅ Verify rewards are legitimate
- ✅ Verify stakes are valid
- ✅ Verify unstakes are mature
- ✅ Prevent gaming the system

### 4. Transparency
- ✅ Anyone can verify transactions
- ✅ Clear chain of custody
- ✅ Provable ownership
- ✅ Auditable supply

---

## Implementation Checklist

- [x] Add traceability fields to StakeEntry
- [x] Update addStake to accept traceability info
- [x] Fix createUnstakeTransaction to include previous tx hash
- [x] Create createRewardTransaction with traceability
- [ ] Update Blockchain to validate traceability
- [ ] Implement feasibility checking
- [ ] Add supply audit function
- [ ] Test complete traceability chain
- [ ] Document traceability for users

---

## Summary

**Traceability is CRITICAL because:**

1. **Staking:** Links to original coins, proves ownership
2. **Rewards:** Links to stake, proves feasibility
3. **Unstaking:** Links to stake, proves legitimacy
4. **Supply:** Maintains integrity, prevents inflation bugs

**Every transaction MUST include:**
- Previous transaction hash
- Referenced amount
- Proper signatures
- Complete input/output chain

**This ensures:**
- ✅ No fake coins
- ✅ No double-spending
- ✅ No unauthorized rewards
- ✅ Complete audit trail
- ✅ Supply integrity

The traceability system is the foundation of trust in the GXC blockchain.
