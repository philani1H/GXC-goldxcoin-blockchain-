#include "../include/Network.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <thread>
#include <chrono>
#include <algorithm>

Network::Network(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), listenPort(port), isRunning(false) {
    LOG_NETWORK(LogLevel::INFO, "Network initialized on port " + std::to_string(port));
}

Network::~Network() {
    stop();
}

bool Network::start() {
    if (isRunning) {
        LOG_NETWORK(LogLevel::WARNING, "Network is already running");
        return true;
    }
    
    LOG_NETWORK(LogLevel::INFO, "Starting P2P network");
    
    try {
        isRunning = true;
        
        // Start network threads
        networkThread = std::thread(&Network::networkLoop, this);
        peerDiscoveryThread = std::thread(&Network::peerDiscoveryLoop, this);
        messageHandlerThread = std::thread(&Network::messageHandlerLoop, this);
        
        LOG_NETWORK(LogLevel::INFO, "P2P network started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to start network: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void Network::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_NETWORK(LogLevel::INFO, "Stopping P2P network");
    
    isRunning = false;
    
    // Disconnect all peers
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto& peer : peers) {
            peer.second.connected = false;
        }
        peers.clear();
    }
    
    // Join threads
    if (networkThread.joinable()) {
        networkThread.join();
    }
    
    if (peerDiscoveryThread.joinable()) {
        peerDiscoveryThread.join();
    }
    
    if (messageHandlerThread.joinable()) {
        messageHandlerThread.join();
    }
    
    LOG_NETWORK(LogLevel::INFO, "P2P network stopped");
}

void Network::networkLoop() {
    LOG_NETWORK(LogLevel::INFO, "Network loop started");
    
    while (isRunning) {
        try {
            // Process incoming connections
            processIncomingConnections();
            
            // Maintain existing connections
            maintainConnections();
            
            // Send periodic messages
            sendPeriodicMessages();
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in network loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Network loop stopped");
}

void Network::peerDiscoveryLoop() {
    LOG_NETWORK(LogLevel::INFO, "Peer discovery loop started");
    
    while (isRunning) {
        try {
            discoverPeers();
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in peer discovery: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Peer discovery loop stopped");
}

void Network::messageHandlerLoop() {
    LOG_NETWORK(LogLevel::INFO, "Message handler loop started");
    
    while (isRunning) {
        try {
            processMessageQueue();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in message handler: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Message handler loop stopped");
}

bool Network::connectToPeer(const std::string& address, uint16_t port) {
    std::string peerId = address + ":" + std::to_string(port);
    
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        if (peers.find(peerId) != peers.end()) {
            LOG_NETWORK(LogLevel::WARNING, "Already connected to peer: " + peerId);
            return true;
        }
    }
    
    try {
        LOG_NETWORK(LogLevel::INFO, "Connecting to peer: " + peerId);
        
        // Create new peer
        Peer peer;
        peer.id = peerId;
        peer.address = address;
        peer.port = port;
        peer.connected = false;
        peer.lastSeen = Utils::getCurrentTimestamp();
        peer.bytesReceived = 0;
        peer.bytesSent = 0;
        peer.version = 1;
        peer.userAgent = "GXC/1.0";
        peer.height = 0;
        
        // Simulate connection (in real implementation would use actual sockets)
        if (simulateConnection(peer)) {
            peer.connected = true;
            peer.connectTime = Utils::getCurrentTimestamp();
            
            {
                std::lock_guard<std::mutex> lock(peersMutex);
                peers[peerId] = peer;
            }
            
            // Send version message
            sendVersionMessage(peerId);
            
            LOG_NETWORK(LogLevel::INFO, "Successfully connected to peer: " + peerId);
            return true;
        }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to connect to peer " + peerId + ": " + std::string(e.what()));
    }
    
    return false;
}

void Network::disconnectPeer(const std::string& peerId) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto it = peers.find(peerId);
    if (it != peers.end()) {
        it->second.connected = false;
        peers.erase(it);
        
        LOG_NETWORK(LogLevel::INFO, "Disconnected from peer: " + peerId);
    }
}

void Network::broadcastMessage(const NetworkMessage& message) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    for (auto& peer : peers) {
        if (peer.second.connected) {
            sendMessageToPeer(peer.first, message);
        }
    }
    
    LOG_NETWORK(LogLevel::DEBUG, "Broadcasted message type: " + std::to_string(static_cast<int>(message.type)));
}

void Network::sendMessageToPeer(const std::string& peerId, const NetworkMessage& message) {
    auto it = peers.find(peerId);
    if (it == peers.end() || !it->second.connected) {
        LOG_NETWORK(LogLevel::WARNING, "Cannot send message to disconnected peer: " + peerId);
        return;
    }
    
    try {
        // Serialize and send message (simplified)
        std::string serializedMessage = serializeMessage(message);
        
        // Update peer statistics
        it->second.bytesSent += serializedMessage.length();
        it->second.lastSeen = Utils::getCurrentTimestamp();
        
        LOG_NETWORK(LogLevel::DEBUG, "Sent message to peer " + peerId + " (type: " + 
                   std::to_string(static_cast<int>(message.type)) + ")");
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to send message to peer " + peerId + ": " + std::string(e.what()));
        disconnectPeer(peerId);
    }
}

std::vector<Peer> Network::getConnectedPeers() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    std::vector<Peer> connectedPeers;
    for (const auto& peer : peers) {
        if (peer.second.connected) {
            connectedPeers.push_back(peer.second);
        }
    }
    
    return connectedPeers;
}

uint32_t Network::getPeerCount() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    uint32_t count = 0;
    for (const auto& peer : peers) {
        if (peer.second.connected) {
            count++;
        }
    }
    
    return count;
}

NetworkStats Network::getNetworkStats() const {
    NetworkStats stats;
    
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        
        stats.connectedPeers = 0;
        stats.totalBytesReceived = 0;
        stats.totalBytesSent = 0;
        
        for (const auto& peer : peers) {
            if (peer.second.connected) {
                stats.connectedPeers++;
                stats.totalBytesReceived += peer.second.bytesReceived;
                stats.totalBytesSent += peer.second.bytesSent;
            }
        }
    }
    
    stats.messagesReceived = messageStats.messagesReceived;
    stats.messagesSent = messageStats.messagesSent;
    stats.uptime = Utils::getCurrentTimestamp() - startTime;
    
    return stats;
}

