#include "p2p_network.h"
#include "blockchain.h"
#include "block.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>

P2PNetwork::P2PNetwork(Blockchain* bc, int port, int maxConn)
    : blockchain(bc), listenPort(port), maxConnections(maxConn), running(false), listenSocket(-1) {
    
    // Add default seed nodes (can be configured)
    // For now, these are placeholders - replace with actual seed nodes
    // seedNodes.push_back({"seed.gxc.network", 8333});
    // seedNodes.push_back({"seed2.gxc.network", 8333});
}

P2PNetwork::~P2PNetwork() {
    stop();
}

void P2PNetwork::start() {
    if (running) return;
    
    running = true;
    
    // Create listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        LOG_P2P(LogLevel::ERROR, "Failed to create listen socket");
        return;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind to port
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(listenPort);
    
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_P2P(LogLevel::ERROR, "Failed to bind to port " + std::to_string(listenPort));
        close(listenSocket);
        return;
    }
    
    // Listen for connections
    if (listen(listenSocket, maxConnections) < 0) {
        LOG_P2P(LogLevel::ERROR, "Failed to listen on port " + std::to_string(listenPort));
        close(listenSocket);
        return;
    }
    
    LOG_P2P(LogLevel::INFO, "P2P Network listening on port " + std::to_string(listenPort));
    
    // Start listening thread
    listenThread = std::thread(&P2PNetwork::listenForConnections, this);
    
    // Connect to seed nodes
    connectToSeeds();
}

void P2PNetwork::stop() {
    if (!running) return;
    
    running = false;
    
    // Close all peer connections
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto& peer : peers) {
            if (peer->socket >= 0) {
                close(peer->socket);
            }
            peer->isActive = false;
        }
        peers.clear();
    }
    
    // Close listen socket
    if (listenSocket >= 0) {
        close(listenSocket);
        listenSocket = -1;
    }
    
    // Wait for threads
    if (listenThread.joinable()) {
        listenThread.join();
    }
    
    for (auto& thread : peerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    peerThreads.clear();
    
    LOG_P2P(LogLevel::INFO, "P2P Network stopped");
}

void P2PNetwork::listenForConnections() {
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (running) {
                LOG_P2P(LogLevel::WARNING, "Failed to accept connection");
            }
            continue;
        }
        
        // Get peer address
        char addrStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, INET_ADDRSTRLEN);
        int peerPort = ntohs(clientAddr.sin_port);
        
        // Create peer
        auto peer = std::make_shared<Peer>(clientSocket, std::string(addrStr), peerPort, true);
        
        {
            std::lock_guard<std::mutex> lock(peersMutex);
            if (peers.size() >= maxConnections) {
                LOG_P2P(LogLevel::WARNING, "Max connections reached, rejecting peer");
                close(clientSocket);
                continue;
            }
            peers.push_back(peer);
        }
        
        LOG_P2P(LogLevel::INFO, "New inbound connection from " + std::string(addrStr) + ":" + std::to_string(peerPort));
        
        // Handle peer in separate thread
        peerThreads.push_back(std::thread(&P2PNetwork::handlePeer, this, peer));
    }
}

