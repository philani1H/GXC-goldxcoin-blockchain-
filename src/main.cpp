#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Database.h"
#include "../include/Network.h"
#include "../include/Utils.h"
#include <iostream>
#include <signal.h>
#include <thread>

bool g_shutdown = false;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Initiating shutdown..." << std::endl;
    g_shutdown = true;
}

int main() {
    std::cout << R"(
   _____ _  _  ___    ____  _            _        _           _       
  / ____| \| |/ __|  |  _ \| |          | |      | |         (_)      
 | |  __|  .` | (__   | |_) | | ___  ___| | _____| |__   __ _ _ _ __   
 | | |_ | |\ | \__|  |  _ <| |/ _ \/ __| |/ / __| '_ \ / _` | | '_ \  
 | |__| | | | |___   | |_) | | (_) \__ \   < (__| | | | (_| | | | | | 
  \_____|_| |_|___|  |____/|_|\___/|___/_|\_\___|_| |_|\__,_|_|_| |_| 
                                                                      
GXC Blockchain Core Node - Advanced Cryptocurrency with Traceability
Version 2.0.0
    )" << std::endl;

    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Initialize logging
        Logger::initialize();
        LOG_CORE(LogLevel::INFO, "GXC Blockchain Core starting up");

        // Initialize configuration
        Config::initialize();
        
        // Initialize database
        std::string dataDir = Config::get("data_dir", "./gxc_data");
        Database::initialize(dataDir + "/blockchain.db");
        
        // Initialize blockchain
        Blockchain blockchain;
        if (!blockchain.initialize()) {
            LOG_CORE(LogLevel::ERROR, "Failed to initialize blockchain");
            return 1;
        }
        
        // Initialize network
        Network network;
        int port = Config::getInt("network_port", 9333);
        if (!network.start(port)) {
            LOG_CORE(LogLevel::ERROR, "Failed to start network on port " + std::to_string(port));
            return 1;
        }
        
        LOG_CORE(LogLevel::INFO, "GXC node initialized successfully");
        LOG_CORE(LogLevel::INFO, "Blockchain height: " + std::to_string(blockchain.getHeight()));
        LOG_CORE(LogLevel::INFO, "Network listening on port: " + std::to_string(port));
        
        // Display traceability information
        std::cout << "\n=== GXC Traceability System ===" << std::endl;
        std::cout << "Formula: Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount" << std::endl;
        std::cout << "Status: " << (blockchain.validateTraceability() ? "VALID" : "INVALID") << std::endl;
        std::cout << "===============================" << std::endl;
        
        // Main event loop
        LOG_CORE(LogLevel::INFO, "Entering main event loop. Press Ctrl+C to shutdown.");
        
        auto lastUpdate = Utils::getCurrentTimestamp();
        while (!g_shutdown) {
            try {
                // Process blockchain operations
                blockchain.processTransactions();
                
                // Update network
                network.update();
                
                // Periodic logging every 60 seconds
                auto currentTime = Utils::getCurrentTimestamp();
                if (currentTime - lastUpdate >= 60) {
                    LOG_CORE(LogLevel::INFO, "Node Status - Height: " + 
                            std::to_string(blockchain.getHeight()) + 
                            ", Peers: " + std::to_string(network.getPeerCount()) +
                            ", Difficulty: " + Utils::formatAmount(blockchain.getDifficulty(), 2));
                    lastUpdate = currentTime;
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
