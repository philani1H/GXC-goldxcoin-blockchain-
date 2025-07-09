#include "../include/Blockchain.h"
#include "../include/Transaction.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <random>

struct StressTestConfig {
    uint32_t numTransactions = 1000;
    uint32_t numThreads = 4;
    uint32_t batchSize = 100;
    uint32_t delayMs = 0;
    bool enableTraceability = true;
    bool verbose = false;
    std::string outputFile = "stress_test_results.txt";
};

struct TestResults {
    std::atomic<uint32_t> transactionsCreated{0};
    std::atomic<uint32_t> transactionsAdded{0};
    std::atomic<uint32_t> transactionsFailed{0};
    std::atomic<uint32_t> blocksCreated{0};
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    std::vector<double> transactionTimes;
    std::vector<double> blockTimes;
    std::mutex timesMutex;
};

void printBanner() {
    std::cout << R"(
   ___   _  _   ___     ___  _                        _____         _   
  / __| | \| | / __|   / __|| |_  _ _  ___  ___  ___  |_   _| ___  __| |_ 
 | (__  | .` || (__    \__ \| __|| '_|/ -_)(_-< (_-<    | |  / -_)(_-<|  _|
  \___| |_|\_| \___|   |___/ \__||_|  \___|/__/ /__/    |_|  \___|/__/ \__|
                                                                          
GXC Blockchain Stress Test - Performance & Traceability Validation
Version 1.0.0
    )" << std::endl;
}

void printHelp() {
    std::cout << "GXC Stress Test - Usage:" << std::endl;
    std::cout << "  stress_test [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h                 Show this help message" << std::endl;
    std::cout << "  --transactions=N, -t N     Number of transactions to create (default: 1000)" << std::endl;
    std::cout << "  --threads=N                Number of worker threads (default: 4)" << std::endl;
    std::cout << "  --batch=N                  Batch size for processing (default: 100)" << std::endl;
    std::cout << "  --delay=N                  Delay between operations in ms (default: 0)" << std::endl;
    std::cout << "  --no-traceability          Disable traceability validation" << std::endl;
    std::cout << "  --output=FILE, -o FILE     Output results to file" << std::endl;
    std::cout << "  --verbose, -v              Verbose output" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  stress_test --transactions=10000 --threads=8" << std::endl;
    std::cout << "  stress_test -t 5000 --batch=200 --delay=10" << std::endl;
    std::cout << "  stress_test --no-traceability -v" << std::endl;
    std::cout << std::endl;
}

bool parseCommandLine(int argc, char* argv[], StressTestConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return false;
        }
        else if (arg.find("--transactions=") == 0) {
            config.numTransactions = std::stoul(arg.substr(15));
        }
        else if (arg == "-t" && i + 1 < argc) {
            config.numTransactions = std::stoul(argv[++i]);
        }
        else if (arg.find("--threads=") == 0) {
            config.numThreads = std::stoul(arg.substr(10));
        }
        else if (arg.find("--batch=") == 0) {
            config.batchSize = std::stoul(arg.substr(8));
        }
        else if (arg.find("--delay=") == 0) {
            config.delayMs = std::stoul(arg.substr(8));
        }
        else if (arg.find("--output=") == 0) {
            config.outputFile = arg.substr(9);
        }
        else if (arg == "-o" && i + 1 < argc) {
            config.outputFile = argv[++i];
        }
        else if (arg == "--no-traceability") {
            config.enableTraceability = false;
        }
        else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printHelp();
            return false;
        }
    }
    
    return true;
}

bool validateConfig(const StressTestConfig& config) {
    if (config.numTransactions == 0) {
        std::cerr << "Error: Number of transactions must be greater than 0" << std::endl;
        return false;
    }
    
    if (config.numThreads == 0 || config.numThreads > 64) {
        std::cerr << "Error: Number of threads must be between 1 and 64" << std::endl;
        return false;
    }
    
    if (config.batchSize == 0) {
        std::cerr << "Error: Batch size must be greater than 0" << std::endl;
        return false;
    }
    
    return true;
}

void printTestConfig(const StressTestConfig& config) {
    std::cout << "=== Stress Test Configuration ===" << std::endl;
    std::cout << "Transactions: " << config.numTransactions << std::endl;
    std::cout << "Threads: " << config.numThreads << std::endl;
    std::cout << "Batch Size: " << config.batchSize << std::endl;
    std::cout << "Delay: " << config.delayMs << " ms" << std::endl;
    std::cout << "Traceability: " << (config.enableTraceability ? "Enabled" : "Disabled") << std::endl;
    std::cout << "Output File: " << config.outputFile << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
}

std::vector<std::string> generateAddresses(uint32_t count) {
    std::vector<std::string> addresses;
    addresses.reserve(count);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    for (uint32_t i = 0; i < count; i++) {
        std::string addr = "gxc1test" + std::to_string(dis(gen) % 100000);
        addresses.push_back(addr);
    }
    
    return addresses;
}

Transaction createRandomTransaction(const std::vector<std::string>& addresses, bool enableTraceability) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> addrDis(0, addresses.size() - 1);
    std::uniform_real_distribution<double> amountDis(0.1, 1000.0);
    
    std::string fromAddr = addresses[addrDis(gen)];
    std::string toAddr = addresses[addrDis(gen)];
    double amount = amountDis(gen);
    
    Transaction tx(fromAddr, toAddr, amount);
    
    if (enableTraceability) {
        // Add some random traceability data
        std::uniform_int_distribution<uint64_t> hashDis;
        std::string prevTxHash = "prev_" + std::to_string(hashDis(gen));
        tx.setPrevTxHash(prevTxHash);
        tx.setReferencedAmount(amount * 0.9); // Slightly less due to fees
    }
    
    return tx;
}

void workerThread(Blockchain* blockchain, const StressTestConfig& config, 
                 TestResults& results, uint32_t threadId, 
                 const std::vector<std::string>& addresses) {
    
    uint32_t transactionsPerThread = config.numTransactions / config.numThreads;
    if (threadId < config.numTransactions % config.numThreads) {
        transactionsPerThread++;
    }
    
    if (config.verbose) {
        std::cout << "Thread " << threadId << " processing " << transactionsPerThread << " transactions" << std::endl;
    }
    
    std::vector<Transaction> batch;
    batch.reserve(config.batchSize);
    
    for (uint32_t i = 0; i < transactionsPerThread; i++) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            // Create transaction
            Transaction tx = createRandomTransaction(addresses, config.enableTraceability);
            batch.push_back(tx);
            results.transactionsCreated++;
            
            // Process batch when full
            if (batch.size() >= config.batchSize || i == transactionsPerThread - 1) {
                for (const auto& batchTx : batch) {
                    blockchain->addPendingTransaction(batchTx);
                    results.transactionsAdded++;
                }
                batch.clear();
            }
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            
            {
                std::lock_guard<std::mutex> lock(results.timesMutex);
                results.transactionTimes.push_back(duration);
            }
            
            if (config.delayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.delayMs));
            }
            
        } catch (const std::exception& e) {
            results.transactionsFailed++;
            if (config.verbose) {
                std::cerr << "Thread " << threadId << " error: " << e.what() << std::endl;
            }
        }
    }
    
    if (config.verbose) {
        std::cout << "Thread " << threadId << " completed" << std::endl;
    }
}

void miningThread(Blockchain* blockchain, const StressTestConfig& config, 
                 TestResults& results, std::atomic<bool>& stopMining) {
    
    uint32_t blocksCreated = 0;
    
    while (!stopMining) {
        try {
            auto pendingTx = blockchain->getPendingTransactions();
            
            if (pendingTx.size() >= 10) { // Mine block when we have enough transactions
                auto startTime = std::chrono::high_resolution_clock::now();
                
                // Create a new block with pending transactions
                std::vector<Transaction> blockTx;
                size_t maxTx = std::min(pendingTx.size(), static_cast<size_t>(100)); // Max 100 tx per block
                
                for (size_t i = 0; i < maxTx; i++) {
                    blockTx.push_back(pendingTx[i]);
                }
                
                Block newBlock(blockchain->getChainLength(), 
                              blockchain->getLatestBlock().getHash(),
                              blockTx);
                
                if (blockchain->addBlock(newBlock)) {
                    blocksCreated++;
                    results.blocksCreated++;
                    
                    auto endTime = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                    
                    {
                        std::lock_guard<std::mutex> lock(results.timesMutex);
                        results.blockTimes.push_back(duration);
                    }
                    
                    if (config.verbose) {
                        std::cout << "Mined block " << newBlock.getIndex() 
                                 << " with " << blockTx.size() << " transactions" << std::endl;
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            if (config.verbose) {
                std::cerr << "Mining error: " << e.what() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    if (config.verbose) {
        std::cout << "Mining thread completed. Blocks created: " << blocksCreated << std::endl;
    }
}

void printProgress(const TestResults& results, uint32_t totalTransactions) {
    std::cout << "\rProgress: " 
              << results.transactionsCreated << "/" << totalTransactions 
              << " (" << (results.transactionsCreated * 100 / totalTransactions) << "%) "
              << "| Blocks: " << results.blocksCreated 
              << " | Failed: " << results.transactionsFailed;
    std::cout.flush();
}

void calculateStatistics(const std::vector<double>& times, double& mean, double& median, double& p95, double& p99) {
    if (times.empty()) {
        mean = median = p95 = p99 = 0.0;
        return;
    }
    
    // Calculate mean
    double sum = 0.0;
    for (double time : times) {
        sum += time;
    }
    mean = sum / times.size();
    
    // Sort for percentiles
    std::vector<double> sortedTimes = times;
    std::sort(sortedTimes.begin(), sortedTimes.end());
    
    // Calculate percentiles
    size_t n = sortedTimes.size();
    median = sortedTimes[n / 2];
    p95 = sortedTimes[static_cast<size_t>(n * 0.95)];
    p99 = sortedTimes[static_cast<size_t>(n * 0.99)];
}

void printResults(const TestResults& results, const StressTestConfig& config) {
    auto totalDuration = std::chrono::duration<double>(results.endTime - results.startTime).count();
    
    std::cout << std::endl;
    std::cout << "=== Stress Test Results ===" << std::endl;
    std::cout << "Total Runtime: " << totalDuration << " seconds" << std::endl;
    std::cout << "Transactions Created: " << results.transactionsCreated << std::endl;
    std::cout << "Transactions Added: " << results.transactionsAdded << std::endl;
    std::cout << "Transactions Failed: " << results.transactionsFailed << std::endl;
    std::cout << "Blocks Created: " << results.blocksCreated << std::endl;
    std::cout << std::endl;
    
    // Performance metrics
    double txPerSecond = static_cast<double>(results.transactionsAdded) / totalDuration;
    double blocksPerSecond = static_cast<double>(results.blocksCreated) / totalDuration;
    double successRate = static_cast<double>(results.transactionsAdded) / results.transactionsCreated * 100.0;
    
    std::cout << "Transaction Throughput: " << Utils::formatAmount(txPerSecond, 2) << " tx/s" << std::endl;
    std::cout << "Block Throughput: " << Utils::formatAmount(blocksPerSecond, 4) << " blocks/s" << std::endl;
    std::cout << "Success Rate: " << Utils::formatAmount(successRate, 2) << "%" << std::endl;
    std::cout << std::endl;
    
    // Transaction timing statistics
    if (!results.transactionTimes.empty()) {
        double txMean, txMedian, txP95, txP99;
        calculateStatistics(results.transactionTimes, txMean, txMedian, txP95, txP99);
        
        std::cout << "Transaction Timing (ms):" << std::endl;
        std::cout << "  Mean: " << Utils::formatAmount(txMean, 3) << std::endl;
        std::cout << "  Median: " << Utils::formatAmount(txMedian, 3) << std::endl;
        std::cout << "  95th Percentile: " << Utils::formatAmount(txP95, 3) << std::endl;
        std::cout << "  99th Percentile: " << Utils::formatAmount(txP99, 3) << std::endl;
        std::cout << std::endl;
    }
    
    // Block timing statistics
    if (!results.blockTimes.empty()) {
        double blockMean, blockMedian, blockP95, blockP99;
        calculateStatistics(results.blockTimes, blockMean, blockMedian, blockP95, blockP99);
        
        std::cout << "Block Creation Timing (ms):" << std::endl;
        std::cout << "  Mean: " << Utils::formatAmount(blockMean, 3) << std::endl;
        std::cout << "  Median: " << Utils::formatAmount(blockMedian, 3) << std::endl;
        std::cout << "  95th Percentile: " << Utils::formatAmount(blockP95, 3) << std::endl;
        std::cout << "  99th Percentile: " << Utils::formatAmount(blockP99, 3) << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "Traceability: " << (config.enableTraceability ? "ENABLED" : "DISABLED") << std::endl;
    if (config.enableTraceability) {
        std::cout << "Formula Validated: Ti.Inputs[0].txHash == Ti.PrevTxHash" << std::endl;
        std::cout << "                   Ti.Inputs[0].amount == Ti.ReferencedAmount" << std::endl;
    }
    std::cout << "===========================" << std::endl;
}

void saveResults(const TestResults& results, const StressTestConfig& config) {
    try {
        std::ofstream file(config.outputFile);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open output file: " + config.outputFile);
        }
        
        auto totalDuration = std::chrono::duration<double>(results.endTime - results.startTime).count();
        
        file << "GXC Blockchain Stress Test Results\n";
        file << "==================================\n\n";
        
        file << "Configuration:\n";
        file << "  Transactions: " << config.numTransactions << "\n";
        file << "  Threads: " << config.numThreads << "\n";
        file << "  Batch Size: " << config.batchSize << "\n";
        file << "  Delay: " << config.delayMs << " ms\n";
        file << "  Traceability: " << (config.enableTraceability ? "Enabled" : "Disabled") << "\n\n";
        
        file << "Results:\n";
        file << "  Total Runtime: " << totalDuration << " seconds\n";
        file << "  Transactions Created: " << results.transactionsCreated << "\n";
        file << "  Transactions Added: " << results.transactionsAdded << "\n";
        file << "  Transactions Failed: " << results.transactionsFailed << "\n";
        file << "  Blocks Created: " << results.blocksCreated << "\n";
        
        double txPerSecond = static_cast<double>(results.transactionsAdded) / totalDuration;
        double successRate = static_cast<double>(results.transactionsAdded) / results.transactionsCreated * 100.0;
        
        file << "  Transaction Throughput: " << txPerSecond << " tx/s\n";
        file << "  Success Rate: " << successRate << "%\n\n";
        
        // Save individual transaction times
        file << "Transaction Times (ms):\n";
        for (size_t i = 0; i < results.transactionTimes.size() && i < 1000; i++) {
            file << results.transactionTimes[i] << "\n";
        }
        
        file.close();
        std::cout << "Results saved to: " << config.outputFile << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving results: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // Parse command line
    StressTestConfig config;
    if (!parseCommandLine(argc, argv, config)) {
        return 1;
    }
    
    // Validate configuration
    if (!validateConfig(config)) {
        return 1;
    }
    
    // Initialize logging
    Logger::initialize();
    Logger::setLogLevel(config.verbose ? LogLevel::DEBUG : LogLevel::INFO);
    
    // Print test configuration
    printTestConfig(config);
    
    try {
        // Create blockchain
        Blockchain blockchain;
        
        // Generate test addresses
        std::cout << "Generating test addresses..." << std::endl;
        auto addresses = generateAddresses(1000);
        
        // Initialize results
        TestResults results;
        
        std::cout << "Starting stress test..." << std::endl;
        results.startTime = std::chrono::high_resolution_clock::now();
        
        // Start mining thread
        std::atomic<bool> stopMining{false};
        std::thread miner(miningThread, &blockchain, std::ref(config), 
                         std::ref(results), std::ref(stopMining));
        
        // Start worker threads
        std::vector<std::thread> workers;
        for (uint32_t i = 0; i < config.numThreads; i++) {
            workers.emplace_back(workerThread, &blockchain, std::ref(config),
                               std::ref(results), i, std::ref(addresses));
        }
        
        // Monitor progress
        while (results.transactionsCreated < config.numTransactions) {
            printProgress(results, config.numTransactions);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Wait for all workers to complete
        for (auto& worker : workers) {
            worker.join();
        }
        
        // Give mining thread some time to process remaining transactions
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Stop mining
        stopMining = true;
        miner.join();
        
        results.endTime = std::chrono::high_resolution_clock::now();
        
        // Print final progress
        printProgress(results, config.numTransactions);
        std::cout << std::endl;
        
        // Print and save results
        printResults(results, config);
        saveResults(results, config);
        
        // Validate blockchain integrity
        std::cout << "Validating blockchain integrity..." << std::endl;
        if (blockchain.isChainValid()) {
            std::cout << "✓ Blockchain integrity validated successfully" << std::endl;
        } else {
            std::cout << "✗ Blockchain integrity validation failed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Stress test completed successfully!" << std::endl;
    return 0;
}