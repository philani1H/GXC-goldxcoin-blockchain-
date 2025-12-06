# GXC Hybrid Consensus & Security Engine

## Overview

GXC uses a **Hybrid Proof-of-Work (PoW) and Proof-of-Stake (PoS)** consensus mechanism protected by an **AI-powered Security Engine**. This document explains how miners, validators, and the security engine work together to secure the blockchain.

## Core Principle: Miners + Validators = Secure Blockchain

```
Block Height 0: PoW (Mined)
Block Height 1: PoS (Validated)
Block Height 2: PoW (Mined)
Block Height 3: PoS (Validated)
...and so on
```

**Even heights = PoW blocks (Miners)**  
**Odd heights = PoS blocks (Validators)**

---

## 1. How Miners Work (PoW Blocks)

### Mining Process

```
1. Miner gets block template
   ↓
2. Miner solves hash puzzle (finds nonce)
   ↓
3. Miner submits block to node
   ↓
4. Node validates:
   - Hash meets difficulty target
   - Transactions are valid
   - Security engine checks for attacks
   ↓
5. Block accepted and added to chain
   ↓
6. Miner receives block reward
```

### PoW Validation

```cpp
bool Blockchain::validateProofOfWork(const Block& block) {
    // 1. Check hash meets difficulty target
    std::string hash = block.getHash();
    double validationDifficulty = difficulty;
    
    // 2. Verify hash has required leading zeros
    bool isValid = meetsTarget(hash, validationDifficulty);
    
    // 3. Security engine checks for hashrate attacks
    if (securityEngine->detectAttack(currentHashrate, blockTime)) {
        // Attack detected - reject or adjust difficulty
    }
    
    return isValid;
}
```

### What Miners Do

- ✅ Solve cryptographic puzzles (find valid nonce)
- ✅ Create blocks with transactions
- ✅ Receive block rewards
- ✅ Secure the network through computational work
- ✅ Compete based on hashrate

---

## 2. How Validators Work (PoS Blocks)

### Staking Process

```
1. User stakes coins (locks them)
   ↓
2. User registers as validator
   ↓
3. Validator becomes active
   ↓
4. Validator selected based on weighted stake
   ↓
5. Validator creates and signs block
   ↓
6. Node validates:
   - Validator is active
   - Validator has sufficient stake
   - Signature is valid
   ↓
7. Block accepted and added to chain
   ↓
8. Validator receives block reward
```

### PoS Validation

```cpp
bool Blockchain::validateProofOfStake(const Block& block) {
    // 1. Check block is PoS type
    if (block.getBlockType() != BlockType::POS) return false;
    
    // 2. Get validator address
    std::string validatorAddress = block.getMinerAddress();
    
    // 3. Check validator exists and is active
    auto it = validatorMap.find(validatorAddress);
    if (it == validatorMap.end()) return false;
    
    const Validator& validator = it->second;
    if (!validator.getIsActive()) return false;
    
    // 4. Verify validator signature
    std::string signature = block.getValidatorSignature();
    if (!verifyValidatorSignature(signature, validator)) return false;
    
    // 5. Check validator has sufficient stake
    if (validator.getStakeAmount() < MIN_STAKE) return false;
    
    return true;
}
```

### Validator Selection

```cpp
Validator Blockchain::selectValidatorForBlock() const {
    // Weighted random selection based on stake amount
    double totalWeightedStake = 0.0;
    
    // Calculate total weighted stake
    for (const auto& validator : validators) {
        if (validator.getIsActive()) {
            totalWeightedStake += validator.getWeightedStake();
        }
    }
    
    // Random selection weighted by stake
    double randomValue = (rand() / (double)RAND_MAX) * totalWeightedStake;
    double cumulativeStake = 0.0;
    
    for (const auto& validator : validators) {
        if (validator.getIsActive()) {
            cumulativeStake += validator.getWeightedStake();
            if (randomValue <= cumulativeStake) {
                return validator;  // Selected!
            }
        }
    }
    
    // Fallback to first validator
    return validators[0];
}
```

### What Validators Do

- ✅ Stake coins (lock them as collateral)
- ✅ Validate transactions
- ✅ Create and sign blocks
- ✅ Receive block rewards
- ✅ Secure the network through economic stake
- ✅ Selected based on weighted stake

---

## 3. Security Engine: Protecting the Blockchain

The **Security Engine** protects against attacks and ensures fair operation.

### Components

#### 1. AI Hashrate Sentinel
**Protects against:** 51% attacks, ASIC floods, botnet attacks

