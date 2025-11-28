#pragma once

#include "../blockchain.h"
#include "../Logger.h"
#include "../transaction.h"
#include "../Block.h"
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>

enum class MiningAlgorithm {
    SHA256,
    ETHASH,
    GXHASH
};

struct BlockTemplate {
    std::string previousHash;
    uint32_t height;
    double difficulty;
    uint64_t timestamp;
    std::string minerAddress;
    std::vector<Transaction> transactions;
};

struct MinerInfo {
    std::string id;
    MiningAlgorithm algorithm;
    bool active;
    double hashRate;
    uint64_t startTime;
    std::thread thread;
};

struct MiningStats {
    MiningAlgorithm algorithm;
    double hashRate;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    uint64_t uptime;
    std::string minerAddress;
    std::string poolUrl;
    double difficulty;
};

class MiningManager {
private:
    Blockchain* blockchain;
    std::atomic<bool> isRunning;
    MiningAlgorithm currentAlgorithm;
    std::string minerAddress;
    double hashRate;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    std::thread coordinatorThread;
    std::unordered_map<std::string, MinerInfo> miners;
    mutable std::mutex miningMutex;
    double currentDifficulty;
    std::string poolUrl;
    std::string poolUsername;
    uint64_t startTime;
    
    void coordinatorLoop();
    void startMiningThreads();
    void startMiner(const std::string& minerId, MiningAlgorithm algorithm);
    void miningLoop(const std::string& minerId, MiningAlgorithm algorithm);
    BlockTemplate getCurrentBlockTemplate();
    bool mineSHA256(const BlockTemplate& blockTemplate, uint64_t nonce);
    bool mineEthash(const BlockTemplate& blockTemplate, uint64_t nonce);
    bool mineGXHash(const BlockTemplate& blockTemplate, uint64_t nonce);
    bool checkDifficultyTarget(const std::string& hash, double difficulty);
    void submitShare(const BlockTemplate& blockTemplate, uint64_t nonce, MiningAlgorithm algorithm);
    void submitToPool(const BlockTemplate& blockTemplate, uint64_t nonce, MiningAlgorithm algorithm);
    Block createBlockFromTemplate(const BlockTemplate& blockTemplate, uint64_t nonce);
    void updateStatistics();
    void checkForNewBlocks();
    void adjustDifficulty();
    void printMiningStats();
    void updateMinerHashRate(const std::string& minerId, uint64_t hashCount, uint64_t startTime);
    std::string serializeBlockTemplate(const BlockTemplate& blockTemplate, uint64_t nonce);
    std::string algorithmToString(MiningAlgorithm algorithm);
    
public:
    MiningManager(Blockchain* blockchain);
    ~MiningManager();
    
    bool start(const std::string& minerAddress);
    void stop();
    bool isMining() const { return isRunning; }
    
    void setMiningAlgorithm(MiningAlgorithm algorithm);
    void setDifficulty(double difficulty);
    void setPoolInfo(const std::string& poolUrl, const std::string& username);
    MiningAlgorithm getAlgorithm() const { return currentAlgorithm; }
    
    MiningStats getStats() const;
    std::vector<MinerInfo> getActiveMiners() const;
    
    double getHashRate() const;
    uint64_t getAcceptedShares() const;
    uint64_t getRejectedShares() const;
};
