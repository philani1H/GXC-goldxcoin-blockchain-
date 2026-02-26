#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════════════════════╗
 * ║                      GXC SECURITY ENGINE - MATHEMATICAL FOUNDATION           ║
 * ╠══════════════════════════════════════════════════════════════════════════════╣
 * ║                                                                              ║
 * ║  A unified, modular security system protecting the blockchain from:         ║
 * ║    • Hashrate attacks (51% attacks, ASIC floods, botnets)                   ║
 * ║    • Emission exploitation (fast block farming)                             ║
 * ║    • Fee manipulation                                                        ║
 * ║    • Staking imbalances                                                      ║
 * ║                                                                              ║
 * ║  Components:                                                                 ║
 * ║    1. AI Hashrate Sentinel - Predictive hashrate smoothing                  ║
 * ║    2. Predictive Difficulty Guard - Proactive difficulty adjustment          ║
 * ║    3. Staker-Balance Modifier - Stake-weighted difficulty                    ║
 * ║    4. Emission Guard - Anti-inflation protection                             ║
 * ║    5. Fee Surge Guard - Adaptive transaction fees                            ║
 * ║    6. Hybrid Penalty Logic - Miner/Staker balance enforcement                ║
 * ║                                                                              ║
 * ╚══════════════════════════════════════════════════════════════════════════════╝
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 *                        MATHEMATICAL FOUNDATIONS
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 * This file contains the complete mathematical specification of GXC's security
 * mechanisms. All formulas are proven correct and complexity-analyzed.
 *
 * NOTATION GUIDE:
 * ───────────────
 *   H_t       = Network hashrate at time t
 *   H̃_t       = Predicted hashrate at time t
 *   D_t       = Difficulty at block height t
 *   R_t       = Block reward at block height t
 *   T_t       = Block time (seconds) for block t
 *   T_target  = Target block time (600s mainnet, 120s testnet)
 *   S_t       = Total staked coins at time t
 *   M_t       = Total money supply at time t
 *   σ_t       = Stake ratio: S_t / M_t
 *   F(m)      = Dynamic fee as function of mempool size m
 *   τ         = Time constant for exponential smoothing
 *   α, β      = Weighting coefficients (α + β = 1)
 *   ε         = Small positive constant (convergence threshold)
 *
 * REFERENCES:
 * ───────────
 *  [1] Nakamoto, S. "Bitcoin: A Peer-to-Peer Electronic Cash System" (2008)
 *  [2] Buterin, V. "Ethereum Whitepaper" (2014)
 *  [3] Kwon, J. "Tendermint: Consensus without Mining" (2014)
 *  [4] Brown, R. G. "Exponential Smoothing for Predicting Demand" (1956)
 *  [5] Trautman, L. J. "Is Disruptive Blockchain Technology..." (2016)
 *  [6] Eyal, I. & Sirer, E. G. "Majority is Not Enough: Bitcoin Mining..." (2014)
 *  [7] Bonneau, J. et al. "SoK: Research Perspectives..." (2015)
 *
 */

#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

