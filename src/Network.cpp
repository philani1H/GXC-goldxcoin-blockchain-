#include "../include/Network.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <thread>
#include <chrono>
#include <algorithm>

Network::Network() : isRunning(false), serverSocket(-1), port(0) {
    LOG_NETWORK(LogLevel::INFO, "Network instance created");
}

Network::~Network() {
    stop();
}

bool Network::start(int networkPort) {
    if (isRunning) {
        LOG_NETWORK(LogLevel::WARNING, "Network already running");
        return true;
    }
    
    port = networkPort;
    
    try {
        LOG_NETWORK(LogLevel::INFO, "Starting network on port " + std::to_string(port));
        
        // Initialize socket
        if (!initializeSocket()) {
            LOG_NETWORK(LogLevel::ERROR, "Failed to initialize network socket");
            return false;
        }
        
        isRunning = true;
        
        // Start network threads
        serverThread = std::thread(&Network::serverLoop, this);
        peerManagerThread = std::thread(&Network::peerManagerLoop, this);
        messageProcessorThread = std::thread(&Network::messageProcessorLoop, this);
        
        LOG_NETWORK(LogLevel::INFO, "Network started successfully on port " + std::to_string(port));
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
    
    LOG_NETWORK(LogLevel::INFO, "Stopping network");
    
    isRunning = false;
    
    // Close server socket
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    
    // Disconnect all peers
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto& peer : peers) {
            disconnectPeer(peer.second);
        }
        peers.clear();
    }
    
    // Join threads
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    if (peerManagerThread.joinable()) {
        peerManagerThread.join();
    }
    
    if (messageProcessorThread.joinable()) {
        messageProcessorThread.join();
    }
    
    LOG_NETWORK(LogLevel::INFO, "Network stopped");
}

bool Network::initializeSocket() {
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to create server socket");
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_NETWORK(LogLevel::WARNING, "Failed to set SO_REUSEADDR");
    }
    
    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to bind socket to port " + std::to_string(port));
        close(serverSocket);
        serverSocket = -1;
        return false;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to listen on socket");
        close(serverSocket);
        serverSocket = -1;
        return false;
    }
    
    return true;
}

