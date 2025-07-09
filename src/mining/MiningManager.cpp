#include "MiningManager.h"
#include "SHA256Miner.h"
#include "EthashMiner.h"
#include "GXHashMiner.h"
#include "HardwareDetector.h"
#include "MiningOptimizer.h"
#include "Logger.h"
#include <thread>
#include <algorithm>
#include <random>

MiningManager::MiningManager(Blockchain* blockchain, Database* database)
    : blockchain_(blockchain), database_(database), isRunning_(false), 
      currentDifficulty_(1.0), networkHashrate_(0.0), totalMinersReward_(0.0) {
    
    hardwareDetector_ = std::make_unique<HardwareDetector>();
    optimizer_ = std::make_unique<MiningOptimizer>();
    
    // Initialize mining algorithms
    initializeAlgorithms();
    
    // Detect available hardware
    detectHardware();
    
    Logger::info("MiningManager initialized");
}

MiningManager::~MiningManager() {
    stop();
}

void MiningManager::initializeAlgorithms() {
    // Register available mining algorithms
    algorithms_["SHA256"] = std::make_unique<SHA256Miner>();
    algorithms_["Ethash"] = std::make_unique<EthashMiner>();
    algorithms_["GXHash"] = std::make_unique<GXHashMiner>();
    
    // Set default algorithm
    currentAlgorithm_ = "GXHash";
    
    Logger::info("Mining algorithms initialized: SHA256, Ethash, GXHash");
}

void MiningManager::detectHardware() {
    try {
        hardwareInfo_ = hardwareDetector_->detectHardware();
        
        Logger::info("Hardware detection complete:");
        Logger::info("  CPU Cores: " + std::to_string(hardwareInfo_.cpuCores));
        Logger::info("  CPU Threads: " + std::to_string(hardwareInfo_.cpuThreads));
        Logger::info("  GPU Count: " + std::to_string(hardwareInfo_.gpuCount));
        Logger::info("  Available Memory: " + std::to_string(hardwareInfo_.availableMemory) + " MB");
        
        for (const auto& gpu : hardwareInfo_.gpuInfo) {
            Logger::info("  GPU: " + gpu.name + " (Memory: " + std::to_string(gpu.memory) + " MB)");
        }
        
    } catch (const std::exception& e) {
        Logger::error("Hardware detection failed: " + std::string(e.what()));
    }
}

bool MiningManager::start() {
    if (isRunning_) {
        Logger::warning("Mining manager is already running");
        return false;
    }
    
    try {
        isRunning_ = true;
        
        // Update difficulty
        updateDifficulty();
        
        // Start mining threads based on available hardware
        startMiningThreads();
        
        // Start monitoring thread
        monitoringThread_ = std::thread(&MiningManager::monitoringLoop, this);
        
        // Start difficulty adjustment thread
        difficultyThread_ = std::thread(&MiningManager::difficultyAdjustmentLoop, this);
        
        Logger::info("Mining manager started successfully");
        Logger::info("Algorithm: " + currentAlgorithm_);
        Logger::info("Difficulty: " + std::to_string(currentDifficulty_));
        Logger::info("Threads: " + std::to_string(miningThreads_.size()));
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to start mining manager: " + std::string(e.what()));
        isRunning_ = false;
        return false;
    }
}