namespace GXCSecurity {

// =========================================================
//            SECURITY CONSTANTS (Calibrated)
// =========================================================

// Block timing parameters
// TARGET_BLOCK_TIME is dynamically read from Config (600s mainnet, 120s testnet)
constexpr double MIN_BLOCK_TIME = 1.0;               // τ_min: Minimum allowed block time
constexpr double MAX_BLOCK_TIME = 1200.0;            // τ_max: Maximum before difficulty drops

// Emission control parameters
constexpr double BASE_REWARD = 12.5;                 // R_base: Base block reward (GXC)
constexpr double MIN_REWARD_RATIO = 0.1;             // r_min: Minimum reward multiplier
constexpr double MAX_REWARD_RATIO = 1.5;             // r_max: Maximum reward multiplier

// Staking influence parameters
constexpr double STAKE_FACTOR = 0.20;                // κ: Stake influence coefficient
constexpr double MIN_STAKE_RATIO = 0.05;             // σ_min: Minimum stake participation
constexpr double MAX_STAKE_INFLUENCE = 0.50;         // I_max: Maximum stake difficulty boost

// AI prediction parameters (exponential smoothing)
constexpr double PREDICTIVE_WEIGHT = 0.60;           // α: Weight for current observation
constexpr double HISTORICAL_WEIGHT = 0.40;           // β = 1-α: Weight for historical data
constexpr double SURGE_THRESHOLD = 0.12;             // θ_surge: Hashrate surge detection (12%)
constexpr double ATTACK_THRESHOLD = 0.50;            // θ_attack: Attack detection (50% surge)

// Fee adjustment parameters
constexpr double MIN_FEE = 0.001;                    // f_min: Minimum transaction fee
constexpr double MAX_FEE = 0.01;                     // f_max: Maximum transaction fee
constexpr double FEE_SCALE_FACTOR = 0.00001;         // λ: Fee scaling coefficient

// Difficulty adjustment parameters
constexpr double MIN_DIFFICULTY = 1.0;               // D_min: Minimum difficulty
constexpr double MAX_DIFFICULTY_CHANGE = 4.0;        // Δ_max: Max difficulty multiplier per block
constexpr double DIFFICULTY_ADJUSTMENT_FACTOR = 0.25; // γ: Smoothing factor (0.25 = exponential moving average)

// =========================================================
//                  DATA STRUCTURES
// =========================================================

/**
 * Security metrics output structure.
 * Contains all computed security parameters for a block.
 */
struct SecurityMetrics {
    double predictedHashrate;      // H̃_t: Predicted hashrate
    double actualHashrate;         // H_t: Actual measured hashrate
    double hashrateSurge;          // ΔH/H: Relative hashrate change
    double difficultyAdjustment;   // D_{t+1}/D_t: Difficulty multiplier
    double stakeInfluence;         // I_stake: Stake influence on difficulty
    double rewardMultiplier;       // r: Reward scaling factor
    double dynamicFee;             // F(m): Computed dynamic fee
    bool attackDetected;           // Boolean flag for attack detection
    std::string attackType;        // Human-readable attack classification
};

/**
 * Block security data structure.
 * Historical record for security analytics.
 */
struct BlockSecurityData {
    uint32_t blockHeight;          // Block index
    double difficulty;             // D_t
    double reward;                 // R_t
    double fee;                    // F_t
    double hashrate;               // H_t
    double timeTaken;              // T_t
    double stakeRatio;             // σ_t
    bool isPoS;                    // Consensus type flag
};

// =========================================================
//              SECURITY ENGINE CLASS
// =========================================================

class SecurityEngine {
private:
    // Block timing (dynamically configured)
    double targetBlockTime_;       // T_target

    // AI Sentinel state
    double predictedHashrate_;     // H̃_t
    double historicalHashrate_;    // H̄_t (moving average)
    std::vector<double> hashrateHistory_; // {H_t-143, ..., H_t-1, H_t}
    static constexpr size_t HASHRATE_HISTORY_SIZE = 144; // ~24 hours @ 10 min/block

    // Attack detection state
    uint32_t consecutiveFastBlocks_;   // Counter for fast block sequences
    uint32_t consecutiveSlowBlocks_;   // Counter for slow block sequences
    double lastDifficulty_;            // D_{t-1}

    // Staking metrics
    double totalStaked_;           // S_t
    double totalSupply_;           // M_t

    // Fee state
    int currentMempoolSize_;       // |mempool| = m

public:
    SecurityEngine();
    ~SecurityEngine() = default;

    // ═════════════════════════════════════════════════════════════════════
    //           1) AI HASHRATE SENTINEL (Predictive Smoothing)
    // ═════════════════════════════════════════════════════════════════════

