#include "../include/blockchain.h"
#include "../include/Config.h"
#include "../include/Logger.h"
#include "../include/Database.h"
#include "../include/Utils.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <atomic>

// Global variables for signal handling
std::atomic<bool> shutdownRequested(false);
std::unique_ptr<Blockchain> blockchain;
std::unique_ptr<Database> database;
std::unique_ptr<Config> config;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", initiating graceful shutdown..." << std::endl;
    shutdownRequested = true;
}

// Print usage information
void printUsage(const char* programName) {
    std::cout << "GXC Blockchain Node v2.0.0" << std::endl;
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help              Show this help message" << std::endl;
    std::cout << "  -c, --config <file>     Specify configuration file (default: gxc.conf)" << std::endl;
    std::cout << "  -d, --datadir <dir>     Specify data directory (default: ~/.gxc)" << std::endl;
    std::cout << "  --testnet               Run on testnet" << std::endl;
    std::cout << "  --debug                 Enable debug logging" << std::endl;
    std::cout << "  --mining                Enable mining" << std::endl;
    std::cout << "  --validator             Enable validator mode" << std::endl;
    std::cout << "  --rpc-port <port>       RPC server port (default: 8332)" << std::endl;
    std::cout << "  --rest-port <port>      REST API port (default: 8334)" << std::endl;
    std::cout << "  --max-connections <n>   Maximum peer connections (default: 125)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << "                     # Start node with default settings" << std::endl;
    std::cout << "  " << programName << " --testnet           # Start on testnet" << std::endl;
    std::cout << "  " << programName << " --mining            # Start with mining enabled" << std::endl;
    std::cout << "  " << programName << " --validator         # Start as validator" << std::endl;
    std::cout << std::endl;
}

// Parse command line arguments
bool parseArguments(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        else if (arg == "-c" || arg == "--config") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --config requires a filename" << std::endl;
                return false;
            }
            // Config file path is handled in main()
            ++i;
        }
        else if (arg == "-d" || arg == "--datadir") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --datadir requires a directory path" << std::endl;
                return false;
            }
            config.setString("data_directory", argv[++i]);
        }
        else if (arg == "--testnet") {
            config.setBool("testnet", true);
        }
        else if (arg == "--debug") {
            config.setString("log_level", "DEBUG");
        }
        else if (arg == "--mining") {
            config.setBool("mining_enabled", true);
        }
        else if (arg == "--validator") {
            config.setBool("validator_enabled", true);
        }
        else if (arg == "--rpc-port") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --rpc-port requires a port number" << std::endl;
                return false;
            }
            try {
                int port = std::stoi(argv[++i]);
                if (port <= 0 || port > 65535) {
                    std::cerr << "Error: Invalid RPC port: " << port << std::endl;
                    return false;
                }
                config.setInt("rpc_port", port);
            } catch (const std::exception&) {
                std::cerr << "Error: Invalid RPC port number" << std::endl;
                return false;
            }
        }
        else if (arg == "--rest-port") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --rest-port requires a port number" << std::endl;
                return false;
            }
            try {
                int port = std::stoi(argv[++i]);
                if (port <= 0 || port > 65535) {
                    std::cerr << "Error: Invalid REST port: " << port << std::endl;
                    return false;
                }
                config.setInt("rest_port", port);
            } catch (const std::exception&) {
                std::cerr << "Error: Invalid REST port number" << std::endl;
                return false;
            }
        }
        else if (arg == "--max-connections") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --max-connections requires a number" << std::endl;
                return false;
            }
            try {
                int connections = std::stoi(argv[++i]);
                if (connections <= 0 || connections > 1000) {
                    std::cerr << "Error: Invalid max connections: " << connections << std::endl;
                    return false;
                }
                config.setInt("max_connections", connections);
            } catch (const std::exception&) {
                std::cerr << "Error: Invalid max connections number" << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "Error: Unknown option: " << arg << std::endl;
            std::cerr << "Use --help for usage information" << std::endl;
            return false;
        }
    }
    
    return true;
}

