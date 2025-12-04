#include <gtest/gtest.h>
#include "../include/transaction.h"

TEST(TransactionTest, Creation) {
    // Test coinbase transaction creation (which is always valid)
    Transaction coinbase("miner_address", 12.5);
    EXPECT_TRUE(coinbase.isCoinbaseTransaction());
    EXPECT_TRUE(coinbase.isValid());
}

TEST(TransactionTest, CoinbaseProperties) {
    Transaction coinbase("miner_address", 12.5);
    
    // Coinbase should have no inputs
    EXPECT_TRUE(coinbase.getInputs().empty());
    
    // Coinbase should have one output
    EXPECT_EQ(coinbase.getOutputs().size(), 1);
    EXPECT_EQ(coinbase.getOutputs()[0].amount, 12.5);
    EXPECT_EQ(coinbase.getOutputs()[0].address, "miner_address");
}

TEST(TransactionTest, TransactionTypes) {
    // Check transaction type enum values
    EXPECT_NE(TransactionType::NORMAL, TransactionType::STAKE);
    EXPECT_NE(TransactionType::STAKE, TransactionType::UNSTAKE);
}

TEST(TransactionTest, TraceabilityFormula) {
    // Coinbase transactions are exempt from traceability
    Transaction coinbase("miner_address", 12.5);
    EXPECT_TRUE(coinbase.verifyTraceabilityFormula());
}
