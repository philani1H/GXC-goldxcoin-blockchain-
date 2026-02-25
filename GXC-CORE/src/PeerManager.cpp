#include "../include/PeerManager.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>

PeerManager::PeerManager(uint16_t port) 
    : serverPort(port), isRunning(false), maxPeers(50), 
      lastPeerCleanup(Utils::getCurrentTimestamp()),
      startTime(Utils::getCurrentTimestamp()) {
    LOG_NETWORK(LogLevel::INFO, "PeerManager initialized on port " + std::to_string(port));
}

PeerManager::~PeerManager() {
    stop();
}

bool PeerManager::start() {
    if (isRunning) {
        LOG_NETWORK(LogLevel::WARNING, "PeerManager already running");
        return true;
    }
    
    LOG_NETWORK(LogLevel::INFO, "Starting PeerManager");
    
    try {
        isRunning = true;
        
        // Start peer management thread
        managementThread = std::thread(&PeerManager::managementLoop, this);
        
        // Start discovery thread
        discoveryThread = std::thread(&PeerManager::discoveryLoop, this);
        
        LOG_NETWORK(LogLevel::INFO, "PeerManager started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to start PeerManager: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void PeerManager::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_NETWORK(LogLevel::INFO, "Stopping PeerManager");
    
    isRunning = false;
    
    // Disconnect all peers
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto& peer : peers) {
            peer.second.connected = false;
        }
        peers.clear();
    }
    
    // Stop threads
    if (managementThread.joinable()) {
        managementThread.join();
    }
    
    if (discoveryThread.joinable()) {
        discoveryThread.join();
    }
    
    LOG_NETWORK(LogLevel::INFO, "PeerManager stopped");
}

bool PeerManager::connectToPeer(const std::string& address, uint16_t port) {
    std::string peerKey = address + ":" + std::to_string(port);
    
    std::lock_guard<std::mutex> lock(peersMutex);
    
    // Check if already connected
    if (peers.find(peerKey) != peers.end()) {
        LOG_NETWORK(LogLevel::WARNING, "Already connected to peer: " + peerKey);
        return false;
    }
    
    // Check peer limit
    if (getPeerCount() >= maxPeers) {
        LOG_NETWORK(LogLevel::WARNING, "Maximum peer limit reached");
        return false;
    }
    
    try {
        PeerInfo peer;
        peer.address = address;
        peer.port = port;
        peer.connected = true;
        peer.connectTime = Utils::getCurrentTimestamp();
        peer.lastSeen = Utils::getCurrentTimestamp();
        peer.version = 1;
        peer.height = 0;
        peer.bytesSent = 0;
        peer.bytesReceived = 0;
        peer.messagesSent = 0;
        peer.messagesReceived = 0;
        peer.uptime = 0;
        
        peers[peerKey] = peer;
        
        LOG_NETWORK(LogLevel::INFO, "Connected to peer: " + peerKey);
        return true;
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to connect to peer " + peerKey + ": " + std::string(e.what()));
        return false;
    }
}

void PeerManager::disconnectPeer(const std::string& address, uint16_t port) {
    std::string peerKey = address + ":" + std::to_string(port);
    
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto it = peers.find(peerKey);
    if (it != peers.end()) {
        it->second.connected = false;
        peers.erase(it);
        LOG_NETWORK(LogLevel::INFO, "Disconnected from peer: " + peerKey);
    }
}

std::vector<PeerInfo> PeerManager::getConnectedPeers() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    std::vector<PeerInfo> connectedPeers;
    for (const auto& peer : peers) {
        if (peer.second.connected) {
            connectedPeers.push_back(peer.second);
        }
    }
    
    return connectedPeers;
}

uint32_t PeerManager::getConnectedPeerCount() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    uint32_t count = 0;
    for (const auto& peer : peers) {
        if (peer.second.connected) {
            count++;
        }
    }
    
    return count;
}

void PeerManager::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    for (auto& peer : peers) {
        if (peer.second.connected) {
            sendMessageToPeer(peer.second, message);
        }
    }
}

