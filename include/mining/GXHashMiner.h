#pragma once

#include "MiningTypes.h"

// GXHash-specific constants
constexpr uint32_t GXHASH_ROUNDS = 16;

// GXHash result structure
struct GXHashResult {
    std::string hash;
    std::string traceabilityHash;
    std::string crossValidationHash;
    bool traceabilityValid = false;
};

class GXHashMiner {
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
    
    // GXHash-specific
    std::atomic<uint64_t> traceabilityValidations;
    bool traceabilityOptimized;
    std::unordered_map<std::string, bool> traceabilityCache;
    std::mutex traceabilityCacheMutex;
    
    SolutionCallback solutionCallback;
    
    void miningThread(uint32_t threadId);
    bool mineGXHashBlock(const MiningJob& job, uint64_t nonce);
    std::string constructBlockHeader(const MiningJob& job, uint64_t nonce);
    std::string constructGXHashHeader(const MiningJob& job, uint64_t nonce);
    GXHashResult computeGXHash(const std::string& blockHeader, uint64_t nonce, 
                              const std::vector<std::string>& transactions);
    bool checkDifficultyTarget(const std::string& hash, double difficulty);
    void submitSolution(const MiningJob& job, uint64_t nonce);
    void updateThreadStats(uint32_t threadId, uint64_t hashCount, std::time_t startTime, 
                          uint64_t traceabilityChecks);
    void statsLoop();
    
    // Traceability methods
    void initializeTraceabilityCache();
    void updateTraceabilityCache(const MiningJob& job);
    bool validateTraceability(const std::string& txHash, const std::string& prevTxHash);

public:
    GXHashMiner();
    ~GXHashMiner();
    
    bool start(uint32_t threads = 0);
    void stop();
    void setJob(const MiningJob& job);
    MiningStats getStats() const;
    void setSolutionCallback(const SolutionCallback& callback);
    bool isMiningCapable();
    void setTraceabilityOptimization(bool enabled);
    std::string getOptimizationInfo();
};
