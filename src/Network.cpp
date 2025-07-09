#include "../include/Network.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <iostream>
#include <thread>
#include <chrono>

Network::Network(uint16_t port, uint32_t maxConnections)
    : listenPort(port), maxConnections(maxConnections), isRunning(false) {
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
    
    LOG_NETWORK(LogLevel::INFO, "Starting network service");
    
    try {
        // Initialize network subsystem
        isRunning = true;
        
        // Start network threads
        networkThread = std::thread(&Network::networkLoop, this);
        
        LOG_NETWORK(LogLevel::INFO, "Network service started successfully");
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
    
    LOG_NETWORK(LogLevel::INFO, "Stopping network service");
    
    isRunning = false;
    
    // Close all connections
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto& peer : peers) {
            peer.second.disconnect();
        }
        peers.clear();
    }
    
    // Join network thread
    if (networkThread.joinable()) {
        networkThread.join();
    }
    
    LOG_NETWORK(LogLevel::INFO, "Network service stopped");
}

bool Network::connectToPeer(const std::string& address, uint16_t port) {
    if (!isRunning) {
        LOG_NETWORK(LogLevel::ERROR, "Cannot connect to peer: network not running");
        return false;
    }
    
    std::string peerAddr = address + ":" + std::to_string(port);
    
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        if (peers.find(peerAddr) != peers.end()) {
            LOG_NETWORK(LogLevel::WARNING, "Already connected to peer: " + peerAddr);
            return true;
        }
        
        if (peers.size() >= maxConnections) {
            LOG_NETWORK(LogLevel::WARNING, "Maximum connections reached");
            return false;
        }
    }
    
    try {
        NetworkPeer peer;
        peer.address = address;
        peer.port = port;
        peer.isConnected = true;
        peer.lastSeen = Utils::getCurrentTimestamp();
        peer.version = PROTOCOL_VERSION;
        
        {
            std::lock_guard<std::mutex> lock(peersMutex);
            peers[peerAddr] = peer;
        }
        
        LOG_NETWORK(LogLevel::INFO, "Connected to peer: " + peerAddr);
        return true;
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to connect to peer " + peerAddr + ": " + std::string(e.what()));
        return false;
    }
}

void Network::disconnectPeer(const std::string& address, uint16_t port) {
    std::string peerAddr = address + ":" + std::to_string(port);
    
    std::lock_guard<std::mutex> lock(peersMutex);
    auto it = peers.find(peerAddr);
    if (it != peers.end()) {
        it->second.disconnect();
        peers.erase(it);
        LOG_NETWORK(LogLevel::INFO, "Disconnected from peer: " + peerAddr);
    }
}

bool Network::broadcastMessage(const NetworkMessage& message) {
    if (!isRunning) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(peersMutex);
    
    for (auto& peerPair : peers) {
        auto& peer = peerPair.second;
        if (peer.isConnected) {
            sendMessageToPeer(peer, message);
        }
    }
    
    return true;
}

bool Network::sendMessageToPeer(const NetworkPeer& peer, const NetworkMessage& message) {
    try {
        // Simplified message sending - in real implementation would use actual sockets
        LOG_NETWORK(LogLevel::DEBUG, "Sending message type " + std::to_string(static_cast<int>(message.type)) + 
                   " to peer " + peer.address + ":" + std::to_string(peer.port));
        return true;
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to send message to peer: " + std::string(e.what()));
        return false;
    }
}

std::vector<NetworkPeer> Network::getConnectedPeers() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    std::vector<NetworkPeer> connectedPeers;
    for (const auto& peerPair : peers) {
        if (peerPair.second.isConnected) {
            connectedPeers.push_back(peerPair.second);
        }
    }
    
    return connectedPeers;
}

uint32_t Network::getConnectionCount() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    uint32_t count = 0;
    for (const auto& peerPair : peers) {
        if (peerPair.second.isConnected) {
            count++;
        }
    }
    
    return count;
}