```cpp
double SecurityEngine::predictHashrate(double currentHashrate) {
    // Exponential moving average for prediction
    predictedHashrate_ = (PREDICTIVE_WEIGHT * currentHashrate) + 
                         (HISTORICAL_WEIGHT * predictedHashrate_);
    
    // Record in history
    hashrateHistory_.push_back(currentHashrate);
    if (hashrateHistory_.size() > HASHRATE_HISTORY_SIZE) {
        hashrateHistory_.erase(hashrateHistory_.begin());
    }
    
    return predictedHashrate_;
}

bool SecurityEngine::detectHashrateSurge(double currentHashrate) {
    double surgeFactor = currentHashrate / predictedHashrate_;
    
    // If hashrate suddenly increases by >12%, it's a surge
    if (surgeFactor > (1.0 + SURGE_THRESHOLD)) {
        return true;  // Attack detected!
    }
    
    return false;
}
```

**How it works:**
- Predicts expected hashrate based on history
- Detects sudden hashrate surges (>12% increase)
- Adjusts difficulty proactively to prevent attacks

#### 2. Predictive Difficulty Guard
**Protects against:** Fast block farming, difficulty manipulation

```cpp
double SecurityEngine::calculateNextDifficulty(
    double previousDifficulty,
    double currentHashrate,
    double timeTaken
) {
    // 1. Predict hashrate
    double predicted = predictHashrate(currentHashrate);
    
    // 2. Calculate target difficulty
    double targetDifficulty = previousDifficulty;
    
    // If block too fast, increase difficulty
    if (timeTaken < TARGET_BLOCK_TIME) {
        double speedFactor = TARGET_BLOCK_TIME / timeTaken;
        targetDifficulty *= speedFactor;
    }
    // If block too slow, decrease difficulty
    else if (timeTaken > TARGET_BLOCK_TIME) {
        double slowFactor = timeTaken / TARGET_BLOCK_TIME;
        targetDifficulty /= slowFactor;
    }
    
    // 3. Apply surge penalty if detected
    if (detectHashrateSurge(currentHashrate)) {
        targetDifficulty *= 1.5;  // 50% difficulty increase
    }
    
    // 4. Smooth the change (prevent oscillation)
    double smoothedDifficulty = smoothDifficultyChange(
        previousDifficulty, 
        targetDifficulty
    );
    
    // 5. Clamp to reasonable bounds
    return clamp(smoothedDifficulty, MIN_DIFFICULTY, 
                 previousDifficulty * MAX_DIFFICULTY_CHANGE);
}
```

**How it works:**
- Adjusts difficulty based on block time
- Applies penalty if hashrate surge detected
- Smooths changes to prevent oscillation
- Limits maximum change per block

#### 3. Staker-Balance Modifier
**Protects against:** Stake centralization, validator monopolies

```cpp
double SecurityEngine::applyStakerInfluence(double baseDifficulty) {
    double stakeRatio = getStakeRatio();
    
    // More stakers = higher difficulty = stronger chain
    double stakeInfluence = stakeRatio * STAKE_FACTOR;
    stakeInfluence = clamp(stakeInfluence, 0.0, MAX_STAKE_INFLUENCE);
    
    // Apply influence to difficulty
    return baseDifficulty * (1.0 + stakeInfluence);
}

double SecurityEngine::getStakeRatio() const {
    if (totalSupply_ <= 0) return 0.0;
    return totalStaked_ / totalSupply_;
}
```

**How it works:**
- More staked coins = higher difficulty
- Encourages staking participation
- Prevents single validator dominance
- Balances PoW and PoS security

#### 4. Emission Guard
**Protects against:** Inflation attacks, fast block farming

```cpp
double SecurityEngine::calculateBlockReward(double timeTaken) {
    double baseReward = BASE_REWARD;
    
    // If block mined too fast, reduce reward
    if (timeTaken < TARGET_BLOCK_TIME) {
        double timeFactor = timeTaken / TARGET_BLOCK_TIME;
        baseReward *= timeFactor;
    }
    
    // Clamp reward to reasonable bounds
    return clamp(baseReward, 
                 BASE_REWARD * MIN_REWARD_RATIO,
                 BASE_REWARD * MAX_REWARD_RATIO);
}
```

**How it works:**
- Fast blocks = reduced rewards
- Prevents inflation from rapid mining
- Maintains predictable emission schedule
- Discourages block farming

#### 5. Fee Surge Guard
**Protects against:** Fee manipulation, spam attacks

