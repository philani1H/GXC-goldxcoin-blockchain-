#include "cpu_miner.h"
#include "blockchain.h"
#include "block.h"
#include "logger.h"
#include <iostream>

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
        try {
            // Get block template
            Block block = blockchain->createBlock(minerAddress);
            
            // Try to mine the block
            uint64_t nonce = threadId * 1000000; // Offset nonce by thread ID
            if (tryMineBlock(block, nonce)) {
                // Block mined successfully!
                LOG_MINER(LogLevel::INFO, "Thread " + std::to_string(threadId) + 
                         " mined block " + std::to_string(block.getIndex()) + 
                         "! Hash: " + block.getHash().substr(0, 16) + "...");
                
                // Add block to blockchain
                if (blockchain->addBlock(block, minerAddress)) {
                    LOG_MINER(LogLevel::INFO, "Block added to blockchain successfully");
                } else {
                    LOG_MINER(LogLevel::WARNING, "Failed to add mined block to blockchain");
                }
                
                // Small delay before mining next block
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } catch (const std::exception& e) {
            LOG_MINER(LogLevel::ERROR, "Mining thread error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_MINER(LogLevel::INFO, "Mining thread " + std::to_string(threadId) + " stopped");
}

bool CPUMiner::tryMineBlock(Block& block, uint64_t& nonce) {
    const uint64_t maxAttempts = 1000000; // Try 1M hashes before getting new template
    uint64_t attempts = 0;
    
    while (isMining && attempts < maxAttempts) {
        block.setNonce(nonce);
        block.calculateHash();
        
        totalHashes++;
        hashesSinceLastUpdate++;
        attempts++;
        nonce++;
        
        // Update hashrate every second
        if (attempts % 10000 == 0) {
            updateHashrate();
        }
        
        // Check if hash meets difficulty target
        if (block.isHashValid()) {
            return true;
        }
    }
    
    return false;
}

void CPUMiner::updateHashrate() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHashrateUpdate).count();
    
    if (elapsed >= 1000) { // Update every second
        uint64_t hashes = hashesSinceLastUpdate.exchange(0);
        double seconds = elapsed / 1000.0;
        hashesPerSecond = static_cast<uint64_t>(hashes / seconds);
        lastHashrateUpdate = now;
    }
}
