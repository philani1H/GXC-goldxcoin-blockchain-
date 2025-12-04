#pragma once

#include "../blockchain.h"
#include "../Logger.h"
#include "../Utils.h"
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <cstdint>
#include <chrono>
#include <algorithm>

// Forward declarations
class Blockchain;

struct PoolConfig {
    std::string url;
    std::string username;
    std::string password;
    uint32_t priority = 0;
    std::string algorithm;
};

struct MiningShare {
    std::string jobId;
    uint64_t nonce;
    std::string hash;
    std::time_t timestamp;
    bool accepted = false;
};

struct MiningWork {
    std::string jobId;
    std::string previousHash;
    std::string merkleRoot;
    uint64_t difficulty;
    std::time_t timestamp;
    std::vector<std::string> transactions;
};

using PoolWork = MiningWork;  // Alias for compatibility

class PoolManager {
private:
    Blockchain* blockchain;
    std::vector<PoolConfig> pools;
    mutable std::mutex poolsMutex;
    mutable std::mutex socketMutex;
    
    int socketFd;
    bool connected;
    std::string currentPoolUrl;
    bool shouldStop;
    
    double poolHashRate;
    uint32_t poolDifficulty;
    uint64_t poolShares;
    
    MiningWork currentWork;
    std::function<void(const MiningWork&)> workCallback;
    std::function<void(uint32_t)> difficultyCallback;
    std::function<void(bool)> connectionCallback;
    std::chrono::time_point<std::chrono::steady_clock> lastWorkTime;
    
    // Private methods
    void connectionThread();
    void handleStratumMessage(const std::string& message);
    void sendStratumMessage(const std::string& message);
    bool parseStratumWork(const std::string& message, MiningWork& work);
    void processShareResponse(const std::string& response);

public:
    PoolManager(Blockchain* blockchainPtr);
    ~PoolManager();
    
    void addPool(const PoolConfig& config);
    void removePool(const std::string& url);
    void setPoolPriority(const std::string& url, uint32_t priority);
    std::vector<PoolConfig> getPools() const;
    
    bool connectToPool(const std::string& url = "");
    void disconnectFromPool();
    bool isConnected() const;
    std::string getCurrentPool() const;
    
    void requestWork();
    void submitShare(const MiningShare& share);
    
    MiningWork getCurrentWork() const;
    double getPoolHashRate() const;
    uint32_t getPoolDifficulty() const;
    uint64_t getPoolShares() const;
    std::chrono::time_point<std::chrono::steady_clock> getLastWorkTime() const;
    
    void setWorkCallback(std::function<void(const MiningWork&)> callback);
    void setDifficultyCallback(std::function<void(uint32_t)> callback);
    void setConnectionCallback(std::function<void(bool)> callback);
    
    void startStratumClient();
    void stopStratumClient();
    void processStratumMessage(const std::string& message);
    void handleNetworkError(const std::string& error);
};