```cpp
double SecurityEngine::calculateDynamicFee(int mempoolSize) {
    // Base fee
    double fee = MIN_FEE;
    
    // Increase fee based on mempool congestion
    if (mempoolSize > 0) {
        fee += (mempoolSize * FEE_SCALE_FACTOR);
    }
    
    // Clamp to bounds
    return clamp(fee, MIN_FEE, MAX_FEE);
}
```

**How it works:**
- More pending transactions = higher fees
- Prevents spam attacks
- Ensures transaction priority
- Always within MIN_FEE and MAX_FEE bounds

#### 6. Hybrid Penalty Logic
**Protects against:** PoW/PoS imbalance, network centralization

```cpp
double SecurityEngine::calculateHybridPenalty(
    double minerReward,
    double stakerReward,
    uint32_t recentPoWBlocks,
    uint32_t recentPoSBlocks
) {
    // Calculate ratio of PoW to PoS blocks
    double totalBlocks = recentPoWBlocks + recentPoSBlocks;
    if (totalBlocks == 0) return 1.0;
    
    double powRatio = recentPoWBlocks / totalBlocks;
    double posRatio = recentPoSBlocks / totalBlocks;
    
    // Ideal ratio is 50/50
    double imbalance = std::abs(powRatio - 0.5);
    
    // Apply penalty if imbalanced
    if (imbalance > 0.2) {  // >20% imbalance
        return 0.9;  // 10% penalty
    }
    
    return 1.0;  // No penalty
}
```

**How it works:**
- Monitors PoW/PoS block ratio
- Applies penalty if imbalanced
- Encourages healthy participation
- Maintains hybrid consensus balance

---

## 4. Complete Block Validation Flow

### When a Block is Submitted

```
1. Block received from miner or validator
   ↓
2. Check block type (PoW or PoS)
   ↓
3. Validate consensus:
   - If PoW: validateProofOfWork()
   - If PoS: validateProofOfStake()
   ↓
4. Validate all transactions:
   - Check signatures
   - Verify UTXOs exist
   - Validate balances
   - Check traceability formula
   ↓
5. Security engine evaluation:
   - Detect hashrate attacks
   - Check for surges
   - Validate difficulty
   - Verify reward calculation
   ↓
6. Update UTXO set:
   - Remove spent inputs
   - Add new outputs
   - Update validator stakes
   ↓
7. Add block to chain
   ↓
8. Broadcast to network
   ↓
9. Update security metrics
```

### Code Flow

```cpp
bool Blockchain::addBlock(const Block& block) {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // 1. Validate block structure
    if (!validateBlockInternal(block, chain.size())) {
        return false;
    }
    
    // 2. Validate consensus (PoW or PoS)
    BlockType blockType = block.getBlockType();
    if (blockType == BlockType::POS) {
        if (!validateProofOfStake(block)) {
            return false;
        }
    } else {
        if (!validateProofOfWork(block)) {
            return false;
        }
    }
    
    // 3. Validate all transactions
    for (const auto& tx : block.getTransactions()) {
        if (!validateTransaction(tx)) {
            return false;
        }
    }
    
    // 4. Security engine check
    if (securityEngine) {
        double hashrate = calculateCurrentHashrate();
        double blockTime = block.getTimestamp() - lastBlock->getTimestamp();
        
        if (securityEngine->detectAttack(hashrate, blockTime)) {
            LOG_BLOCKCHAIN(LogLevel::WARNING, 
                "Security engine detected potential attack!");
            // Still accept block but log warning
        }
        
        // Update security metrics
        securityEngine->predictHashrate(hashrate);
        securityEngine->updateStakingMetrics(totalStaked, totalSupply);
    }
    
    // 5. Add block to chain
    chain.push_back(std::make_shared<Block>(block));
    lastBlock = chain.back();
    
    // 6. Update UTXO set
    updateUtxoSet(block);
    
    // 7. Broadcast to network
    if (network) {
        network->broadcastBlock(block);
    }
    
    return true;
}
```

---

## 5. Transaction Confirmation

### How Transactions Get Confirmed

```
1. User creates transaction
   ↓
2. Transaction submitted to node
   ↓
3. Node validates transaction:
   - Check signatures
   - Verify UTXOs exist and are unspent
   - Validate sum(inputs) >= sum(outputs) + fee
   - Check traceability formula
   ↓
4. Transaction added to mempool
   ↓
5. Miner/Validator includes transaction in block
   ↓
6. Block validated and added to chain
   ↓
7. Transaction confirmed (1 confirmation)
   ↓
8. More blocks added on top
   ↓
9. Transaction deeply confirmed (6+ confirmations)
```

### Confirmation Levels

- **0 confirmations** - In mempool, not yet in block
- **1 confirmation** - Included in latest block
- **3 confirmations** - Reasonably safe
- **6 confirmations** - Very safe (standard)
- **12+ confirmations** - Extremely safe

