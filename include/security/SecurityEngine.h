#pragma once

/**
 * GXC Security Engine
 * 
 * A unified, modular security system protecting the blockchain from:
 * - Hashrate attacks (51% attacks, ASIC floods, botnets)
 * - Emission exploitation (fast block farming)
 * - Fee manipulation
 * - Staking imbalances
 * 
 * Components:
 * 1. AI Hashrate Sentinel - Predictive hashrate smoothing
 * 2. Predictive Difficulty Guard - Proactive difficulty adjustment
 * 3. Staker-Balance Modifier - Stake-weighted difficulty
 * 4. Emission Guard - Anti-inflation protection
 * 5. Fee Surge Guard - Adaptive transaction fees
 * 6. Hybrid Penalty Logic - Miner/Staker balance enforcement
 */

#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

namespace GXCSecurity {

// =========================================================
//                  SECURITY CONSTANTS
// =========================================================

// Block timing
constexpr double TARGET_BLOCK_TIME = 10.0;           // Target 10 seconds per block
constexpr double MIN_BLOCK_TIME = 1.0;               // Minimum allowed block time
constexpr double MAX_BLOCK_TIME = 120.0;             // Maximum before difficulty drops

// Rewards (no minting inside blocks - rewards are calculated separately)
constexpr double BASE_REWARD = 12.5;                 // Base block reward
constexpr double MIN_REWARD_RATIO = 0.1;             // Minimum reward multiplier
constexpr double MAX_REWARD_RATIO = 1.5;             // Maximum reward multiplier

// Staking influence
constexpr double STAKE_FACTOR = 0.20;                // How much stake affects difficulty
constexpr double MIN_STAKE_RATIO = 0.05;             // Minimum stake participation
constexpr double MAX_STAKE_INFLUENCE = 0.50;         // Maximum stake difficulty boost

// Predictive AI weights
constexpr double PREDICTIVE_WEIGHT = 0.60;           // Weight for current hashrate
constexpr double HISTORICAL_WEIGHT = 0.40;           // Weight for historical data
constexpr double SURGE_THRESHOLD = 0.12;             // Hashrate surge detection threshold
constexpr double ATTACK_THRESHOLD = 0.50;            // Attack detection threshold

// Fee parameters
constexpr double MIN_FEE = 0.0001;                   // Minimum transaction fee
constexpr double MAX_FEE = 0.001;                    // Maximum transaction fee
constexpr double FEE_SCALE_FACTOR = 0.000001;        // Fee scaling per mempool tx

// Difficulty parameters
constexpr double MIN_DIFFICULTY = 1.0;               // Minimum difficulty
constexpr double MAX_DIFFICULTY_CHANGE = 4.0;        // Max difficulty change per block
constexpr double DIFFICULTY_ADJUSTMENT_FACTOR = 0.25; // Smoothing factor

// =========================================================
//                  DATA STRUCTURES
// =========================================================

struct SecurityMetrics {
    double predictedHashrate;
    double actualHashrate;
    double hashrateSurge;
    double difficultyAdjustment;
    double stakeInfluence;
    double rewardMultiplier;
    double dynamicFee;
    bool attackDetected;
    std::string attackType;
};

struct BlockSecurityData {
    uint32_t blockHeight;
    double difficulty;
    double reward;
    double fee;
    double hashrate;
    double timeTaken;
    double stakeRatio;
    bool isPoS;
};

// =========================================================
//                  SECURITY ENGINE CLASS
// =========================================================

class SecurityEngine {
private:
    // AI Sentinel state
    double predictedHashrate_;
    double historicalHashrate_;
    std::vector<double> hashrateHistory_;
    static constexpr size_t HASHRATE_HISTORY_SIZE = 144; // ~24 hours of blocks
    
    // Attack detection state
    uint32_t consecutiveFastBlocks_;
    uint32_t consecutiveSlowBlocks_;
    double lastDifficulty_;
    
    // Staking metrics
    double totalStaked_;
    double totalSupply_;
    
    // Fee state
    int currentMempoolSize_;
    
public:
    SecurityEngine();
    ~SecurityEngine() = default;
    
    // =========================================================
    //           1) AI HASHRATE SENTINEL (Predictive Smoothing)
    // =========================================================
    /**
     * Predicts future hashrate using exponential smoothing.
     * Makes the blockchain proactive, not reactive.
     */
    double predictHashrate(double currentHashrate);
    
