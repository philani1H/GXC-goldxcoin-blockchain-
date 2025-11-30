#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Database.h"
#include "../include/Network.h"
#include "../include/RPCAPI.h"
#include "../include/RESTServer.h"
#include "../include/Utils.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <fstream>
#include <filesystem>
#include <cstdlib>

bool g_shutdown = false;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Initiating shutdown..." << std::endl;
    g_shutdown = true;
}

void printUsage(const char* program) {
    std::cout << "GXC Node - Usage:" << std::endl;
    std::cout << "  " << program << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h                 Show this help message" << std::endl;
    std::cout << "  --config=FILE, -c FILE     Configuration file path" << std::endl;
    std::cout << "  --datadir=DIR, -d DIR      Data directory path" << std::endl;
    std::cout << "  --port=PORT, -p PORT       Network port (default: 9333)" << std::endl;
    std::cout << "  --rpc-port=PORT            RPC port (default: 8332)" << std::endl;
    std::cout << "  --rest-port=PORT           REST API port (default: 8080)" << std::endl;
    std::cout << "  --testnet                  Use testnet configuration" << std::endl;
    std::cout << "  --daemon                   Run as daemon" << std::endl;
    std::cout << "  --verbose, -v              Verbose logging" << std::endl;
    std::cout << "  --quiet, -q                Quiet mode (errors only)" << std::endl;
    std::cout << std::endl;
}

struct NodeConfig {
    std::string configFile;
    std::string dataDir = "./gxc_data";
    int networkPort = 9333;
    int rpcPort = 8332;
    int restPort = 8080;
    bool testnet = false;
    bool daemon = false;
    bool verbose = false;
    bool quiet = false;
};

bool parseArguments(int argc, char* argv[], NodeConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return false;
        }
        else if (arg.find("--config=") == 0) {
            config.configFile = arg.substr(9);
        }
        else if (arg == "-c" && i + 1 < argc) {
            config.configFile = argv[++i];
        }
        else if (arg.find("--datadir=") == 0) {
            config.dataDir = arg.substr(10);
        }
        else if (arg == "-d" && i + 1 < argc) {
            config.dataDir = argv[++i];
        }
        else if (arg.find("--port=") == 0) {
            config.networkPort = std::stoi(arg.substr(7));
        }
        else if (arg == "-p" && i + 1 < argc) {
            config.networkPort = std::stoi(argv[++i]);
        }
        else if (arg.find("--rpc-port=") == 0) {
            config.rpcPort = std::stoi(arg.substr(11));
        }
        else if (arg.find("--rest-port=") == 0) {
            config.restPort = std::stoi(arg.substr(12));
        }
        else if (arg == "--testnet") {
            config.testnet = true;
        }
        else if (arg == "--daemon") {
            config.daemon = true;
        }
        else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        }
        else if (arg == "--quiet" || arg == "-q") {
            config.quiet = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return false;
        }
    }
    
    return true;
}

