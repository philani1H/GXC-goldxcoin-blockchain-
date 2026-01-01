/**
 * GXC Security Engine - Implementation
 * 
 * Full implementation of all security algorithms:
 * - AI Hashrate Sentinel
 * - Predictive Difficulty Guard
 * - Staker-Balance Modifier
 * - Emission Guard
 * - Fee Surge Guard
 * - Hybrid Penalty Logic
 */

#include "../../include/security/SecurityEngine.h"
#include "../../include/Config.h"
#include <numeric>
#include <cmath>

namespace GXCSecurity {

// =========================================================
//                  CONSTRUCTOR
// =========================================================

SecurityEngine::SecurityEngine()
    : targetBlockTime_(static_cast<double>(Config::getBlockTime()))
    , predictedHashrate_(0.0)
    , historicalHashrate_(0.0)
    , consecutiveFastBlocks_(0)
    , consecutiveSlowBlocks_(0)
    , lastDifficulty_(MIN_DIFFICULTY)
    , totalStaked_(0.0)
    , totalSupply_(0.0)
    , currentMempoolSize_(0)
{
    hashrateHistory_.reserve(HASHRATE_HISTORY_SIZE);
}

// =========================================================
//           1) AI HASHRATE SENTINEL
// =========================================================

double SecurityEngine::predictHashrate(double currentHashrate) {
    // If no history, initialize with current
    if (predictedHashrate_ == 0.0) {
        predictedHashrate_ = currentHashrate;
        historicalHashrate_ = currentHashrate;
        return currentHashrate;
    }
    
    // Exponential smoothing prediction
    // predicted = (WEIGHT * current) + ((1 - WEIGHT) * previous_prediction)
    predictedHashrate_ = (PREDICTIVE_WEIGHT * currentHashrate) +
                         (HISTORICAL_WEIGHT * predictedHashrate_);
    
    // Update historical average
    hashrateHistory_.push_back(currentHashrate);
    if (hashrateHistory_.size() > HASHRATE_HISTORY_SIZE) {
        hashrateHistory_.erase(hashrateHistory_.begin());
    }
    
    if (!hashrateHistory_.empty()) {
        historicalHashrate_ = std::accumulate(hashrateHistory_.begin(), 
                                               hashrateHistory_.end(), 0.0) 
                              / hashrateHistory_.size();
    }
    
    return predictedHashrate_;
}

bool SecurityEngine::detectHashrateSurge(double currentHashrate) {
    if (historicalHashrate_ == 0.0) return false;
    
    double surge = (currentHashrate - historicalHashrate_) / historicalHashrate_;
    return surge > SURGE_THRESHOLD;
}

// =========================================================
//           2) PREDICTIVE DIFFICULTY GUARD
// =========================================================

double SecurityEngine::calculatePredictiveDifficulty(double previousDifficulty,
                                                      double currentHashrate) {
    if (currentHashrate <= 0.0) return previousDifficulty;
    
    // Predict next hashrate
    double predicted = predictHashrate(currentHashrate);
    
    // Calculate surge (predicted vs actual)
    double surge = (predicted - currentHashrate) / currentHashrate;
    
    double newDifficulty = previousDifficulty;
    
    if (surge > SURGE_THRESHOLD) {
        // Hashrate surge detected - increase difficulty
        double penalty = 1.0 + surge;
        newDifficulty = previousDifficulty * penalty;
    } else if (surge < -SURGE_THRESHOLD) {
        // Hashrate drop detected - decrease difficulty
        double relief = 1.0 + surge; // surge is negative, so this reduces
        newDifficulty = previousDifficulty * std::max(0.5, relief);
    }
    
    // Attack detection - massive surge
    if (surge > ATTACK_THRESHOLD) {
        // Potential attack - apply stronger penalty
        newDifficulty *= 2.0;
    }
    
    // Clamp difficulty change
    double maxChange = previousDifficulty * MAX_DIFFICULTY_CHANGE;
    double minChange = previousDifficulty / MAX_DIFFICULTY_CHANGE;
    newDifficulty = clamp(newDifficulty, minChange, maxChange);
    
    // Ensure minimum difficulty
    return std::max(MIN_DIFFICULTY, newDifficulty);
}

double SecurityEngine::smoothDifficultyChange(double currentDifficulty,
                                               double targetDifficulty) {
    // Apply smoothing to prevent oscillation
    return currentDifficulty + 
           DIFFICULTY_ADJUSTMENT_FACTOR * (targetDifficulty - currentDifficulty);
}

// =========================================================
//           3) STAKER-BALANCE DIFFICULTY MODIFIER
// =========================================================

double SecurityEngine::applyStakerInfluence(double baseDifficulty) {
    if (totalSupply_ <= 0.0) return baseDifficulty;
    
    double stakeRatio = totalStaked_ / totalSupply_;
    
    // Clamp stake ratio influence
    double influence = clamp(stakeRatio * STAKE_FACTOR, 
                             0.0, 
                             MAX_STAKE_INFLUENCE);
    
    // Higher stake participation = slightly higher difficulty
    // This makes the chain more secure when stakers are active
    return baseDifficulty * (1.0 + influence);
}

void SecurityEngine::updateStakingMetrics(double totalStaked, double totalSupply) {
    totalStaked_ = totalStaked;
    totalSupply_ = totalSupply;
}

double SecurityEngine::getStakeRatio() const {
    if (totalSupply_ <= 0.0) return 0.0;
    return totalStaked_ / totalSupply_;
}

// =========================================================
//            4) EMISSION GUARD (Anti-Inflation)
// =========================================================

double SecurityEngine::calculateEmissionGuardedReward(double baseReward, 
                                                       double timeTaken) {
    // Fast blocks get reduced rewards (anti-gaming)
    // Slow blocks get normal or slightly increased rewards
    
    if (timeTaken <= 0.0) timeTaken = MIN_BLOCK_TIME;
    
    double ratio = timeTaken / targetBlockTime_;
    
    // Clamp ratio to prevent extreme values
    ratio = clamp(ratio, MIN_REWARD_RATIO, MAX_REWARD_RATIO);
    
    // Calculate guarded reward
    double guardedReward = baseReward * ratio;
    
    // Track fast blocks for attack detection
    if (timeTaken < targetBlockTime_ * 0.5) {
        consecutiveFastBlocks_++;
        consecutiveSlowBlocks_ = 0;
        
        // Multiple fast blocks in a row - apply additional penalty
        if (consecutiveFastBlocks_ > 3) {
            double penalty = 1.0 / (1.0 + (consecutiveFastBlocks_ - 3) * 0.1);
            guardedReward *= penalty;
        }
    } else {
        consecutiveFastBlocks_ = 0;
        if (timeTaken > targetBlockTime_ * 2.0) {
            consecutiveSlowBlocks_++;
        } else {
            consecutiveSlowBlocks_ = 0;
        }
    }
    
    return guardedReward;
}

bool SecurityEngine::validateEmission(double proposedReward, double timeTaken) {
    double expectedReward = calculateEmissionGuardedReward(BASE_REWARD, timeTaken);
    
    // Allow 1% tolerance
    double tolerance = expectedReward * 0.01;
    
    return std::abs(proposedReward - expectedReward) <= tolerance;
}

// =========================================================
//            5) FEE SURGE GUARD (Adaptive Fees)
// =========================================================

double SecurityEngine::calculateDynamicFee(int mempoolSize) {
    if (mempoolSize <= 0) return MIN_FEE;
    
    // Scale fee based on mempool congestion
    double scaledFee = MIN_FEE + (mempoolSize * FEE_SCALE_FACTOR);
    
    // Clamp to allowed range
    return clamp(scaledFee, MIN_FEE, MAX_FEE);
}

double SecurityEngine::getRecommendedFee() const {
    // Use stored mempool size
    if (currentMempoolSize_ <= 0) return MIN_FEE;
    
    double scaledFee = MIN_FEE + (currentMempoolSize_ * FEE_SCALE_FACTOR);
    return clamp(scaledFee, MIN_FEE, MAX_FEE);
}

// =========================================================
//            6) HYBRID PENALTY LOGIC
// =========================================================

double SecurityEngine::calculateHybridPenalty(double /* minerReward */,
                                               double /* stakerReward */,
                                               uint32_t recentPoWBlocks,
                                               uint32_t recentPoSBlocks) {
    // Balance enforcement between PoW and PoS
    uint32_t totalBlocks = recentPoWBlocks + recentPoSBlocks;
    if (totalBlocks == 0) return 1.0; // No penalty
    
    double powRatio = static_cast<double>(recentPoWBlocks) / totalBlocks;
    // posRatio calculated for future use in reward balancing
    (void)powRatio; // TODO: Use in reward penalty calculation
    
    // Ideal ratio is 50/50 for hybrid consensus
    double idealRatio = 0.5;
    
    // Calculate imbalance
    double imbalance = std::abs(powRatio - idealRatio);
    
    // If highly imbalanced, apply penalty to the dominant type
    if (imbalance > 0.3) {
        // Significant imbalance - reduce rewards for dominant type
        return 1.0 - (imbalance * 0.5);
    }
    
    return 1.0; // No penalty for balanced network
}

// =========================================================
//            7) UNIFIED BLOCK SECURITY CHECK
// =========================================================

SecurityMetrics SecurityEngine::evaluateBlock(double currentHashrate,
                                               double timeTaken,
                                               double previousDifficulty,
                                               int mempoolSize) {
    SecurityMetrics metrics;
    
    // 1. Predict hashrate (AI Sentinel)
    metrics.predictedHashrate = predictHashrate(currentHashrate);
    metrics.actualHashrate = currentHashrate;
    
    // 2. Calculate hashrate surge
    if (historicalHashrate_ > 0) {
        metrics.hashrateSurge = (currentHashrate - historicalHashrate_) / historicalHashrate_;
    } else {
        metrics.hashrateSurge = 0.0;
    }
    
    // 3. Calculate difficulty adjustment
    double predictedDiff = calculatePredictiveDifficulty(previousDifficulty, currentHashrate);
    double stakerAdjustedDiff = applyStakerInfluence(predictedDiff);
    metrics.difficultyAdjustment = stakerAdjustedDiff / previousDifficulty;
    
    // 4. Calculate stake influence
    metrics.stakeInfluence = getStakeRatio() * STAKE_FACTOR;
    
    // 5. Calculate reward multiplier (emission guard)
    double ratio = timeTaken / targetBlockTime_;
    metrics.rewardMultiplier = clamp(ratio, MIN_REWARD_RATIO, MAX_REWARD_RATIO);
    
    // 6. Calculate dynamic fee
    metrics.dynamicFee = calculateDynamicFee(mempoolSize);
    
    // 7. Detect attacks
    metrics.attackDetected = detectAttack(currentHashrate, timeTaken);
    metrics.attackType = metrics.attackDetected ? 
                         getAttackType(currentHashrate, timeTaken) : "";
    
    return metrics;
}

double SecurityEngine::calculateNextDifficulty(double previousDifficulty,
                                                double currentHashrate,
                                                double timeTaken) {
    // 1. Base difficulty from predictive guard
    double baseDiff = calculatePredictiveDifficulty(previousDifficulty, currentHashrate);
    
    // 2. Apply staker influence
    double stakerDiff = applyStakerInfluence(baseDiff);
    
    // 3. Apply time-based adjustment
    double timeRatio = targetBlockTime_ / std::max(timeTaken, MIN_BLOCK_TIME);
    double timeAdjusted = stakerDiff * clamp(timeRatio, 0.25, 4.0);
    
    // 4. Smooth the change
    double smoothed = smoothDifficultyChange(previousDifficulty, timeAdjusted);
    
    // 5. Ensure within bounds
    double maxDiff = previousDifficulty * MAX_DIFFICULTY_CHANGE;
    double minDiff = previousDifficulty / MAX_DIFFICULTY_CHANGE;
    
    return clamp(std::max(MIN_DIFFICULTY, smoothed), minDiff, maxDiff);
}

double SecurityEngine::calculateBlockReward(double timeTaken) {
    return calculateEmissionGuardedReward(BASE_REWARD, timeTaken);
}

bool SecurityEngine::detectAttack(double hashrate, double timeTaken) {
    // Fast block attack detection
    if (timeTaken < targetBlockTime_ * 0.1 && consecutiveFastBlocks_ > 5) {
        return true;
    }
    
    // Hashrate surge attack detection
    if (historicalHashrate_ > 0) {
        double surge = (hashrate - historicalHashrate_) / historicalHashrate_;
        if (surge > ATTACK_THRESHOLD) {
            return true;
        }
    }
    
    // Slow block attack (selfish mining detection)
    if (timeTaken > targetBlockTime_ * 10 && consecutiveSlowBlocks_ > 3) {
        return true;
    }
    
    return false;
}

std::string SecurityEngine::getAttackType(double hashrate, double timeTaken) {
    if (timeTaken < targetBlockTime_ * 0.1 && consecutiveFastBlocks_ > 5) {
        return "FAST_BLOCK_ATTACK";
    }
    
    if (historicalHashrate_ > 0) {
        double surge = (hashrate - historicalHashrate_) / historicalHashrate_;
        if (surge > ATTACK_THRESHOLD) {
            return "HASHRATE_SURGE_ATTACK";
        }
    }
    
    if (timeTaken > targetBlockTime_ * 10 && consecutiveSlowBlocks_ > 3) {
        return "SELFISH_MINING_DETECTED";
    }
    
    return "UNKNOWN";
}

// =========================================================
//            8) HISTORY & ANALYTICS
// =========================================================

void SecurityEngine::recordBlock(const BlockSecurityData& blockData) {
    hashrateHistory_.push_back(blockData.hashrate);
    if (hashrateHistory_.size() > HASHRATE_HISTORY_SIZE) {
        hashrateHistory_.erase(hashrateHistory_.begin());
    }
    
    lastDifficulty_ = blockData.difficulty;
}

double SecurityEngine::getAverageHashrate(size_t blockCount) const {
    if (hashrateHistory_.empty()) return 0.0;
    
    size_t count = std::min(blockCount, hashrateHistory_.size());
    size_t startIdx = hashrateHistory_.size() - count;
    
    double sum = 0.0;
    for (size_t i = startIdx; i < hashrateHistory_.size(); i++) {
        sum += hashrateHistory_[i];
    }
    
    return sum / count;
}

void SecurityEngine::reset() {
    predictedHashrate_ = 0.0;
    historicalHashrate_ = 0.0;
    hashrateHistory_.clear();
    consecutiveFastBlocks_ = 0;
    consecutiveSlowBlocks_ = 0;
    lastDifficulty_ = MIN_DIFFICULTY;
    totalStaked_ = 0.0;
    totalSupply_ = 0.0;
    currentMempoolSize_ = 0;
}

// =========================================================
//            UTILITY FUNCTIONS
// =========================================================

double exponentialMovingAverage(double current, double previous, double weight) {
    return (weight * current) + ((1.0 - weight) * previous);
}

bool isValidDifficulty(double difficulty) {
    return difficulty >= MIN_DIFFICULTY && 
           std::isfinite(difficulty);
}

bool isValidFee(double fee) {
    return fee >= MIN_FEE && 
           fee <= MAX_FEE && 
           std::isfinite(fee);
}

} // namespace GXCSecurity
