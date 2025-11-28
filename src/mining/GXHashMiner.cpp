#include "../../include/mining/GXHashMiner.h"
#include "../../include/Logger.h"
#include "../../include/Utils.h"
#include <thread>
#include <random>
#include <chrono>
#include <algorithm>

GXHashMiner::GXHashMiner() 
    : isRunning(false), hashRate(0.0), totalHashes(0), threadsCount(0),
      traceabilityValidations(0), traceabilityOptimized(true) {
    LOG_MINING(LogLevel::INFO, "GXHash miner initialized with traceability optimization");
}

GXHashMiner::~GXHashMiner() {
    stop();
}

bool GXHashMiner::start(uint32_t threads) {
    if (isRunning) {
        LOG_MINING(LogLevel::WARNING, "GXHash miner already running");
        return true;
    }
    
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
    }
    
    threadsCount = threads;
    
    LOG_MINING(LogLevel::INFO, "Starting GXHash miner with " + std::to_string(threads) + " threads");
    
    try {
        isRunning = true;
        startTime = Utils::getCurrentTimestamp();
        
        // Initialize traceability cache
        initializeTraceabilityCache();
        
        // Start mining threads
        for (uint32_t i = 0; i < threads; i++) {
            workerThreads.emplace_back(&GXHashMiner::miningThread, this, i);
        }
        
        // Start stats thread
        statsThread = std::thread(&GXHashMiner::statsLoop, this);
        
        LOG_MINING(LogLevel::INFO, "GXHash miner started successfully with traceability validation");
        return true;
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Failed to start GXHash miner: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void GXHashMiner::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_MINING(LogLevel::INFO, "Stopping GXHash miner");
    
    isRunning = false;
    
    // Stop all worker threads
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
    
    // Stop stats thread
    if (statsThread.joinable()) {
        statsThread.join();
    }
    
    LOG_MINING(LogLevel::INFO, "GXHash miner stopped");
}

void GXHashMiner::setJob(const MiningJob& job) {
    std::lock_guard<std::mutex> lock(jobMutex);
    currentJob = job;
    jobUpdated = true;
    
    // Update traceability cache with new transactions
    updateTraceabilityCache(job);
    
    LOG_MINING(LogLevel::DEBUG, "New GXHash mining job: " + job.jobId + 
              " with " + std::to_string(job.transactions.size()) + " traceable transactions");
}

MiningStats GXHashMiner::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    MiningStats stats;
    stats.algorithm = MiningAlgorithm::GXHASH;
    stats.hashRate = hashRate;
    stats.totalHashes = totalHashes;
    stats.threadsActive = threadsCount;
    stats.uptime = Utils::getCurrentTimestamp() - startTime;
    stats.traceabilityValidations = traceabilityValidations;
    
    return stats;
}

