#include "../include/WebSocketServer.h"
#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <thread>
#include <chrono>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>

WebSocketServer::WebSocketServer(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), serverPort(port), isRunning(false) {
    LOG_API(LogLevel::INFO, "WebSocket server initialized on port " + std::to_string(port));
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start() {
    if (isRunning) {
        LOG_API(LogLevel::WARNING, "WebSocket server is already running");
        return true;
    }
    
    LOG_API(LogLevel::INFO, "Starting WebSocket server");
    
    try {
        isRunning = true;
        serverThread = std::thread(&WebSocketServer::serverLoop, this);
        
        LOG_API(LogLevel::INFO, "WebSocket server started successfully on port " + std::to_string(serverPort));
        return true;
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Failed to start WebSocket server: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void WebSocketServer::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_API(LogLevel::INFO, "Stopping WebSocket server");
    
    isRunning = false;
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& client : clients) {
            if (client.second != -1) {
                close(client.second);
            }
        }
        clients.clear();
    }
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    LOG_API(LogLevel::INFO, "WebSocket server stopped");
}

void WebSocketServer::serverLoop() {
    LOG_API(LogLevel::INFO, "WebSocket server loop started");
    
    while (isRunning) {
        try {
            // Handle incoming connections
            handleIncomingConnections();
            
            // Send periodic updates (placeholder)
            // sendPeriodicUpdates();
            
            // Clean up disconnected clients (placeholder)
            // cleanupClients();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_API(LogLevel::ERROR, "Error in WebSocket server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_API(LogLevel::INFO, "WebSocket server loop stopped");
}

bool WebSocketServer::addClient(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    if (clients.find(clientId) != clients.end()) {
        LOG_API(LogLevel::WARNING, "Client already exists: " + clientId);
        return false;
    }
    
    // Add client to map with socket -1 (placeholder)
    clients[clientId] = -1;  // Would be set from actual connection
    
    LOG_API(LogLevel::INFO, "Added WebSocket client: " + clientId);
    
    // Send initial blockchain state
    // sendBlockchainState(clientId);  // Placeholder
    
    return true;
}

void WebSocketServer::removeClient(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    auto it = clients.find(clientId);
    if (it != clients.end()) {
        clients.erase(it);
        LOG_API(LogLevel::INFO, "Removed WebSocket client: " + clientId);
    }
}

void WebSocketServer::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    for (auto& client : clients) {
        if (client.second != -1) {  // Socket is valid
            // sendMessageToClient(client.first, message);  // Placeholder
        }
    }
}

void WebSocketServer::sendMessageToClient(const std::string& clientId, const std::string& message) {
    auto it = clients.find(clientId);
    if (it == clients.end() || it->second == -1) {
        return;
    }
    
    try {
        // In real implementation, would send actual WebSocket message
        LOG_API(LogLevel::DEBUG, "Sent message to client " + clientId + ": " + message.substr(0, 100));
        
        // Statistics tracking would be done elsewhere
        // it->second.bytesSent += message.length();
        // it->second.messagesSent++;
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Failed to send message to client " + clientId + ": " + std::string(e.what()));
        // Mark client as disconnected
        it->second = -1;
    }
}

void WebSocketServer::notifyNewBlock(const Block& block) {
    std::string message = createBlockNotification(block);
    broadcastMessage(message);
    
    LOG_API(LogLevel::INFO, "Broadcasted new block notification: " + block.getHash().substr(0, 16));
}

void WebSocketServer::notifyNewTransaction(const Transaction& transaction) {
    std::string message = createTransactionNotification(transaction);
    broadcastMessage(message);
    
    LOG_API(LogLevel::DEBUG, "Broadcasted new transaction notification: " + transaction.getHash().substr(0, 16));
}

// NetworkStats methods removed

uint32_t WebSocketServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    uint32_t count = 0;
    for (const auto& client : clients) {
        if (client.second != -1) {  // Socket is valid
            count++;
        }
    }
    
    return count;
}

// getConnectedClients method removed - use getClientCount() instead