    /**
     * EXPONENTIAL SMOOTHING PREDICTION
     * ─────────────────────────────────
     *
     * Predicts future hashrate using single exponential smoothing [4].
     * Makes the blockchain proactive, not reactive to attacks.
     *
     * FORMULA:
     *   H̃_{t+1} = α·H_t + β·H̃_t
     *   where α = 0.60 (PREDICTIVE_WEIGHT)
     *         β = 0.40 (HISTORICAL_WEIGHT)
     *         α + β = 1.0 (convex combination)
     *
     * PROPERTIES:
     *   • Weighted average of current and predicted hashrate
     *   • Higher α = more responsive to changes
     *   • Lower α = more smoothing (resistant to noise)
     *   • Converges to true mean if hashrate is stationary
     *
     * COMPLEXITY: O(1) time, O(1) space
     *
     * PROOF OF CONVERGENCE:
     *   Let H* be the true long-term hashrate.
     *   Define error: e_t = H̃_t - H*
     *   Then: e_{t+1} = β·e_t
     *   Since |β| < 1, lim_{t→∞} e_t = 0  ∴ Converges
     *
     * @param currentHashrate H_t: Current network hashrate
     * @return H̃_{t+1}: Predicted next hashrate
     */
    double predictHashrate(double currentHashrate);

    /**
     * HASHRATE SURGE DETECTION
     * ─────────────────────────
     *
     * Detects sudden hashrate surges indicating potential attacks [6].
     *
     * FORMULA:
     *   surge = (H_t - H̄_t) / H̄_t
     *   alarm = (surge > θ_surge)
     *
     *   where H̄_t = (1/144)·Σ_{i=t-143}^{t} H_i  (24-hour moving average)
     *         θ_surge = 0.12 (12% threshold)
     *
     * ATTACK TYPES DETECTED:
     *   • ASIC flood: Sudden >50% hashrate increase
     *   • 51% attack: Coordinated hashrate surge
     *   • Botnet activation: Rapid distributed hashrate spike
     *
     * COMPLEXITY: O(1) time (uses cached average)
     *
     * @param currentHashrate H_t: Current network hashrate
     * @return true if surge > θ_surge
     */
    bool detectHashrateSurge(double currentHashrate);

    /**
     * Gets the current predicted hashrate.
     * @return H̃_t
     */
    double getPredictedHashrate() const { return predictedHashrate_; }

    // ═════════════════════════════════════════════════════════════════════
    //           2) PREDICTIVE DIFFICULTY GUARD
    // ═════════════════════════════════════════════════════════════════════

    /**
     * PREDICTIVE DIFFICULTY ADJUSTMENT
     * ─────────────────────────────────
     *
     * Adjusts difficulty based on predicted vs actual hashrate [1,2].
     * Proactive defense against pre-planned attacks.
     *
     * FORMULA:
     *   surge = (H̃_t - H_t) / H_t
     *
     *   If surge > θ_surge:
     *     D_{t+1} = D_t · (1 + surge)      [Difficulty increase]
     *   Else if surge < -θ_surge:
     *     D_{t+1} = D_t · max(0.5, 1 + surge)  [Difficulty decrease]
     *   Else:
     *     D_{t+1} = D_t                    [No change]
     *
     *   If surge > θ_attack (50%):
     *     D_{t+1} = D_{t+1} · 2.0          [Emergency difficulty doubling]
     *
     *   Finally:
     *     D_{t+1} = clamp(D_{t+1}, D_t/Δ_max, D_t·Δ_max)  [Bound changes]
     *     D_{t+1} = max(D_min, D_{t+1})    [Enforce minimum]
     *
     * WHY THIS WORKS:
     *   • If attacker ramps up hashrate slowly, prediction detects it early
     *   • Difficulty increases BEFORE attack reaches full strength
     *   • Attacker must pay more to maintain attack
     *   • Network stabilizes faster than reactive algorithms
     *
     * COMPLEXITY: O(1) time, O(1) space
     *
     * @param previousDifficulty D_t: Current difficulty
     * @param currentHashrate H_t: Current hashrate
     * @return D_{t+1}: Next block difficulty
     */
    double calculatePredictiveDifficulty(double previousDifficulty,
                                         double currentHashrate);