void printBanner() {
    std::cout << R"(
   _____ _  _  ___    _   _           _      
  / ____| \| |/ __|  | \ | |         | |     
 | |  __|  .` | (__   |  \| | ___  __| | ___ 
 | | |_ | |\ | \__|  | . ` |/ _ \/ _` |/ _ \
 | |__| | | | |___   | |\  | (_) | (_| |  __/
  \_____|_| |_|___|  |_| \_|\___/ \__,_|\___|
                                             
GXC Blockchain Full Node
Version 2.0.0 - Production Ready
    )" << std::endl;
}

bool createDataDirectory(const std::string& dataDir) {
    try {
        // Create directory if it doesn't exist
        std::filesystem::create_directories(dataDir);
        std::filesystem::create_directories(dataDir + "/blocks");
        std::filesystem::create_directories(dataDir + "/chainstate");
        std::filesystem::create_directories(dataDir + "/wallet");
        std::filesystem::create_directories(dataDir + "/logs");
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create data directory: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // Parse command line arguments
    NodeConfig nodeConfig;
    if (!parseArguments(argc, argv, nodeConfig)) {
        return 1;
    }
    
    // Adjust ports for testnet
    if (nodeConfig.testnet) {
        nodeConfig.networkPort = 19333;
        // Use Railway PORT env var if available, otherwise default to 18332
        const char* railwayPort = std::getenv("PORT");
        if (railwayPort != nullptr) {
            nodeConfig.rpcPort = std::stoi(railwayPort);
            std::cout << "Using Railway PORT: " << nodeConfig.rpcPort << std::endl;
        } else {
            nodeConfig.rpcPort = 18332;
        }
        nodeConfig.restPort = 18080;
        std::cout << "Testnet mode enabled" << std::endl;
    } else {
        // For mainnet, also check Railway PORT
        const char* railwayPort = std::getenv("PORT");
        if (railwayPort != nullptr) {
            nodeConfig.rpcPort = std::stoi(railwayPort);
            std::cout << "Using Railway PORT: " << nodeConfig.rpcPort << std::endl;
        }
    }
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Initialize logging
        Logger::initialize();
        LogLevel logLevel = nodeConfig.quiet ? LogLevel::ERROR : 
                           (nodeConfig.verbose ? LogLevel::DEBUG : LogLevel::INFO);
        // Logger level set via initialization
        
        LOG_CORE(LogLevel::INFO, "GXC Node starting up");

        // Load configuration
        Config::initialize();
        if (!nodeConfig.configFile.empty()) {
            Config::loadFromFile(nodeConfig.configFile);
        }
        
        // Override config with command line options
        Config::set("data_dir", nodeConfig.dataDir);
        Config::set("network_port", std::to_string(nodeConfig.networkPort));
        Config::set("rpc_port", std::to_string(nodeConfig.rpcPort));
        Config::set("rest_port", std::to_string(nodeConfig.restPort));
        Config::set("testnet", nodeConfig.testnet ? "true" : "false");
        
        // Create data directory
        if (!createDataDirectory(nodeConfig.dataDir)) {
            return 1;
        }
        
        LOG_CORE(LogLevel::INFO, "Data directory: " + nodeConfig.dataDir);
        
        // Initialize database
        Database::initialize(nodeConfig.dataDir + "/blockchain.db");
        
        // Initialize blockchain
        Blockchain blockchain;
        if (!blockchain.initialize()) {
            LOG_CORE(LogLevel::ERROR, "Failed to initialize blockchain");
            return 1;
        }
        
        // Initialize network
        Network network;
        if (!network.start(nodeConfig.networkPort)) {
            LOG_CORE(LogLevel::ERROR, "Failed to start network on port " + std::to_string(nodeConfig.networkPort));
            return 1;
        }
        
        // Initialize RPC server
        RPCAPI rpcServer(&blockchain, nodeConfig.rpcPort);
        if (!rpcServer.start()) {
            LOG_CORE(LogLevel::ERROR, "Failed to start RPC server on port " + std::to_string(nodeConfig.rpcPort));
            return 1;
        }
        
        // Initialize REST server
        RESTServer restServer(&blockchain, static_cast<uint16_t>(nodeConfig.restPort));
        if (!restServer.start()) {
            LOG_CORE(LogLevel::ERROR, "Failed to start REST server on port " + std::to_string(nodeConfig.restPort));
            return 1;
        }
        
        LOG_CORE(LogLevel::INFO, "GXC node initialized successfully");
        LOG_CORE(LogLevel::INFO, "Blockchain height: " + std::to_string(blockchain.getHeight()));
        LOG_CORE(LogLevel::INFO, "Network listening on port: " + std::to_string(nodeConfig.networkPort));
        LOG_CORE(LogLevel::INFO, "RPC server listening on port: " + std::to_string(nodeConfig.rpcPort));
        LOG_CORE(LogLevel::INFO, "REST server listening on port: " + std::to_string(nodeConfig.restPort));
        
        // Display node information
        std::cout << "\n=== Node Information ===" << std::endl;
        std::cout << "Network: " << (nodeConfig.testnet ? "Testnet" : "Mainnet") << std::endl;
        std::cout << "Data Directory: " << nodeConfig.dataDir << std::endl;
        std::cout << "Blockchain Height: " << blockchain.getHeight() << std::endl;
        std::cout << "Network Port: " << nodeConfig.networkPort << std::endl;
        std::cout << "RPC Port: " << nodeConfig.rpcPort << std::endl;
        std::cout << "REST Port: " << nodeConfig.restPort << std::endl;
        std::cout << "Traceability: " << (blockchain.validateTraceability() ? "VALID" : "INVALID") << std::endl;
        std::cout << "========================" << std::endl;
        
        // Run as daemon if requested
        if (nodeConfig.daemon) {
            LOG_CORE(LogLevel::INFO, "Running in daemon mode");
        }
        
        // Main event loop
        LOG_CORE(LogLevel::INFO, "Entering main event loop. Press Ctrl+C to shutdown.");
        
        auto nodeStartTime = Utils::getCurrentTimestamp();
        auto lastUpdate = nodeStartTime;
        auto lastStatsDisplay = nodeStartTime;
        
        while (!g_shutdown) {
            try {
                // Process blockchain operations
                blockchain.processTransactions();
                
                // Update network
                network.update();
                
                // Process RPC requests
                rpcServer.processRequests();
                
                // Process REST requests
                // REST server runs in background thread
                
                // Periodic logging every 60 seconds
                auto currentTime = Utils::getCurrentTimestamp();
                if (currentTime - lastUpdate >= 60) {
                    LOG_CORE(LogLevel::INFO, "Node Status - Height: " + 
                            std::to_string(blockchain.getHeight()) + 
                            ", Peers: " + std::to_string(network.getPeerCount()) +
                            ", Difficulty: " + std::to_string(blockchain.getDifficulty()) +
                            "");
                    lastUpdate = currentTime;
                }
                
                // Display stats every 10 minutes if not in daemon mode
                if (!nodeConfig.daemon && !nodeConfig.quiet && currentTime - lastStatsDisplay >= 600) {
                    std::cout << "\n=== Node Statistics ===" << std::endl;
                    std::cout << "Uptime: " << std::to_string(currentTime - nodeStartTime) << " seconds" << std::endl;
                    std::cout << "Height: " << blockchain.getHeight() << std::endl;
                    std::cout << "Peers: " << network.getPeerCount() << std::endl;
                    std::cout << "Difficulty: " << std::to_string(blockchain.getDifficulty()) << std::endl;
                    std::cout << "=======================" << std::endl;
                    lastStatsDisplay = currentTime;
                }
                
                // Sleep to prevent excessive CPU usage
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
            } catch (const std::exception& e) {
                LOG_CORE(LogLevel::ERROR, "Error in main loop: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        
        // Shutdown sequence
        LOG_CORE(LogLevel::INFO, "Starting shutdown sequence");
        
        restServer.stop();
        rpcServer.stop();
        network.stop();
        blockchain.shutdown();
        Database::shutdown();
        Config::shutdown();
        Logger::shutdown();
        
        std::cout << "GXC node shutdown complete." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        LOG_CORE(LogLevel::ERROR, "Fatal error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}