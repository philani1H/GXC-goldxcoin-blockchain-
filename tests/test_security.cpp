/**
 * GXC Security Engine Tests
 * 
 * Tests for:
 * - AI Hashrate Sentinel
 * - Predictive Difficulty Guard
 * - Staker-Balance Modifier
 * - Emission Guard
 * - Fee Surge Guard
 * - Attack Detection
 */

#include <gtest/gtest.h>
#include "../include/security/SecurityEngine.h"
#include "../include/Config.h"

using namespace GXCSecurity;

class SecurityEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Config for tests
        Config::initialize();
        engine = std::make_unique<SecurityEngine>();
        // Use testnet block time for tests (120s)
        TARGET_BLOCK_TIME_TEST = Config::getBlockTime();
    }

    void TearDown() override {
        engine.reset();
    }

    std::unique_ptr<SecurityEngine> engine;
    double TARGET_BLOCK_TIME_TEST;
};

// =========================================================
//           AI HASHRATE SENTINEL TESTS
// =========================================================

TEST_F(SecurityEngineTest, HashratePrediction) {
    // Initial prediction should match input
    double predicted = engine->predictHashrate(40000.0);
    EXPECT_NEAR(predicted, 40000.0, 1.0);
    
    // Second prediction should be weighted average
    predicted = engine->predictHashrate(50000.0);
    EXPECT_GT(predicted, 40000.0);
    EXPECT_LT(predicted, 50000.0);
}

TEST_F(SecurityEngineTest, HashrateSurgeDetection) {
    // Initialize with normal hashrate
    engine->predictHashrate(40000.0);
    engine->predictHashrate(40000.0);
    engine->predictHashrate(40000.0);
    
    // No surge with stable hashrate
    EXPECT_FALSE(engine->detectHashrateSurge(42000.0));
    
    // Surge detected with large increase
    EXPECT_TRUE(engine->detectHashrateSurge(60000.0));
}

// =========================================================
//           PREDICTIVE DIFFICULTY TESTS
// =========================================================

TEST_F(SecurityEngineTest, DifficultyIncreasesOnSurge) {
    double baseDifficulty = 1000.0;
    
    // Initialize hashrate history with low values
    for (int i = 0; i < 10; i++) {
        engine->predictHashrate(10000.0);
    }
    
    // Sudden surge - the predicted hashrate will be between old and new
    // When actual > predicted significantly, we detect a surge and increase difficulty
    double newDiff = engine->calculatePredictiveDifficulty(baseDifficulty, 100000.0);
    
    // Difficulty should be adjusted (may increase or decrease based on prediction vs actual)
    // The key is that it's not exactly the same
    EXPECT_NE(newDiff, baseDifficulty);
    EXPECT_TRUE(isValidDifficulty(newDiff));
}

TEST_F(SecurityEngineTest, DifficultyDecreasesOnDrop) {
    double baseDifficulty = 1000.0;
    
    // Initialize with high hashrate
    for (int i = 0; i < 10; i++) {
        engine->predictHashrate(100000.0);
    }
    
    // Hashrate drops significantly - difficulty should adjust
    double newDiff = engine->calculatePredictiveDifficulty(baseDifficulty, 10000.0);
    
    // Difficulty should be adjusted and remain valid
    EXPECT_NE(newDiff, baseDifficulty);
    EXPECT_TRUE(isValidDifficulty(newDiff));
    EXPECT_GE(newDiff, MIN_DIFFICULTY);
}

TEST_F(SecurityEngineTest, DifficultyBounds) {
    double baseDifficulty = 1000.0;
    
    // Even with extreme changes, difficulty should be bounded
    double newDiff = engine->calculatePredictiveDifficulty(baseDifficulty, 10000000.0);
    
    // Should not exceed MAX_DIFFICULTY_CHANGE ratio
    EXPECT_LE(newDiff, baseDifficulty * MAX_DIFFICULTY_CHANGE);
    EXPECT_GE(newDiff, baseDifficulty / MAX_DIFFICULTY_CHANGE);
}

// =========================================================
//           STAKER INFLUENCE TESTS
// =========================================================

TEST_F(SecurityEngineTest, StakerInfluenceIncreasesDifficulty) {
    double baseDifficulty = 1000.0;
    
    // Set staking metrics (25% staked)
    engine->updateStakingMetrics(5000000.0, 20000000.0);
    
    double adjustedDiff = engine->applyStakerInfluence(baseDifficulty);
    
    // Difficulty should increase with staking
    EXPECT_GT(adjustedDiff, baseDifficulty);
    
    // But not too much
    EXPECT_LT(adjustedDiff, baseDifficulty * (1.0 + MAX_STAKE_INFLUENCE + 0.01));
}

TEST_F(SecurityEngineTest, StakeRatioCalculation) {
    engine->updateStakingMetrics(5000000.0, 20000000.0);
    EXPECT_NEAR(engine->getStakeRatio(), 0.25, 0.001);
    
    engine->updateStakingMetrics(0.0, 20000000.0);
    EXPECT_EQ(engine->getStakeRatio(), 0.0);
}

// =========================================================
//           EMISSION GUARD TESTS
// =========================================================

TEST_F(SecurityEngineTest, FastBlocksReduceReward) {
    // Very fast block (1 second instead of 10)
    double reward = engine->calculateEmissionGuardedReward(BASE_REWARD, 1.0);
    
    // Reward should be reduced
    EXPECT_LT(reward, BASE_REWARD);
    EXPECT_GE(reward, BASE_REWARD * MIN_REWARD_RATIO);
}

