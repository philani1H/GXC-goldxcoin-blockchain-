#include <gtest/gtest.h>
#include "../include/mining/MiningTypes.h"
#include "../include/mining/HardwareDetector.h"

TEST(MiningTest, DetectDevices) {
    auto devices = HardwareDetector::detectAllDevices();
    EXPECT_FALSE(devices.empty());
}

TEST(MiningTest, MiningJobCreation) {
    MiningJob job;
    job.jobId = "test_job";
    job.previousHash = "0000000000000000000000000000000000000000000000000000000000000000";
    job.difficulty = 1.0;
    job.blockNumber = 1;
    
    EXPECT_EQ(job.jobId, "test_job");
    EXPECT_EQ(job.blockNumber, 1);
}

TEST(MiningTest, MiningSolutionCreation) {
    MiningSolution solution;
    solution.jobId = "test_job";
    solution.nonce = 12345;
    solution.algorithm = MiningAlgorithm::GXHASH;
    solution.traceabilityValid = true;
    
    EXPECT_EQ(solution.jobId, "test_job");
    EXPECT_EQ(solution.nonce, 12345);
    EXPECT_EQ(solution.algorithm, MiningAlgorithm::GXHASH);
    EXPECT_TRUE(solution.traceabilityValid);
}

TEST(MiningTest, MiningAlgorithmTypes) {
    // Verify algorithm enum values
    EXPECT_NE(MiningAlgorithm::SHA256, MiningAlgorithm::ETHASH);
    EXPECT_NE(MiningAlgorithm::ETHASH, MiningAlgorithm::GXHASH);
    EXPECT_NE(MiningAlgorithm::SHA256, MiningAlgorithm::GXHASH);
}
