#include "../../include/mining/SHA256Miner.h"
#include "../../include/Logger.h"
#include "../../include/Utils.h"
#include <immintrin.h>
#include <thread>
#include <random>

SHA256Miner::SHA256Miner() 
    : isRunning(false), hashRate(0.0), totalHashes(0), threadsCount(0) {
    LOG_MINING(LogLevel::INFO, "SHA256 miner initialized");
}

SHA256Miner::~SHA256Miner() {
    stop();
}

bool SHA256Miner::start(uint32_t threads) {
    if (isRunning) {
        LOG_MINING(LogLevel::WARNING, "SHA256 miner already running");
        return true;
    }
    
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
    }
    
    threadsCount = threads;
    
    LOG_MINING(LogLevel::INFO, "Starting SHA256 miner with " + std::to_string(threads) + " threads");
    
    try {
        isRunning = true;
        startTime = Utils::getCurrentTimestamp();
        
        // Start mining threads
        for (uint32_t i = 0; i < threads; i++) {
            workerThreads.emplace_back(&SHA256Miner::miningThread, this, i);
        }
        
        // Start stats thread
        statsThread = std::thread(&SHA256Miner::statsLoop, this);
        
        LOG_MINING(LogLevel::INFO, "SHA256 miner started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Failed to start SHA256 miner: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void SHA256Miner::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_MINING(LogLevel::INFO, "Stopping SHA256 miner");
    
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
    
    LOG_MINING(LogLevel::INFO, "SHA256 miner stopped");
}

void SHA256Miner::setJob(const MiningJob& job) {
    std::lock_guard<std::mutex> lock(jobMutex);
    currentJob = job;
    jobUpdated = true;
    
    LOG_MINING(LogLevel::DEBUG, "New SHA256 mining job: " + job.jobId);
}

MiningStats SHA256Miner::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    MiningStats stats;
    stats.algorithm = MiningAlgorithm::SHA256;
    stats.hashRate = hashRate;
    stats.totalHashes = totalHashes;
    stats.threadsActive = threadsCount;
    stats.uptime = Utils::getCurrentTimestamp() - startTime;
    
    return stats;
}

void SHA256Miner::miningThread(uint32_t threadId) {
    LOG_MINING(LogLevel::DEBUG, "SHA256 mining thread " + std::to_string(threadId) + " started");
    
    // Thread-specific random nonce start
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t nonce = dis(gen);
    uint64_t hashCount = 0;
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
            
            // Mine SHA256
            bool found = mineSHA256Block(localJob, nonce);
            hashCount++;
            nonce++;
            
            if (found) {
                submitSolution(localJob, nonce - 1);
                LOG_MINING(LogLevel::INFO, "SHA256 solution found by thread " + std::to_string(threadId));
            }
            
            // Update stats every 1000 hashes
            if (hashCount % 1000 == 0) {
                updateThreadStats(threadId, hashCount, lastStatsUpdate);
                lastStatsUpdate = Utils::getCurrentTimestamp();
            }
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in SHA256 mining thread " + std::to_string(threadId) + 
                                       ": " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_MINING(LogLevel::DEBUG, "SHA256 mining thread " + std::to_string(threadId) + " stopped");
}

bool SHA256Miner::mineSHA256Block(const MiningJob& job, uint64_t nonce) {
    // Construct block header
    std::string blockHeader = constructBlockHeader(job, nonce);
    
    // Double SHA256 hash (Bitcoin-style)
    std::string hash1 = HashUtils::sha256(blockHeader);
    std::string hash2 = HashUtils::sha256(hash1);
    
    // Check if hash meets difficulty target
    return checkDifficultyTarget(hash2, job.difficulty);
}

bool SHA256Miner::mineSHA256BlockOptimized(const MiningJob& job, uint64_t nonce) {
    // Optimized version using SSE/AVX instructions
    #ifdef __SSE2__
    return mineSHA256BlockSSE(job, nonce);
    #else
    return mineSHA256Block(job, nonce);
    #endif
}

#ifdef __SSE2__
bool SHA256Miner::mineSHA256BlockSSE(const MiningJob& job, uint64_t nonce) {
    // SSE2 optimized SHA256 implementation
    // This would contain hand-optimized SHA256 using SIMD instructions
    // For now, fallback to regular implementation
    return mineSHA256Block(job, nonce);
}
#endif

#ifdef __AVX2__
bool SHA256Miner::mineSHA256BlockAVX(const MiningJob& job, uint64_t nonce) {
    // AVX2 optimized SHA256 implementation
    // This would contain hand-optimized SHA256 using AVX2 instructions
    // For now, fallback to regular implementation
    return mineSHA256Block(job, nonce);
}
#endif

std::string SHA256Miner::constructBlockHeader(const MiningJob& job, uint64_t nonce) {
    std::ostringstream oss;
    oss << job.previousHash
        << job.merkleRoot
        << job.timestamp
        << job.bits
        << nonce;
    
    return oss.str();
}

bool SHA256Miner::checkDifficultyTarget(const std::string& hash, double difficulty) {
    // Convert hash to big integer and compare with target
    // Simplified implementation: count leading zeros
    int leadingZeros = 0;
    for (char c : hash) {
        if (c == '0') {
            leadingZeros++;
        } else {
            break;
        }
    }
    
    // Calculate required zeros based on difficulty
    int requiredZeros = static_cast<int>(difficulty / 1000.0) + 4;
    return leadingZeros >= requiredZeros;
}

void SHA256Miner::submitSolution(const MiningJob& job, uint64_t nonce) {
    // Submit solution to pool or blockchain
    if (solutionCallback) {
        MiningSolution solution;
        solution.jobId = job.jobId;
        solution.nonce = nonce;
        solution.extraNonce = 0;
        solution.timestamp = Utils::getCurrentTimestamp();
        solution.algorithm = MiningAlgorithm::SHA256;
        
        solutionCallback(solution);
    }
    
    LOG_MINING(LogLevel::INFO, "Submitted SHA256 solution for job " + job.jobId);
}

void SHA256Miner::updateThreadStats(uint32_t threadId, uint64_t hashCount, std::time_t startTime) {
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
    }
}

void SHA256Miner::statsLoop() {
    LOG_MINING(LogLevel::DEBUG, "SHA256 miner stats loop started");
    
    while (isRunning) {
        try {
            auto stats = getStats();
            
            LOG_MINING(LogLevel::INFO, "SHA256 Stats - Hash Rate: " + 
                      Utils::formatAmount(stats.hashRate, 2) + " H/s, Total: " + 
                      std::to_string(stats.totalHashes) + " hashes");
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in SHA256 stats loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_MINING(LogLevel::DEBUG, "SHA256 miner stats loop stopped");
}

void SHA256Miner::setSolutionCallback(const SolutionCallback& callback) {
    solutionCallback = callback;
}

bool SHA256Miner::isMiningCapable() {
    // Check if system supports SHA256 mining optimizations
    return true; // All systems can do basic SHA256
}

std::string SHA256Miner::getOptimizationInfo() {
    std::ostringstream oss;
    oss << "SHA256 Miner Optimizations:\n";
    
    #ifdef __SSE2__
    oss << "  SSE2: Enabled\n";
    #else
    oss << "  SSE2: Disabled\n";
    #endif
    
    #ifdef __AVX2__
    oss << "  AVX2: Enabled\n";
    #else
    oss << "  AVX2: Disabled\n";
    #endif
    
    oss << "  Threads: " << threadsCount << "\n";
    oss << "  Hardware Concurrency: " << std::thread::hardware_concurrency();
    
    return oss.str();
}