void WebSocketServer::handleIncomingConnections() {
    // Simplified implementation - would handle actual WebSocket handshake
    static uint32_t clientCounter = 0;
    static auto lastConnectionTime = Utils::getCurrentTimestamp();
    
    // Simulate new connections every 30 seconds for demo
    auto currentTime = Utils::getCurrentTimestamp();
    if (currentTime - lastConnectionTime > 30 && getClientCount() < 10) {
        std::string clientId = "client_" + std::to_string(++clientCounter);
        addClient(clientId);
        lastConnectionTime = currentTime;
    }
}

void WebSocketServer::sendPeriodicUpdates() {
    static auto lastUpdate = Utils::getCurrentTimestamp();
    auto currentTime = Utils::getCurrentTimestamp();
    
    // Send updates every 10 seconds
    if (currentTime - lastUpdate >= 10) {
        // Send blockchain stats
        std::string statsMessage = createBlockchainStatsMessage();
        broadcastMessage(statsMessage);
        
        lastUpdate = currentTime;
    }
}

void WebSocketServer::cleanupClients() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (auto it = clients.begin(); it != clients.end();) {
        // Remove clients that haven't pinged in 60 seconds
            // Clean up clients that haven't been active (placeholder)
            // if (currentTime - it->second.lastPing > 60) {
            if (false) {  // Placeholder - would check last ping time
            LOG_API(LogLevel::INFO, "Removing inactive client: " + it->first);
            it = clients.erase(it);
        } else {
            ++it;
        }
    }
}

void WebSocketServer::sendBlockchainState(const std::string& clientId) {
    std::string message = R"({
        "type": "blockchain_state",
        "data": {
            "height": )" + std::to_string(blockchain->getHeight()) + R"(,
            "difficulty": )" + std::to_string(blockchain->getDifficulty()) + R"(,
            "latest_block": ")" + blockchain->getLatestBlock().getHash() + R"(",
            "total_supply": )" + std::to_string(50000000.0) + R"(,
            "traceability_enabled": true
        }
    })";
    
    sendMessageToClient(clientId, message);
}

std::string WebSocketServer::createBlockNotification(const Block& block) {
    std::ostringstream oss;
    oss << R"({
        "type": "new_block",
        "data": {
            "hash": ")" << block.getHash() << R"(",
            "height": )" << block.getIndex() << R"(,
            "timestamp": )" << block.getTimestamp() << R"(,
            "transactions": )" << block.getTransactions().size() << R"(,
            "miner": ")" << block.getMinerAddress() << R"(",
            "difficulty": )" << block.getDifficulty() << R"(
        }
    })";
    
    return oss.str();
}

std::string WebSocketServer::createTransactionNotification(const Transaction& transaction) {
    std::ostringstream oss;
    oss << R"({
        "type": "new_transaction",
        "data": {
            "hash": ")" << transaction.getHash() << R"(",
            "timestamp": )" << transaction.getTimestamp() << R"(,
            "amount": )" << transaction.getTotalOutputAmount() << R"(,
            "fee": )" << transaction.getFee() << R"(",
            "inputs": )" << transaction.getInputs().size() << R"(,
            "outputs": )" << transaction.getOutputs().size() << R"(,
            "prev_tx_hash": ")" << transaction.getPrevTxHash() << R"(",
            "traceable": )" << (transaction.isTraceabilityValid() ? "true" : "false") << R"(
        }
    })";
    
    return oss.str();
}

// createNetworkStatsNotification method removed - functionality moved elsewhere

std::string WebSocketServer::createBlockchainStatsMessage() {
    std::ostringstream oss;
    oss << R"({
        "type": "blockchain_stats",
        "data": {
            "height": )" << blockchain->getHeight() << R"(,
            "difficulty": )" << blockchain->getDifficulty() << R"(,
            "block_reward": )" << 5000000.0 << R"(,
            "total_supply": )" << 50000000.0 << R"(,
            "timestamp": )" << Utils::getCurrentTimestamp() << R"(,
            "traceability_formula": "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount"
        }
    })";
    
    return oss.str();
}