void Network::addSeedNode(const std::string& address, uint16_t port) {
    std::string seedNode = address + ":" + std::to_string(port);
    
    {
        std::lock_guard<std::mutex> lock(seedNodesMutex);
        seedNodes.push_back({address, port});
    }
    
    LOG_NETWORK(LogLevel::INFO, "Added seed node: " + seedNode);
}

void Network::processIncomingConnections() {
    // In real implementation, would handle incoming socket connections
    // For now, this is a placeholder
}

void Network::maintainConnections() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    // Check for dead connections
    for (auto it = peers.begin(); it != peers.end();) {
        if (it->second.connected && (currentTime - it->second.lastSeen) > PEER_TIMEOUT) {
            LOG_NETWORK(LogLevel::INFO, "Peer timeout: " + it->first);
            it = peers.erase(it);
        } else {
            ++it;
        }
    }
}

void Network::sendPeriodicMessages() {
    auto currentTime = Utils::getCurrentTimestamp();
    
    // Send ping messages every 30 seconds
    if (currentTime - lastPingTime > 30) {
        sendPingMessages();
        lastPingTime = currentTime;
    }
    
    // Send inventory updates every 60 seconds
    if (currentTime - lastInventoryTime > 60) {
        sendInventoryUpdates();
        lastInventoryTime = currentTime;
    }
}

void Network::discoverPeers() {
    std::lock_guard<std::mutex> lock(seedNodesMutex);
    
    // Try to connect to seed nodes if we have too few peers
    if (getPeerCount() < MAX_PEERS / 2) {
        for (const auto& seedNode : seedNodes) {
            if (getPeerCount() >= MAX_PEERS) {
                break;
            }
            
            connectToPeer(seedNode.address, seedNode.port);
        }
    }
}

void Network::processMessageQueue() {
    std::lock_guard<std::mutex> lock(messageQueueMutex);
    
    while (!messageQueue.empty()) {
        auto messageData = messageQueue.front();
        messageQueue.pop();
        
        processReceivedMessage(messageData.first, messageData.second);
    }
}

void Network::processReceivedMessage(const std::string& peerId, const NetworkMessage& message) {
    messageStats.messagesReceived++;
    
    switch (message.type) {
        case MessageType::VERSION:
            handleVersionMessage(peerId, message);
            break;
            
        case MessageType::PING:
            handlePingMessage(peerId, message);
            break;
            
        case MessageType::PONG:
            handlePongMessage(peerId, message);
            break;
            
        case MessageType::BLOCK:
            handleBlockMessage(peerId, message);
            break;
            
        case MessageType::TRANSACTION:
            handleTransactionMessage(peerId, message);
            break;
            
        case MessageType::INVENTORY:
            handleInventoryMessage(peerId, message);
            break;
            
        case MessageType::GET_BLOCKS:
            handleGetBlocksMessage(peerId, message);
            break;
            
        case MessageType::GET_DATA:
            handleGetDataMessage(peerId, message);
            break;
            
        default:
            LOG_NETWORK(LogLevel::WARNING, "Unknown message type received from " + peerId);
            break;
    }
}

