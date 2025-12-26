# GXC Staking System - Complete Design

## Executive Summary

**CRITICAL ISSUE IDENTIFIED:** Current unstake implementation creates NEW coins, violating supply integrity.

**Solution:** Implement proper staking pool system where:
1. Staked coins are RESERVED, not destroyed
2. Unstaking RETURNS original coins, doesn't mint new ones
3. Only REWARDS are newly minted coins
4. Delegation allows users to stake with validators
5. Lock periods prevent gaming the system

---

## 1. Core Principles

### Supply Integrity
```
Total Supply = Circulating + Staked + Locked
```

**When staking:**
- Coins move from "Circulating" to "Staked"
- Total supply UNCHANGED

**When unstaking:**
- Coins move from "Staked" back to "Circulating"
- Total supply UNCHANGED

**When rewards distributed:**
- NEW coins created (inflation)
- Total supply INCREASES

### Key Rules
1. ✅ Staking = RESERVE coins (don't destroy)
2. ✅ Unstaking = RETURN reserved coins (don't mint)
3. ✅ Rewards = MINT new coins (controlled inflation)
4. ✅ Lock period = ENFORCE minimum staking time
5. ✅ Slashing = BURN misbehaving validator's stake

---

## 2. Staking Pool Architecture

### A. Validator Pool Structure

```cpp
class ValidatorPool {
private:
    std::string validatorAddress;
    double validatorOwnStake;           // Validator's own coins
    
    // Delegator tracking
    struct Delegator {
        std::string address;
        double stakedAmount;
        uint64_t stakeStartTime;
        uint64_t lockPeriodDays;
        double accumulatedRewards;
    };
    std::unordered_map<std::string, Delegator> delegators;
    
    // Pool statistics
    double totalPoolStake;              // Validator + all delegators
    uint32_t blocksProduced;
    uint32_t missedBlocks;
    double totalRewardsDistributed;
    
public:
    // Delegation
    bool addDelegator(const std::string& address, double amount, uint32_t lockDays);
    bool removeDelegator(const std::string& address);
    
    // Rewards
    void distributeRewards(double blockReward);
    double calculateDelegatorShare(const std::string& address, double totalReward);
    
    // Pool stats
    double getTotalStake() const { return totalPoolStake; }
    size_t getDelegatorCount() const { return delegators.size(); }
};
```

### B. Global Staking Pool Ledger

```cpp
class StakingPoolLedger {
private:
    // Track ALL staked coins
    struct StakeEntry {
        std::string ownerAddress;
        std::string validatorAddress;   // Which validator they're staking with
        double amount;
        uint64_t lockStartTime;
        uint32_t lockPeriodDays;
        bool isLocked;
    };
    
    std::unordered_map<std::string, StakeEntry> stakes;  // key = stakeId
    double totalStakedCoins;                              // Sum of all stakes
    
public:
    // Staking operations
    std::string createStake(const std::string& owner, const std::string& validator, 
                           double amount, uint32_t lockDays);
    bool releaseStake(const std::string& stakeId);
    bool isStakeMature(const std::string& stakeId) const;
    
    // Query
    double getTotalStaked() const { return totalStakedCoins; }
    std::vector<StakeEntry> getStakesForAddress(const std::string& address) const;
};
```

---

## 3. Transaction Flow

### A. Staking Flow (CORRECT)

```
User Wallet: 1000 GXC
User wants to stake 500 GXC with Validator A for 30 days

Step 1: Create STAKE transaction
  - Type: STAKE
  - Inputs: 500 GXC from user's UTXOs (mature coins only)
  - Outputs: NONE (coins go to staking pool)
  - Metadata: validatorAddress, lockPeriod

Step 2: Process transaction
  - Consume 500 GXC from UTXO set
  - Add entry to StakingPoolLedger:
    {
      owner: userAddress,
      validator: validatorA,
      amount: 500,
      lockStart: currentTime,
      lockDays: 30
    }
  - Update ValidatorPool totalStake

Result:
  User Wallet: 500 GXC (circulating)
  Staking Pool: 500 GXC (reserved for user)
  Total Supply: UNCHANGED
```

### B. Unstaking Flow (CORRECT)

```
User has 500 GXC staked, lock period expired

Step 1: Check maturity
  - currentTime >= lockStart + (lockDays * 86400)
  - If not mature: REJECT

Step 2: Create UNSTAKE transaction
  - Type: UNSTAKE
  - Inputs: Reference to stake entry (NOT a UTXO)
  - Outputs: 500 GXC to user's address
  - Metadata: stakeId

Step 3: Process transaction
  - Remove entry from StakingPoolLedger
  - Create NEW UTXO: 500 GXC to userAddress
  - Update ValidatorPool totalStake

Result:
  User Wallet: 1000 GXC (500 original + 500 returned)
  Staking Pool: 0 GXC
  Total Supply: UNCHANGED
  
Note: The 500 GXC returned are the SAME coins that were staked,
      just moved from pool back to circulation.
```

### C. Reward Distribution (NEW COINS)

```
Validator A produces a block
Block reward: 50 GXC

Validator A pool:
  - Validator own stake: 1000 GXC
  - Delegator 1: 500 GXC
  - Delegator 2: 200 GXC
  - Total: 1700 GXC

Reward distribution:
  - Validator: (1000/1700) * 50 = 29.41 GXC
  - Delegator 1: (500/1700) * 50 = 14.71 GXC
  - Delegator 2: (200/1700) * 50 = 5.88 GXC

Step 1: Mint NEW coins (50 GXC)
Step 2: Create reward transactions
  - To validatorA: 29.41 GXC (new UTXO)
  - To delegator1: 14.71 GXC (new UTXO)
  - To delegator2: 5.88 GXC (new UTXO)

Result:
  Total Supply: INCREASED by 50 GXC (inflation)
  Rewards go directly to wallets (spendable immediately)
  Original stakes remain locked in pool
```

---

## 4. Database Schema

### A. Staking Pool Table

```sql
CREATE TABLE staking_pool (
    stake_id TEXT PRIMARY KEY,
    owner_address TEXT NOT NULL,
    validator_address TEXT NOT NULL,
    amount REAL NOT NULL,
    lock_start_time INTEGER NOT NULL,
    lock_period_days INTEGER NOT NULL,
    is_locked BOOLEAN DEFAULT TRUE,
    created_at INTEGER NOT NULL,
    INDEX idx_owner (owner_address),
    INDEX idx_validator (validator_address)
);
```

### B. Validator Pools Table

```sql
CREATE TABLE validator_pools (
    validator_address TEXT PRIMARY KEY,
    own_stake REAL NOT NULL,
    total_delegated REAL NOT NULL,
    total_stake REAL NOT NULL,
    delegator_count INTEGER NOT NULL,
    blocks_produced INTEGER DEFAULT 0,
    missed_blocks INTEGER DEFAULT 0,
    total_rewards REAL DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at INTEGER NOT NULL
);
```

### C. Delegators Table

```sql
CREATE TABLE delegators (
    id TEXT PRIMARY KEY,
    delegator_address TEXT NOT NULL,
    validator_address TEXT NOT NULL,
    staked_amount REAL NOT NULL,
    accumulated_rewards REAL DEFAULT 0,
    stake_start_time INTEGER NOT NULL,
    lock_period_days INTEGER NOT NULL,
    INDEX idx_delegator (delegator_address),
    INDEX idx_validator (validator_address)
);
```

---

## 5. Implementation Plan

### Phase 1: Core Staking Pool (Priority 1)

**File:** `include/StakingPool.h`, `src/StakingPool.cpp`

```cpp
class StakingPool {
private:
    struct StakeEntry {
        std::string stakeId;
        std::string ownerAddress;
        std::string validatorAddress;
        double amount;
        uint64_t lockStartTime;
        uint32_t lockPeriodDays;
        bool isLocked;
    };
    
    std::unordered_map<std::string, StakeEntry> stakes;
    double totalStaked;
    
public:
    // Core operations
    std::string addStake(const std::string& owner, const std::string& validator,
                        double amount, uint32_t lockDays);
    bool removeStake(const std::string& stakeId);
    bool isStakeMature(const std::string& stakeId) const;
    
    // Queries
    double getTotalStaked() const;
    double getStakedByAddress(const std::string& address) const;
    std::vector<StakeEntry> getStakesForAddress(const std::string& address) const;
    std::vector<StakeEntry> getStakesForValidator(const std::string& validator) const;
};
```

### Phase 2: Fix Wallet Staking Methods

**File:** `src/Wallet.cpp`

**BEFORE (WRONG):**
```cpp
Transaction Wallet::createUnstakeTransaction(double unstakeAmount, double fee) {
    // Creates NEW coins - WRONG!
    TransactionOutput unstakeOutput;
    unstakeOutput.address = address;
    unstakeOutput.amount = unstakeAmount;  // Minting new coins!
    outputs.push_back(unstakeOutput);
}
```

**AFTER (CORRECT):**
```cpp
Transaction Wallet::createUnstakeTransaction(const std::string& stakeId, double fee) {
    // Returns RESERVED coins - CORRECT!
    
    // 1. Verify stake exists and is mature
    if (!stakingPool->isStakeMature(stakeId)) {
        throw std::runtime_error("Stake not yet mature");
    }
    
    // 2. Get stake details
    auto stake = stakingPool->getStake(stakeId);
    
    // 3. Create transaction that references the stake
    Transaction tx;
    tx.setType(TransactionType::UNSTAKE);
    tx.setMetadata("stakeId", stakeId);
    
    // 4. Output returns the staked amount
    TransactionOutput output;
    output.address = stake.ownerAddress;
    output.amount = stake.amount;  // Original staked coins
    tx.addOutput(output);
    
    // 5. Sign to prove ownership
    tx.sign(privateKey);
    
    return tx;
}
```

### Phase 3: Update Blockchain Validation

**File:** `src/Blockchain.cpp`

```cpp
bool Blockchain::validateTransaction(const Transaction& tx) {
    if (tx.getType() == TransactionType::STAKE) {
        // Validate staking transaction
        // 1. Check inputs are mature UTXOs
        // 2. Verify validator exists
        // 3. Check minimum stake amount
        // 4. Verify lock period is valid
        return validateStakeTransaction(tx);
    }
    
    if (tx.getType() == TransactionType::UNSTAKE) {
        // Validate unstaking transaction
        // 1. Verify stake exists
        // 2. Check stake is mature
        // 3. Verify ownership
        // 4. Ensure stake not already unstaked
        return validateUnstakeTransaction(tx);
    }
    
    // ... other validations
}

bool Blockchain::processTransaction(const Transaction& tx) {
    if (tx.getType() == TransactionType::STAKE) {
        // 1. Consume UTXOs (remove from UTXO set)
        for (const auto& input : tx.getInputs()) {
            utxoSet.erase(input.txHash + "_" + std::to_string(input.outputIndex));
        }
        
        // 2. Add to staking pool (reserve coins)
        std::string stakeId = stakingPool->addStake(
            tx.getSenderAddress(),
            tx.getMetadata("validatorAddress"),
            tx.getReferencedAmount(),
            std::stoi(tx.getMetadata("lockPeriodDays"))
        );
        
        // 3. Update validator pool
        auto validator = getValidator(tx.getMetadata("validatorAddress"));
        validator->addDelegatedStake(tx.getSenderAddress(), tx.getReferencedAmount());
        
        return true;
    }
    
    if (tx.getType() == TransactionType::UNSTAKE) {
        // 1. Get stake details
        std::string stakeId = tx.getMetadata("stakeId");
        auto stake = stakingPool->getStake(stakeId);
        
        // 2. Remove from staking pool (release coins)
        stakingPool->removeStake(stakeId);
        
        // 3. Create UTXO (return coins to circulation)
        for (const auto& output : tx.getOutputs()) {
            std::string utxoKey = tx.getHash() + "_" + std::to_string(0);
            utxoSet[utxoKey] = output;
        }
        
        // 4. Update validator pool
        auto validator = getValidator(stake.validatorAddress);
        validator->removeDelegatedStake(stake.ownerAddress, stake.amount);
        
        return true;
    }
    
    // ... other processing
}
```

### Phase 4: Reward Distribution

**File:** `src/Blockchain.cpp`

```cpp
void Blockchain::distributeBlockRewards(const Block& block) {
    double blockReward = calculateBlockReward(block.getIndex());
    std::string validatorAddress = block.getValidatorAddress();
    
    // Get validator pool
    auto validator = getValidator(validatorAddress);
    auto pool = validator->getPool();
    
    // Calculate shares
    double totalStake = pool->getTotalStake();
    
    // Validator's share
    double validatorStake = validator->getOwnStake();
    double validatorReward = (validatorStake / totalStake) * blockReward;
    
    // Create reward transaction for validator
    Transaction validatorRewardTx = createRewardTransaction(
        validatorAddress,
        validatorReward,
        block.getHash()
    );
    addTransaction(validatorRewardTx);
    
    // Delegators' shares
    for (const auto& [delegatorAddr, delegator] : pool->getDelegators()) {
        double delegatorReward = (delegator.stakedAmount / totalStake) * blockReward;
        
        Transaction delegatorRewardTx = createRewardTransaction(
            delegatorAddr,
            delegatorReward,
            block.getHash()
        );
        addTransaction(delegatorRewardTx);
    }
    
    // Update total supply (NEW coins minted)
    totalSupply += blockReward;
}

Transaction Blockchain::createRewardTransaction(const std::string& recipient,
                                               double amount,
                                               const std::string& blockHash) {
    Transaction tx;
    tx.setType(TransactionType::REWARD);
    tx.setMetadata("blockHash", blockHash);
    
    // No inputs (new coins)
    
    // Output to recipient
    TransactionOutput output;
    output.address = recipient;
    output.amount = amount;
    tx.addOutput(output);
    
    return tx;
}
```

### Phase 5: Lock Period Enforcement

**File:** `src/StakingPool.cpp`

```cpp
bool StakingPool::isStakeMature(const std::string& stakeId) const {
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        return false;
    }
    
    const auto& stake = it->second;
    
    // Calculate maturity time
    uint64_t lockDurationSeconds = stake.lockPeriodDays * 86400;
    uint64_t maturityTime = stake.lockStartTime + lockDurationSeconds;
    uint64_t currentTime = Utils::getCurrentTimestamp();
    
    return currentTime >= maturityTime;
}

bool StakingPool::canUnstake(const std::string& stakeId) const {
    if (!isStakeMature(stakeId)) {
        return false;
    }
    
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        return false;
    }
    
    return it->second.isLocked;  // Can only unstake if still locked
}
```

---

## 6. RPC Methods

### A. Stake Coins

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"stake",
    "params":[
      "validatorAddress",
      500.0,
      30
    ],
    "id":1
  }'
```

**Response:**
```json
{
  "success": true,
  "stakeId": "stake_abc123",
  "amount": 500.0,
  "validator": "validatorAddress",
  "lockPeriod": 30,
  "maturityDate": "2025-01-25T00:00:00Z",
  "txHash": "0x..."
}
```

### B. Unstake Coins

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"unstake",
    "params":["stake_abc123"],
    "id":1
  }'
```

**Response:**
```json
{
  "success": true,
  "stakeId": "stake_abc123",
  "amount": 500.0,
  "returned": true,
  "txHash": "0x..."
}
```

### C. Get Staking Info

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"getstakinginfo",
    "params":["userAddress"],
    "id":1
  }'