// Initialize the node
bool initializeNode() {
    try {
        // Expand data directory path
        std::string dataDir = Utils::expandPath(config->getDataDirectory());
        
        // Create data directory if it doesn't exist
        if (!Utils::createDirectory(dataDir)) {
            std::cerr << "Failed to create data directory: " << dataDir << std::endl;
            return false;
        }
        
        // Initialize logger
        std::string logPath = dataDir + "/" + config->getLogFile();
        LogLevel logLevel = LogLevel::INFO;
        
        std::string logLevelStr = config->getLogLevel();
        if (logLevelStr == "DEBUG") logLevel = LogLevel::DEBUG;
        else if (logLevelStr == "WARNING") logLevel = LogLevel::WARNING;
        else if (logLevelStr == "ERROR") logLevel = LogLevel::ERROR;
        else if (logLevelStr == "CRITICAL") logLevel = LogLevel::CRITICAL;
        
        if (!Logger::getInstance().initialize(logPath, logLevel)) {
            std::cerr << "Failed to initialize logger" << std::endl;
            return false;
        }
        
        Logger::getInstance().setFileRotation(config->getLogMaxSize(), config->getLogMaxFiles());
        
        LOG_INFO("=== GXC Blockchain Node v2.0.0 Starting ===");
        LOG_INFO("Data directory: " + dataDir);
        LOG_INFO("Network: " + std::string(config->isTestNet() ? "testnet" : "mainnet"));
        
        // Initialize database
        database = std::make_unique<Database>(dataDir);
        if (!database->initialize()) {
            LOG_ERROR("Failed to initialize database");
            return false;
        }
        
        LOG_INFO("Database initialized successfully");
        
        // Initialize blockchain
        blockchain = std::make_unique<Blockchain>();
        if (!blockchain) {
            LOG_ERROR("Failed to create blockchain instance");
            return false;
        }
        
        LOG_INFO("Blockchain initialized successfully");
        LOG_INFO("Current block height: " + std::to_string(blockchain->getChainLength()));
        LOG_INFO("Current difficulty: " + Utils::doubleToString(blockchain->getDifficulty(), 6));
        LOG_INFO("Block reward: " + Utils::formatAmount(blockchain->getBlockReward(), "GXC"));
        
        // Validate configuration
        if (!config->validateConfig()) {
            LOG_ERROR("Configuration validation failed");
            auto errors = config->getValidationErrors();
            for (const auto& error : errors) {
                LOG_ERROR("Config error: " + error);
            }
            return false;
        }
        
        LOG_INFO("Configuration validated successfully");
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        return false;
    }
}

// Main node loop
void runNode() {
    LOG_INFO("Node started successfully");
    LOG_INFO("Listening on " + config->getListenAddress() + ":" + std::to_string(config->getListenPort()));
    
    if (config->isRpcEnabled()) {
        LOG_INFO("RPC server enabled on port " + std::to_string(config->getRpcPort()));
    }
    
    if (config->isRestEnabled()) {
        LOG_INFO("REST API enabled on port " + std::to_string(config->getRestPort()));
    }
    
    if (config->isMiningEnabled()) {
        LOG_INFO("Mining enabled with algorithm: " + config->getMiningAlgorithm());
        if (!config->getMinerAddress().empty()) {
            LOG_INFO("Mining address: " + config->getMinerAddress());
        }
    }
    
    if (config->isValidatorEnabled()) {
        LOG_INFO("Validator mode enabled");
    }
    
    // Main event loop
    auto lastStatusReport = Utils::getCurrentTimestamp();
    uint32_t lastBlockHeight = blockchain->getChainLength();
    
    while (!shutdownRequested) {
        try {
            // Process pending operations
            // In a real implementation, this would include:
            // - Network message processing
            // - Mining operations
            // - Validator operations
            // - RPC/API request handling
            // - Peer management
            // - Block validation and propagation
            
            // For now, just sleep and report status periodically
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Report status every 60 seconds
            auto now = Utils::getCurrentTimestamp();
            if (now - lastStatusReport >= 60) {
                uint32_t currentHeight = blockchain->getChainLength();
                
                LOG_INFO("=== Node Status Report ===");
                LOG_INFO("Uptime: " + std::to_string(now - lastStatusReport) + " seconds");
                LOG_INFO("Block height: " + std::to_string(currentHeight));
                LOG_INFO("Blocks processed: " + std::to_string(currentHeight - lastBlockHeight));
                LOG_INFO("Difficulty: " + Utils::doubleToString(blockchain->getDifficulty(), 6));
                LOG_INFO("Memory usage: " + std::to_string(database->getDatabaseSize() / 1024 / 1024) + " MB");
                
                lastStatusReport = now;
                lastBlockHeight = currentHeight;
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("Error in main loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    
    LOG_INFO("Shutdown requested, stopping node...");
}

// Cleanup resources
void cleanup() {
    LOG_INFO("Cleaning up resources...");
    
    // Close blockchain
    if (blockchain) {
        blockchain.reset();
        LOG_INFO("Blockchain closed");
    }
    
    // Close database
    if (database) {
        database->close();
        database.reset();
        LOG_INFO("Database closed");
    }
    
    // Close logger
    Logger::getInstance().info("=== GXC Blockchain Node Shutdown Complete ===");
    Logger::getInstance().close();
    Logger::cleanup();
    
    std::cout << "GXC Node shutdown complete." << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // Handle command line arguments to find config file
        std::string configFile = "gxc.conf";
        for (int i = 1; i < argc - 1; ++i) {
            if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config") {
                configFile = argv[i + 1];
                break;
            }
        }
        
        // Initialize configuration
        config = std::make_unique<Config>(configFile);
        if (!config->loadConfig()) {
            std::cerr << "Failed to load configuration" << std::endl;
            return 1;
        }
        
        // Parse command line arguments (may override config)
        if (!parseArguments(argc, argv, *config)) {
            return 1;
        }
        
        // Set up signal handlers
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
#ifndef _WIN32
        std::signal(SIGHUP, signalHandler);
#endif
        
        // Initialize node
        if (!initializeNode()) {
            std::cerr << "Failed to initialize node" << std::endl;
            cleanup();
            return 1;
        }
        
        // Save configuration (in case we updated it with command line args)
        config->saveConfig();
        
        // Run the node
        runNode();
        
        // Cleanup
        cleanup();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        cleanup();
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        cleanup();
        return 1;
    }
}