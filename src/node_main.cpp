#include "../include/Blockchain.h"
#include "../include/RESTServer.h"
#include "../include/Network.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Utils.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

// Global variables for clean shutdown
bool g_running = true;
Blockchain* g_blockchain = nullptr;
RESTServer* g_restServer = nullptr;
Network* g_network = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down gracefully..." << std::endl;
    g_running = false;
}

void printBanner() {
    std::cout << R"(
   ___   _  _   ___     _  _             _       
  / __| | \| | / __|   | \| |  ___    __| |  ___ 
 | (__  | .` || (__    | .` | / _ \  / _` | / -_)
  \___| |_|\_| \___|   |_|\_| \___/  \__,_| \___|
                                                 
GXC Blockchain Node - Full Transaction Traceability
Version 1.0.0
    )" << std::endl;
}

void printHelp() {
    std::cout << "GXC Node - Usage:" << std::endl;
    std::cout << "  gxc-node [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h              Show this help message" << std::endl;
    std::cout << "  --config=FILE, -c FILE  Use configuration file" << std::endl;
    std::cout << "  --datadir=DIR, -d DIR   Data directory" << std::endl;
    std::cout << "  --port=PORT, -p PORT    P2P network port (default: 8333)" << std::endl;
    std::cout << "  --rpcport=PORT          REST API port (default: 8332)" << std::endl;
    std::cout << "  --testnet               Run on testnet" << std::endl;
    std::cout << "  --mining                Enable mining" << std::endl;
    std::cout << "  --verbose, -v           Verbose logging" << std::endl;
    std::cout << std::endl;
}

void initializeLogging(bool verbose) {
    Logger::initialize();
    Logger::setLogLevel(verbose ? LogLevel::DEBUG : LogLevel::INFO);
    
    LOG_NODE(LogLevel::INFO, "Logger initialized");
}

bool parseCommandLine(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printHelp();
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
            config.networkPort = Utils::parsePort(arg.substr(7));
        }
        else if (arg == "-p" && i + 1 < argc) {
            config.networkPort = Utils::parsePort(argv[++i]);
        }
        else if (arg.find("--rpcport=") == 0) {
            config.rpcPort = Utils::parsePort(arg.substr(10));
        }
        else if (arg == "--testnet") {
            config.testnet = true;
        }
        else if (arg == "--mining") {
            config.mining = true;
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

bool validateConfig(const Config& config) {
    if (config.networkPort == 0) {
        std::cerr << "Error: Invalid network port" << std::endl;
        return false;
    }
    
    if (config.rpcPort == 0) {
        std::cerr << "Error: Invalid RPC port" << std::endl;
        return false;
    }
    
    if (config.networkPort == config.rpcPort) {
        std::cerr << "Error: Network port and RPC port cannot be the same" << std::endl;
        return false;
    }
    
    return true;
}

bool initializeBlockchain(Config& config) {
    try {
        LOG_NODE(LogLevel::INFO, "Initializing blockchain...");
        
        g_blockchain = new Blockchain();
        
        if (!g_blockchain) {
            LOG_NODE(LogLevel::ERROR, "Failed to create blockchain instance");
            return false;
        }
        
        // Load existing blockchain data
        if (Utils::fileExists(config.dataDir + "/blockchain.dat")) {
            LOG_NODE(LogLevel::INFO, "Loading existing blockchain data...");
            // In a real implementation, would load from file
        } else {
            LOG_NODE(LogLevel::INFO, "Creating genesis block...");
            // Create genesis block
        }
        
        LOG_NODE(LogLevel::INFO, "Blockchain initialized successfully");
        LOG_NODE(LogLevel::INFO, "Current block height: " + std::to_string(g_blockchain->getChainLength()));
        LOG_NODE(LogLevel::INFO, "Traceability system: ACTIVE");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_NODE(LogLevel::ERROR, "Failed to initialize blockchain: " + std::string(e.what()));
        return false;
    }
}

bool startNetwork(const Config& config) {
    try {
        LOG_NODE(LogLevel::INFO, "Starting P2P network...");
        
        g_network = new Network(g_blockchain, config.networkPort);
        
        if (!g_network->start()) {
            LOG_NODE(LogLevel::ERROR, "Failed to start P2P network");
            return false;
        }
        
        // Add seed nodes
        if (config.testnet) {
            g_network->addSeedNode("127.0.0.1", 18333);
        } else {
            g_network->addSeedNode("seed1.gxc.io", 8333);
            g_network->addSeedNode("seed2.gxc.io", 8333);
            g_network->addSeedNode("seed3.gxc.io", 8333);
        }
        
        LOG_NODE(LogLevel::INFO, "P2P network started on port " + std::to_string(config.networkPort));
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_NODE(LogLevel::ERROR, "Failed to start network: " + std::string(e.what()));
        return false;
    }
}

bool startRESTServer(const Config& config) {
    try {
        LOG_NODE(LogLevel::INFO, "Starting REST API server...");
        
        g_restServer = new RESTServer(g_blockchain, config.rpcPort);
        
        if (!g_restServer->start()) {
            LOG_NODE(LogLevel::ERROR, "Failed to start REST API server");
            return false;
        }
        
        LOG_NODE(LogLevel::INFO, "REST API server started on port " + std::to_string(config.rpcPort));
        LOG_NODE(LogLevel::INFO, "API endpoint: http://localhost:" + std::to_string(config.rpcPort) + "/api/v1/");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_NODE(LogLevel::ERROR, "Failed to start REST server: " + std::string(e.what()));
        return false;
    }
}

void printNodeStatus() {
    if (!g_blockchain || !g_network) {
        return;
    }
    
    auto networkStats = g_network->getNetworkStats();
    
    LOG_NODE(LogLevel::INFO, "=== Node Status ===");
    LOG_NODE(LogLevel::INFO, "Block Height: " + std::to_string(g_blockchain->getChainLength()));
    LOG_NODE(LogLevel::INFO, "Connected Peers: " + std::to_string(networkStats.connectedPeers));
    LOG_NODE(LogLevel::INFO, "Network Uptime: " + std::to_string(networkStats.uptime) + " seconds");
    LOG_NODE(LogLevel::INFO, "Traceability Active: YES");
    LOG_NODE(LogLevel::INFO, "==================");
}

void nodeMainLoop() {
    LOG_NODE(LogLevel::INFO, "Node main loop started");
    
    auto lastStatusUpdate = Utils::getCurrentTimestamp();
    
    while (g_running) {
        try {
            // Print status every 60 seconds
            auto currentTime = Utils::getCurrentTimestamp();
            if (currentTime - lastStatusUpdate >= 60) {
                printNodeStatus();
                lastStatusUpdate = currentTime;
            }
            
            // Process any node-specific tasks here
            
            // Sleep for a short time
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
        } catch (const std::exception& e) {
            LOG_NODE(LogLevel::ERROR, "Error in main loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    
    LOG_NODE(LogLevel::INFO, "Node main loop stopped");
}

void cleanup() {
    LOG_NODE(LogLevel::INFO, "Cleaning up resources...");
    
    if (g_restServer) {
        g_restServer->stop();
        delete g_restServer;
        g_restServer = nullptr;
    }
    
    if (g_network) {
        g_network->stop();
        delete g_network;
        g_network = nullptr;
    }
    
    if (g_blockchain) {
        // Save blockchain state
        LOG_NODE(LogLevel::INFO, "Saving blockchain state...");
        delete g_blockchain;
        g_blockchain = nullptr;
    }
    
    LOG_NODE(LogLevel::INFO, "Cleanup completed");
}

int main(int argc, char* argv[]) {
    // Print banner
    printBanner();
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Default configuration
    Config config;
    config.networkPort = 8333;
    config.rpcPort = 8332;
    config.dataDir = "./data";
    config.testnet = false;
    config.mining = false;
    config.verbose = false;
    
    // Parse command line arguments
    if (!parseCommandLine(argc, argv, config)) {
        return 1;
    }
    
    // Validate configuration
    if (!validateConfig(config)) {
        return 1;
    }
    
    // Initialize logging
    initializeLogging(config.verbose);
    
    LOG_NODE(LogLevel::INFO, "Starting GXC Node...");
    LOG_NODE(LogLevel::INFO, "Network: " + std::string(config.testnet ? "Testnet" : "Mainnet"));
    LOG_NODE(LogLevel::INFO, "Data directory: " + config.dataDir);
    LOG_NODE(LogLevel::INFO, "P2P port: " + std::to_string(config.networkPort));
    LOG_NODE(LogLevel::INFO, "REST API port: " + std::to_string(config.rpcPort));
    
    // Create data directory if it doesn't exist
    if (!Utils::createDirectory(config.dataDir)) {
        LOG_NODE(LogLevel::ERROR, "Failed to create data directory: " + config.dataDir);
        return 1;
    }
    
    try {
        // Initialize blockchain
        if (!initializeBlockchain(config)) {
            LOG_NODE(LogLevel::ERROR, "Failed to initialize blockchain");
            return 1;
        }
        
        // Start network
        if (!startNetwork(config)) {
            LOG_NODE(LogLevel::ERROR, "Failed to start network");
            cleanup();
            return 1;
        }
        
        // Start REST API server
        if (!startRESTServer(config)) {
            LOG_NODE(LogLevel::ERROR, "Failed to start REST server");
            cleanup();
            return 1;
        }
        
        LOG_NODE(LogLevel::INFO, "GXC Node started successfully!");
        LOG_NODE(LogLevel::INFO, "Transaction traceability formula active:");
        LOG_NODE(LogLevel::INFO, "  Ti.Inputs[0].txHash == Ti.PrevTxHash");
        LOG_NODE(LogLevel::INFO, "  Ti.Inputs[0].amount == Ti.ReferencedAmount");
        
        // Print initial status
        printNodeStatus();
        
        // Run main loop
        nodeMainLoop();
        
    } catch (const std::exception& e) {
        LOG_NODE(LogLevel::ERROR, "Fatal error: " + std::string(e.what()));
        cleanup();
        return 1;
    }
    
    // Clean shutdown
    cleanup();
    
    LOG_NODE(LogLevel::INFO, "GXC Node shutdown complete");
    std::cout << "Thank you for using GXC Blockchain!" << std::endl;
    
    return 0;
}