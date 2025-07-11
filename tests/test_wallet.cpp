#include <gtest/gtest.h>
#include "Wallet.h"

TEST(WalletTest, Creation) {
    Wallet w;
    EXPECT_EQ(w.getBalance(), 0);
}