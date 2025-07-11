#include <gtest/gtest.h>
#include "GoldToken.h"

TEST(TokensTest, MintAndTransfer) {
    GoldToken token;
    EXPECT_TRUE(token.mint("alice", 10.0, "attest"));
    EXPECT_TRUE(token.transfer("alice", "bob", 5.0));
    EXPECT_EQ(token.balanceOf("bob"), 5.0);
}