#include "../../include/mining/MiningManager.h"
#include "../../include/Logger.h"
#include "../../include/Utils.h"
#include "../../include/HashUtils.h"
#include "../../include/Wallet.h"
#include <thread>
#include <chrono>
#include <algorithm>
#include <sstream>

MiningManager::MiningManager(Blockchain* blockchain)
    : blockchain(blockchain), isRunning(false), currentAlgorithm(MiningAlgorithm::SHA256),
      hashRate(0.0), acceptedShares(0), rejectedShares(0) {
    LOG_MINING(LogLevel::INFO, "Mining manager initialized");
}

MiningManager::~MiningManager() {
    stop();
}

bool MiningManager::start(const std::string& minerAddress) {
    if (isRunning) {
        LOG_MINING(LogLevel::WARNING, "Mining manager is already running");
        return true;
    }
    
    if (minerAddress.empty()) {
        LOG_MINING(LogLevel::ERROR, "Miner address is required");
        return false;
    }
    
    // Validate miner address format
    if (!Wallet::isValidAddress(minerAddress)) {
        LOG_MINING(LogLevel::ERROR, "Invalid miner address format: " + minerAddress);
        LOG_MINING(LogLevel::ERROR, "Address must start with 'GXC' (mainnet) or 'tGXC' (testnet)");
        return false;
    }
    
    // Log address type
    if (Wallet::isTestnetAddress(minerAddress)) {
        LOG_MINING(LogLevel::INFO, "Using testnet miner address");
    } else if (Wallet::isMainnetAddress(minerAddress)) {
        LOG_MINING(LogLevel::INFO, "Using mainnet miner address");
    }
    
    this->minerAddress = minerAddress;
    
    LOG_MINING(LogLevel::INFO, "Starting mining manager");
    LOG_MINING(LogLevel::INFO, "Miner address: " + minerAddress);
    
    try {
        isRunning = true;
        
        // Start mining threads for each algorithm
        startMiningThreads();
        
        // Start coordination thread
        coordinatorThread = std::thread(&MiningManager::coordinatorLoop, this);
        
        LOG_MINING(LogLevel::INFO, "Mining manager started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Failed to start mining manager: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void MiningManager::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_MINING(LogLevel::INFO, "Stopping mining manager");
    
    isRunning = false;
    
    // Stop all mining threads
    for (auto& miner : miners) {
        if (miner.second.thread.joinable()) {
            miner.second.thread.join();
        }
    }
    miners.clear();
    
    // Stop coordinator thread
    if (coordinatorThread.joinable()) {
        coordinatorThread.join();
    }
    
    LOG_MINING(LogLevel::INFO, "Mining manager stopped");
}

void MiningManager::setMiningAlgorithm(MiningAlgorithm algorithm) {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    if (currentAlgorithm != algorithm) {
        currentAlgorithm = algorithm;
        LOG_MINING(LogLevel::INFO, "Switched to mining algorithm: " + algorithmToString(algorithm));
        
        // Reset performance counters
        hashRate = 0.0;
        acceptedShares = 0;
        rejectedShares = 0;
    }
}

void MiningManager::setDifficulty(double difficulty) {
    std::lock_guard<std::mutex> lock(miningMutex);
    this->currentDifficulty = difficulty;
    
    LOG_MINING(LogLevel::INFO, "Mining difficulty updated: " + std::to_string(difficulty));
}

void MiningManager::setPoolInfo(const std::string& poolUrl, const std::string& username) {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    this->poolUrl = poolUrl;
    this->poolUsername = username;
    
    if (!poolUrl.empty()) {
        LOG_MINING(LogLevel::INFO, "Pool mining configured: " + poolUrl + " (user: " + username + ")");
    } else {
        LOG_MINING(LogLevel::INFO, "Solo mining mode");
    }
}

MiningStats MiningManager::getStats() const {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    MiningStats stats;
    stats.algorithm = currentAlgorithm;
    stats.hashRate = hashRate;
    stats.acceptedShares = acceptedShares;
    stats.rejectedShares = rejectedShares;
    stats.uptime = Utils::getCurrentTimestamp() - startTime;
    stats.minerAddress = minerAddress;
    stats.poolUrl = poolUrl;
    stats.difficulty = currentDifficulty;
    
    return stats;
}

std::vector<MinerInfo> MiningManager::getActiveMiners() const {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    std::vector<MinerInfo> activeMiners;
    for (const auto& miner : miners) {
        if (miner.second.active) {
            MinerInfo info;
            info.id = miner.second.id;
            info.algorithm = miner.second.algorithm;
            info.active = miner.second.active;
            info.hashRate = miner.second.hashRate;
            info.startTime = miner.second.startTime;
            // Note: thread is not copied as it's not copyable
            activeMiners.push_back(std::move(info));
        }
    }
    
    return activeMiners;
}

void MiningManager::startMiningThreads() {
    startTime = Utils::getCurrentTimestamp();
    
    // Start SHA256 miner
    startMiner("sha256", MiningAlgorithm::SHA256);
    
    // Start Ethash miner
    startMiner("ethash", MiningAlgorithm::ETHASH);
    
    // Start GXHash miner
    startMiner("gxhash", MiningAlgorithm::GXHASH);
    
    LOG_MINING(LogLevel::INFO, "Started " + std::to_string(miners.size()) + " mining threads");
}

void MiningManager::startMiner(const std::string& minerId, MiningAlgorithm algorithm) {
    MinerInfo miner;
    miner.id = minerId;
    miner.algorithm = algorithm;
    miner.active = true;
    miner.hashRate = 0.0;
    miner.startTime = Utils::getCurrentTimestamp();
    
    // Start mining thread
    miner.thread = std::thread(&MiningManager::miningLoop, this, minerId, algorithm);
    
    {
        std::lock_guard<std::mutex> lock(miningMutex);
        miners[minerId] = std::move(miner);
    }
    
    LOG_MINING(LogLevel::INFO, "Started miner: " + minerId + " (" + algorithmToString(algorithm) + ")");
}

void MiningManager::coordinatorLoop() {
    LOG_MINING(LogLevel::INFO, "Mining coordinator started");
    
    auto lastStatsUpdate = Utils::getCurrentTimestamp();
    
    while (isRunning) {
        try {
            // Update mining statistics
            updateStatistics();
            
            // Check for new blocks
            checkForNewBlocks();
            
            // Adjust difficulty if needed
            adjustDifficulty();
            
            // Print stats every 60 seconds
            auto currentTime = Utils::getCurrentTimestamp();
            if (currentTime - lastStatsUpdate >= 60) {
                printMiningStats();
                lastStatsUpdate = currentTime;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in mining coordinator: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_MINING(LogLevel::INFO, "Mining coordinator stopped");
}

void MiningManager::miningLoop(const std::string& minerId, MiningAlgorithm algorithm) {
    LOG_MINING(LogLevel::INFO, "Mining loop started for " + minerId);
    
    uint64_t nonce = Utils::randomUint64();
    uint64_t hashCount = 0;
    auto startTime = Utils::getCurrentTimestamp();
    
    while (isRunning) {
        try {
            // Get current block template
            BlockTemplate blockTemplate = getCurrentBlockTemplate();
            
            // Mine based on algorithm
            bool found = false;
            switch (algorithm) {
                case MiningAlgorithm::SHA256:
                    found = mineSHA256(blockTemplate, nonce);
                    break;
                case MiningAlgorithm::ETHASH:
                    found = mineEthash(blockTemplate, nonce);
                    break;
                case MiningAlgorithm::GXHASH:
                    found = mineGXHash(blockTemplate, nonce);
                    break;
            }
            
            hashCount++;
            nonce++;
            
            // Update hash rate every 1000 hashes
            if (hashCount % 1000 == 0) {
                updateMinerHashRate(minerId, hashCount, startTime);
            }
            
            if (found) {
                submitShare(blockTemplate, nonce, algorithm);
                LOG_MINING(LogLevel::INFO, "Found valid share with " + algorithmToString(algorithm));
            }
            
            // Small delay to prevent overwhelming the CPU
            if (hashCount % 100 == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in mining loop for " + minerId + ": " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_MINING(LogLevel::INFO, "Mining loop stopped for " + minerId);
}

BlockTemplate MiningManager::getCurrentBlockTemplate() {
    BlockTemplate template_;
    
    // Get latest block
    Block latestBlock = blockchain->getLatestBlock();
    
    template_.previousHash = latestBlock.getHash();
    template_.height = latestBlock.getIndex() + 1;
    template_.difficulty = blockchain->getDifficulty();
    template_.timestamp = Utils::getCurrentTimestamp();
    template_.minerAddress = minerAddress;
    
    // Add pending transactions with traceability validation
    auto pendingTx = blockchain->getPendingTransactions(100); // Get up to 100 pending transactions
    for (const auto& tx : pendingTx) {
        if (tx.isTraceabilityValid()) {
            template_.transactions.push_back(tx);
        }
    }
    
    // Create coinbase transaction (use default reward of 12.5 GXC)
    double blockReward = 12.5; // Default block reward
    Transaction coinbase(minerAddress, blockReward);
    template_.transactions.insert(template_.transactions.begin(), coinbase);
    
    return template_;
}

bool MiningManager::mineSHA256(const BlockTemplate& blockTemplate, uint64_t nonce) {
    // Simplified SHA256 mining - in real implementation would use proper SHA256
    std::string blockData = serializeBlockTemplate(blockTemplate, nonce);
    std::string hash = sha256(blockData);
    
    // Check if hash meets difficulty target
    return checkDifficultyTarget(hash, blockTemplate.difficulty);
}

bool MiningManager::mineEthash(const BlockTemplate& blockTemplate, uint64_t nonce) {
    // Simplified Ethash mining - in real implementation would use proper Ethash
    std::string blockData = serializeBlockTemplate(blockTemplate, nonce);
    std::string hash = sha256(blockData + "ethash");
    
    return checkDifficultyTarget(hash, blockTemplate.difficulty);
}

bool MiningManager::mineGXHash(const BlockTemplate& blockTemplate, uint64_t nonce) {
    // GXHash - custom algorithm optimized for traceability
    std::string blockData = serializeBlockTemplate(blockTemplate, nonce);
    
    // Include traceability data in hash calculation
    for (const auto& tx : blockTemplate.transactions) {
        if (!tx.isCoinbaseTransaction()) {
            blockData += tx.getPrevTxHash() + std::to_string(tx.getReferencedAmount());
        }
    }
    
    std::string hash = sha256(blockData + "gxhash");
    
    return checkDifficultyTarget(hash, blockTemplate.difficulty);
}

bool MiningManager::checkDifficultyTarget(const std::string& hash, double difficulty) {
    // Simplified difficulty check - count leading zeros
    int leadingZeros = 0;
    for (char c : hash) {
        if (c == '0') {
            leadingZeros++;
        } else {
            break;
        }
    }
    
    // Require more leading zeros for higher difficulty
    int requiredZeros = static_cast<int>(difficulty / 1000.0) + 4;
    return leadingZeros >= requiredZeros;
}

void MiningManager::submitShare(const BlockTemplate& blockTemplate, uint64_t nonce, MiningAlgorithm algorithm) {
    try {
        if (poolUrl.empty()) {
            // Solo mining - submit to blockchain
            Block newBlock = createBlockFromTemplate(blockTemplate, nonce);
            if (blockchain->addBlock(newBlock)) {
                {
                    std::lock_guard<std::mutex> lock(miningMutex);
                    acceptedShares++;
                }
                LOG_MINING(LogLevel::INFO, "Successfully mined block " + std::to_string(newBlock.getIndex()));
            } else {
                {
                    std::lock_guard<std::mutex> lock(miningMutex);
                    rejectedShares++;
                }
                LOG_MINING(LogLevel::WARNING, "Block rejected by blockchain");
            }
        } else {
            // Pool mining - submit to pool
            submitToPool(blockTemplate, nonce, algorithm);
        }
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Failed to submit share: " + std::string(e.what()));
        std::lock_guard<std::mutex> lock(miningMutex);
        rejectedShares++;
    }
}

void MiningManager::submitToPool(const BlockTemplate& blockTemplate, uint64_t nonce, MiningAlgorithm algorithm) {
    // Simplified pool submission
    LOG_MINING(LogLevel::INFO, "Submitting share to pool: " + poolUrl);
    
    std::lock_guard<std::mutex> lock(miningMutex);
    acceptedShares++;
}

Block MiningManager::createBlockFromTemplate(const BlockTemplate& blockTemplate, uint64_t nonce) {
    Block block(blockTemplate.height, blockTemplate.previousHash, BlockType::POW_SHA256);
    block.setTimestamp(blockTemplate.timestamp);
    block.setNonce(nonce);
    block.setDifficulty(blockTemplate.difficulty);
    block.setMinerAddress(blockTemplate.minerAddress);
    
    // Add transactions
    for (const auto& tx : blockTemplate.transactions) {
        block.addTransaction(tx);
    }
    
    return block;
}

std::string MiningManager::serializeBlockTemplate(const BlockTemplate& blockTemplate, uint64_t nonce) {
    std::ostringstream oss;
    
    oss << blockTemplate.previousHash << blockTemplate.height << blockTemplate.timestamp 
        << blockTemplate.difficulty << nonce;
    
    for (const auto& tx : blockTemplate.transactions) {
        oss << tx.getHash();
    }
    
    return oss.str();
}

void MiningManager::updateStatistics() {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    // Calculate total hash rate
    hashRate = 0.0;
    for (const auto& miner : miners) {
        if (miner.second.active) {
            hashRate += miner.second.hashRate;
        }
    }
}

void MiningManager::updateMinerHashRate(const std::string& minerId, uint64_t hashCount, std::time_t startTime) {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    auto it = miners.find(minerId);
    if (it != miners.end()) {
        auto elapsed = Utils::getCurrentTimestamp() - startTime;
        if (elapsed > 0) {
            it->second.hashRate = static_cast<double>(hashCount) / elapsed;
        }
    }
}

void MiningManager::checkForNewBlocks() {
    // Check if blockchain height has changed
    Block latestBlock = blockchain->getLatestBlock();
    uint32_t currentHeight = latestBlock.getIndex();
    
    static uint32_t lastKnownHeight = 0;
    if (currentHeight > lastKnownHeight) {
        LOG_MINING(LogLevel::INFO, "New block detected at height " + std::to_string(currentHeight));
        lastKnownHeight = currentHeight;
        
        // Update difficulty
        currentDifficulty = blockchain->getDifficulty();
    }
}

void MiningManager::adjustDifficulty() {
    // Let the blockchain handle difficulty adjustment
    double newDifficulty = blockchain->getDifficulty();
    
    if (std::abs(newDifficulty - currentDifficulty) > 0.001) {
        setDifficulty(newDifficulty);
    }
}

void MiningManager::printMiningStats() {
    std::lock_guard<std::mutex> lock(miningMutex);
    
    LOG_MINING(LogLevel::INFO, "=== Mining Statistics ===");
    LOG_MINING(LogLevel::INFO, "Algorithm: " + algorithmToString(currentAlgorithm));
    LOG_MINING(LogLevel::INFO, "Hash Rate: " + std::to_string(hashRate) + " H/s");
    LOG_MINING(LogLevel::INFO, "Accepted Shares: " + std::to_string(acceptedShares));
    LOG_MINING(LogLevel::INFO, "Rejected Shares: " + std::to_string(rejectedShares));
    LOG_MINING(LogLevel::INFO, "Difficulty: " + std::to_string(currentDifficulty));
    LOG_MINING(LogLevel::INFO, "Active Miners: " + std::to_string(miners.size()));
    
    double efficiency = (acceptedShares + rejectedShares) > 0 ? 
        (static_cast<double>(acceptedShares) / (acceptedShares + rejectedShares)) * 100.0 : 0.0;
    LOG_MINING(LogLevel::INFO, "Efficiency: " + std::to_string(efficiency) + "%");
    LOG_MINING(LogLevel::INFO, "========================");
}

std::string MiningManager::algorithmToString(MiningAlgorithm algorithm) {
    switch (algorithm) {
        case MiningAlgorithm::SHA256: return "SHA256";
        case MiningAlgorithm::ETHASH: return "Ethash";
        case MiningAlgorithm::GXHASH: return "GXHash";
        default: return "Unknown";
    }
}