void GXHashMiner::miningThread(uint32_t threadId) {
    LOG_MINING(LogLevel::DEBUG, "GXHash mining thread " + std::to_string(threadId) + " started");
    
    // Thread-specific random nonce start
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t nonce = dis(gen);
    uint64_t hashCount = 0;
    uint64_t traceabilityChecks = 0;
    auto lastStatsUpdate = Utils::getCurrentTimestamp();
    
    MiningJob localJob;
    bool hasJob = false;
    
    while (isRunning) {
        try {
            // Check for job updates
            {
                std::lock_guard<std::mutex> lock(jobMutex);
                if (jobUpdated) {
                    localJob = currentJob;
                    hasJob = true;
                    jobUpdated = false;
                    nonce = dis(gen); // Reset nonce for new job
                }
            }
            
            if (!hasJob) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            // Mine GXHash with traceability validation
            bool found = mineGXHashBlock(localJob, nonce);
            hashCount++;
            nonce++;
            
            // Perform traceability validation every few hashes
            if (hashCount % 10 == 0) {
                validateTraceability(localJob);
                traceabilityChecks++;
            }
            
            if (found) {
                submitSolution(localJob, nonce - 1);
                LOG_MINING(LogLevel::INFO, "GXHash solution found by thread " + std::to_string(threadId) + 
                          " with verified traceability");
            }
            
            // Update stats every 500 hashes
            if (hashCount % 500 == 0) {
                updateThreadStats(threadId, hashCount, lastStatsUpdate, traceabilityChecks);
                lastStatsUpdate = Utils::getCurrentTimestamp();
            }
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in GXHash mining thread " + std::to_string(threadId) + 
                                       ": " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_MINING(LogLevel::DEBUG, "GXHash mining thread " + std::to_string(threadId) + " stopped");
}

bool GXHashMiner::mineGXHashBlock(const MiningJob& job, uint64_t nonce) {
    // Construct block header with traceability data
    std::string blockHeader = constructGXHashHeader(job, nonce);
    
    // GXHash computation with traceability optimization
    GXHashResult result = computeGXHash(blockHeader, nonce, job.transactions);
    
    // Check if hash meets difficulty target and traceability requirements
    return checkDifficultyTarget(result.hash, job.difficulty) && 
           result.traceabilityValid;
}

GXHashResult GXHashMiner::computeGXHash(const std::string& blockHeader, uint64_t nonce, 
                                       const std::vector<Transaction>& transactions) {
    GXHashResult result;
    
    // GXHash Algorithm:
    // 1. Standard hash computation
    // 2. Traceability data integration
    // 3. Cross-transaction validation
    // 4. Enhanced security through transaction linking
    
    // Phase 1: Base hash computation
    std::string baseHash = sha256(blockHeader + std::to_string(nonce));
    
    // Phase 2: Integrate traceability data
    std::string traceabilityData = extractTraceabilityData(transactions);
    std::string traceHash = sha256(baseHash + traceabilityData);
    
    // Phase 3: Cross-transaction validation hash
    std::string crossValidationHash = computeCrossValidationHash(transactions);
    
    // Phase 4: Final GXHash computation
    std::string finalInput = traceHash + crossValidationHash + std::to_string(nonce);
    
    // Apply GXHash rounds (more rounds for better security)
    std::string gxHash = finalInput;
    for (int round = 0; round < GXHASH_ROUNDS; round++) {
        gxHash = sha256(gxHash + std::to_string(round));
        
        // Integrate transaction data in specific rounds
        if (round % 4 == 0 && !transactions.empty()) {
            size_t txIndex = round / 4 % transactions.size();
            gxHash = sha256(gxHash + transactions[txIndex].getHash());
        }
    }
    
    result.hash = gxHash;
    result.traceabilityHash = traceHash;
    result.crossValidationHash = crossValidationHash;
    result.traceabilityValid = validateTransactionTraceability(transactions);
    result.rounds = GXHASH_ROUNDS;
    
    return result;
}

std::string GXHashMiner::extractTraceabilityData(const std::vector<Transaction>& transactions) {
    std::ostringstream oss;
    
    for (const auto& tx : transactions) {
        if (!tx.isCoinbaseTransaction()) {
            // Extract key traceability fields
            oss << tx.getPrevTxHash()
                << tx.getReferencedAmount()
                << tx.getTimestamp()
                << tx.getTraceabilityProof();
        }
    }
    
    return oss.str();
}

std::string GXHashMiner::computeCrossValidationHash(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) {
        return "0000000000000000000000000000000000000000000000000000000000000000";
    }
    
    std::vector<std::string> txHashes;
    std::vector<std::string> traceLinks;
    
    for (const auto& tx : transactions) {
        txHashes.push_back(tx.getHash());
        if (!tx.isCoinbaseTransaction()) {
            traceLinks.push_back(tx.getPrevTxHash());
        }
    }
    
    // Sort for deterministic ordering
    std::sort(txHashes.begin(), txHashes.end());
    std::sort(traceLinks.begin(), traceLinks.end());
    
    // Combine all hashes
    std::string combined;
    for (const auto& hash : txHashes) {
        combined += hash;
    }
    for (const auto& link : traceLinks) {
        combined += link;
    }
    
    return sha256(combined);
}

bool GXHashMiner::validateTransactionTraceability(const std::vector<Transaction>& transactions) {
    for (const auto& tx : transactions) {
        if (!tx.isCoinbaseTransaction()) {
            // Validate GXC traceability formula:
            // Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount
            if (!tx.isTraceabilityValid()) {
                return false;
            }
            
            // Additional GXHash-specific validations
            if (!validateGXHashSpecificRules(tx)) {
                return false;
            }
        }
    }
    
    return true;
}

bool GXHashMiner::validateGXHashSpecificRules(const Transaction& tx) {
    // GXHash-specific traceability rules:
    
    // 1. Previous transaction hash must be valid format
    if (tx.getPrevTxHash().length() != 64) {
        return false;
    }
    
    // 2. Referenced amount must match input amount
    if (std::abs(tx.getReferencedAmount() - tx.getTotalInputAmount()) > 0.00000001) {
        return false;
    }
    
    // 3. Timestamp consistency check
    if (tx.getTimestamp() < 1640995200) { // After 2022-01-01
        return false;
    }
    
    // 4. Check if transaction is in traceability cache
    return isInTraceabilityCache(tx.getHash());
}

void GXHashMiner::initializeTraceabilityCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    // Initialize cache for fast traceability lookups
    traceabilityCache.clear();
    traceabilityCache.reserve(100000); // Reserve space for efficiency
    
    LOG_MINING(LogLevel::INFO, "Initialized GXHash traceability cache");
}

