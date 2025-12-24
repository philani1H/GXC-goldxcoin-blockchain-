#include "cpu_miner.h"
#include "blockchain.h"
#include "Block.h"
#include "Logger.h"
#include <iostream>

// Define LOG_MINER macro for mining logging
#define LOG_MINER(level, msg) LOG_MINING(level, "[MINER] " + std::string(msg))

CPUMiner::CPUMiner(Blockchain* bc)
    : blockchain(bc), isMining(false), hashesPerSecond(0), totalHashes(0), numThreads(1) {
    lastHashrateUpdate = std::chrono::steady_clock::now();
    hashesSinceLastUpdate = 0;
}

CPUMiner::~CPUMiner() {
    stopMining();
}

void CPUMiner::startMining(const std::string& address, int threads) {
    if (isMining) {
        LOG_MINER(LogLevel::WARNING, "Mining already active");
        return;
    }
    
    if (address.empty()) {
        LOG_MINER(LogLevel::ERROR, "Miner address cannot be empty");
        return;
    }
    
    minerAddress = address;
    numThreads = threads > 0 ? threads : 1;
    isMining = true;
    totalHashes = 0;
    hashesPerSecond = 0;
    
    LOG_MINER(LogLevel::INFO, "Starting CPU miner with " + std::to_string(numThreads) + " thread(s)");
    LOG_MINER(LogLevel::INFO, "Mining to address: " + minerAddress);
    
    // Start mining threads
    for (int i = 0; i < numThreads; i++) {
        miningThreads.push_back(std::thread(&CPUMiner::mineThread, this, i));
    }
}

void CPUMiner::stopMining() {
    if (!isMining) return;
    
    LOG_MINER(LogLevel::INFO, "Stopping CPU miner...");
    isMining = false;
    
    // Wait for all threads to finish
    for (auto& thread : miningThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    miningThreads.clear();
    
    LOG_MINER(LogLevel::INFO, "CPU miner stopped. Total hashes: " + std::to_string(totalHashes.load()));
}

void CPUMiner::mineThread(int threadId) {
    LOG_MINER(LogLevel::INFO, "Mining thread " + std::to_string(threadId) + " started");
    
    while (isMining) {
        // Stub implementation - just simulate mining activity
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Simulate hash calculations
        totalHashes += 1000000;
        hashesSinceLastUpdate += 1000000;
        updateHashrate();
    }
    
    LOG_MINER(LogLevel::INFO, "Mining thread " + std::to_string(threadId) + " stopped");
}

void CPUMiner::updateHashrate() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHashrateUpdate).count();
    
    if (elapsed >= 1000) { // Update every second
        uint64_t hashes = hashesSinceLastUpdate.exchange(0);
        double seconds = elapsed / 1000.0;
        hashesPerSecond = static_cast<double>(hashes) / seconds;
        lastHashrateUpdate = now;
    }
}
