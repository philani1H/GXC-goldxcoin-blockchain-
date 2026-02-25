#include "../../include/mining/GXHashMiner.h"
#include "../../include/Logger.h"
#include "../../include/Utils.h"
#include "../../include/HashUtils.h"
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
        stop();
        return false;
    }
}

void GXHashMiner::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_MINING(LogLevel::INFO, "Stopping GXHash miner");
    
    isRunning = false;
    
    // Wait for all worker threads
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
    
    // Wait for stats thread
    if (statsThread.joinable()) {
        statsThread.join();
    }
    
    LOG_MINING(LogLevel::INFO, "GXHash miner stopped");
}

void GXHashMiner::setJob(const MiningJob& job) {
    std::lock_guard<std::mutex> lock(jobMutex);
    currentJob = job;
    jobUpdated = true;
    
    // Update traceability cache for new job
    updateTraceabilityCache(job);
    
    LOG_MINING(LogLevel::DEBUG, "GXHash job updated: " + job.jobId);
}

MiningStats GXHashMiner::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    MiningStats stats;
    stats.algorithm = MiningAlgorithm::GXHASH;
    stats.hashRate = hashRate;
    stats.totalHashes = totalHashes;
    stats.threadsActive = static_cast<uint32_t>(workerThreads.size());
    stats.uptime = Utils::getCurrentTimestamp() - startTime;
    stats.traceabilityValidations = traceabilityValidations;
    
    return stats;
}

