#pragma once

#include "../blockchain.h"
#include "../Logger.h"
#include "MiningTypes.h"
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

// Forward declarations
class Blockchain;

enum class DeviceType {
    CPU,
    GPU,
    ASIC
};

struct MiningDevice {
    uint32_t id;
    DeviceType type;
    std::string name;
    std::string vendor;
    uint64_t memory;
    uint32_t computeUnits;
    uint32_t clockSpeed;
    double temperature;
    double power;
    bool available;
    bool active;
    double hashRate;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    uint64_t staleShares;
    std::chrono::time_point<std::chrono::steady_clock> lastActivity;
};

class HardwareDetector {
public:
    static std::vector<MiningDevice> detectCPUs(Blockchain* blockchain = nullptr);
    static std::vector<MiningDevice> detectNVIDIAGPUs();
    static std::vector<MiningDevice> detectAMDGPUs();
    static std::vector<MiningDevice> detectASICDevices();
    static std::vector<MiningDevice> detectAllDevices();
    
    static std::string getCPUInfo();
    static std::string getGPUInfo(uint32_t deviceId);
    static std::string getASICInfo(uint32_t deviceId);
    
    static bool isDeviceSupported(const MiningDevice& device, MiningAlgorithm algorithm);
};
