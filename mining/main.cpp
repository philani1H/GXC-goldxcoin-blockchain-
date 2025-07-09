#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>
#include <chrono>
#include "GXCMiner.h"
#include "Logger.h"
#include "Config.h"
#include "Database.h"
#include "blockchain.h"
#include "MiningManager.h"

class GXCMinerApp {
private:
    std::unique_ptr<MiningManager> miningManager_;
    std::unique_ptr<Database> database_;
    std::unique_ptr<Blockchain> blockchain_;
    std::unique_ptr<Config> config_;
    bool isRunning_;
    std::string minerAddress_;
    std::string algorithm_;
    int threads_;

public:
    GXCMinerApp() : isRunning_(false), threads_(0) {}
    
    ~GXCMinerApp() {
        stop();
    }
    
    bool initialize(const std::string& configPath = "config/gxc.conf") {
        try {
            Logger::info("Initializing GXC Miner...");
            
            // Load configuration
            config_ = std::make_unique<Config>(configPath);
            
            // Initialize database
            std::string dbPath = config_->getDatabasePath();
            database_ = std::make_unique<Database>(dbPath);
            Logger::info("Database connected: " + dbPath);
            
            // Initialize blockchain
            blockchain_ = std::make_unique<Blockchain>(database_.get());
            
            // Initialize mining manager
            miningManager_ = std::make_unique<MiningManager>(blockchain_.get(), database_.get());
            
            // Set mining parameters from config
            minerAddress_ = config_->getMiningAddress();
            algorithm_ = config_->getMiningAlgorithm();
            threads_ = config_->getMiningThreads();
            
            if (minerAddress_.empty()) {
                Logger::error("Mining address not configured! Please set mining.address in config file.");
                return false;
            }
            
            miningManager_->setMinerAddress(minerAddress_);
            miningManager_->setAlgorithm(algorithm_);
            
            Logger::info("GXC Miner initialized successfully");
            Logger::info("Miner Address: " + minerAddress_);
            Logger::info("Algorithm: " + algorithm_);
            Logger::info("Threads: " + (threads_ > 0 ? std::to_string(threads_) : "auto-detect"));
            
            printEarlyMinerInfo();
            
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Failed to initialize miner: " + std::string(e.what()));
            return false;
        }
    }
    
    bool start() {
        if (isRunning_) {
            Logger::warning("Miner is already running");
            return false;
        }
        
        try {
            Logger::info("Starting GXC Miner...");
            
            // Check current block height for early miner bonus information
            int currentHeight = blockchain_->getHeight();
            double currentDifficulty = blockchain_->getCurrentDifficulty();
            
            Logger::info("Current blockchain height: " + std::to_string(currentHeight));
            Logger::info("Current difficulty: " + std::to_string(currentDifficulty));
            
            // Calculate potential rewards
            double baseReward = calculatePotentialReward(currentHeight);
            Logger::info("Potential reward per block: " + std::to_string(baseReward) + " GXC");
            
            if (currentHeight < 10000) {
                Logger::info("🎉 EARLY MINER BONUS ACTIVE! 🎉");
                Logger::info("Enhanced rewards available for first 10,000 blocks!");
            }
            
            // Start mining
            if (!miningManager_->start()) {
                Logger::error("Failed to start mining manager");
                return false;
            }
            
            isRunning_ = true;
            
            Logger::info("GXC Miner started successfully!");
            Logger::info("Mining with " + algorithm_ + " algorithm");
            Logger::info("Press Ctrl+C to stop mining");
            
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Failed to start miner: " + std::string(e.what()));
            return false;
        }
    }
    
    void stop() {
        if (!isRunning_) {
            return;
        }
        
        Logger::info("Stopping GXC Miner...");
        isRunning_ = false;
        
        if (miningManager_) {
            miningManager_->stop();
        }
        
        // Print final statistics
        printFinalStats();
        
        Logger::info("GXC Miner stopped");
    }
    