void GXHashMiner::updateTraceabilityCache(const MiningJob& job) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    for (const auto& tx : job.transactions) {
        TraceabilityEntry entry;
        entry.txHash = tx.getHash();
        entry.prevTxHash = tx.getPrevTxHash();
        entry.amount = tx.getReferencedAmount();
        entry.timestamp = tx.getTimestamp();
        entry.valid = tx.isTraceabilityValid();
        
        traceabilityCache[tx.getHash()] = entry;
    }
    
    // Cleanup old entries if cache gets too large
    if (traceabilityCache.size() > 50000) {
        cleanupTraceabilityCache();
    }
}

void GXHashMiner::cleanupTraceabilityCache() {
    // Remove oldest entries to keep cache size manageable
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (auto it = traceabilityCache.begin(); it != traceabilityCache.end();) {
        if (currentTime - it->second.timestamp > 3600) { // Remove entries older than 1 hour
            it = traceabilityCache.erase(it);
        } else {
            ++it;
        }
    }
    
    LOG_MINING(LogLevel::DEBUG, "Cleaned up traceability cache, size: " + 
              std::to_string(traceabilityCache.size()));
}

bool GXHashMiner::isInTraceabilityCache(const std::string& txHash) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return traceabilityCache.find(txHash) != traceabilityCache.end();
}

void GXHashMiner::validateTraceability(const MiningJob& job) {
    // Periodic traceability validation during mining
    for (const auto& tx : job.transactions) {
        if (!tx.isCoinbaseTransaction() && !tx.isTraceabilityValid()) {
            LOG_MINING(LogLevel::WARNING, "Traceability validation failed for transaction: " + 
                      tx.getHash().substr(0, 16));
        }
    }
    
    traceabilityValidations++;
}

std::string GXHashMiner::constructGXHashHeader(const MiningJob& job, uint64_t nonce) {
    std::ostringstream oss;
    oss << job.previousHash
        << job.merkleRoot
        << job.timestamp
        << job.difficulty
        << nonce
        << job.traceabilityRoot; // GXHash-specific field
    
    return oss.str();
}

