#include "../include/mining/MiningManager.h"
#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <iomanip>
#include <sstream>

// Global variables for clean shutdown
bool g_running = true;
MiningManager* g_miningManager = nullptr;

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

// Helper to format percentage
std::string formatPercent(double percent) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << percent << "%";
    return oss.str();
}

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down mining..." << std::endl;
    g_running = false;
}

void printBanner() {
    std::cout << R"(
   ___   _  _   ___      ___  __  __  _  _              _     
  / __| | \| | / __|    / __| \ \/ / | || | __ _  ___ | |_   
 | (__  | .` || (__    | (_ |  >  <  | __ |/ _` |(_-< | ' \  
  \___| |_|\_| \___|    \___|/_/\_\ |_||_|\__,_|/__/ |_||_| 
                                                            
GXC GXHash Miner - Custom Traceability-Optimized Algorithm
Version 1.0.0
    )" << std::endl;
}

void printHelp() {
    std::cout << "GXC GXHash Miner - Usage:" << std::endl;
    std::cout << "  gxc-gxhash-miner [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h                 Show this help message" << std::endl;
    std::cout << "  --address=ADDR, -a ADDR    Miner address (required)" << std::endl;
    std::cout << "  --pool=URL                 Mining pool URL" << std::endl;
    std::cout << "  --username=USER            Pool username" << std::endl;
    std::cout << "  --threads=N                Number of mining threads" << std::endl;
    std::cout << "  --node=URL                 Blockchain node URL" << std::endl;
    std::cout << "  --verbose, -v              Verbose logging" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  * GXHash algorithm (custom traceability-optimized)" << std::endl;
    std::cout << "  * Built-in transaction traceability verification" << std::endl;
    std::cout << "  * Optimized for stock contract tracking" << std::endl;
    std::cout << "  * Full chain-of-custody support" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  gxc-gxhash-miner -a gxc1abcd..." << std::endl;
    std::cout << "  gxc-gxhash-miner -a gxc1abcd... --pool=stratum+tcp://pool.gxc.io:4444 --username=miner1" << std::endl;
    std::cout << std::endl;
}

struct MinerConfig {
    std::string minerAddress;
    std::string poolUrl;
    std::string poolUsername;
    std::string nodeUrl = "http://localhost:8332";
    uint32_t threads = 1;
    bool verbose = false;
};

bool parseCommandLine(int argc, char* argv[], MinerConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return false;
        }
        else if (arg.find("--address=") == 0) {
            config.minerAddress = arg.substr(10);
        }
        else if (arg == "-a" && i + 1 < argc) {
            config.minerAddress = argv[++i];
        }
        else if (arg.find("--pool=") == 0) {
            config.poolUrl = arg.substr(7);
        }
        else if (arg.find("--username=") == 0) {
            config.poolUsername = arg.substr(11);
        }
        else if (arg.find("--threads=") == 0) {
            config.threads = std::stoul(arg.substr(10));
        }
        else if (arg.find("--node=") == 0) {
            config.nodeUrl = arg.substr(7);
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

bool validateConfig(const MinerConfig& config) {
    if (config.minerAddress.empty()) {
        std::cerr << "Error: Miner address is required" << std::endl;
        std::cerr << "Use --address=YOUR_ADDRESS or -a YOUR_ADDRESS" << std::endl;
        return false;
    }
    
    if (!Utils::isValidAddress(config.minerAddress)) {
        std::cerr << "Error: Invalid miner address format" << std::endl;
        return false;
    }
    
    if (config.threads == 0 || config.threads > 64) {
        std::cerr << "Error: Invalid thread count. Use 1-64 threads" << std::endl;
        return false;
    }
    
    return true;
}

void printMiningInfo(const MinerConfig& config) {
    std::cout << "=== Mining Configuration ===" << std::endl;
    std::cout << "Miner Address: " << config.minerAddress << std::endl;
    std::cout << "Algorithm: GXHash (Traceability-Optimized)" << std::endl;
    std::cout << "Threads: " << config.threads << std::endl;
    
    if (!config.poolUrl.empty()) {
        std::cout << "Pool URL: " << config.poolUrl << std::endl;
        std::cout << "Pool Username: " << config.poolUsername << std::endl;
        std::cout << "Mining Mode: Pool" << std::endl;
    } else {
        std::cout << "Mining Mode: Solo" << std::endl;
    }
    
    std::cout << "Node URL: " << config.nodeUrl << std::endl;
    std::cout << "Traceability: ENABLED" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;
}

void printMiningStats() {
    if (!g_miningManager) return;
    
    auto stats = g_miningManager->getStats();
    auto miners = g_miningManager->getActiveMiners();
    
    std::cout << "\r"; // Clear line
    std::cout << "Hash Rate: " << formatHashRate(stats.hashRate) << " | ";
    std::cout << "Accepted: " << stats.acceptedShares << " | ";
    std::cout << "Rejected: " << stats.rejectedShares << " | ";
    std::cout << "Miners: " << miners.size() << " | ";
    std::cout << "Uptime: " << stats.uptime << "s";
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
    
    // Validate configuration
    if (!validateConfig(config)) {
        return 1;
    }
    
    // Initialize logging
    Logger::initialize();
    Logger::getInstance().setLogLevel(config.verbose ? LogLevel::DEBUG : LogLevel::INFO);
    
    LOG_MINING(LogLevel::INFO, "Starting GXC GXHash Miner");
    
    try {
        // Print mining configuration
        printMiningInfo(config);
        
        // Create blockchain instance (simplified for miner)
        Blockchain blockchain;
        
        // Create mining manager
        g_miningManager = new MiningManager(&blockchain);
        
        // Configure mining algorithm
        g_miningManager->setMiningAlgorithm(MiningAlgorithm::GXHASH);
        
        // Configure pool if specified
        if (!config.poolUrl.empty()) {
            g_miningManager->setPoolInfo(config.poolUrl, config.poolUsername);
        }
        
        // Start mining
        if (!g_miningManager->start(config.minerAddress)) {
            std::cerr << "Failed to start mining manager" << std::endl;
            delete g_miningManager;
            return 1;
        }
        
        std::cout << "Mining started! Press Ctrl+C to stop." << std::endl;
        std::cout << "Traceability Formula: Ti.Inputs[0].txHash == Ti.PrevTxHash" << std::endl;
        std::cout << "                     Ti.Inputs[0].amount == Ti.ReferencedAmount" << std::endl;
        std::cout << std::endl;
        
        // Main mining loop
        auto lastStatsUpdate = Utils::getCurrentTimestamp();
        while (g_running) {
            // Print stats every 5 seconds
            auto currentTime = Utils::getCurrentTimestamp();
            if (currentTime - lastStatsUpdate >= 5) {
                printMiningStats();
                lastStatsUpdate = currentTime;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << std::endl; // New line after stats
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Fatal error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        
        if (g_miningManager) {
            delete g_miningManager;
        }
        return 1;
    }
    
    // Clean shutdown
    if (g_miningManager) {
        std::cout << "Stopping mining..." << std::endl;
        g_miningManager->stop();
        
        // Print final stats
        auto finalStats = g_miningManager->getStats();
        std::cout << std::endl;
        std::cout << "=== Final Mining Statistics ===" << std::endl;
        std::cout << "Total Runtime: " << finalStats.uptime << " seconds" << std::endl;
        std::cout << "Average Hash Rate: " << formatHashRate(finalStats.hashRate) << std::endl;
        std::cout << "Accepted Shares: " << finalStats.acceptedShares << std::endl;
        std::cout << "Rejected Shares: " << finalStats.rejectedShares << std::endl;
        
        if (finalStats.acceptedShares + finalStats.rejectedShares > 0) {
            double efficiency = (static_cast<double>(finalStats.acceptedShares) / 
                               (finalStats.acceptedShares + finalStats.rejectedShares)) * 100.0;
            std::cout << "Efficiency: " << formatPercent(efficiency) << std::endl;
        }
        
        std::cout << "===============================" << std::endl;
        
        delete g_miningManager;
    }
    
    LOG_MINING(LogLevel::INFO, "GXC GXHash Miner shutdown complete");
    std::cout << "Thank you for mining GXC!" << std::endl;
    
    return 0;
}