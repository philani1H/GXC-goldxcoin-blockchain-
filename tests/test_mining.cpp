#include <gtest/gtest.h>
#include "GXCMiner.h"

TEST(MiningTest, DetectDevices) {
    auto devices = HardwareDetector::detectAllDevices();
    EXPECT_FALSE(devices.empty());
}