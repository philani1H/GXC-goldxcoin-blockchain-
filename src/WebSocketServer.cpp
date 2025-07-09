#include "../include/WebSocketServer.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <thread>
#include <chrono>
#include <sstream>

WebSocketServer::WebSocketServer(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), serverPort(port), isRunning(false) {
    LOG_WEBSOCKET(LogLevel::INFO, "WebSocket server initialized on port " + std::to_string(port));
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start() {
    if (isRunning) {
        LOG_WEBSOCKET(LogLevel::WARNING, "WebSocket server is already running");
        return true;
    }
    
    LOG_WEBSOCKET(LogLevel::INFO, "Starting WebSocket server");
    
    try {
        isRunning = true;
        serverThread = std::thread(&WebSocketServer::serverLoop, this);
        
        LOG_WEBSOCKET(LogLevel::INFO, "WebSocket server started successfully on port " + std::to_string(serverPort));
        return true;
        
    } catch (const std::exception& e) {
        LOG_WEBSOCKET(LogLevel::ERROR, "Failed to start WebSocket server: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void WebSocketServer::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_WEBSOCKET(LogLevel::INFO, "Stopping WebSocket server");
    
    isRunning = false;
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& client : clients) {
            client.second.connected = false;
        }
        clients.clear();
    }
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    LOG_WEBSOCKET(LogLevel::INFO, "WebSocket server stopped");
}

void WebSocketServer::serverLoop() {
    LOG_WEBSOCKET(LogLevel::INFO, "WebSocket server loop started");
    
    while (isRunning) {
        try {
            // Handle incoming connections
            handleIncomingConnections();
            
            // Send periodic updates
            sendPeriodicUpdates();
            
            // Clean up disconnected clients
            cleanupClients();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_WEBSOCKET(LogLevel::ERROR, "Error in WebSocket server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_WEBSOCKET(LogLevel::INFO, "WebSocket server loop stopped");
}

bool WebSocketServer::addClient(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    if (clients.find(clientId) != clients.end()) {
        LOG_WEBSOCKET(LogLevel::WARNING, "Client already exists: " + clientId);
        return false;
    }
    
    WebSocketClient client;
    client.id = clientId;
    client.connected = true;
    client.connectTime = Utils::getCurrentTimestamp();
    client.lastPing = Utils::getCurrentTimestamp();
    
    clients[clientId] = client;
    
    LOG_WEBSOCKET(LogLevel::INFO, "Added WebSocket client: " + clientId);
    
    // Send initial blockchain state
    sendBlockchainState(clientId);
    
    return true;
}

void WebSocketServer::removeClient(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    auto it = clients.find(clientId);
    if (it != clients.end()) {
        clients.erase(it);
        LOG_WEBSOCKET(LogLevel::INFO, "Removed WebSocket client: " + clientId);
    }
}

void WebSocketServer::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    for (auto& client : clients) {
        if (client.second.connected) {
            sendMessageToClient(client.first, message);
        }
    }
}

void WebSocketServer::sendMessageToClient(const std::string& clientId, const std::string& message) {
    auto it = clients.find(clientId);
    if (it == clients.end() || !it->second.connected) {
        return;
    }
    
    try {
        // In real implementation, would send actual WebSocket message
        LOG_WEBSOCKET(LogLevel::DEBUG, "Sent message to client " + clientId + ": " + message.substr(0, 100));
        
        it->second.bytesSent += message.length();
        it->second.messagesSent++;
        
    } catch (const std::exception& e) {
        LOG_WEBSOCKET(LogLevel::ERROR, "Failed to send message to client " + clientId + ": " + std::string(e.what()));
        it->second.connected = false;
    }
}

void WebSocketServer::notifyNewBlock(const Block& block) {
    std::string message = createBlockNotification(block);
    broadcastMessage(message);
    
    LOG_WEBSOCKET(LogLevel::INFO, "Broadcasted new block notification: " + block.getHash().substr(0, 16));
}

void WebSocketServer::notifyNewTransaction(const Transaction& transaction) {
    std::string message = createTransactionNotification(transaction);
    broadcastMessage(message);
    
    LOG_WEBSOCKET(LogLevel::DEBUG, "Broadcasted new transaction notification: " + transaction.getHash().substr(0, 16));
}

void WebSocketServer::notifyNetworkStats(const NetworkStats& stats) {
    std::string message = createNetworkStatsNotification(stats);
    broadcastMessage(message);
    
    LOG_WEBSOCKET(LogLevel::DEBUG, "Broadcasted network stats update");
}

uint32_t WebSocketServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    uint32_t count = 0;
    for (const auto& client : clients) {
        if (client.second.connected) {
            count++;
        }
    }
    
    return count;
}

std::vector<WebSocketClient> WebSocketServer::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    std::vector<WebSocketClient> connectedClients;
    for (const auto& client : clients) {
        if (client.second.connected) {
            connectedClients.push_back(client.second);
        }
    }
    
    return connectedClients;
}

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
        if (currentTime - it->second.lastPing > 60) {
            LOG_WEBSOCKET(LogLevel::INFO, "Removing inactive client: " + it->first);
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
            "height": )" + std::to_string(blockchain->getChainLength()) + R"(,
            "difficulty": )" + std::to_string(blockchain->getDifficulty()) + R"(,
            "latest_block": ")" + blockchain->getLatestBlock().getHash() + R"(",
            "total_supply": )" + std::to_string(blockchain->getTotalSupply()) + R"(,
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

std::string WebSocketServer::createNetworkStatsNotification(const NetworkStats& stats) {
    std::ostringstream oss;
    oss << R"({
        "type": "network_stats",
        "data": {
            "connected_peers": )" << stats.connectedPeers << R"(,
            "total_bytes_sent": )" << stats.totalBytesSent << R"(,
            "total_bytes_received": )" << stats.totalBytesReceived << R"(,
            "messages_sent": )" << stats.messagesSent << R"(,
            "messages_received": )" << stats.messagesReceived << R"(,
            "uptime": )" << stats.uptime << R"(
        }
    })";
    
    return oss.str();
}

std::string WebSocketServer::createBlockchainStatsMessage() {
    std::ostringstream oss;
    oss << R"({
        "type": "blockchain_stats",
        "data": {
            "height": )" << blockchain->getChainLength() << R"(,
            "difficulty": )" << blockchain->getDifficulty() << R"(,
            "block_reward": )" << blockchain->getBlockReward() << R"(,
            "total_supply": )" << blockchain->getTotalSupply() << R"(,
            "timestamp": )" << Utils::getCurrentTimestamp() << R"(,
            "traceability_formula": "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount"
        }
    })";
    
    return oss.str();
}