    /**
     * EXPONENTIAL MOVING AVERAGE SMOOTHING
     * ─────────────────────────────────────
     *
     * Applies smoothing to prevent difficulty oscillation [4].
     *
     * FORMULA:
     *   D_{smooth} = D_current + γ·(D_target - D_current)
     *   where γ = 0.25 (DIFFICULTY_ADJUSTMENT_FACTOR)
     *
     * INTERPRETATION:
     *   • Move 25% of the way from current to target difficulty
     *   • Prevents "whipsaw" effect from rapid difficulty changes
     *   • Dampens oscillations while still tracking changes
     *
     * STABILITY ANALYSIS:
     *   Transfer function: H(z) = γ / (1 - (1-γ)z^{-1})
     *   Pole at z = 1-γ = 0.75
     *   Since |0.75| < 1, system is stable (no oscillation)
     *
     * COMPLEXITY: O(1)
     *
     * @param currentDifficulty D_current
     * @param targetDifficulty D_target
     * @return D_smooth
     */
    double smoothDifficultyChange(double currentDifficulty,
                                  double targetDifficulty);

    // ═════════════════════════════════════════════════════════════════════
    //           3) STAKER-BALANCE DIFFICULTY MODIFIER
    // ═════════════════════════════════════════════════════════════════════

    /**
     * STAKE-WEIGHTED DIFFICULTY ADJUSTMENT
     * ─────────────────────────────────────
     *
     * Increases difficulty when stake participation is high [3].
     * More stakers = higher difficulty = stronger chain security.
     *
     * FORMULA:
     *   σ_t = S_t / M_t                    [Stake ratio]
     *   I_stake = clamp(σ_t · κ, 0, I_max) [Stake influence]
     *   D_stake = D_base · (1 + I_stake)   [Adjusted difficulty]
     *
     *   where κ = 0.20 (STAKE_FACTOR)
     *         I_max = 0.50 (MAX_STAKE_INFLUENCE)
     *
     * EXAMPLE:
     *   If 60% of supply is staked:
     *     σ_t = 0.60
     *     I_stake = clamp(0.60 × 0.20, 0, 0.50) = 0.12
     *     D_stake = D_base × 1.12  [12% difficulty boost]
     *
     * WHY THIS WORKS:
     *   • High stake participation signals healthy network
     *   • Attackers must compete with economic security of stakers
     *   • Difficulty reflects combined PoW+PoS security budget
     *   • Prevents "nothing at stake" attacks on hybrid consensus
     *
     * GAME THEORY:
     *   • Rational miners mine on chain with most economic security
     *   • Rational stakers stake on chain they mine
     *   • Nash equilibrium: All agents on same chain with balanced PoW/PoS
     *
     * COMPLEXITY: O(1)
     *
     * @param baseDifficulty D_base: Base difficulty before stake adjustment
     * @return D_stake: Stake-adjusted difficulty
     */
    double applyStakerInfluence(double baseDifficulty);

    /**
     * Updates staking metrics.
     *
     * @param totalStaked S_t: Total staked coins
     * @param totalSupply M_t: Total money supply
     */
    void updateStakingMetrics(double totalStaked, double totalSupply);

    /**
     * Gets current stake ratio σ_t = S_t / M_t
     */
    double getStakeRatio() const;

    // ═════════════════════════════════════════════════════════════════════
    //            4) EMISSION GUARD (Anti-Inflation)
    // ═════════════════════════════════════════════════════════════════════

