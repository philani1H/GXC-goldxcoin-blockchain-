#ifndef CPU_MINER_H
#define CPU_MINER_H

#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>

class Blockchain;
class Block;

class CPUMiner {
private:
    Blockchain* blockchain;
    std::vector<std::thread> miningThreads;
    std::atomic<bool> isMining;
    std::atomic<uint64_t> hashesPerSecond;
    std::atomic<uint64_t> totalHashes;
    std::string minerAddress;
    int numThreads;
    
    std::chrono::steady_clock::time_point lastHashrateUpdate;
    std::atomic<uint64_t> hashesSinceLastUpdate;
    
    void mineThread(int threadId);
    bool tryMineBlock(Block& block, uint64_t& nonce);
    void updateHashrate();
    
public:
    CPUMiner(Blockchain* bc);
    ~CPUMiner();
    
    void startMining(const std::string& address, int threads = 1);
    void stopMining();
    bool isMiningActive() const { return isMining; }
    uint64_t getHashrate() const { return hashesPerSecond; }
    uint64_t getTotalHashes() const { return totalHashes; }
    std::string getMinerAddress() const { return minerAddress; }
};

#endif // CPU_MINER_H