```

**Response:**
```json
{
  "address": "userAddress",
  "totalStaked": 500.0,
  "activeStakes": [
    {
      "stakeId": "stake_abc123",
      "validator": "validatorAddress",
      "amount": 500.0,
      "lockStart": "2024-12-26T00:00:00Z",
      "lockPeriod": 30,
      "maturityDate": "2025-01-25T00:00:00Z",
      "isMature": false,
      "daysRemaining": 10
    }
  ],
  "totalRewards": 15.5,
  "pendingRewards": 2.3
}
```

---

## 7. Supply Tracking

### Before Staking
```
Total Supply: 1,000,000 GXC
Circulating: 1,000,000 GXC
Staked: 0 GXC
```

### After User Stakes 500 GXC
```
Total Supply: 1,000,000 GXC (UNCHANGED)
Circulating: 999,500 GXC
Staked: 500 GXC
```

### After Block Reward (50 GXC)
```
Total Supply: 1,000,050 GXC (INCREASED)
Circulating: 999,550 GXC (rewards added)
Staked: 500 GXC (unchanged)
```

### After Unstaking
```
Total Supply: 1,000,050 GXC (UNCHANGED)
Circulating: 1,000,050 GXC
Staked: 0 GXC
```

---

## 8. Security Considerations

### A. Slashing

```cpp
void Validator::slash(double amount, const std::string& reason) {
    // Reduce validator's own stake
    if (amount > validatorOwnStake) {
        amount = validatorOwnStake;
    }
    
    validatorOwnStake -= amount;
    slashedAmount += amount;
    
    // Burn slashed coins (reduce total supply)
    blockchain->burnCoins(amount);
    
    // Log slashing event
    LOG_VALIDATOR(LogLevel::WARNING, 
        "Validator " + address + " slashed " + std::to_string(amount) + 
        " GXC. Reason: " + reason);
}
```

### B. Delegator Protection

```cpp
// Delegators are NOT slashed for validator misbehavior
// Only validator's own stake is at risk
// Delegators can withdraw if validator performance is poor
```

### C. Lock Period Bypass Prevention

```cpp
bool StakingPool::canUnstake(const std::string& stakeId) const {
    // Check maturity
    if (!isStakeMature(stakeId)) {
        return false;
    }
    
    // Check not already unstaked
    auto it = stakes.find(stakeId);
    if (it == stakes.end() || !it->second.isLocked) {
        return false;
    }
    
    // Verify ownership in transaction validation
    return true;
}
```

---

## 9. Testing Checklist

- [ ] Stake coins and verify supply unchanged
- [ ] Unstake before maturity (should fail)
- [ ] Unstake after maturity (should succeed)
- [ ] Verify returned coins are spendable
- [ ] Test reward distribution
- [ ] Verify rewards increase supply
- [ ] Test multiple delegators to same validator
- [ ] Test validator slashing
- [ ] Verify delegators not affected by slashing
- [ ] Test lock period enforcement
- [ ] Test double unstake prevention
- [ ] Verify UTXO maturity for staking

---

## 10. Summary

### ✅ Correct Design

1. **Staking:** Coins RESERVED in pool, supply unchanged
2. **Unstaking:** Coins RETURNED from pool, supply unchanged
3. **Rewards:** NEW coins minted, supply increases
4. **Lock Period:** Enforced minimum staking time
5. **Delegation:** Users stake with validators
6. **Slashing:** Only validator's stake at risk

### ❌ Wrong Design (Current)

1. **Unstaking:** Creates NEW coins (inflates supply incorrectly)
2. **No Pool:** Coins not tracked properly
3. **No Delegation:** Can't stake with validators
4. **No Lock Period:** Can unstake immediately

### 🎯 Implementation Priority

1. **Phase 1:** Create StakingPool class
2. **Phase 2:** Fix Wallet staking methods
3. **Phase 3:** Update Blockchain validation
4. **Phase 4:** Implement reward distribution
5. **Phase 5:** Add lock period enforcement
6. **Phase 6:** Test complete flow

---

## Conclusion

The current implementation violates supply integrity by minting new coins during unstaking. The correct approach is to maintain a staking pool ledger that tracks reserved coins, returning them (not minting) when users unstake. Only rewards should create new coins, implementing controlled inflation for network security.

This design ensures:
- ✅ Supply integrity maintained
- ✅ Proper delegation support
- ✅ Lock periods enforced
- ✅ Validator accountability
- ✅ Delegator protection
- ✅ Transparent reward distribution
