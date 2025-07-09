#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>
#include <chrono>
#include "blockchain.h"
#include "Database.h"
#include "Logger.h"
#include "Config.h"
#include "Network.h"
#include "RPCAPI.h"
#include "RESTServer.h"
#include "WebSocketServer.h"
#include "PeerManager.h"

class GXCNode {
private:
    std::unique_ptr<Blockchain> blockchain_;
    std::unique_ptr<Database> database_;
    std::unique_ptr<Network> network_;
    std::unique_ptr<RPCAPI> rpcApi_;
    std::unique_ptr<RESTServer> restServer_;
    std::unique_ptr<WebSocketServer> wsServer_;
    std::unique_ptr<PeerManager> peerManager_;
    std::unique_ptr<Config> config_;
    bool running_;
    std::thread blockValidationThread_;
    std::thread networkThread_;
    std::thread metricsThread_;

public:
    GXCNode() : running_(false) {}
    
    ~GXCNode() {
        shutdown();
    }
    
    bool initialize(const std::string& configPath = "config/gxc.conf") {
        try {
            Logger::info("Initializing GXC Node...");
            
            // Load configuration
            config_ = std::make_unique<Config>(configPath);
            Logger::setLevel(config_->getLogLevel());
            
            // Initialize database
            std::string dbPath = config_->getDatabasePath();
            database_ = std::make_unique<Database>(dbPath);
            Logger::info("Database initialized: " + dbPath);
            
            // Initialize blockchain
            blockchain_ = std::make_unique<Blockchain>(database_.get());
            Logger::info("Blockchain initialized");
            
            // Initialize network components
            network_ = std::make_unique<Network>(config_.get());
            peerManager_ = std::make_unique<PeerManager>(config_.get());
            
            // Initialize API servers
            int rpcPort = config_->getRPCPort();
            int restPort = config_->getRESTPort();
            int wsPort = config_->getWebSocketPort();
            
            rpcApi_ = std::make_unique<RPCAPI>(blockchain_.get(), rpcPort);
            restServer_ = std::make_unique<RESTServer>(blockchain_.get(), restPort);
            wsServer_ = std::make_unique<WebSocketServer>(blockchain_.get(), wsPort);
            
            Logger::info("Node components initialized successfully");
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Failed to initialize node: " + std::string(e.what()));
            return false;
        }
    }
    
    void start() {
        if (running_) {
            Logger::warning("Node is already running");
            return;
        }
        
        Logger::info("Starting GXC Node...");
        running_ = true;
        
        try {
            // Start network services
            network_->start();
            peerManager_->start();
            
            // Start API servers
            rpcApi_->start();
            restServer_->start();
            wsServer_->start();
            
            // Start background threads
            startBackgroundThreads();
            
            Logger::info("GXC Node started successfully");
            Logger::info("RPC API listening on port: " + std::to_string(config_->getRPCPort()));
            Logger::info("REST API listening on port: " + std::to_string(config_->getRESTPort()));
            Logger::info("WebSocket API listening on port: " + std::to_string(config_->getWebSocketPort()));
            
            // Print node information
            printNodeInfo();
            
        } catch (const std::exception& e) {
            Logger::error("Failed to start node: " + std::string(e.what()));
            running_ = false;
            throw;
        }
    }
    
    void shutdown() {
        if (!running_) {
            return;
        }
        
        Logger::info("Shutting down GXC Node...");
        running_ = false;
        
        // Stop background threads
        if (blockValidationThread_.joinable()) {
            blockValidationThread_.join();
        }
        if (networkThread_.joinable()) {
            networkThread_.join();
        }
        if (metricsThread_.joinable()) {
            metricsThread_.join();
        }
        
        // Shutdown API servers
        if (wsServer_) wsServer_->stop();
        if (restServer_) restServer_->stop();
        if (rpcApi_) rpcApi_->stop();
        
        // Shutdown network
        if (peerManager_) peerManager_->stop();
        if (network_) network_->stop();
        
        Logger::info("GXC Node shutdown complete");
    }
    