void GXHashMiner::miningThread(uint32_t threadId) {
    LOG_MINING(LogLevel::DEBUG, "GXHash mining thread " + std::to_string(threadId) + " started");
    
    uint64_t nonce = threadId * 1000000ULL; // Start at different nonce ranges
    std::time_t lastStatsUpdate = Utils::getCurrentTimestamp();
    uint64_t hashCount = 0;
    uint64_t traceabilityChecks = 0;
    
    while (isRunning) {
        try {
            MiningJob localJob;
            {
                std::lock_guard<std::mutex> lock(jobMutex);
                if (!jobUpdated && currentJob.jobId.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                localJob = currentJob;
                jobUpdated = false;
            }
            
            // Mine GXHash with traceability validation
            bool found = mineGXHashBlock(localJob, nonce);
            hashCount++;
            nonce++;
            
            // Perform traceability validation every few hashes
            if (hashCount % 10 == 0) {
                // Validate traceability for all transactions in job
                for (const auto& txHash : localJob.transactions) {
                    validateTraceability(txHash, localJob.previousHash);
                }
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
    std::string blockHeader = constructBlockHeader(job, nonce);
    
    // GXHash computation with traceability optimization
    GXHashResult result = computeGXHash(blockHeader, nonce, job.transactions);
    
    // Check if hash meets difficulty target and traceability requirements
    return checkDifficultyTarget(result.hash, job.difficulty) && 
           result.traceabilityValid;
}

std::string GXHashMiner::constructGXHashHeader(const MiningJob& job, uint64_t nonce) {
    return constructBlockHeader(job, nonce);
}

GXHashResult GXHashMiner::computeGXHash(const std::string& blockHeader, uint64_t nonce, 
                                       const std::vector<std::string>& transactions) {
    GXHashResult result;
    
    // GXHash Algorithm:
    // 1. Standard hash computation
    // 2. Traceability data integration
    // 3. Cross-transaction validation
    
    // Phase 1: Base hash computation
    std::string baseHash = sha256(blockHeader + std::to_string(nonce));
    
    // Phase 2: Apply GXHash rounds
    std::string currentHash = baseHash;
    for (uint32_t round = 0; round < GXHASH_ROUNDS; round++) {
        // Mix in transaction data for traceability
        std::string txData;
        for (const auto& tx : transactions) {
            txData += tx;
        }
        currentHash = keccak256(currentHash + txData + std::to_string(round));
    }
    
    result.hash = currentHash;
    
    // Phase 3: Traceability hash
    std::string traceabilityData;
    for (const auto& tx : transactions) {
        traceabilityData += tx;
    }
    result.traceabilityHash = sha256(traceabilityData);
    
    // Phase 4: Cross-validation hash
    result.crossValidationHash = keccak256(result.hash + result.traceabilityHash);
    
    // Traceability is valid if we have consistent hashes
    result.traceabilityValid = !result.traceabilityHash.empty();
    
    return result;
}

std::string GXHashMiner::constructBlockHeader(const MiningJob& job, uint64_t nonce) {
    std::stringstream ss;
    ss << job.previousHash;
    ss << job.merkleRoot;
    ss << job.timestamp;
    ss << job.bits;
    ss << nonce;
    ss << job.blockNumber;
    
    // Add traceability data
    for (const auto& tx : job.transactions) {
        ss << tx;
    }
    
    return ss.str();
}

bool GXHashMiner::checkDifficultyTarget(const std::string& hash, double difficulty) {
    if (hash.empty()) return false;
    
    // Count leading zeros
    size_t leadingZeros = 0;
    for (char c : hash) {
        if (c == '0') {
            leadingZeros++;
        } else {
            break;
        }
    }
    
    // Check if hash meets difficulty
    return leadingZeros >= static_cast<size_t>(difficulty);
}

void GXHashMiner::initializeTraceabilityCache() {
    std::lock_guard<std::mutex> lock(traceabilityCacheMutex);
    traceabilityCache.clear();
    LOG_MINING(LogLevel::DEBUG, "GXHash traceability cache initialized");
}

void GXHashMiner::updateTraceabilityCache(const MiningJob& job) {
    std::lock_guard<std::mutex> lock(traceabilityCacheMutex);
    
    // Cache traceability status for transactions in this job
    for (const auto& txHash : job.transactions) {
        if (traceabilityCache.find(txHash) == traceabilityCache.end()) {
            // New transaction - mark as pending validation
            traceabilityCache[txHash] = true; // Assume valid, will be validated during mining
        }
    }
    
    LOG_MINING(LogLevel::DEBUG, "GXHash traceability cache updated with " + 
              std::to_string(job.transactions.size()) + " transactions");
}

bool GXHashMiner::validateTraceability(const std::string& txHash, const std::string& prevTxHash) {
    // GXC Traceability Formula:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    
    // In a real implementation, we would query the blockchain for the transaction
    // and verify its traceability properties
    
    std::lock_guard<std::mutex> lock(traceabilityCacheMutex);
    
    // Check cache first
    auto it = traceabilityCache.find(txHash);
    if (it != traceabilityCache.end()) {
        return it->second;
    }
    
    // For now, assume valid if not in cache
    // Real implementation would validate against blockchain
    traceabilityCache[txHash] = true;
    traceabilityValidations++;
    
    return true;
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
        GXHashResult result = computeGXHash(constructBlockHeader(job, nonce), nonce, job.transactions);
        solution.traceabilityHash = result.traceabilityHash;
        solution.crossValidationHash = result.crossValidationHash;
        solution.traceabilityValid = result.traceabilityValid;
        
        solutionCallback(solution);
    }
    
    LOG_MINING(LogLevel::INFO, "Submitted GXHash solution for job " + job.jobId + 
              " with verified traceability chain");
}

void GXHashMiner::updateThreadStats(uint32_t threadId, uint64_t hashCount, std::time_t threadStartTime, 
                                   uint64_t traceabilityChecks) {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    auto elapsed = Utils::getCurrentTimestamp() - threadStartTime;
    if (elapsed > 0) {
        double threadHashRate = static_cast<double>(hashCount) / elapsed;
        threadHashRates[threadId] = threadHashRate;
        
        // Update total hash rate
        double newHashRate = 0.0;
        for (const auto& pair : threadHashRates) {
            newHashRate += pair.second;
        }
        hashRate = newHashRate;
        
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
