#pragma once

#include "blockchain.h"
#include "Logger.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstdint>

// Forward declarations
class Blockchain;

struct StratumJob {
    std::string jobId;
    std::string prevHash;
    std::string prevBlockHash;
    std::string coinbase1;
    std::string coinbase2;
    std::vector<std::string> merkleBranches;
    std::string version;
    std::string nBits;
    std::string nTime;
    bool cleanJobs;
    int64_t timestamp;
    double difficulty;
    uint32_t height;
    std::vector<Transaction> transactions;
    Transaction coinbaseTransaction;
    std::string merkleRoot;
    uint32_t nonceMask;
    std::string extraNonce1;
    uint32_t extraNonce2Size;
};

struct StratumMiner {
    std::string id;
    std::string address;
    bool connected;
    bool authorized;
    std::string username;
    int64_t connectTime;
    int64_t lastActivity;
    double difficulty;
    double hashRate;
    uint64_t sharesAccepted;
    uint64_t sharesRejected;
    std::string currentJobId;
};

struct StratumShare {
    std::string jobId;
    std::string nonce;
    std::string extraNonce2;
    std::string nTime;
    std::string minerId;
    double difficulty;
};

struct StratumStats {
    size_t connectedMiners;
    double totalHashRate;
    uint64_t totalShares;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    uint64_t blocksFound;
};

class StratumServer {
public:
    StratumServer(Blockchain* blockchain, uint16_t port);
    ~StratumServer();

    bool start();
    void stop();
    bool isServerRunning() const { return isRunning; }

    // Miner management
    void addMiner(const std::string& minerId, const std::string& address);
    void removeMiner(const std::string& minerId);
    bool authorizeMiner(const std::string& minerId, const std::string& username, const std::string& password);

    // Share submission
    void submitShare(const std::string& minerId, const StratumShare& share);

    // Job management
    void broadcastJob(const StratumJob& job);
    StratumJob generateMiningJob();

    // Statistics
    std::vector<StratumMiner> getConnectedMiners() const;
    StratumStats getStats() const;

    double getDifficulty() const { return difficulty; }
    void setDifficulty(double diff) { difficulty = diff; }

private:
    Blockchain* blockchain;
    uint16_t serverPort;
    std::atomic<bool> isRunning;
    double difficulty;
    uint32_t nextJobId;

    std::mutex jobsMutex;
    std::unordered_map<std::string, StratumJob> jobs;
    std::unordered_map<std::string, StratumJob> currentJobs;

    std::mutex minersMutex;
    std::unordered_map<std::string, StratumMiner> miners;

    std::thread serverThread;
    std::thread jobThread;
    std::atomic<uint64_t> blocksFound;

    void serverLoop();
    void jobGenerationLoop();
    void handleIncomingConnections();
    void processMinerMessages();
    void cleanupInactiveMiners();
    void updateStatistics();
    void sendMiningJob(const std::string& minerId);
    void sendJobToMiner(const std::string& minerId, const StratumJob& job);
    void sendShareResponse(const std::string& minerId, bool accepted, const std::string& reason);
    bool validateShare(const StratumShare& share);
    bool shareIsBlockSolution(const StratumShare& share);
    void submitBlockSolution(const StratumShare& share);
    Block createBlockFromShare(const StratumJob& job, const StratumShare& share);
    void updateMinerHashRate(const std::string& minerId);
    std::string calculateMerkleRoot(const std::vector<Transaction>& transactions);
    std::string generateExtraNonce();
    std::string constructBlockHeader(const StratumJob& job, const StratumShare& share);
    bool checkDifficulty(const std::string& hash, double difficulty);
    std::string createJobMessage(const StratumJob& job);
    std::string createShareResponse(bool accepted, const std::string& reason);
};