    void run() {
        start();
        
        // Main event loop
        while (running_) {
            try {
                // Process pending blocks
                blockchain_->processePendingBlocks();
                
                // Update network statistics
                updateNetworkStats();
                
                // Clean up old data periodically
                cleanupOldData();
                
                // Sleep to prevent busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
            } catch (const std::exception& e) {
                Logger::error("Error in main loop: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    
private:
    void startBackgroundThreads() {
        // Block validation thread
        blockValidationThread_ = std::thread([this]() {
            while (running_) {
                try {
                    blockchain_->validatePendingBlocks();
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                } catch (const std::exception& e) {
                    Logger::error("Block validation thread error: " + std::string(e.what()));
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
        });
        
        // Network management thread
        networkThread_ = std::thread([this]() {
            while (running_) {
                try {
                    peerManager_->maintainConnections();
                    network_->processMessages();
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                } catch (const std::exception& e) {
                    Logger::error("Network thread error: " + std::string(e.what()));
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                }
            }
        });
        
        // Metrics collection thread
        metricsThread_ = std::thread([this]() {
            while (running_) {
                try {
                    collectMetrics();
                    std::this_thread::sleep_for(std::chrono::minutes(1));
                } catch (const std::exception& e) {
                    Logger::error("Metrics thread error: " + std::string(e.what()));
                    std::this_thread::sleep_for(std::chrono::minutes(5));
                }
            }
        });
    }
    
    void updateNetworkStats() {
        static auto lastUpdate = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        
        if (std::chrono::duration_cast<std::chrono::minutes>(now - lastUpdate).count() >= 5) {
            try {
                int peerCount = peerManager_->getActivePeerCount();
                int blockHeight = blockchain_->getHeight();
                double networkHashrate = blockchain_->getNetworkHashrate();
                
                // Update database metrics
                database_->insertPerformanceMetric("network", "peer_count", peerCount, blockHeight);
                database_->insertPerformanceMetric("network", "block_height", blockHeight, blockHeight);
                database_->insertPerformanceMetric("network", "hashrate", networkHashrate, blockHeight);
                
                lastUpdate = now;
            } catch (const std::exception& e) {
                Logger::error("Failed to update network stats: " + std::string(e.what()));
            }
        }
    }
    
    void collectMetrics() {
        try {
            int blockHeight = blockchain_->getHeight();
            
            // Collect blockchain metrics
            double avgBlockTime = blockchain_->getAverageBlockTime();
            double difficulty = blockchain_->getCurrentDifficulty();
            size_t mempoolSize = blockchain_->getMempoolSize();
            size_t totalTransactions = blockchain_->getTotalTransactionCount();
            
            // Collect system metrics
            double cpuUsage = getCPUUsage();
            double memoryUsage = getMemoryUsage();
            double diskUsage = getDiskUsage();
            
            // Insert metrics into database
            database_->insertPerformanceMetric("blockchain", "avg_block_time", avgBlockTime, blockHeight);
            database_->insertPerformanceMetric("blockchain", "difficulty", difficulty, blockHeight);
            database_->insertPerformanceMetric("blockchain", "mempool_size", mempoolSize, blockHeight);
            database_->insertPerformanceMetric("blockchain", "total_transactions", totalTransactions, blockHeight);
            
            database_->insertPerformanceMetric("system", "cpu_usage", cpuUsage, blockHeight);
            database_->insertPerformanceMetric("system", "memory_usage", memoryUsage, blockHeight);
            database_->insertPerformanceMetric("system", "disk_usage", diskUsage, blockHeight);
            
            Logger::debug("Metrics collected successfully");
            
        } catch (const std::exception& e) {
            Logger::error("Failed to collect metrics: " + std::string(e.what()));
        }
    }
    
    void cleanupOldData() {
        static auto lastCleanup = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        
        // Clean up old data once per hour
        if (std::chrono::duration_cast<std::chrono::hours>(now - lastCleanup).count() >= 1) {
            try {
                // Clean up old performance metrics (keep only last 30 days)
                int retentionDays = config_->getMetricsRetentionDays();
                long cutoffTime = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() - (retentionDays * 24 * 3600);
                
                std::string sql = "DELETE FROM performance_metrics WHERE timestamp < " + std::to_string(cutoffTime);
                database_->execute(sql);
                
                // Clean up disconnected peers
                peerManager_->cleanupDisconnectedPeers();
                
                lastCleanup = now;
                Logger::info("Old data cleanup completed");
                
            } catch (const std::exception& e) {
                Logger::error("Failed to cleanup old data: " + std::string(e.what()));
            }
        }
    }
    
    void printNodeInfo() {
        Logger::info("=== GXC Node Information ===");
        Logger::info("Version: 2.0.0");
        Logger::info("Block Height: " + std::to_string(blockchain_->getHeight()));
        Logger::info("Network Hash Rate: " + std::to_string(blockchain_->getNetworkHashrate()) + " H/s");
        Logger::info("Active Peers: " + std::to_string(peerManager_->getActivePeerCount()));
        Logger::info("Mempool Size: " + std::to_string(blockchain_->getMempoolSize()));
        Logger::info("Database: " + config_->getDatabasePath());
        Logger::info("============================");
    }
    
    // System monitoring functions
    double getCPUUsage() {
        // Simplified CPU usage calculation
        // In a real implementation, this would use system-specific APIs
        return 0.0;
    }
    
    double getMemoryUsage() {
        // Simplified memory usage calculation
        // In a real implementation, this would use system-specific APIs
        return 0.0;
    }
    
    double getDiskUsage() {
        // Simplified disk usage calculation
        // In a real implementation, this would use filesystem APIs
        return 0.0;
    }
};

// Global node instance for signal handling
GXCNode* g_node = nullptr;

void signalHandler(int signal) {
    Logger::info("Received signal " + std::to_string(signal) + ", shutting down gracefully...");
    if (g_node) {
        g_node->shutdown();
    }
    exit(0);
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -c, --config <path>    Configuration file path (default: config/gxc.conf)\n"
              << "  -d, --daemon           Run as daemon\n"
              << "  -v, --version          Show version information\n"
              << "  -h, --help             Show this help message\n"
              << "  --testnet              Run on testnet\n"
              << "  --regtest              Run in regression test mode\n";
}

void printVersion() {
    std::cout << "GXC Blockchain Node v2.0.0\n"
              << "Advanced cryptocurrency with hybrid consensus\n"
              << "Copyright (c) 2024 GXC Development Team\n";
}

int main(int argc, char* argv[]) {
    std::string configPath = "config/gxc.conf";
    bool daemon = false;
    bool testnet = false;
    bool regtest = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                configPath = argv[++i];
            } else {
                std::cerr << "Error: --config requires a path argument\n";
                return 1;
            }
        } else if (arg == "-d" || arg == "--daemon") {
            daemon = true;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--testnet") {
            testnet = true;
        } else if (arg == "--regtest") {
            regtest = true;
        } else {
            std::cerr << "Error: Unknown option " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    try {
        // Initialize logging
        Logger::initialize("logs/gxc-node.log");
        Logger::info("Starting GXC Node...");
        
        // Adjust config path for testnet/regtest
        if (testnet) {
            configPath = "config/gxc-testnet.conf";
            Logger::info("Running on testnet");
        } else if (regtest) {
            configPath = "config/gxc-regtest.conf";
            Logger::info("Running in regression test mode");
        }
        
        // Create and initialize node
        GXCNode node;
        g_node = &node;
        
        // Set up signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
#ifdef SIGQUIT
        signal(SIGQUIT, signalHandler);
#endif
        
        if (!node.initialize(configPath)) {
            Logger::error("Failed to initialize node");
            return 1;
        }
        
        if (daemon) {
            Logger::info("Running as daemon");
            // In a real implementation, this would fork the process
            // For now, just run normally
        }
        
        // Run the node
        node.run();
        
    } catch (const std::exception& e) {
        Logger::error("Fatal error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        Logger::error("Unknown fatal error occurred");
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
    
    Logger::info("GXC Node exited normally");
    return 0;
}