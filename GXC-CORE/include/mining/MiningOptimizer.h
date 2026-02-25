#pragma once

#include "../blockchain.h"
#include "../Logger.h"
#include <string>
#include <vector>
#include <cstdint>

// Forward declarations
class Blockchain;

class MiningOptimizer {
public:
    static void optimizeSystem(Blockchain* blockchain = nullptr);
    static void setCPUAffinity(const std::vector<uint32_t>& cores);
    static void setPriority(int priority);
    static void enableLargePagesSupport();
    static void disableWindowsDefender();
    static void configureGPUSettings();
    static void optimizeNetworkSettings();
    
    static std::vector<uint32_t> findOptimalClockSpeeds(uint32_t deviceId);
    static uint32_t findOptimalThreadCount();
    static uint32_t findOptimalIntensity(uint32_t deviceId);
};