void P2PNetwork::handlePeer(std::shared_ptr<Peer> peer) {
    // Send version handshake
    sendMessage(peer->socket, "VERSION:GXC:2.0.0");
    
    // Receive version
    std::string response = receiveMessage(peer->socket);
    if (response.find("VERSION:") != 0) {
        LOG_P2P(LogLevel::WARNING, "Invalid handshake from peer");
        peer->isActive = false;
        return;
    }
    
    LOG_P2P(LogLevel::INFO, "Handshake complete with " + peer->address);
    
    // Send current blockchain height
    int height = blockchain->getHeight();
    sendMessage(peer->socket, "HEIGHT:" + std::to_string(height));
    
    // Main message loop
    while (running && peer->isActive) {
        std::string message = receiveMessage(peer->socket);
        if (message.empty()) {
            peer->isActive = false;
            break;
        }
        
        // Parse message
        if (message.find("HEIGHT:") == 0) {
            int peerHeight = std::stoi(message.substr(7));
            peer->syncedBlocks = peerHeight;
            
            // If peer has more blocks, request them
            if (peerHeight > height) {
                requestBlocks(peer, height + 1);
            }
        }
        else if (message.find("GETBLOCKS:") == 0) {
            int fromHeight = std::stoi(message.substr(10));
            // Send blocks to peer
            for (int i = fromHeight; i <= blockchain->getHeight() && i < fromHeight + 500; i++) {
                Block block = blockchain->getBlockByHeight(i);
                // Serialize and send block (simplified)
                sendMessage(peer->socket, "BLOCK:" + block.getHash());
            }
        }
        else if (message.find("BLOCK:") == 0) {
            // Receive and validate block
            std::string blockHash = message.substr(6);
            LOG_P2P(LogLevel::INFO, "Received block: " + blockHash);
        }
        else if (message.find("PING") == 0) {
            sendMessage(peer->socket, "PONG");
        }
    }
    
    LOG_P2P(LogLevel::INFO, "Peer disconnected: " + peer->address);
    
    // Remove peer from list
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        peers.erase(std::remove_if(peers.begin(), peers.end(),
            [&peer](const std::shared_ptr<Peer>& p) { return p->socket == peer->socket; }),
            peers.end());
    }
    
    close(peer->socket);
}

void P2PNetwork::connectToPeer(const std::string& address, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_P2P(LogLevel::ERROR, "Failed to create socket for outbound connection");
        return;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        LOG_P2P(LogLevel::ERROR, "Invalid address: " + address);
        close(sock);
        return;
    }
    
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_P2P(LogLevel::WARNING, "Failed to connect to " + address + ":" + std::to_string(port));
        close(sock);
        return;
    }
    
    auto peer = std::make_shared<Peer>(sock, address, port, false);
    
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        peers.push_back(peer);
    }
    
    LOG_P2P(LogLevel::INFO, "Connected to peer: " + address + ":" + std::to_string(port));
    
    // Handle peer in separate thread
    peerThreads.push_back(std::thread(&P2PNetwork::handlePeer, this, peer));
}

void P2PNetwork::sendMessage(int socket, const std::string& message) {
    uint32_t length = htonl(message.length());
    send(socket, &length, sizeof(length), 0);
    send(socket, message.c_str(), message.length(), 0);
}

std::string P2PNetwork::receiveMessage(int socket) {
    uint32_t length;
    int received = recv(socket, &length, sizeof(length), 0);
    if (received <= 0) return "";
    
    length = ntohl(length);
    if (length > 1024 * 1024) return ""; // Max 1MB message
    
    std::vector<char> buffer(length);
    received = recv(socket, buffer.data(), length, 0);
    if (received <= 0) return "";
    
    return std::string(buffer.begin(), buffer.end());
}

void P2PNetwork::addSeedNode(const std::string& address, int port) {
    seedNodes.push_back({address, port});
}

void P2PNetwork::connectToSeeds() {
    for (const auto& seed : seedNodes) {
        connectToPeer(seed.first, seed.second);
    }
}

std::vector<std::shared_ptr<Peer>> P2PNetwork::getPeers() {
    std::lock_guard<std::mutex> lock(peersMutex);
    return peers;
}

int P2PNetwork::getPeerCount() {
    std::lock_guard<std::mutex> lock(peersMutex);
    return peers.size();
}

void P2PNetwork::broadcastNewBlock(const Block& block) {
    std::lock_guard<std::mutex> lock(peersMutex);
    for (auto& peer : peers) {
        if (peer->isActive) {
            sendMessage(peer->socket, "BLOCK:" + block.getHash());
        }
    }
}

void P2PNetwork::requestBlocks(std::shared_ptr<Peer> peer, int fromHeight) {
    sendMessage(peer->socket, "GETBLOCKS:" + std::to_string(fromHeight));
}
