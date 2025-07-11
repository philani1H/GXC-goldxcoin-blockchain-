#include <gtest/gtest.h>
#include "blockchain.h"

TEST(BlockchainTest, Initialization) {
    Blockchain chain;
    EXPECT_EQ(chain.getHeight(), 0);
}

TEST(BlockchainTest, AddBlock) {
    Blockchain chain;
    // Add a block (stub)
    // chain.addBlock(...);
    SUCCEED();
}