void Network::addBootstrapNodes(const std::vector<std::string>& nodes) {
    for (const auto& node : nodes) {
        auto parts = Utils::split(node, ':');
        if (parts.size() == 2) {
            std::string address = parts[0];
            uint16_t port = static_cast<uint16_t>(std::stoi(parts[1]));
            bootstrapNodes.push_back({address, port});
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Added " + std::to_string(nodes.size()) + " bootstrap nodes");
}

void Network::networkLoop() {
    LOG_NETWORK(LogLevel::INFO, "Network loop started");
    
    while (isRunning) {
        try {
            // Process incoming connections
            processIncomingConnections();
            
            // Maintain existing connections
            maintainConnections();
            
            // Connect to bootstrap nodes if needed
            connectToBootstrapNodes();
            
            // Process message queue
            processMessageQueue();
            
            // Sleep for a short time
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in network loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Network loop stopped");
}

void Network::processIncomingConnections() {
    // Simplified implementation - would handle actual socket connections
}

void Network::maintainConnections() {
    uint64_t now = Utils::getCurrentTimestamp();
    std::vector<std::string> peersToRemove;
    
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        
        for (auto& peerPair : peers) {
            auto& peer = peerPair.second;
            
            // Check if peer has timed out
            if (now - peer.lastSeen > CONNECTION_TIMEOUT) {
                peer.isConnected = false;
                peersToRemove.push_back(peerPair.first);
                LOG_NETWORK(LogLevel::INFO, "Peer timed out: " + peerPair.first);
            }
            
            // Send ping messages
            if (peer.isConnected && now - peer.lastPing > PING_INTERVAL) {
                NetworkMessage ping;
                ping.type = MessageType::PING;
                ping.timestamp = now;
                
                sendMessageToPeer(peer, ping);
                peer.lastPing = now;
            }
        }
    }
    
    // Remove timed out peers
    for (const auto& peerAddr : peersToRemove) {
        auto parts = Utils::split(peerAddr, ':');
        if (parts.size() == 2) {
            disconnectPeer(parts[0], static_cast<uint16_t>(std::stoi(parts[1])));
        }
    }
}

void Network::connectToBootstrapNodes() {
    if (getConnectionCount() >= maxConnections / 2) {
        return; // Already have enough connections
    }
    
    for (const auto& node : bootstrapNodes) {
        if (getConnectionCount() >= maxConnections) {
            break;
        }
        
        connectToPeer(node.address, node.port);
    }
}

void Network::processMessageQueue() {
    // Process any queued messages
    std::lock_guard<std::mutex> lock(messageQueueMutex);
    
    while (!messageQueue.empty()) {
        auto message = messageQueue.front();
        messageQueue.pop();
        
        // Handle the message based on its type
        handleMessage(message);
    }
}

void Network::handleMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Handling message type: " + std::to_string(static_cast<int>(message.type)));
    
    switch (message.type) {
        case MessageType::PING:
            handlePingMessage(message);
            break;
            
        case MessageType::PONG:
            handlePongMessage(message);
            break;
            
        case MessageType::VERSION:
            handleVersionMessage(message);
            break;
            
        case MessageType::ADDR:
            handleAddrMessage(message);
            break;
            
        case MessageType::GETADDR:
            handleGetAddrMessage(message);
            break;
            
        case MessageType::BLOCK:
            handleBlockMessage(message);
            break;
            
        case MessageType::TX:
            handleTransactionMessage(message);
            break;
            
        case MessageType::GETBLOCKS:
            handleGetBlocksMessage(message);
            break;
            
        case MessageType::GETDATA:
            handleGetDataMessage(message);
            break;
            
        case MessageType::INV:
            handleInventoryMessage(message);
            break;
            
        default:
            LOG_NETWORK(LogLevel::WARNING, "Unknown message type: " + std::to_string(static_cast<int>(message.type)));
            break;
    }
}

void Network::handlePingMessage(const NetworkMessage& message) {
    // Respond with pong
    NetworkMessage pong;
    pong.type = MessageType::PONG;
    pong.timestamp = Utils::getCurrentTimestamp();
    
    // Send to originating peer (simplified)
    LOG_NETWORK(LogLevel::DEBUG, "Responding to ping with pong");
}

void Network::handlePongMessage(const NetworkMessage& message) {
    // Update peer last seen time
    LOG_NETWORK(LogLevel::DEBUG, "Received pong message");
}

void Network::handleVersionMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received version message");
    // Handle peer version negotiation
}

void Network::handleAddrMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received address message");
    // Handle peer address list
}

void Network::handleGetAddrMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received getaddr message");
    // Send our known peer addresses
}

void Network::handleBlockMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::INFO, "Received block message");
    // Handle new block
}

void Network::handleTransactionMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received transaction message");
    // Handle new transaction
}

void Network::handleGetBlocksMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received getblocks message");
    // Send block inventory
}

void Network::handleGetDataMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received getdata message");
    // Send requested data
}

void Network::handleInventoryMessage(const NetworkMessage& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received inventory message");
    // Handle inventory advertisement
}

void Network::queueMessage(const NetworkMessage& message) {
    std::lock_guard<std::mutex> lock(messageQueueMutex);
    messageQueue.push(message);
}

NetworkStats Network::getNetworkStats() const {
    NetworkStats stats;
    
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        stats.connectionCount = getConnectionCount();
        stats.totalPeers = peers.size();
    }
    
    stats.bytesReceived = 0; // Would track actual bytes in real implementation
    stats.bytesSent = 0;
    stats.messagesReceived = 0;
    stats.messagesSent = 0;
    stats.uptime = Utils::getCurrentTimestamp() - startTime;
    
    return stats;
}

bool Network::isConnectedToPeer(const std::string& address, uint16_t port) const {
    std::string peerAddr = address + ":" + std::to_string(port);
    
    std::lock_guard<std::mutex> lock(peersMutex);
    auto it = peers.find(peerAddr);
    
    return it != peers.end() && it->second.isConnected;
}

void Network::banPeer(const std::string& address, uint32_t duration) {
    LOG_NETWORK(LogLevel::WARNING, "Banning peer " + address + " for " + std::to_string(duration) + " seconds");
    
    uint64_t banUntil = Utils::getCurrentTimestamp() + duration;
    bannedPeers[address] = banUntil;
    
    // Disconnect if currently connected
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto it = peers.begin(); it != peers.end(); ) {
            if (it->second.address == address) {
                it->second.disconnect();
                it = peers.erase(it);
            } else {
                ++it;
            }
        }
    }
}

bool Network::isPeerBanned(const std::string& address) const {
    auto it = bannedPeers.find(address);
    if (it == bannedPeers.end()) {
        return false;
    }
    
    uint64_t now = Utils::getCurrentTimestamp();
    if (now >= it->second) {
        // Ban has expired
        bannedPeers.erase(it);
        return false;
    }
    
    return true;
}

// NetworkPeer implementation
void NetworkPeer::disconnect() {
    isConnected = false;
    LOG_NETWORK(LogLevel::DEBUG, "Peer disconnected: " + address + ":" + std::to_string(port));
}