void MiningManager::stop() {
    if (!isRunning_) {
        return;
    }
    
    Logger::info("Stopping mining manager...");
    isRunning_ = false;
    
    // Stop all mining threads
    for (auto& thread : miningThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    miningThreads_.clear();
    
    // Stop monitoring threads
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    
    if (difficultyThread_.joinable()) {
        difficultyThread_.join();
    }
    
    Logger::info("Mining manager stopped");
}

void MiningManager::startMiningThreads() {
    int threadCount = getOptimalThreadCount();
    
    for (int i = 0; i < threadCount; i++) {
        miningThreads_.emplace_back(&MiningManager::miningLoop, this, i);
    }
    
    Logger::info("Started " + std::to_string(threadCount) + " mining threads");
}

void MiningManager::miningLoop(int threadId) {
    Logger::debug("Mining thread " + std::to_string(threadId) + " started");
    
    auto& algorithm = algorithms_[currentAlgorithm_];
    MiningStats threadStats;
    
    while (isRunning_) {
        try {
            // Get current block template
            Block blockTemplate = createBlockTemplate();
            
            // Generate random nonce range for this thread
            uint64_t nonceStart = generateNonceRange(threadId);
            uint64_t nonceEnd = nonceStart + 1000000; // 1M nonce range
            
            // Mine the block
            auto result = algorithm->mine(blockTemplate, nonceStart, nonceEnd, currentDifficulty_);
            
            if (result.found) {
                Logger::info("Block found by thread " + std::to_string(threadId) + 
                           "! Nonce: " + std::to_string(result.nonce) + 
                           ", Hash: " + result.hash);
                
                // Submit the block
                if (submitBlock(result.block)) {
                    // Calculate enhanced reward for early miners
                    double reward = calculateMiningReward(result.block.height);
                    
                    Logger::info("Block submitted successfully! Reward: " + std::to_string(reward) + " GXC");
                    
                    // Update stats
                    threadStats.blocksFound++;
                    totalMinersReward_ += reward;
                    
                    // Record mining reward in database
                    database_->insertMiningReward(result.block.hash, minerAddress_, 
                                                reward, currentAlgorithm_, currentDifficulty_);
                }
            }
            
            // Update thread statistics
            threadStats.hashesComputed += (nonceEnd - nonceStart);
            threadStats.lastActivity = std::chrono::steady_clock::now();
            
            {
                std::lock_guard<std::mutex> lock(statsMutex_);
                threadStats_[threadId] = threadStats;
            }
            
        } catch (const std::exception& e) {
            Logger::error("Mining thread " + std::to_string(threadId) + " error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    Logger::debug("Mining thread " + std::to_string(threadId) + " stopped");
}

Block MiningManager::createBlockTemplate() {
    Block block;
    
    // Get the latest block from blockchain
    Block latestBlock = blockchain_->getLatestBlock();
    
    // Set block properties
    block.height = latestBlock.height + 1;
    block.previousHash = latestBlock.hash;
    block.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    block.difficulty = currentDifficulty_;
    block.validator = minerAddress_;
    
    // Get pending transactions
    block.transactions = blockchain_->getPendingTransactions();
    
    // Calculate merkle root
    block.merkleRoot = blockchain_->calculateMerkleRoot(block.transactions);
    
    // Initialize nonce
    block.nonce = 0;
    
    return block;
}

uint64_t MiningManager::generateNonceRange(int threadId) {
    // Generate deterministic but distributed nonce ranges for different threads
    std::random_device rd;
    std::mt19937_64 gen(rd() + threadId);
    return gen() % (UINT64_MAX / miningThreads_.size()) + 
           (threadId * (UINT64_MAX / miningThreads_.size()));
}

bool MiningManager::submitBlock(const Block& block) {
    try {
        return blockchain_->addBlock(block);
    } catch (const std::exception& e) {
        Logger::error("Failed to submit block: " + std::string(e.what()));
        return false;
    }
}

double MiningManager::calculateMiningReward(int blockHeight) {
    double baseReward = 10.0; // Base reward per block
    double earlyMinerBonus = 0.0;
    
    // Enhanced reward system (increased from 12 to 100+ coins for early miners)
    if (blockHeight < 1000) {
        // First 1,000 blocks: massive early adopter bonus
        earlyMinerBonus = 500.0;
    } else if (blockHeight < 5000) {
        // Next 4,000 blocks: large early miner bonus
        earlyMinerBonus = 250.0;
    } else if (blockHeight < 10000) {
        // Next 5,000 blocks: substantial bonus
        earlyMinerBonus = 100.0;
    } else if (blockHeight < 25000) {
        // Next 15,000 blocks: good bonus
        earlyMinerBonus = 50.0;
    } else if (blockHeight < 50000) {
        // Next 25,000 blocks: moderate bonus
        earlyMinerBonus = 25.0;
    } else if (blockHeight < 100000) {
        // Next 50,000 blocks: small bonus
        earlyMinerBonus = 10.0;
    }
    
    // Halving mechanism every 100,000 blocks
    int halvingCount = blockHeight / 100000;
    double halvingFactor = 1.0 / std::pow(2, halvingCount);
    
    double totalReward = (baseReward + earlyMinerBonus) * halvingFactor;
    
    // Minimum reward to ensure mining remains profitable
    return std::max(totalReward, 1.0);
}

void MiningManager::updateDifficulty() {
    try {
        // Get recent blocks for difficulty calculation
        std::vector<Block> recentBlocks = blockchain_->getRecentBlocks(100);
        
        if (recentBlocks.size() < 2) {
            currentDifficulty_ = 1.0;
            return;
        }
        
        // Calculate average block time
        double totalTime = 0.0;
        for (size_t i = 1; i < recentBlocks.size(); i++) {
            totalTime += (recentBlocks[i-1].timestamp - recentBlocks[i].timestamp);
        }
        double avgBlockTime = totalTime / (recentBlocks.size() - 1);
        
        // Target block time (e.g., 60 seconds)
        double targetBlockTime = 60.0;
        
        // Adjust difficulty based on block time
        double ratio = avgBlockTime / targetBlockTime;
        
        // Smooth difficulty adjustment (max 25% change per adjustment)
        if (ratio > 1.25) {
            currentDifficulty_ *= 0.8; // Decrease difficulty
        } else if (ratio < 0.75) {
            currentDifficulty_ *= 1.2; // Increase difficulty
        } else {
            currentDifficulty_ *= ratio;
        }
        
        // Ensure minimum difficulty
        currentDifficulty_ = std::max(currentDifficulty_, 0.01);
        
        Logger::debug("Difficulty updated: " + std::to_string(currentDifficulty_) + 
                     " (avg block time: " + std::to_string(avgBlockTime) + "s)");
        
    } catch (const std::exception& e) {
        Logger::error("Failed to update difficulty: " + std::string(e.what()));
    }
}

void MiningManager::monitoringLoop() {
    while (isRunning_) {
        try {
            updateMiningStats();
            logMiningStats();
            
            // Update performance metrics in database
            int blockHeight = blockchain_->getHeight();
            database_->insertPerformanceMetric("mining", "hashrate", getCurrentHashrate(), blockHeight);
            database_->insertPerformanceMetric("mining", "difficulty", currentDifficulty_, blockHeight);
            database_->insertPerformanceMetric("mining", "active_threads", miningThreads_.size(), blockHeight);
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            Logger::error("Monitoring loop error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
    }
}

void MiningManager::difficultyAdjustmentLoop() {
    while (isRunning_) {
        try {
            updateDifficulty();
            std::this_thread::sleep_for(std::chrono::minutes(10)); // Adjust every 10 minutes
            
        } catch (const std::exception& e) {
            Logger::error("Difficulty adjustment loop error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::minutes(30));
        }
    }
}

void MiningManager::updateMiningStats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    
    // Calculate total hashrate
    double totalHashrate = 0.0;
    int activeThreads = 0;
    auto now = std::chrono::steady_clock::now();
    
    for (const auto& [threadId, stats] : threadStats_) {
        auto timeSinceLastActivity = std::chrono::duration_cast<std::chrono::seconds>(
            now - stats.lastActivity).count();
        
        if (timeSinceLastActivity < 60) { // Consider thread active if it was active in last 60 seconds
            activeThreads++;
            // Calculate hashrate based on recent activity
            totalHashrate += stats.hashesComputed / std::max(1.0, static_cast<double>(timeSinceLastActivity));
        }
    }
    
    networkHashrate_ = totalHashrate;
}

void MiningManager::logMiningStats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    
    int totalBlocksFound = 0;
    for (const auto& [threadId, stats] : threadStats_) {
        totalBlocksFound += stats.blocksFound;
    }
    
    Logger::info("=== Mining Statistics ===");
    Logger::info("Algorithm: " + currentAlgorithm_);
    Logger::info("Difficulty: " + std::to_string(currentDifficulty_));
    Logger::info("Hashrate: " + std::to_string(networkHashrate_) + " H/s");
    Logger::info("Active Threads: " + std::to_string(miningThreads_.size()));
    Logger::info("Blocks Found: " + std::to_string(totalBlocksFound));
    Logger::info("Total Rewards: " + std::to_string(totalMinersReward_) + " GXC");
    Logger::info("========================");
}

int MiningManager::getOptimalThreadCount() {
    int cpuThreads = hardwareInfo_.cpuThreads;
    int gpuCount = hardwareInfo_.gpuCount;
    
    // Use optimizer to determine optimal thread count
    int optimalThreads = optimizer_->calculateOptimalThreads(hardwareInfo_, currentAlgorithm_);
    
    // Ensure we don't exceed hardware capabilities
    int maxThreads = cpuThreads + (gpuCount * 2); // Allow 2 threads per GPU
    
    return std::min(optimalThreads, maxThreads);
}

void MiningManager::setMinerAddress(const std::string& address) {
    minerAddress_ = address;
    Logger::info("Miner address set: " + address);
}

void MiningManager::setAlgorithm(const std::string& algorithm) {
    if (algorithms_.find(algorithm) != algorithms_.end()) {
        currentAlgorithm_ = algorithm;
        Logger::info("Mining algorithm changed to: " + algorithm);
        
        // Restart mining if it's currently running
        if (isRunning_) {
            stop();
            start();
        }
    } else {
        Logger::error("Unknown mining algorithm: " + algorithm);
    }
}

double MiningManager::getCurrentHashrate() const {
    return networkHashrate_;
}

double MiningManager::getCurrentDifficulty() const {
    return currentDifficulty_;
}

std::vector<std::string> MiningManager::getAvailableAlgorithms() const {
    std::vector<std::string> algorithms;
    for (const auto& [name, _] : algorithms_) {
        algorithms.push_back(name);
    }
    return algorithms;
}

MiningStats MiningManager::getMiningStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    
    MiningStats totalStats;
    for (const auto& [threadId, stats] : threadStats_) {
        totalStats.hashesComputed += stats.hashesComputed;
        totalStats.blocksFound += stats.blocksFound;
    }
    
    return totalStats;
}

bool MiningManager::isRunning() const {
    return isRunning_;
}

void MiningManager::pauseMining() {
    if (isRunning_) {
        Logger::info("Pausing mining operations...");
        // Implementation for pausing without fully stopping
        // This could involve setting a pause flag that mining threads check
    }
}

void MiningManager::resumeMining() {
    if (isRunning_) {
        Logger::info("Resuming mining operations...");
        // Implementation for resuming paused mining
    }
}