    void run() {
        start();
        
        // Main mining loop
        while (isRunning_) {
            try {
                // Print mining statistics every minute
                printMiningStats();
                
                // Sleep for 60 seconds
                std::this_thread::sleep_for(std::chrono::seconds(60));
                
            } catch (const std::exception& e) {
                Logger::error("Error in mining loop: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }
    
    void setMinerAddress(const std::string& address) {
        minerAddress_ = address;
        if (miningManager_) {
            miningManager_->setMinerAddress(address);
        }
    }
    
    void setAlgorithm(const std::string& algorithm) {
        algorithm_ = algorithm;
        if (miningManager_) {
            miningManager_->setAlgorithm(algorithm);
        }
    }
    
    void setThreads(int threads) {
        threads_ = threads;
        // Thread count is set during mining manager start
    }
    
private:
    void printEarlyMinerInfo() {
        int currentHeight = blockchain_->getHeight();
        
        Logger::info("===============================================");
        Logger::info("           GXC EARLY MINER REWARDS           ");
        Logger::info("===============================================");
        
        if (currentHeight < 1000) {
            Logger::info("🌟 MASSIVE EARLY ADOPTER BONUS: 510 GXC per block!");
            Logger::info("   Valid for first 1,000 blocks");
        } else if (currentHeight < 5000) {
            Logger::info("🌟 LARGE EARLY MINER BONUS: 260 GXC per block!");
            Logger::info("   Valid until block 5,000");
        } else if (currentHeight < 10000) {
            Logger::info("🌟 SUBSTANTIAL BONUS: 110 GXC per block!");
            Logger::info("   Valid until block 10,000");
        } else if (currentHeight < 25000) {
            Logger::info("🌟 GOOD BONUS: 60 GXC per block!");
            Logger::info("   Valid until block 25,000");
        } else if (currentHeight < 50000) {
            Logger::info("🌟 MODERATE BONUS: 35 GXC per block!");
            Logger::info("   Valid until block 50,000");
        } else if (currentHeight < 100000) {
            Logger::info("🌟 SMALL BONUS: 20 GXC per block!");
            Logger::info("   Valid until block 100,000");
        } else {
            Logger::info("Standard rewards: 10 GXC per block (base)");
        }
        
        Logger::info("Current block height: " + std::to_string(currentHeight));
        Logger::info("===============================================");
    }
    
    double calculatePotentialReward(int blockHeight) {
        double baseReward = 10.0;
        double earlyMinerBonus = 0.0;
        
        if (blockHeight < 1000) {
            earlyMinerBonus = 500.0;
        } else if (blockHeight < 5000) {
            earlyMinerBonus = 250.0;
        } else if (blockHeight < 10000) {
            earlyMinerBonus = 100.0;
        } else if (blockHeight < 25000) {
            earlyMinerBonus = 50.0;
        } else if (blockHeight < 50000) {
            earlyMinerBonus = 25.0;
        } else if (blockHeight < 100000) {
            earlyMinerBonus = 10.0;
        }
        
        // Apply halving
        int halvingCount = blockHeight / 100000;
        double halvingFactor = 1.0 / std::pow(2, halvingCount);
        
        return (baseReward + earlyMinerBonus) * halvingFactor;
    }
    
    void printMiningStats() {
        try {
            if (!miningManager_->isRunning()) {
                return;
            }
            
            auto stats = miningManager_->getMiningStats();
            double hashrate = miningManager_->getCurrentHashrate();
            double difficulty = miningManager_->getCurrentDifficulty();
            int currentHeight = blockchain_->getHeight();
            
            Logger::info("=== Mining Statistics ===");
            Logger::info("Algorithm: " + algorithm_);
            Logger::info("Block Height: " + std::to_string(currentHeight));
            Logger::info("Difficulty: " + std::to_string(difficulty));
            Logger::info("Hashrate: " + formatHashrate(hashrate));
            Logger::info("Hashes Computed: " + std::to_string(stats.hashesComputed));
            Logger::info("Blocks Found: " + std::to_string(stats.blocksFound));
            Logger::info("Potential Reward: " + std::to_string(calculatePotentialReward(currentHeight)) + " GXC");
            
            // Calculate estimated time to find a block
            if (hashrate > 0 && difficulty > 0) {
                double timeToBlock = difficulty / hashrate;
                Logger::info("Est. Time to Block: " + formatTime(timeToBlock));
            }
            
            Logger::info("========================");
            
        } catch (const std::exception& e) {
            Logger::error("Error printing mining stats: " + std::string(e.what()));
        }
    }
    
    void printFinalStats() {
        try {
            if (!miningManager_) {
                return;
            }
            
            auto stats = miningManager_->getMiningStats();
            
            Logger::info("=== Final Mining Statistics ===");
            Logger::info("Total Hashes: " + std::to_string(stats.hashesComputed));
            Logger::info("Blocks Found: " + std::to_string(stats.blocksFound));
            Logger::info("Algorithm Used: " + algorithm_);
            Logger::info("Miner Address: " + minerAddress_);
            Logger::info("===============================");
            
        } catch (const std::exception& e) {
            Logger::error("Error printing final stats: " + std::string(e.what()));
        }
    }
    
    std::string formatHashrate(double hashrate) {
        if (hashrate >= 1e12) {
            return std::to_string(hashrate / 1e12) + " TH/s";
        } else if (hashrate >= 1e9) {
            return std::to_string(hashrate / 1e9) + " GH/s";
        } else if (hashrate >= 1e6) {
            return std::to_string(hashrate / 1e6) + " MH/s";
        } else if (hashrate >= 1e3) {
            return std::to_string(hashrate / 1e3) + " KH/s";
        } else {
            return std::to_string(hashrate) + " H/s";
        }
    }
    
    std::string formatTime(double seconds) {
        if (seconds >= 86400) {
            return std::to_string(seconds / 86400) + " days";
        } else if (seconds >= 3600) {
            return std::to_string(seconds / 3600) + " hours";
        } else if (seconds >= 60) {
            return std::to_string(seconds / 60) + " minutes";
        } else {
            return std::to_string(seconds) + " seconds";
        }
    }
};

// Global miner instance for signal handling
GXCMinerApp* g_miner = nullptr;

void signalHandler(int signal) {
    Logger::info("Received signal " + std::to_string(signal) + ", stopping miner...");
    if (g_miner) {
        g_miner->stop();
    }
    exit(0);
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -a, --address <address>    Mining address for rewards\n"
              << "  -t, --threads <count>      Number of mining threads (0 = auto-detect)\n"
              << "  -g, --algorithm <algo>     Mining algorithm (SHA256, Ethash, GXHash)\n"
              << "  -c, --config <path>        Configuration file path\n"
              << "  -h, --help                 Show this help message\n"
              << "  -v, --version              Show version information\n"
              << "  --stats                    Show mining statistics and exit\n";
}

void printVersion() {
    std::cout << "GXC Miner v2.0.0\n"
              << "Multi-algorithm cryptocurrency miner\n"
              << "Supported algorithms: SHA256, Ethash, GXHash\n"
              << "Enhanced early miner rewards: Up to 510 GXC per block!\n"
              << "Copyright (c) 2024 GXC Development Team\n";
}

void showAvailableAlgorithms() {
    std::cout << "Available mining algorithms:\n"
              << "  SHA256  - Bitcoin-compatible SHA256 mining\n"
              << "  Ethash  - Ethereum-compatible Ethash mining\n"
              << "  GXHash  - GXC custom algorithm (recommended)\n";
}

int main(int argc, char* argv[]) {
    std::string configPath = "config/gxc.conf";
    std::string minerAddress;
    std::string algorithm;
    int threads = 0;
    bool showStats = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-a" || arg == "--address") {
            if (i + 1 < argc) {
                minerAddress = argv[++i];
            } else {
                std::cerr << "Error: --address requires an address\n";
                return 1;
            }
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                threads = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --threads requires a number\n";
                return 1;
            }
        } else if (arg == "-g" || arg == "--algorithm") {
            if (i + 1 < argc) {
                algorithm = argv[++i];
            } else {
                std::cerr << "Error: --algorithm requires an algorithm name\n";
                return 1;
            }
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                configPath = argv[++i];
            } else {
                std::cerr << "Error: --config requires a path\n";
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            showAvailableAlgorithms();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "--stats") {
            showStats = true;
        } else {
            std::cerr << "Error: Unknown option " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    try {
        // Initialize logging
        Logger::initialize("logs/gxc-miner.log");
        Logger::info("Starting GXC Miner...");
        
        // Create and initialize miner
        GXCMinerApp miner;
        g_miner = &miner;
        
        // Set up signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
#ifdef SIGQUIT
        signal(SIGQUIT, signalHandler);
#endif
        
        if (!miner.initialize(configPath)) {
            Logger::error("Failed to initialize miner");
            return 1;
        }
        
        // Override config with command line parameters
        if (!minerAddress.empty()) {
            miner.setMinerAddress(minerAddress);
        }
        
        if (!algorithm.empty()) {
            miner.setAlgorithm(algorithm);
        }
        
        if (threads > 0) {
            miner.setThreads(threads);
        }
        
        if (showStats) {
            // Show statistics and exit
            miner.printMiningStats();
            return 0;
        }
        
        // Run the miner
        miner.run();
        
    } catch (const std::exception& e) {
        Logger::error("Fatal error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        Logger::error("Unknown fatal error occurred");
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
    
    Logger::info("GXC Miner exited normally");
    return 0;
}