void Network::handleVersionMessage(const std::string& peerId, const NetworkMessage& message) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto it = peers.find(peerId);
    if (it != peers.end()) {
        // Update peer version info (simplified)
        it->second.version = 1;
        it->second.userAgent = "GXC/1.0";
        it->second.height = blockchain->getChainLength();
        
        // Send version acknowledgment
        NetworkMessage response;
        response.type = MessageType::VERSION_ACK;
        response.data = "version_ack";
        
        sendMessageToPeer(peerId, response);
        
        LOG_NETWORK(LogLevel::DEBUG, "Handled version message from " + peerId);
    }
}

void Network::handlePingMessage(const std::string& peerId, const NetworkMessage& message) {
    // Respond with pong
    NetworkMessage pong;
    pong.type = MessageType::PONG;
    pong.data = message.data; // Echo the nonce
    
    sendMessageToPeer(peerId, pong);
    
    LOG_NETWORK(LogLevel::DEBUG, "Responded to ping from " + peerId);
}

void Network::handlePongMessage(const std::string& peerId, const NetworkMessage& message) {
    // Update peer's last seen time
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto it = peers.find(peerId);
    if (it != peers.end()) {
        it->second.lastSeen = Utils::getCurrentTimestamp();
    }
    
    LOG_NETWORK(LogLevel::DEBUG, "Received pong from " + peerId);
}

void Network::handleBlockMessage(const std::string& peerId, const NetworkMessage& message) {
    try {
        // In real implementation, would deserialize block from message.data
        LOG_NETWORK(LogLevel::INFO, "Received block from " + peerId);
        
        // Validate and add block to blockchain
        // Block block = deserializeBlock(message.data);
        // if (blockchain->addBlock(block)) {
        //     LOG_NETWORK(LogLevel::INFO, "Added new block from network");
        //     broadcastBlock(block);
        // }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing block from " + peerId + ": " + std::string(e.what()));
    }
}

void Network::handleTransactionMessage(const std::string& peerId, const NetworkMessage& message) {
    try {
        // In real implementation, would deserialize transaction from message.data
        LOG_NETWORK(LogLevel::INFO, "Received transaction from " + peerId);
        
        // Validate and add transaction to mempool
        // Transaction tx = deserializeTransaction(message.data);
        // if (blockchain->addTransaction(tx)) {
        //     LOG_NETWORK(LogLevel::INFO, "Added new transaction from network");
        //     broadcastTransaction(tx);
        // }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing transaction from " + peerId + ": " + std::string(e.what()));
    }
}

void Network::handleInventoryMessage(const std::string& peerId, const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received inventory from " + peerId);
    
    // Process inventory and request missing items
    // std::vector<std::string> missingItems = processInventory(message.data);
    // if (!missingItems.empty()) {
    //     requestData(peerId, missingItems);
    // }
}

void Network::handleGetBlocksMessage(const std::string& peerId, const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received get_blocks request from " + peerId);
    
    // Send block inventory
    // std::vector<std::string> blockHashes = getBlockHashes();
    // sendInventory(peerId, blockHashes);
}

void Network::handleGetDataMessage(const std::string& peerId, const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received get_data request from " + peerId);
    
    // Send requested data
    // std::vector<std::string> requestedItems = parseDataRequest(message.data);
    // sendRequestedData(peerId, requestedItems);
}

void Network::sendVersionMessage(const std::string& peerId) {
    NetworkMessage message;
    message.type = MessageType::VERSION;
    message.data = "version:1;user_agent:GXC/1.0;height:" + std::to_string(blockchain->getChainLength());
    
    sendMessageToPeer(peerId, message);
}

void Network::sendPingMessages() {
    NetworkMessage ping;
    ping.type = MessageType::PING;
    ping.data = std::to_string(Utils::randomUint32()); // Nonce
    
    broadcastMessage(ping);
}

void Network::sendInventoryUpdates() {
    NetworkMessage inventory;
    inventory.type = MessageType::INVENTORY;
    inventory.data = "blocks:" + std::to_string(blockchain->getChainLength());
    
    broadcastMessage(inventory);
}

std::string Network::serializeMessage(const NetworkMessage& message) {
    // Simplified serialization
    return std::to_string(static_cast<int>(message.type)) + ":" + message.data;
}

NetworkMessage Network::deserializeMessage(const std::string& data) {
    NetworkMessage message;
    
    size_t colonPos = data.find(':');
    if (colonPos != std::string::npos) {
        int typeInt = std::stoi(data.substr(0, colonPos));
        message.type = static_cast<MessageType>(typeInt);
        message.data = data.substr(colonPos + 1);
    }
    
    return message;
}

bool Network::simulateConnection(const Peer& peer) {
    // Simplified connection simulation
    // In real implementation, would establish actual socket connection
    return Utils::validateIPAddress(peer.address) && peer.port > 0;
}