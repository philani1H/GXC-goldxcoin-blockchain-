#pragma once

#include "../Logger.h"
#include "../Utils.h"
#include "../HashUtils.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <functional>
#include <sstream>

// MiningAlgorithm enum (shared across miners)
enum class MiningAlgorithm {
    SHA256,
    ETHASH,
    GXHASH
};

struct MiningJob {
    std::string jobId;
    std::string previousHash;
    std::string merkleRoot;
    std::time_t timestamp;
    std::string bits;
    double difficulty;
    uint32_t blockNumber;
    std::vector<std::string> transactions;
};

struct MiningStats {
    MiningAlgorithm algorithm;
    double hashRate;
    uint64_t totalHashes;
    uint32_t threadsActive;
    std::time_t uptime;
    uint64_t dagSize = 0;
    uint64_t epoch = 0;
    uint64_t traceabilityValidations = 0;
};

struct MiningSolution {
    std::string jobId;
    uint64_t nonce;
    uint64_t extraNonce;
    std::time_t timestamp;
    MiningAlgorithm algorithm;
};

using SolutionCallback = std::function<void(const MiningSolution&)>;

class SHA256Miner {
private:
    std::atomic<bool> isRunning;
    std::atomic<double> hashRate;
    std::atomic<uint64_t> totalHashes;
    uint32_t threadsCount;
    std::time_t startTime;
    
    MiningJob currentJob;
    std::mutex jobMutex;
    bool jobUpdated;
    
    std::vector<std::thread> workerThreads;
    std::thread statsThread;
    
    mutable std::mutex statsMutex;
    std::unordered_map<uint32_t, double> threadHashRates;
    
    SolutionCallback solutionCallback;
    
    void miningThread(uint32_t threadId);
    bool mineSHA256Block(const MiningJob& job, uint64_t nonce);
    bool mineSHA256BlockOptimized(const MiningJob& job, uint64_t nonce);
    bool mineSHA256BlockSSE(const MiningJob& job, uint64_t nonce);
    bool mineSHA256BlockAVX(const MiningJob& job, uint64_t nonce);
    std::string constructBlockHeader(const MiningJob& job, uint64_t nonce);
    bool checkDifficultyTarget(const std::string& hash, double difficulty);
    void submitSolution(const MiningJob& job, uint64_t nonce);
    void updateThreadStats(uint32_t threadId, uint64_t hashCount, std::time_t startTime);
    void statsLoop();

public:
    SHA256Miner();
    ~SHA256Miner();
    
    bool start(uint32_t threads = 0);
    void stop();
    void setJob(const MiningJob& job);
    MiningStats getStats() const;
    void setSolutionCallback(const SolutionCallback& callback);
    bool isMiningCapable();
    std::string getOptimizationInfo();
};