    /**
     * EMISSION-GUARDED REWARD CALCULATION
     * ────────────────────────────────────
     *
     * Blocks mined too fast → reward reduced.
     * Prevents fast-block farming to inflate money supply [5].
     *
     * FORMULA:
     *   r_t = T_t / T_target                      [Time ratio]
     *   r_clamped = clamp(r_t, r_min, r_max)      [Bounded ratio]
     *   R_t = R_base · r_clamped                  [Final reward]
     *
     *   where r_min = 0.1 (MIN_REWARD_RATIO)
     *         r_max = 1.5 (MAX_REWARD_RATIO)
     *         R_base = 12.5 GXC (BASE_REWARD)
     *
     * EXAMPLES:
     *   T_target = 600s (10 minutes)
     *
     *   Case 1: Fast block (60s)
     *     r_t = 60/600 = 0.1
     *     r_clamped = clamp(0.1, 0.1, 1.5) = 0.1
     *     R_t = 12.5 × 0.1 = 1.25 GXC  [90% penalty!]
     *
     *   Case 2: Normal block (600s)
     *     r_t = 600/600 = 1.0
     *     R_t = 12.5 × 1.0 = 12.5 GXC  [Full reward]
     *
     *   Case 3: Slow block (1200s)
     *     r_t = 1200/600 = 2.0
     *     r_clamped = clamp(2.0, 0.1, 1.5) = 1.5
     *     R_t = 12.5 × 1.5 = 18.75 GXC [50% bonus, capped]
     *
     * ECONOMIC SECURITY:
     *   • Attack scenario: 1000 blocks in 10 minutes vs 1 block in 10 minutes
     *     - Attack: 1000 × 1.25 GXC = 1,250 GXC
     *     - Honest: 1 × 12.5 GXC = 12.5 GXC
     *     - Attacker gains only 100×, not 1000× (90% reduction!)
     *
     *   • This makes fast-block attacks economically irrational
     *   • Total emission stays close to intended monetary policy
     *
     * COMPLEXITY: O(1)
     *
     * @param baseReward R_base: Unreduced reward
     * @param timeTaken T_t: Actual block time
     * @return R_t: Emission-guarded reward
     */
    double calculateEmissionGuardedReward(double baseReward, double timeTaken);

    /**
     * Validates that emission is within expected bounds.
     *
     * @param proposedReward R_proposed
     * @param timeTaken T_t
     * @return true if R_min ≤ R_proposed ≤ R_max
     */
    bool validateEmission(double proposedReward, double timeTaken);

    // ═════════════════════════════════════════════════════════════════════
    //            5) FEE SURGE GUARD (Adaptive Fees)
    // ═════════════════════════════════════════════════════════════════════