bool GXHashMiner::checkDifficultyTarget(const std::string& hash, double difficulty) {
    // GXHash difficulty check with traceability consideration
    
    // Count leading zeros
    int leadingZeros = 0;
    for (char c : hash) {
        if (c == '0') {
            leadingZeros++;
        } else {
            break;
        }
    }
    
    // GXHash requires slightly more work due to traceability validation
    int requiredZeros = static_cast<int>(difficulty / 800.0) + 5;
    
    // Bonus for blocks with high traceability validation
    if (traceabilityOptimized) {
        requiredZeros -= 1; // Easier difficulty for traceable blocks
    }
    
    return leadingZeros >= requiredZeros;
}

void GXHashMiner::submitSolution(const MiningJob& job, uint64_t nonce) {
    // Submit solution with traceability proof
    if (solutionCallback) {
        MiningSolution solution;
        solution.jobId = job.jobId;
        solution.nonce = nonce;
        solution.extraNonce = 0;
        solution.timestamp = Utils::getCurrentTimestamp();
        solution.algorithm = MiningAlgorithm::GXHASH;
        
        // Add GXHash-specific data
        GXHashResult result = computeGXHash(constructGXHashHeader(job, nonce), nonce, job.transactions);
        solution.traceabilityHash = result.traceabilityHash;
        solution.crossValidationHash = result.crossValidationHash;
        solution.traceabilityValid = result.traceabilityValid;
        
        solutionCallback(solution);
    }
    
    LOG_MINING(LogLevel::INFO, "Submitted GXHash solution for job " + job.jobId + 
              " with verified traceability chain");
}

void GXHashMiner::updateThreadStats(uint32_t threadId, uint64_t hashCount, std::time_t startTime, 
                                   uint64_t traceabilityChecks) {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    auto elapsed = Utils::getCurrentTimestamp() - startTime;
    if (elapsed > 0) {
        double threadHashRate = static_cast<double>(hashCount) / elapsed;
        threadHashRates[threadId] = threadHashRate;
        
        // Update total hash rate
        hashRate = 0.0;
        for (const auto& pair : threadHashRates) {
            hashRate += pair.second;
        }
        
        totalHashes += hashCount;
        traceabilityValidations += traceabilityChecks;
    }
}

void GXHashMiner::statsLoop() {
    LOG_MINING(LogLevel::DEBUG, "GXHash miner stats loop started");
    
    while (isRunning) {
        try {
            auto stats = getStats();
            
            double traceabilityRate = stats.uptime > 0 ? 
                static_cast<double>(stats.traceabilityValidations) / stats.uptime : 0.0;
            
            LOG_MINING(LogLevel::INFO, "GXHash Stats - Hash Rate: " + 
                      std::to_string(stats.hashRate) + " H/s, Total: " + 
                      std::to_string(stats.totalHashes) + " hashes, Traceability: " + 
                      std::to_string(stats.traceabilityValidations) + " validations (" +
                      std::to_string(traceabilityRate) + " val/s)");
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in GXHash stats loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_MINING(LogLevel::DEBUG, "GXHash miner stats loop stopped");
}

void GXHashMiner::setSolutionCallback(const SolutionCallback& callback) {
    solutionCallback = callback;
}

bool GXHashMiner::isMiningCapable() {
    // GXHash doesn't require special hardware, just CPU
    return true;
}

void GXHashMiner::setTraceabilityOptimization(bool enabled) {
    traceabilityOptimized = enabled;
    LOG_MINING(LogLevel::INFO, "GXHash traceability optimization: " + 
              std::string(enabled ? "enabled" : "disabled"));
}

std::string GXHashMiner::getOptimizationInfo() {
    std::ostringstream oss;
    oss << "GXHash Miner Information:\n";
    oss << "  Algorithm: GXHash (Traceability Optimized)\n";
    oss << "  Rounds: " << GXHASH_ROUNDS << "\n";
    oss << "  Threads: " << threadsCount << "\n";
    oss << "  Traceability Cache Size: " << traceabilityCache.size() << "\n";
    oss << "  Traceability Optimization: " << (traceabilityOptimized ? "Enabled" : "Disabled") << "\n";
    oss << "  Total Validations: " << traceabilityValidations << "\n";
    oss << "  Formula: Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount";
    
    return oss.str();
}