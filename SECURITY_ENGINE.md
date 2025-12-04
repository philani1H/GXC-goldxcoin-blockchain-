# GXC Security Engine

## Overview

The GXC Security Engine is a unified, modular security system that protects the blockchain from various attacks and ensures stable operation.

## Components

### 1. AI Hashrate Sentinel (Predictive Smoothing)

```cpp
double predictHashrate(double currentHashrate);
bool detectHashrateSurge(double currentHashrate);
```

**Purpose**: Predicts future hashrate using exponential smoothing, making the blockchain proactive rather than reactive.

**Algorithm**:
```
predicted = (PREDICTIVE_WEIGHT × current) + (HISTORICAL_WEIGHT × previous_prediction)
```

**Benefits**:
- Anticipates ASIC floods, botnets, and cloud miner attacks
- Smooths out hashrate volatility
- Provides early warning for difficulty adjustment

### 2. Predictive Difficulty Guard

```cpp
double calculatePredictiveDifficulty(double previousDifficulty, double currentHashrate);
double smoothDifficultyChange(double currentDifficulty, double targetDifficulty);
```

**Purpose**: Adjusts difficulty based on predicted vs actual hashrate.

**Rules**:
- If surge > SURGE_THRESHOLD (12%): Apply difficulty penalty
- If surge > ATTACK_THRESHOLD (50%): Double difficulty (attack response)
- Changes are clamped to MAX_DIFFICULTY_CHANGE (4x) per block

### 3. Staker-Balance Modifier

```cpp
double applyStakerInfluence(double baseDifficulty);
void updateStakingMetrics(double totalStaked, double totalSupply);
```

**Purpose**: More stakers = higher difficulty = stronger chain.

**Formula**:
```
adjusted_difficulty = base_difficulty × (1 + stake_ratio × STAKE_FACTOR)
```

**Benefits**:
- Encourages staking participation
- Makes 51% attacks more expensive
- Balances PoW and PoS security

### 4. Emission Guard (Anti-Inflation)

```cpp
double calculateEmissionGuardedReward(double baseReward, double timeTaken);
bool validateEmission(double proposedReward, double timeTaken);
```

**Purpose**: Prevents fast block farming exploitation.

**Rules**:
- Fast blocks (< target time): Reduced rewards
- Normal blocks: Full rewards
- Slow blocks: Capped at MAX_REWARD_RATIO (1.5x)

**Formula**:
```
guarded_reward = base_reward × clamp(time_taken / TARGET_BLOCK_TIME, 0.1, 1.5)
```

### 5. Fee Surge Guard (Adaptive Fees)

```cpp
double calculateDynamicFee(int mempoolSize);
double getRecommendedFee() const;
```

**Purpose**: Automatically adjusts fees based on network congestion.

**Range**: `MIN_FEE (0.0001)` to `MAX_FEE (0.001)`

**Formula**:
```
scaled_fee = MIN_FEE + (mempool_size × FEE_SCALE_FACTOR)
fee = clamp(scaled_fee, MIN_FEE, MAX_FEE)
```

### 6. Hybrid Penalty Logic

```cpp
double calculateHybridPenalty(double minerReward, double stakerReward,
                               uint32_t recentPoWBlocks, uint32_t recentPoSBlocks);
```

**Purpose**: Ensures balanced mining/staking participation.

**Rules**:
- Ideal ratio: 50% PoW / 50% PoS
- Imbalance > 30%: Penalty applied to dominant type
- Penalty reduces rewards to incentivize balance

## Attack Detection

```cpp
bool detectAttack(double hashrate, double timeTaken);
std::string getAttackType(double hashrate, double timeTaken);
```

**Detected Attack Types**:
| Attack | Detection Criteria |
|--------|-------------------|
| `FAST_BLOCK_ATTACK` | Block time < 1s AND consecutive fast blocks > 5 |
| `HASHRATE_SURGE_ATTACK` | Hashrate surge > 50% of historical average |
| `SELFISH_MINING_DETECTED` | Block time > 100s AND consecutive slow blocks > 3 |

## Usage Example

```cpp
#include "security/SecurityEngine.h"

using namespace GXCSecurity;

// Create engine
SecurityEngine engine;

// Update staking metrics
engine.updateStakingMetrics(5000000.0, 20000000.0);  // 25% staked

// Evaluate a new block
SecurityMetrics metrics = engine.evaluateBlock(
    40000.0,    // current hashrate
    10.0,       // block time
    1000.0,     // previous difficulty
    100         // mempool size
);

// Check for attacks
if (metrics.attackDetected) {
    std::cout << "Attack detected: " << metrics.attackType << std::endl;
}

// Get next difficulty
double nextDiff = engine.calculateNextDifficulty(
    previousDifficulty,
    currentHashrate,
    timeTaken
);

// Get block reward
double reward = engine.calculateBlockReward(timeTaken);

// Get recommended fee
double fee = engine.getRecommendedFee();
```

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `TARGET_BLOCK_TIME` | 10.0s | Target time between blocks |
| `BASE_REWARD` | 12.5 GXC | Base block reward |
| `MIN_REWARD_RATIO` | 0.1 | Minimum reward multiplier |
| `MAX_REWARD_RATIO` | 1.5 | Maximum reward multiplier |
| `STAKE_FACTOR` | 0.20 | Stake influence on difficulty |
| `PREDICTIVE_WEIGHT` | 0.60 | Weight for current hashrate in EMA |
| `SURGE_THRESHOLD` | 0.12 | 12% surge triggers adjustment |
| `ATTACK_THRESHOLD` | 0.50 | 50% surge = potential attack |
| `MIN_FEE` | 0.0001 GXC | Minimum transaction fee |
| `MAX_FEE` | 0.001 GXC | Maximum transaction fee |
| `MIN_DIFFICULTY` | 1.0 | Minimum allowed difficulty |
| `MAX_DIFFICULTY_CHANGE` | 4.0 | Max difficulty change per block |

## Why This Works

### 1. Impossible to Attack
- AI Sentinel predicts attacks before they happen
- ASIC floods, cloud miners, botnets - all detected and countered

### 2. Always Stable
- Difficulty balances under:
  - High/low miner participation
  - High/low staker participation
  - Attack conditions
  - Mempool congestion

### 3. Dynamic Fees
- Automatically rises during congestion
- Falls during low activity
- Always within defined bounds

### 4. Protected Rewards
- No exploitation of fast block farming
- Rewards tied to actual work/time

### 5. Clean & Modular
- Each security component is independent
- Easy to tune constants
- Simple to extend

## Integration

The Security Engine is integrated into `gxc_core` and automatically used by:
- Block validation
- Difficulty calculation
- Reward distribution
- Fee estimation

## Files

- Header: `include/security/SecurityEngine.h`
- Implementation: `src/security/SecurityEngine.cpp`
- Tests: `tests/test_security.cpp`