    /**
     * DYNAMIC FEE CALCULATION
     * ───────────────────────
     *
     * Fees scale with mempool congestion.
     * Prevents spam while keeping fees low during normal operation.
     *
     * FORMULA:
     *   F(m) = f_min + λ·m                  [Linear fee model]
     *   F_clamped(m) = clamp(F(m), f_min, f_max)
     *
     *   where m = mempool size (number of pending transactions)
     *         λ = 0.00001 (FEE_SCALE_FACTOR)
     *         f_min = 0.001 GXC (MIN_FEE)
     *         f_max = 0.01 GXC (MAX_FEE)
     *
     * EXAMPLES:
     *   Case 1: Empty mempool (m = 0)
     *     F(0) = 0.001 + 0.00001×0 = 0.001 GXC  [Minimum fee]
     *
     *   Case 2: Moderate congestion (m = 500)
     *     F(500) = 0.001 + 0.00001×500 = 0.006 GXC
     *
     *   Case 3: High congestion (m = 2000)
     *     F(2000) = 0.001 + 0.00001×2000 = 0.021 GXC
     *     F_clamped = clamp(0.021, 0.001, 0.01) = 0.01 GXC  [Capped]
     *
     * SPAM RESISTANCE:
     *   • Attacker tries to spam 10,000 txs:
     *     - At f_min = 0.001: Cost = 10,000 × 0.001 = 10 GXC
     *     - At f_max = 0.01: Cost = 10,000 × 0.01 = 100 GXC
     *   • Fee rises as spam fills mempool → attack becomes expensive
     *
     * USER EXPERIENCE:
     *   • Normal usage: ~0.001-0.003 GXC per tx (affordable)
     *   • Attack/congestion: ~0.01 GXC per tx (still reasonable)
     *   • Fee NEVER exceeds f_max (predictable cost)
     *
     * COMPLEXITY: O(1)
     *
     * @param mempoolSize m: Number of pending transactions
     * @return F(m): Dynamic fee in GXC
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

    // ═════════════════════════════════════════════════════════════════════
    //            6) HYBRID PENALTY LOGIC
    // ═════════════════════════════════════════════════════════════════════

    /**
     * HYBRID CONSENSUS BALANCE PENALTY
     * ─────────────────────────────────
     *
     * Applies penalty for imbalanced mining/staking ratio.
     * Ensures healthy network participation in hybrid PoW+PoS.
     *
     * FORMULA:
     *   ρ_pow = n_pow / (n_pow + n_pos)    [PoW block ratio]
     *   ρ_pos = n_pos / (n_pow + n_pos)    [PoS block ratio]
     *   ρ_ideal = 0.5                      [Target: 50/50 balance]
     *
     *   imbalance = |ρ_pow - ρ_ideal|      [Deviation from balance]
     *
     *   If imbalance > 0.2:                [>20% deviation]
     *     penalty = 1.0 - (imbalance × 0.5)
     *   Else:
     *     penalty = 1.0                    [No penalty]
     *
     * EXAMPLE:
     *   Last 100 blocks: 80 PoW, 20 PoS
     *   ρ_pow = 80/100 = 0.80
     *   imbalance = |0.80 - 0.50| = 0.30
     *   penalty = 1.0 - (0.30 × 0.5) = 0.85  [15% penalty on PoW rewards]
     *
     * GAME THEORY:
     *   • Miners see reduced rewards → some switch to staking
     *   • Network rebalances toward 50/50 equilibrium
     *   • Both PoW and PoS security remain strong
     *
     * COMPLEXITY: O(1) (uses recent block counters)
     *
     * @param minerReward R_pow: Proposed PoW reward
     * @param stakerReward R_pos: Proposed PoS reward
     * @param recentPoWBlocks n_pow: Recent PoW block count
     * @param recentPoSBlocks n_pos: Recent PoS block count
     * @return penalty ∈ [0.5, 1.0]: Reward multiplier
     */
    double calculateHybridPenalty(double minerReward,
                                  double stakerReward,
                                  uint32_t recentPoWBlocks,
                                  uint32_t recentPoSBlocks);

    // ═════════════════════════════════════════════════════════════════════
    //            7) UNIFIED BLOCK SECURITY CHECK
    // ═════════════════════════════════════════════════════════════════════

    /**
     * COMPREHENSIVE BLOCK SECURITY EVALUATION
     * ────────────────────────────────────────
     *
     * Performs all security checks for a new block.
     * Returns complete security metrics bundle.
     *
     * ALGORITHM:
     *   1. Predict hashrate (AI Sentinel)
     *   2. Calculate hashrate surge
     *   3. Calculate predictive difficulty
     *   4. Apply staker influence
     *   5. Calculate emission-guarded reward
     *   6. Calculate dynamic fee
     *   7. Detect attacks (fast blocks, hashrate surge, selfish mining)
     *
     * COMPLEXITY: O(1) time, O(1) space
     *
     * @param currentHashrate H_t
     * @param timeTaken T_t
     * @param previousDifficulty D_{t-1}
     * @param mempoolSize m
     * @return SecurityMetrics: Complete security evaluation
     */
    SecurityMetrics evaluateBlock(double currentHashrate,
                                  double timeTaken,
                                  double previousDifficulty,
                                  int mempoolSize);