void Network::serverLoop() {
    LOG_NETWORK(LogLevel::DEBUG, "Server loop started");
    
    while (isRunning) {
        try {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            // Accept connection with timeout
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(serverSocket, &readfds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int activity = select(serverSocket + 1, &readfds, NULL, NULL, &timeout);
            
            if (activity > 0 && FD_ISSET(serverSocket, &readfds)) {
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
                
                if (clientSocket >= 0) {
                    std::string peerIP = inet_ntoa(clientAddr.sin_addr);
                    int peerPort = ntohs(clientAddr.sin_port);
                    
                    LOG_NETWORK(LogLevel::INFO, "Incoming connection from " + peerIP + ":" + std::to_string(peerPort));
                    
                    // Add peer
                    addPeer(peerIP, peerPort, clientSocket);
                }
            }
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_NETWORK(LogLevel::DEBUG, "Server loop stopped");
}

void Network::peerManagerLoop() {
    LOG_NETWORK(LogLevel::DEBUG, "Peer manager loop started");
    
    auto lastHealthCheck = Utils::getCurrentTimestamp();
    
    while (isRunning) {
        try {
            auto currentTime = Utils::getCurrentTimestamp();
            
            // Perform health checks every 30 seconds
            if (currentTime - lastHealthCheck >= 30) {
                performPeerHealthChecks();
                lastHealthCheck = currentTime;
            }
            
            // Maintain desired peer count
            maintainPeerConnections();
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in peer manager loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_NETWORK(LogLevel::DEBUG, "Peer manager loop stopped");
}

void Network::messageProcessorLoop() {
    LOG_NETWORK(LogLevel::DEBUG, "Message processor loop started");
    
    while (isRunning) {
        try {
            processIncomingMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in message processor loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_NETWORK(LogLevel::DEBUG, "Message processor loop stopped");
}

bool Network::connectToPeer(const std::string& host, int peerPort) {
    try {
        LOG_NETWORK(LogLevel::DEBUG, "Connecting to peer " + host + ":" + std::to_string(peerPort));
        
        // Create socket
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            LOG_NETWORK(LogLevel::ERROR, "Failed to create client socket");
            return false;
        }
        
        // Set socket timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        // Connect to peer
        struct sockaddr_in peerAddr;
        peerAddr.sin_family = AF_INET;
        peerAddr.sin_port = htons(peerPort);
        
        if (inet_pton(AF_INET, host.c_str(), &peerAddr.sin_addr) <= 0) {
            LOG_NETWORK(LogLevel::ERROR, "Invalid peer address: " + host);
            close(clientSocket);
            return false;
        }
        
        if (connect(clientSocket, (struct sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
            LOG_NETWORK(LogLevel::ERROR, "Failed to connect to peer " + host + ":" + std::to_string(peerPort));
            close(clientSocket);
            return false;
        }
        
        // Add peer
        if (addPeer(host, peerPort, clientSocket)) {
            LOG_NETWORK(LogLevel::INFO, "Successfully connected to peer " + host + ":" + std::to_string(peerPort));
            
            // Send handshake
            sendHandshake(host + ":" + std::to_string(peerPort));
            
            return true;
        } else {
            close(clientSocket);
            return false;
        }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Exception connecting to peer: " + std::string(e.what()));
        return false;
    }
}

bool Network::addPeer(const std::string& host, int peerPort, int socket) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    std::string peerKey = host + ":" + std::to_string(peerPort);
    
    // Check if peer already exists
    if (peers.find(peerKey) != peers.end()) {
        LOG_NETWORK(LogLevel::WARNING, "Peer already exists: " + peerKey);
        return false;
    }
    
    // Check peer limit
    if (peers.size() >= MAX_PEERS) {
        LOG_NETWORK(LogLevel::WARNING, "Maximum peer count reached");
        return false;
    }
    
    // Create peer
    NetworkPeer peer;
    peer.host = host;
    peer.port = peerPort;
    peer.socket = socket;
    peer.connected = true;
    peer.lastSeen = Utils::getCurrentTimestamp();
    peer.bytesReceived = 0;
    peer.bytesSent = 0;
    peer.messagesReceived = 0;
    peer.messagesSent = 0;
    
    peers[peerKey] = peer;
    
    LOG_NETWORK(LogLevel::INFO, "Added peer: " + peerKey + " (total: " + std::to_string(peers.size()) + ")");
    
    return true;
}

void Network::disconnectPeer(NetworkPeer& peer) {
    if (peer.socket != -1) {
        close(peer.socket);
        peer.socket = -1;
    }
    peer.connected = false;
    
    LOG_NETWORK(LogLevel::INFO, "Disconnected peer: " + peer.host + ":" + std::to_string(peer.port));
}

void Network::performPeerHealthChecks() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    std::vector<std::string> peersToRemove;
    
    for (auto& [peerKey, peer] : peers) {
        // Check if peer is responsive
        if (currentTime - peer.lastSeen > 120) { // 2 minutes timeout
            LOG_NETWORK(LogLevel::WARNING, "Peer timeout: " + peerKey);
            disconnectPeer(peer);
            peersToRemove.push_back(peerKey);
        } else if (peer.connected) {
            // Send ping message
            sendPing(peerKey);
        }
    }
    
    // Remove timed out peers
    for (const auto& peerKey : peersToRemove) {
        peers.erase(peerKey);
        LOG_NETWORK(LogLevel::INFO, "Removed peer: " + peerKey);
    }
}

void Network::maintainPeerConnections() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    // Try to maintain at least MIN_PEERS connections
    if (peers.size() < MIN_PEERS) {
        // Try to connect to seed nodes
        connectToSeedNodes();
    }
}

void Network::connectToSeedNodes() {
    // Default seed nodes
    std::vector<std::pair<std::string, int>> seedNodes = {
        {"seed1.gxc.network", 9333},
        {"seed2.gxc.network", 9333},
        {"seed3.gxc.network", 9333},
        {"127.0.0.1", 9334} // Local test node
    };
    
    for (const auto& [host, port] : seedNodes) {
        if (peers.size() >= MIN_PEERS) {
            break;
        }
        
        std::string peerKey = host + ":" + std::to_string(port);
        if (peers.find(peerKey) == peers.end()) {
            // Try to connect (without lock to avoid deadlock)
            peersMutex.unlock();
            connectToPeer(host, port);
            peersMutex.lock();
        }
    }
}

void Network::processIncomingMessages() {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    for (auto& [peerKey, peer] : peers) {
        if (peer.connected && peer.socket != -1) {
            receiveMessages(peer);
        }
    }
}

void Network::receiveMessages(NetworkPeer& peer) {
    char buffer[4096];
    
    // Non-blocking receive
    ssize_t bytesReceived = recv(peer.socket, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
    
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        
        peer.lastSeen = Utils::getCurrentTimestamp();
        peer.bytesReceived += bytesReceived;
        peer.messagesReceived++;
        
        // Process message
        std::string message(buffer, bytesReceived);
        processMessage(peer, message);
        
    } else if (bytesReceived == 0) {
        // Peer disconnected
        LOG_NETWORK(LogLevel::INFO, "Peer disconnected: " + peer.host + ":" + std::to_string(peer.port));
        disconnectPeer(peer);
        
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        // Error occurred
        LOG_NETWORK(LogLevel::ERROR, "Receive error from peer: " + peer.host + ":" + std::to_string(peer.port));
        disconnectPeer(peer);
    }
}

void Network::processMessage(NetworkPeer& peer, const std::string& message) {
    try {
        // Parse message (simplified JSON parsing)
        if (message.find("\"type\":\"ping\"") != std::string::npos) {
            handlePingMessage(peer, message);
        } else if (message.find("\"type\":\"pong\"") != std::string::npos) {
            handlePongMessage(peer, message);
        } else if (message.find("\"type\":\"handshake\"") != std::string::npos) {
            handleHandshakeMessage(peer, message);
        } else if (message.find("\"type\":\"transaction\"") != std::string::npos) {
            handleTransactionMessage(peer, message);
        } else if (message.find("\"type\":\"block\"") != std::string::npos) {
            handleBlockMessage(peer, message);
        } else {
            LOG_NETWORK(LogLevel::DEBUG, "Unknown message type from peer: " + peer.host);
        }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing message: " + std::string(e.what()));
    }
}

void Network::handlePingMessage(NetworkPeer& peer, const std::string& message) {
    // Send pong response
    sendPong(peer.host + ":" + std::to_string(peer.port));
}

void Network::handlePongMessage(NetworkPeer& peer, const std::string& message) {
    // Update peer's last seen time
    peer.lastSeen = Utils::getCurrentTimestamp();
}

void Network::handleHandshakeMessage(NetworkPeer& peer, const std::string& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received handshake from " + peer.host);
    
    // Process handshake data
    peer.version = "1.0.0"; // Extract from message
    peer.lastSeen = Utils::getCurrentTimestamp();
    
    // Send handshake response
    sendHandshake(peer.host + ":" + std::to_string(peer.port));
}

void Network::handleTransactionMessage(NetworkPeer& peer, const std::string& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received transaction from " + peer.host);
    
    // Forward to transaction handler
    if (transactionHandler) {
        transactionHandler(message);
    }
}

void Network::handleBlockMessage(NetworkPeer& peer, const std::string& message) {
    LOG_NETWORK(LogLevel::DEBUG, "Received block from " + peer.host);
    
    // Forward to block handler
    if (blockHandler) {
        blockHandler(message);
    }
}

bool Network::sendMessage(const std::string& peerKey, const std::string& message) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    auto it = peers.find(peerKey);
    if (it == peers.end() || !it->second.connected) {
        return false;
    }
    
    NetworkPeer& peer = it->second;
    
    ssize_t bytesSent = send(peer.socket, message.c_str(), message.length(), 0);
    
    if (bytesSent > 0) {
        peer.bytesSent += bytesSent;
        peer.messagesSent++;
        return true;
    } else {
        LOG_NETWORK(LogLevel::ERROR, "Failed to send message to peer: " + peerKey);
        disconnectPeer(peer);
        return false;
    }
}

void Network::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    for (auto& [peerKey, peer] : peers) {
        if (peer.connected) {
            sendMessage(peerKey, message);
        }
    }
}

void Network::sendPing(const std::string& peerKey) {
    std::string pingMessage = R"({"type":"ping","timestamp":)" + 
                             std::to_string(Utils::getCurrentTimestamp()) + "}";
    
    sendMessage(peerKey, pingMessage);
}

void Network::sendPong(const std::string& peerKey) {
    std::string pongMessage = R"({"type":"pong","timestamp":)" + 
                             std::to_string(Utils::getCurrentTimestamp()) + "}";
    
    sendMessage(peerKey, pongMessage);
}

void Network::sendHandshake(const std::string& peerKey) {
    std::string handshakeMessage = R"({"type":"handshake","version":"1.0.0","timestamp":)" + 
                                  std::to_string(Utils::getCurrentTimestamp()) + 
                                  R"(,"port":)" + std::to_string(port) + "}";
    
    sendMessage(peerKey, handshakeMessage);
}

bool Network::broadcastTransaction(const Transaction& tx) {
    std::string message = R"({"type":"transaction","data":)" + tx.toJson() + "}";
    broadcastMessage(message);
    
    LOG_NETWORK(LogLevel::DEBUG, "Broadcasted transaction: " + tx.getHash());
    return true;
}

bool Network::broadcastBlock(const Block& block) {
    std::string message = R"({"type":"block","data":)" + block.toJson() + "}";
    broadcastMessage(message);
    
    LOG_NETWORK(LogLevel::DEBUG, "Broadcasted block: " + block.getHash());
    return true;
}

std::vector<std::string> Network::getPeers() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    std::vector<std::string> peerList;
    for (const auto& [peerKey, peer] : peers) {
        if (peer.connected) {
            peerList.push_back(peerKey);
        }
    }
    
    return peerList;
}

size_t Network::getPeerCount() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    size_t count = 0;
    for (const auto& [peerKey, peer] : peers) {
        if (peer.connected) {
            count++;
        }
    }
    
    return count;
}

NetworkStats Network::getNetworkStats() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    
    NetworkStats stats;
    stats.peersConnected = 0;
    stats.bytesSent = 0;
    stats.bytesReceived = 0;
    stats.messagesSent = 0;
    stats.messagesReceived = 0;
    
    for (const auto& [peerKey, peer] : peers) {
        if (peer.connected) {
            stats.peersConnected++;
        }
        stats.bytesSent += peer.bytesSent;
        stats.bytesReceived += peer.bytesReceived;
        stats.messagesSent += peer.messagesSent;
        stats.messagesReceived += peer.messagesReceived;
    }
    
    return stats;
}

void Network::update() {
    // Periodic network maintenance
    static auto lastUpdate = Utils::getCurrentTimestamp();
    auto currentTime = Utils::getCurrentTimestamp();
    
    if (currentTime - lastUpdate >= 10) { // Update every 10 seconds
        // Remove disconnected peers
        std::lock_guard<std::mutex> lock(peersMutex);
        
        auto it = peers.begin();
        while (it != peers.end()) {
            if (!it->second.connected) {
                LOG_NETWORK(LogLevel::DEBUG, "Removing disconnected peer: " + it->first);
                it = peers.erase(it);
            } else {
                ++it;
            }
        }
        
        lastUpdate = currentTime;
    }
}

void Network::setTransactionHandler(std::function<void(const std::string&)> handler) {
    transactionHandler = handler;
}

void Network::setBlockHandler(std::function<void(const std::string&)> handler) {
    blockHandler = handler;
}