### Code

```cpp
uint32_t getConfirmations(const std::string& txHash) {
    // Find block containing transaction
    uint32_t txBlockHeight = findTransactionBlock(txHash);
    if (txBlockHeight == 0) return 0;  // Not found
    
    // Calculate confirmations
    uint32_t currentHeight = getHeight();
    return currentHeight - txBlockHeight + 1;
}
```

---

## 6. How Everything Works Together

### Scenario 1: Normal Operation

```
Block 100 (PoW):
  - Miner solves puzzle
  - Security engine: No attack detected
  - Difficulty: 1000
  - Reward: 12.5 GXC
  - Transactions: 50 confirmed
  ✅ Block accepted

Block 101 (PoS):
  - Validator selected (weighted by stake)
  - Validator signs block
  - Security engine: Stake ratio healthy
  - Reward: 12.5 GXC
  - Transactions: 45 confirmed
  ✅ Block accepted

Block 102 (PoW):
  - Miner solves puzzle
  - Security engine: No attack detected
  - Difficulty: 1005 (slight increase)
  - Reward: 12.5 GXC
  - Transactions: 60 confirmed
  ✅ Block accepted
```

### Scenario 2: Hashrate Attack Detected

```
Block 200 (PoW):
  - Miner solves puzzle
  - Security engine: HASHRATE SURGE DETECTED!
    - Current: 1000 TH/s
    - Predicted: 500 TH/s
    - Surge: 100% increase
  - Action: Increase difficulty by 50%
  - New difficulty: 1500
  - Reward: 12.5 GXC (normal)
  ✅ Block accepted with increased difficulty

Block 201 (PoS):
  - Validator selected
  - Security engine: Monitoring hashrate
  - Difficulty: 1500 (maintained)
  - Reward: 12.5 GXC
  ✅ Block accepted

Block 202 (PoW):
  - Attacker tries to mine fast
  - Security engine: Still detecting surge
  - Difficulty: 2250 (increased again)
  - Attacker's blocks rejected (too slow)
  ✅ Attack mitigated!
```

### Scenario 3: Fast Block Farming Attempt

```
Block 300 (PoW):
  - Miner finds block in 2 seconds (too fast!)
  - Security engine: Fast block detected
  - Action: Reduce reward
  - Reward: 12.5 * (2/10) = 2.5 GXC (reduced!)
  - Difficulty: Increased for next block
  ✅ Block accepted but reward reduced

Block 301 (PoS):
  - Validator creates block normally
  - Security engine: Normal timing
  - Reward: 12.5 GXC (normal)
  ✅ Block accepted

Block 302 (PoW):
  - Difficulty now higher
  - Miner takes 10 seconds (normal)
  - Reward: 12.5 GXC (normal)
  ✅ System stabilized
```

---

## 7. Key Guarantees

### For Miners
✅ Fair competition based on hashrate  
✅ Predictable rewards  
✅ Protection against ASIC floods  
✅ Difficulty adjusts to maintain 10-second blocks  

### For Validators
✅ Fair selection based on stake  
✅ Predictable rewards  
✅ Protection against stake centralization  
✅ Active participation required  

### For Users
✅ Transactions confirmed reliably  
✅ No double-spending possible  
✅ Traceability maintained  
✅ Security engine protects network  

### For the Network
✅ Hybrid consensus (PoW + PoS)  
✅ Attack detection and mitigation  
✅ Balanced participation  
✅ Predictable emission  
✅ Adaptive fees  

---

## 8. Summary

**Miners do what they do:**
- Mine PoW blocks (even heights)
- Solve cryptographic puzzles
- Receive block rewards
- Secure through computation

**Validators do what they do:**
- Validate PoS blocks (odd heights)
- Stake coins as collateral
- Sign blocks
- Secure through economic stake

**Security Engine does what it does:**
- Detects hashrate attacks
- Adjusts difficulty proactively
- Balances PoW/PoS participation
- Guards against inflation
- Manages dynamic fees
- Protects the blockchain 24/7

**All work together seamlessly:**
- Blocks alternate between PoW and PoS
- All transactions validated by both
- Security engine monitors everything
- Network remains secure and decentralized

**Everything is confirmed:**
- Transactions validated before mempool
- Blocks validated before acceptance
- UTXO set updated atomically
- Confirmations accumulate with each block
- Deep confirmations ensure finality

The GXC blockchain is a **secure, hybrid, AI-protected** system where miners, validators, and the security engine work together to maintain a robust and decentralized network! 🎉
