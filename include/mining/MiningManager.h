#pragma once

#include "../blockchain.h"
#include "../Logger.h"
#include "../Utils.h"
#include "../Block.h"
#include "../transaction.h"
#include "EthashMiner.h"  // For shared MiningAlgorithm enum
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstdint>

// Forward declarations
class Blockchain;

// Note: MiningAlgorithm is defined in EthashMiner.h

struct BlockTemplate {
    std::string previousHash;
    uint32_t height;
    double difficulty;
    std::time_t timestamp;
    std::string minerAddress;
    std::vector<Transaction> transactions;
};

struct MinerInfo {
    std::string id;
    MiningAlgorithm algorithm;
    bool active;
    double hashRate;
    std::time_t startTime;
    std::thread thread;
    
    // Delete copy constructor and assignment operator because thread is not copyable
    MinerInfo(const MinerInfo&) = delete;
    MinerInfo& operator=(const MinerInfo&) = delete;
    
    // Allow move constructor and assignment
    MinerInfo(MinerInfo&&) = default;
    MinerInfo& operator=(MinerInfo&&) = default;
    
    MinerInfo() = default;
};

struct MiningManagerStats {
    MiningAlgorithm algorithm;
    double hashRate;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    std::time_t uptime;
    std::string minerAddress;
    std::string poolUrl;
    double difficulty;
};

class MiningManager {
private:
    Blockchain* blockchain;
    std::atomic<bool> isRunning;
    MiningAlgorithm currentAlgorithm;
    double hashRate;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    std::string minerAddress;
    std::string poolUrl;
    std::string poolUsername;
    double currentDifficulty;
    std::time_t startTime;
    
    std::unordered_map<std::string, MinerInfo> miners;
    mutable std::mutex miningMutex;
    std::thread coordinatorThread;
    
    void startMiningThreads();
    void startMiner(const std::string& minerId, MiningAlgorithm algorithm);
    void coordinatorLoop();
    void miningLoop(const std::string& minerId, MiningAlgorithm algorithm);
    BlockTemplate getCurrentBlockTemplate();
    bool mineSHA256(const BlockTemplate& blockTemplate, uint64_t nonce);
    bool mineEthash(const BlockTemplate& blockTemplate, uint64_t nonce);
    bool mineGXHash(const BlockTemplate& blockTemplate, uint64_t nonce);
    bool checkDifficultyTarget(const std::string& hash, double difficulty);
    void submitShare(const BlockTemplate& blockTemplate, uint64_t nonce, MiningAlgorithm algorithm);
    void submitToPool(const BlockTemplate& blockTemplate, uint64_t nonce, MiningAlgorithm algorithm);
    Block createBlockFromTemplate(const BlockTemplate& blockTemplate, uint64_t nonce);
    std::string serializeBlockTemplate(const BlockTemplate& blockTemplate, uint64_t nonce);
    void updateStatistics();
    void updateMinerHashRate(const std::string& minerId, uint64_t hashCount, std::time_t startTime);
    void checkForNewBlocks();
    void adjustDifficulty();
    void printMiningStats();
    std::string algorithmToString(MiningAlgorithm algorithm);

public:
    MiningManager(Blockchain* blockchain);
    ~MiningManager();
    
    bool start(const std::string& minerAddress);
    void stop();
    void setMiningAlgorithm(MiningAlgorithm algorithm);
    void setDifficulty(double difficulty);
    void setPoolInfo(const std::string& poolUrl, const std::string& username);
    MiningManagerStats getStats() const;
    std::vector<MinerInfo> getActiveMiners() const;
};