void PeerManager::sendMessageToPeer(const PeerInfo& peer, const std::string& message) {
    try {
        // In real implementation, would send actual network message
        LOG_NETWORK(LogLevel::DEBUG, "Sent message to " + peer.address + ":" + std::to_string(peer.port));
        
        // Update statistics
        auto& mutablePeer = const_cast<PeerInfo&>(peer);
        mutablePeer.bytesSent += message.length();
        mutablePeer.messagesSent++;
        mutablePeer.lastSeen = Utils::getCurrentTimestamp();
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to send message to peer: " + std::string(e.what()));
    }
}

void PeerManager::addKnownPeer(const std::string& address, uint16_t port) {
    std::string peerKey = address + ":" + std::to_string(port);
    
    std::lock_guard<std::mutex> lock(knownPeersMutex);
    
    if (knownPeers.find(peerKey) == knownPeers.end()) {
        PeerInfo info;
        info.address = address;
        info.port = port;
        info.lastSeen = Utils::getCurrentTimestamp();
        // info.reliable = false;  // Removed - not in PeerInfo
        
        knownPeers[peerKey] = info;
        
        LOG_NETWORK(LogLevel::DEBUG, "Added known peer: " + peerKey);
    }
}

std::vector<PeerInfo> PeerManager::getKnownPeers() const {
    std::lock_guard<std::mutex> lock(knownPeersMutex);
    
    std::vector<PeerInfo> peerList;
    for (const auto& peer : knownPeers) {
        peerList.push_back(peer.second);
    }
    
    return peerList;
}