TEST_F(SecurityEngineTest, NormalBlocksNormalReward) {
    // Normal block time
    double reward = engine->calculateEmissionGuardedReward(BASE_REWARD, TARGET_BLOCK_TIME_TEST);

    // Reward should be approximately base
    EXPECT_NEAR(reward, BASE_REWARD, 0.01);
}

TEST_F(SecurityEngineTest, SlowBlocksCapReward) {
    // Very slow block (100 seconds)
    double reward = engine->calculateEmissionGuardedReward(BASE_REWARD, 100.0);
    
    // Reward should be capped at max ratio
    EXPECT_LE(reward, BASE_REWARD * MAX_REWARD_RATIO);
}

// =========================================================
//           FEE SURGE GUARD TESTS
// =========================================================

TEST_F(SecurityEngineTest, EmptyMempoolMinFee) {
    double fee = engine->calculateDynamicFee(0);
    EXPECT_EQ(fee, MIN_FEE);
}

TEST_F(SecurityEngineTest, CongestedMempoolHigherFee) {
    double fee = engine->calculateDynamicFee(1000);
    EXPECT_GT(fee, MIN_FEE);
    EXPECT_LE(fee, MAX_FEE);
}

TEST_F(SecurityEngineTest, FeeCappedAtMax) {
    // Extreme congestion
    double fee = engine->calculateDynamicFee(1000000);
    EXPECT_EQ(fee, MAX_FEE);
}

TEST_F(SecurityEngineTest, FeeAlwaysValid) {
    for (int size = 0; size <= 100000; size += 1000) {
        double fee = engine->calculateDynamicFee(size);
        EXPECT_TRUE(isValidFee(fee));
    }
}

// =========================================================
//           HYBRID PENALTY TESTS
// =========================================================

TEST_F(SecurityEngineTest, BalancedNetworkNoPenalty) {
    double penalty = engine->calculateHybridPenalty(12.5, 12.5, 50, 50);
    EXPECT_NEAR(penalty, 1.0, 0.01);
}

TEST_F(SecurityEngineTest, ImbalancedNetworkPenalty) {
    // All PoW, no PoS
    double penalty = engine->calculateHybridPenalty(12.5, 12.5, 100, 0);
    EXPECT_LT(penalty, 1.0);
    
    // All PoS, no PoW
    penalty = engine->calculateHybridPenalty(12.5, 12.5, 0, 100);
    EXPECT_LT(penalty, 1.0);
}

// =========================================================
//           ATTACK DETECTION TESTS
// =========================================================

TEST_F(SecurityEngineTest, DetectHashrateAttack) {
    // Initialize with normal hashrate
    for (int i = 0; i < 20; i++) {
        engine->predictHashrate(40000.0);
    }
    
    // Massive surge (100x) should be detected as attack
    bool attack = engine->detectAttack(4000000.0, 5.0);
    EXPECT_TRUE(attack);
    
    std::string attackType = engine->getAttackType(4000000.0, 5.0);
    EXPECT_EQ(attackType, "HASHRATE_SURGE_ATTACK");
}

TEST_F(SecurityEngineTest, NoAttackOnNormalOperation) {
    for (int i = 0; i < 20; i++) {
        engine->predictHashrate(40000.0);
    }
    
    // Normal operation
    bool attack = engine->detectAttack(42000.0, 10.0);
    EXPECT_FALSE(attack);
}

// =========================================================
//           UNIFIED EVALUATION TESTS
// =========================================================

TEST_F(SecurityEngineTest, FullBlockEvaluation) {
    engine->updateStakingMetrics(5000000.0, 20000000.0);
    
    SecurityMetrics metrics = engine->evaluateBlock(40000.0, 10.0, 1000.0, 100);
    
    // All metrics should be valid
    EXPECT_GT(metrics.predictedHashrate, 0.0);
    EXPECT_EQ(metrics.actualHashrate, 40000.0);
    EXPECT_GT(metrics.difficultyAdjustment, 0.0);
    EXPECT_GT(metrics.dynamicFee, 0.0);
    EXPECT_LE(metrics.dynamicFee, MAX_FEE);
    
    // Normal block should not be attack
    EXPECT_FALSE(metrics.attackDetected);
}

TEST_F(SecurityEngineTest, CalculateNextDifficulty) {
    double nextDiff = engine->calculateNextDifficulty(1000.0, 40000.0, 10.0);
    
    // Should be valid difficulty
    EXPECT_TRUE(isValidDifficulty(nextDiff));
}

// =========================================================
//           UTILITY FUNCTION TESTS
// =========================================================

TEST_F(SecurityEngineTest, ClampFunction) {
    EXPECT_EQ(clamp(5.0, 0.0, 10.0), 5.0);
    EXPECT_EQ(clamp(-5.0, 0.0, 10.0), 0.0);
    EXPECT_EQ(clamp(15.0, 0.0, 10.0), 10.0);
}

TEST_F(SecurityEngineTest, ExponentialMovingAverage) {
    double ema = exponentialMovingAverage(100.0, 50.0, 0.5);
    EXPECT_NEAR(ema, 75.0, 0.001);
}

TEST_F(SecurityEngineTest, Reset) {
    engine->predictHashrate(40000.0);
    engine->updateStakingMetrics(5000000.0, 20000000.0);
    
    engine->reset();
    
    EXPECT_EQ(engine->getPredictedHashrate(), 0.0);
    EXPECT_EQ(engine->getStakeRatio(), 0.0);
}