    /**
     * Detects sudden hashrate surges that may indicate an attack.
     */
    bool detectHashrateSurge(double currentHashrate);
    
    /**
     * Gets the current predicted hashrate.
     */
    double getPredictedHashrate() const { return predictedHashrate_; }
    
    // =========================================================
    //           2) PREDICTIVE DIFFICULTY GUARD
    // =========================================================
    /**
     * Adjusts difficulty based on predicted vs actual hashrate.
     * If predicted > actual by threshold → apply difficulty penalty.
     */
    double calculatePredictiveDifficulty(double previousDifficulty,
                                         double currentHashrate);
    
    /**
     * Applies smoothing to prevent difficulty oscillation.
     */
    double smoothDifficultyChange(double currentDifficulty,
                                  double targetDifficulty);
    
    // =========================================================
    //           3) STAKER-BALANCE DIFFICULTY MODIFIER
    // =========================================================
    /**
     * More stakers = higher difficulty = stronger chain.
     * Prevents stake centralization attacks.
     */
    double applyStakerInfluence(double baseDifficulty);
    
    /**
     * Updates staking metrics.
     */
    void updateStakingMetrics(double totalStaked, double totalSupply);
    
    /**
     * Gets current stake ratio.
     */
    double getStakeRatio() const;
    
    // =========================================================
    //            4) EMISSION GUARD (Anti-Inflation)
    // =========================================================
    /**
     * Blocks mined too fast → reward reduced.
     * Prevents fast block farming exploitation.
     */
    double calculateEmissionGuardedReward(double baseReward, double timeTaken);
    
    /**
     * Validates that emission is within expected bounds.
     */
    bool validateEmission(double proposedReward, double timeTaken);
    
    // =========================================================
    //            5) FEE SURGE GUARD (Adaptive Fees)
    // =========================================================
    /**
     * More pending transactions → higher fee.
     * Fee is ALWAYS between MIN_FEE and MAX_FEE.
     */
    double calculateDynamicFee(int mempoolSize);
    
    /**
     * Updates mempool size for fee calculations.
     */
    void updateMempoolSize(int size) { currentMempoolSize_ = size; }
    
    /**
     * Gets recommended fee based on current conditions.
     */
    double getRecommendedFee() const;
    
    // =========================================================
    //            6) HYBRID PENALTY LOGIC
    // =========================================================
    /**
     * Applies penalty for imbalanced mining/staking ratio.
     * Ensures healthy network participation.
     */
    double calculateHybridPenalty(double minerReward, 
                                  double stakerReward,
                                  uint32_t recentPoWBlocks,
                                  uint32_t recentPoSBlocks);
    
    // =========================================================
    //            7) UNIFIED BLOCK SECURITY CHECK
    // =========================================================
    /**
     * Performs all security checks for a new block.
     * Returns complete security metrics.
     */
    SecurityMetrics evaluateBlock(double currentHashrate,
                                  double timeTaken,
                                  double previousDifficulty,
                                  int mempoolSize);
    
    /**
     * Calculates the final difficulty for the next block.
     */
    double calculateNextDifficulty(double previousDifficulty,
                                   double currentHashrate,
                                   double timeTaken);
    
    /**
     * Calculates the final reward for a block.
     */
    double calculateBlockReward(double timeTaken);
    
    /**
     * Detects potential attacks based on block patterns.
     */
    bool detectAttack(double hashrate, double timeTaken);
    
    /**
     * Gets attack type if detected.
     */
    std::string getAttackType(double hashrate, double timeTaken);
    
    // =========================================================
    //            8) HISTORY & ANALYTICS
    // =========================================================
    /**
     * Records block data for historical analysis.
     */
    void recordBlock(const BlockSecurityData& blockData);
    
    /**
     * Gets average hashrate over recent blocks.
     */
    double getAverageHashrate(size_t blockCount = 10) const;
    
    /**
     * Resets the security engine state.
     */
    void reset();
};

// =========================================================
//            UTILITY FUNCTIONS
// =========================================================

/**
 * Clamps a value between min and max.
 */
template<typename T>
T clamp(T value, T minVal, T maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

/**
 * Calculates exponential moving average.
 */
double exponentialMovingAverage(double current, double previous, double weight);

/**
 * Validates difficulty is within acceptable bounds.
 */
bool isValidDifficulty(double difficulty);

/**
 * Validates fee is within acceptable bounds.
 */
bool isValidFee(double fee);

} // namespace GXCSecurity