void PeerManager::managementLoop() {
    LOG_NETWORK(LogLevel::INFO, "Peer management loop started");
    
    while (isRunning) {
        try {
            // Clean up disconnected peers
            cleanupDisconnectedPeers();
            
            // Check peer health
            checkPeerHealth();
            
            // Maintain optimal peer count
            maintainPeerConnections();
            
            // Update peer statistics
            updatePeerStatistics();
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in peer management loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Peer management loop stopped");
}

void PeerManager::discoveryLoop() {
    LOG_NETWORK(LogLevel::INFO, "Peer discovery loop started");
    
    while (isRunning) {
        try {
            // Discover new peers
            discoverNewPeers();
            
            // Exchange peer lists with connected peers
            exchangePeerLists();
            
            std::this_thread::sleep_for(std::chrono::seconds(60));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in peer discovery loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Peer discovery loop stopped");
}

void PeerManager::cleanupDisconnectedPeers() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (auto it = peers.begin(); it != peers.end();) {
        // Remove peers that haven't been seen for 5 minutes
        if (currentTime - it->second.lastSeen > 300) {
            LOG_NETWORK(LogLevel::INFO, "Removing inactive peer: " + it->first);
            it = peers.erase(it);
        } else {
            ++it;
        }
    }
}

void PeerManager::checkPeerHealth() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (auto& peer : peers) {
        if (peer.second.connected) {
            // Send ping if no activity for 2 minutes
            if (currentTime - peer.second.lastSeen > 120) {
                // In real implementation, would send actual ping
                LOG_NETWORK(LogLevel::DEBUG, "Pinging peer: " + peer.first);
                peer.second.lastSeen = currentTime;
            }
        }
    }
}

void PeerManager::maintainPeerConnections() {
    uint32_t connectedCount = getConnectedPeerCount();
    
    // Try to maintain at least 8 peers
    if (connectedCount < 8) {
        connectToRandomKnownPeers(8 - connectedCount);
    }
    
    // Disconnect excess peers if we have too many
    if (connectedCount > maxPeers) {
        disconnectExcessPeers(connectedCount - maxPeers);
    }
}

void PeerManager::connectToRandomKnownPeers(uint32_t count) {
    std::lock_guard<std::mutex> lock(knownPeersMutex);
    
    if (knownPeers.empty()) {
        // Add some default peers
        addDefaultPeers();
        return;
    }
    
    std::vector<PeerInfo> availablePeers;
    for (const auto& peer : knownPeers) {
        std::string peerKey = peer.second.address + ":" + std::to_string(peer.second.port);
        
        // Check if not already connected
        std::lock_guard<std::mutex> peerLock(peersMutex);
        if (peers.find(peerKey) == peers.end()) {
            availablePeers.push_back(peer.second);
        }
    }
    
    if (availablePeers.empty()) {
        return;
    }
    
    // Shuffle and select random peers
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(availablePeers.begin(), availablePeers.end(), g);
    
    uint32_t connectCount = std::min(count, static_cast<uint32_t>(availablePeers.size()));
    
    for (uint32_t i = 0; i < connectCount; i++) {
        connectToPeer(availablePeers[i].address, availablePeers[i].port);
    }
}

void PeerManager::disconnectExcessPeers(uint32_t count) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    std::vector<std::string> peerKeys;
    for (const auto& peer : peers) {
        if (peer.second.connected) {
            peerKeys.push_back(peer.first);
        }
    }
    
    // Disconnect oldest connections first
    std::sort(peerKeys.begin(), peerKeys.end(), [this](const std::string& a, const std::string& b) {
        return peers[a].connectTime < peers[b].connectTime;
    });
    
    uint32_t disconnectCount = std::min(count, static_cast<uint32_t>(peerKeys.size()));
    
    for (uint32_t i = 0; i < disconnectCount; i++) {
        auto& peer = peers[peerKeys[i]];
        peer.connected = false;
        peers.erase(peerKeys[i]);
        
        LOG_NETWORK(LogLevel::INFO, "Disconnected excess peer: " + peerKeys[i]);
    }
}

void PeerManager::addDefaultPeers() {
    // Add some default seed nodes
    std::vector<std::pair<std::string, uint16_t>> seedNodes = {
        {"seed1.gxc.network", 8332},
        {"seed2.gxc.network", 8332},
        {"seed3.gxc.network", 8332},
        {"127.0.0.1", 8333},
        {"127.0.0.1", 8334}
    };
    
    for (const auto& seed : seedNodes) {
        addKnownPeer(seed.first, seed.second);
    }
    
    LOG_NETWORK(LogLevel::INFO, "Added " + std::to_string(seedNodes.size()) + " default peers");
}

void PeerManager::discoverNewPeers() {
    // In real implementation, would use DNS seeds, peer exchange, etc.
    LOG_NETWORK(LogLevel::DEBUG, "Discovering new peers");
    
    // Simulate peer discovery
    static uint32_t discoveryCounter = 0;
    if (discoveryCounter % 10 == 0) {
        std::string newPeerAddr = "peer" + std::to_string(discoveryCounter) + ".gxc.network";
        addKnownPeer(newPeerAddr, 8332);
    }
    discoveryCounter++;
}

void PeerManager::exchangePeerLists() {
    auto connectedPeers = getConnectedPeers();
    
    if (connectedPeers.empty()) {
        return;
    }
    
    // Create peer list message
    std::string peerListMsg = createPeerListMessage();
    
    // Send to random connected peer
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<size_t> dis(0, connectedPeers.size() - 1);
    
    auto& randomPeer = connectedPeers[dis(g)];
    sendMessageToPeer(randomPeer, peerListMsg);
    
    LOG_NETWORK(LogLevel::DEBUG, "Exchanged peer list with " + randomPeer.address);
}

std::string PeerManager::createPeerListMessage() {
    auto knownPeersList = getKnownPeers();
    
    std::ostringstream oss;
    oss << "PEER_LIST:";
    
    size_t maxPeers = std::min(knownPeersList.size(), static_cast<size_t>(20));
    
    for (size_t i = 0; i < maxPeers; i++) {
        oss << knownPeersList[i].address << ":" << knownPeersList[i].port;
        if (i < maxPeers - 1) {
            oss << ",";
        }
    }
    
    return oss.str();
}

void PeerManager::updatePeerStatistics() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    for (auto& peer : peers) {
        if (peer.second.connected) {
            auto currentTime = Utils::getCurrentTimestamp();
            peer.second.uptime = currentTime - peer.second.connectTime;
        }
    }
}

// NetworkStats method removed - functionality moved to getPeerCount()