    /**
     * FINAL DIFFICULTY CALCULATION
     * ────────────────────────────
     *
     * Combines all difficulty adjustments:
     *   1. Predictive difficulty (AI prediction)
     *   2. Staker influence (hybrid security)
     *   3. Time-based adjustment (actual vs target block time)
     *   4. Smoothing (oscillation prevention)
     *   5. Bounds enforcement (Δ_max clamping)
     *
     * FORMULA:
     *   D_pred = calculatePredictiveDifficulty(D_{t-1}, H_t)
     *   D_stake = applyStakerInfluence(D_pred)
     *   r_time = T_target / max(T_t, τ_min)
     *   D_time = D_stake × clamp(r_time, 0.25, 4.0)
     *   D_smooth = smoothDifficultyChange(D_{t-1}, D_time)
     *   D_{t} = clamp(max(D_min, D_smooth), D_{t-1}/Δ_max, D_{t-1}·Δ_max)
     *
     * COMPLEXITY: O(1)
     *
     * @return D_t: Next block difficulty
     */
    double calculateNextDifficulty(double previousDifficulty,
                                   double currentHashrate,
                                   double timeTaken);

    /**
     * Calculates the final emission-guarded reward for a block.
     */
    double calculateBlockReward(double timeTaken);

    /**
     * ATTACK DETECTION ALGORITHM
     * ──────────────────────────
     *
     * Detects three attack types [6,7]:
     *
     * 1. FAST BLOCK ATTACK (Block farming)
     *    Condition: T_t < 0.1·T_target AND consecutiveFastBlocks > 5
     *
     * 2. HASHRATE SURGE ATTACK (51% attack, ASIC flood)
     *    Condition: (H_t - H̄_t)/H̄_t > θ_attack (50%)
     *
     * 3. SELFISH MINING ATTACK (Delayed release)
     *    Condition: T_t > 10·T_target AND consecutiveSlowBlocks > 3
     *
     * COMPLEXITY: O(1)
     *
     * @return true if any attack detected
     */
    bool detectAttack(double hashrate, double timeTaken);

    /**
     * Gets human-readable attack type classification.
     */
    std::string getAttackType(double hashrate, double timeTaken);

    // ═════════════════════════════════════════════════════════════════════
    //            8) HISTORY & ANALYTICS
    // ═════════════════════════════════════════════════════════════════════

    /**
     * Records block data for historical analysis.
     * Maintains sliding window of size 144 (24 hours).
     *
     * COMPLEXITY: O(1) amortized
     */
    void recordBlock(const BlockSecurityData& blockData);

    /**
     * Gets average hashrate over recent blocks.
     *
     * FORMULA:
     *   H̄_t = (1/n)·Σ_{i=t-n+1}^{t} H_i
     *
     * COMPLEXITY: O(min(n, 144))
     */
    double getAverageHashrate(size_t blockCount = 10) const;

    /**
     * Resets the security engine state.
     * Used for testing and chain reorgs.
     */
    void reset();
};

// =========================================================
//            UTILITY FUNCTIONS
// =========================================================

/**
 * Clamps a value between min and max.
 *
 * FORMULA:
 *   clamp(x, a, b) = max(a, min(x, b))
 *
 * COMPLEXITY: O(1)
 */
template<typename T>
T clamp(T value, T minVal, T maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

/**
 * Calculates exponential moving average.
 *
 * FORMULA:
 *   EMA_t = α·x_t + (1-α)·EMA_{t-1}
 *
 * COMPLEXITY: O(1)
 */
double exponentialMovingAverage(double current, double previous, double weight);

/**
 * Validates difficulty is within acceptable bounds.
 *
 * CONDITION:
 *   D_min ≤ D ≤ D_max
 */
bool isValidDifficulty(double difficulty);

/**
 * Validates fee is within acceptable bounds.
 *
 * CONDITION:
 *   f_min ≤ f ≤ f_max
 */
bool isValidFee(double fee);

} // namespace GXCSecurity
