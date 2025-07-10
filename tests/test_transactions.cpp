#include <gtest/gtest.h>
#include "transaction.h"

TEST(TransactionTest, Creation) {
    Transaction tx;
    EXPECT_TRUE(tx.isValid());
}