#include "../include/mining/SHA256Miner.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <iomanip>
#include <sstream>

// Global variables for clean shutdown
bool g_running = true;
SHA256Miner* g_miner = nullptr;

// Helper function to format hash rate
std::string formatHashRate(double rate) {
    std::ostringstream oss;
    if (rate >= 1e9) {
        oss << std::fixed << std::setprecision(2) << (rate / 1e9) << " GH/s";
    } else if (rate >= 1e6) {
        oss << std::fixed << std::setprecision(2) << (rate / 1e6) << " MH/s";
    } else if (rate >= 1e3) {
        oss << std::fixed << std::setprecision(2) << (rate / 1e3) << " KH/s";
    } else {
        oss << std::fixed << std::setprecision(2) << rate << " H/s";
    }
    return oss.str();
}

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down SHA256 miner..." << std::endl;
    g_running = false;
}

void printBanner() {
    std::cout << R"(
   ___   _  _   ___     ___  _  _    _    ___   ___    __   
  / __| | \| | / __|   / __|| || |  /_\  |_  ) | __|  / /   
 | (__  | .` || (__    \__ \| __ | / _ \  / /  |__ \ / _ \
  \___| |_|\_| \___|   |___/|_||_|/_/ \_\/___| |___/\___/  
                                                          
GXC SHA256 Specialized Miner - Bitcoin-Compatible Algorithm
Version 1.0.0
    )" << std::endl;
}

void printHelp() {
    std::cout << "GXC SHA256 Miner - Usage:" << std::endl;
    std::cout << "  gxc-sha256-miner [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h                 Show this help message" << std::endl;
    std::cout << "  --threads=N, -t N          Number of mining threads (default: auto)" << std::endl;
    std::cout << "  --server=URL               Pool server URL" << std::endl;
    std::cout << "  --user=USER                Pool username" << std::endl;
    std::cout << "  --pass=PASS                Pool password" << std::endl;
    std::cout << "  --verbose, -v              Verbose logging" << std::endl;
    std::cout << "  --benchmark                Run performance benchmark" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  * Optimized SHA256 double hashing" << std::endl;
    std::cout << "  * SSE2/AVX2 acceleration support" << std::endl;
    std::cout << "  * Multi-threaded mining" << std::endl;
    std::cout << "  * Pool and solo mining support" << std::endl;
    std::cout << "  * Real-time performance statistics" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  gxc-sha256-miner --threads=8" << std::endl;
    std::cout << "  gxc-sha256-miner --server=stratum+tcp://pool.example.com:4444 --user=worker1" << std::endl;
    std::cout << "  gxc-sha256-miner --benchmark" << std::endl;
    std::cout << std::endl;
}

struct MinerConfig {
    uint32_t threads = 0;
    std::string server;
    std::string username;
    std::string password;
    bool verbose = false;
    bool benchmark = false;
};

bool parseCommandLine(int argc, char* argv[], MinerConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return false;
        }
        else if (arg.find("--threads=") == 0) {
            config.threads = std::stoul(arg.substr(10));
        }
        else if (arg == "-t" && i + 1 < argc) {
            config.threads = std::stoul(argv[++i]);
        }
        else if (arg.find("--server=") == 0) {
            config.server = arg.substr(9);
        }
        else if (arg.find("--user=") == 0) {
            config.username = arg.substr(7);
        }
        else if (arg.find("--pass=") == 0) {
            config.password = arg.substr(7);
        }
        else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        }
        else if (arg == "--benchmark") {
            config.benchmark = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printHelp();
            return false;
        }
    }
    
    return true;
}

void printMinerInfo(const MinerConfig& config) {
    std::cout << "=== SHA256 Miner Configuration ===" << std::endl;
    std::cout << "Algorithm: SHA256 (Bitcoin-compatible)" << std::endl;
    std::cout << "Threads: " << (config.threads ? std::to_string(config.threads) : "auto") << std::endl;
    
    if (!config.server.empty()) {
        std::cout << "Pool Mode: " << config.server << std::endl;
        std::cout << "Username: " << config.username << std::endl;
    } else {
        std::cout << "Mode: Solo mining" << std::endl;
    }
    
    std::cout << "Optimizations: ";
    #ifdef __SSE2__
    std::cout << "SSE2 ";
    #endif
    #ifdef __AVX2__
    std::cout << "AVX2 ";
    #endif
    std::cout << std::endl;
    
    std::cout << "===================================" << std::endl;
    std::cout << std::endl;
}

void runBenchmark(uint32_t threads) {
    std::cout << "Running SHA256 performance benchmark..." << std::endl;
    std::cout << "Threads: " << threads << std::endl;
    std::cout << "Duration: 30 seconds" << std::endl;
    std::cout << std::endl;
    
    SHA256Miner miner;
    
    // Set up a test job
    MiningJob testJob;
    testJob.jobId = "benchmark";
    testJob.previousHash = "0000000000000000000000000000000000000000000000000000000000000000";
    testJob.merkleRoot = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b";
    testJob.timestamp = Utils::getCurrentTimestamp();
    testJob.bits = "1d00ffff";
    testJob.difficulty = 1000.0;
    
    miner.setJob(testJob);
    miner.start(threads);
    
    auto startTime = Utils::getCurrentTimestamp();
    auto endTime = startTime + 30; // 30 second benchmark
    
    while (Utils::getCurrentTimestamp() < endTime && g_running) {
        auto stats = miner.getStats();
        
        std::cout << "\rBenchmark Progress: " 
                  << (Utils::getCurrentTimestamp() - startTime) << "/30s - "
                  << "Hash Rate: " << formatHashRate(stats.hashRate) << "    ";
        std::cout.flush();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    miner.stop();
    
    auto finalStats = miner.getStats();
    
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "=== Benchmark Results ===" << std::endl;
    std::cout << "Average Hash Rate: " << formatHashRate(finalStats.hashRate) << std::endl;
    std::cout << "Total Hashes: " << finalStats.totalHashes << std::endl;
    std::cout << "Efficiency: " << formatHashRate(finalStats.hashRate / threads) << " per thread" << std::endl;
    std::cout << "=========================" << std::endl;
}

void printMiningStats(SHA256Miner* miner) {
    if (!miner) return;
    
    auto stats = miner->getStats();
    
    std::cout << "\r"; // Clear line
    std::cout << "SHA256 - Hash Rate: " << formatHashRate(stats.hashRate) << " | ";
    std::cout << "Total: " << stats.totalHashes << " | ";
    std::cout << "Threads: " << stats.threadsActive << " | ";
    std::cout << "Uptime: " << stats.uptime << "s    ";
    std::cout.flush();
}

int main(int argc, char* argv[]) {
    // Print banner
    printBanner();
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Parse command line
    MinerConfig config;
    if (!parseCommandLine(argc, argv, config)) {
        return 1;
    }
    
    // Initialize logging
    Logger::initialize();
    Logger::getInstance().setLogLevel(config.verbose ? LogLevel::DEBUG : LogLevel::INFO);
    
    LOG_MINING(LogLevel::INFO, "Starting GXC SHA256 Miner");
    
    try {
        // Handle benchmark mode
        if (config.benchmark) {
            uint32_t threads = config.threads ? config.threads : std::thread::hardware_concurrency();
            runBenchmark(threads);
            return 0;
        }
        
        // Print miner configuration
        printMinerInfo(config);
        
        // Create and configure miner
        g_miner = new SHA256Miner();
        
        // Set up solution callback
        g_miner->setSolutionCallback([](const MiningSolution& solution) {
            std::cout << std::endl;
            std::cout << "*** SHA256 SOLUTION FOUND ***" << std::endl;
            std::cout << "Job ID: " << solution.jobId << std::endl;
            std::cout << "Nonce: " << solution.nonce << std::endl;
            std::cout << "Timestamp: " << solution.timestamp << std::endl;
            std::cout << "*****************************" << std::endl;
        });
        
        // Create a mining job (in real implementation, would come from pool/blockchain)
        MiningJob job;
        job.jobId = "gxc_sha256_job_1";
        job.previousHash = "0000000000000000000000000000000000000000000000000000000000000000";
        job.merkleRoot = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b";
        job.timestamp = Utils::getCurrentTimestamp();
        job.bits = "1d00ffff";
        job.difficulty = 1000.0;
        
        g_miner->setJob(job);
        
        // Start mining
        if (!g_miner->start(config.threads)) {
            std::cerr << "Failed to start SHA256 miner" << std::endl;
            delete g_miner;
            return 1;
        }
        
        std::cout << "SHA256 mining started! Press Ctrl+C to stop." << std::endl;
        std::cout << "Algorithm: Double SHA256 (Bitcoin-compatible)" << std::endl;
        std::cout << "Optimizations: " << g_miner->getOptimizationInfo() << std::endl;
        std::cout << std::endl;
        
        // Main mining loop
        auto lastStatsUpdate = Utils::getCurrentTimestamp();
        while (g_running) {
            // Print stats every 5 seconds
            auto currentTime = Utils::getCurrentTimestamp();
            if (currentTime - lastStatsUpdate >= 5) {
                printMiningStats(g_miner);
                lastStatsUpdate = currentTime;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << std::endl; // New line after stats
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Fatal error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        
        if (g_miner) {
            delete g_miner;
        }
        return 1;
    }
    
    // Clean shutdown
    if (g_miner) {
        std::cout << "Stopping SHA256 miner..." << std::endl;
        g_miner->stop();
        
        // Print final stats
        auto finalStats = g_miner->getStats();
        std::cout << std::endl;
        std::cout << "=== Final SHA256 Mining Statistics ===" << std::endl;
        std::cout << "Total Runtime: " << finalStats.uptime << " seconds" << std::endl;
        std::cout << "Average Hash Rate: " << formatHashRate(finalStats.hashRate) << std::endl;
        std::cout << "Total Hashes: " << finalStats.totalHashes << std::endl;
        std::cout << "Efficiency: " << formatHashRate(finalStats.hashRate / finalStats.threadsActive) << " per thread" << std::endl;
        std::cout << "=======================================" << std::endl;
        
        delete g_miner;
    }
    
    LOG_MINING(LogLevel::INFO, "SHA256 miner shutdown complete");
    std::cout << "Thank you for mining with GXC SHA256!" << std::endl;
    
    return 0;
}