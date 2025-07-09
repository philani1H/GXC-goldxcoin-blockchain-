#include "Network.h"
#include "Logger.h"
#include "Config.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace boost::asio;
using tcp = boost::asio::ip::tcp;

Network::Network(Config* config) 
    : config_(config), ioContext_(), acceptor_(ioContext_), isRunning_(false) {
    
    port_ = config_->getNetworkPort();
    maxPeers_ = config_->getMaxPeers();
    
    Logger::info("Network initialized on port " + std::to_string(port_));
}

Network::~Network() {
    stop();
}

bool Network::start() {
    if (isRunning_) {
        Logger::warning("Network is already running");
        return false;
    }
    
    try {
        // Set up acceptor to listen for incoming connections
        tcp::endpoint endpoint(tcp::v4(), port_);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
        
        isRunning_ = true;
        
        // Start accepting connections
        startAccept();
        
        // Start network thread
        networkThread_ = std::thread([this]() {
            try {
                ioContext_.run();
            } catch (const std::exception& e) {
                Logger::error("Network thread error: " + std::string(e.what()));
            }
        });
        
        // Start message processing thread
        messageThread_ = std::thread(&Network::messageProcessingLoop, this);
        
        // Start peer discovery
        startPeerDiscovery();
        
        Logger::info("Network started successfully on port " + std::to_string(port_));
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to start network: " + std::string(e.what()));
        isRunning_ = false;
        return false;
    }
}

void Network::stop() {
    if (!isRunning_) {
        return;
    }
    
    Logger::info("Stopping network...");
    isRunning_ = false;
    
    // Close acceptor
    acceptor_.close();
    
    // Close all connections
    std::lock_guard<std::mutex> lock(peersMutex_);
    for (auto& [id, peer] : peers_) {
        peer->disconnect();
    }
    peers_.clear();
    
    // Stop io context
    ioContext_.stop();
    
    // Join threads
    if (networkThread_.joinable()) {
        networkThread_.join();
    }
    
    if (messageThread_.joinable()) {
        messageThread_.join();
    }
    
    Logger::info("Network stopped");
}

void Network::startAccept() {
    auto newPeer = std::make_shared<NetworkPeer>(ioContext_);
    
    acceptor_.async_accept(newPeer->getSocket(),
        [this, newPeer](const boost::system::error_code& error) {
            if (!error && isRunning_) {
                handleNewConnection(newPeer);
                startAccept(); // Continue accepting new connections
            } else if (error != boost::asio::error::operation_aborted) {
                Logger::error("Accept error: " + error.message());
                if (isRunning_) {
                    startAccept(); // Retry
                }
            }
        });
}

void Network::handleNewConnection(std::shared_ptr<NetworkPeer> peer) {
    try {
        // Check if we have room for more peers
        if (getActivePeerCount() >= maxPeers_) {
            Logger::warning("Maximum peer limit reached, rejecting connection");
            peer->disconnect();
            return;
        }
        
        // Get peer info
        std::string peerAddress = peer->getRemoteAddress();
        
        // Check if this peer is banned
        if (isBanned(peerAddress)) {
            Logger::warning("Rejecting connection from banned peer: " + peerAddress);
            peer->disconnect();
            return;
        }
        
        // Initialize peer
        peer->setId(generatePeerId());
        peer->setOnMessageReceived([this](const std::shared_ptr<NetworkPeer>& sender, const NetworkMessage& msg) {
            handleMessage(sender, msg);
        });
        
        peer->setOnDisconnected([this](const std::shared_ptr<NetworkPeer>& disconnectedPeer) {
            handlePeerDisconnected(disconnectedPeer);
        });
        
        // Add to peer list
        {
            std::lock_guard<std::mutex> lock(peersMutex_);
            peers_[peer->getId()] = peer;
        }
        
        // Start peer communication
        peer->start();
        
        // Send version message
        sendVersionMessage(peer);
        
        Logger::info("New peer connected: " + peerAddress + " (ID: " + peer->getId() + ")");
        
        // Update peer statistics
        updatePeerStats(peer, "connected");
        
    } catch (const std::exception& e) {
        Logger::error("Error handling new connection: " + std::string(e.what()));
        peer->disconnect();
    }
}

void Network::handleMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        // Update peer activity
        sender->updateLastActivity();
        
        // Add message to processing queue
        {
            std::lock_guard<std::mutex> lock(messageQueueMutex_);
            messageQueue_.push({sender, message});
        }
        
        messageQueueCondition_.notify_one();
        
    } catch (const std::exception& e) {
        Logger::error("Error handling message: " + std::string(e.what()));
    }
}

void Network::handlePeerDisconnected(std::shared_ptr<NetworkPeer> peer) {
    try {
        std::lock_guard<std::mutex> lock(peersMutex_);
        peers_.erase(peer->getId());
        
        Logger::info("Peer disconnected: " + peer->getRemoteAddress() + " (ID: " + peer->getId() + ")");
        
        // Update peer statistics
        updatePeerStats(peer, "disconnected");
        
    } catch (const std::exception& e) {
        Logger::error("Error handling peer disconnection: " + std::string(e.what()));
    }
}

void Network::messageProcessingLoop() {
    while (isRunning_) {
        try {
            std::unique_lock<std::mutex> lock(messageQueueMutex_);
            
            // Wait for messages
            messageQueueCondition_.wait(lock, [this] {
                return !messageQueue_.empty() || !isRunning_;
            });
            
            while (!messageQueue_.empty() && isRunning_) {
                auto [sender, message] = messageQueue_.front();
                messageQueue_.pop();
                
                lock.unlock();
                processMessage(sender, message);
                lock.lock();
            }
            
        } catch (const std::exception& e) {
            Logger::error("Message processing loop error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void Network::processMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        switch (message.type) {
            case MessageType::VERSION:
                handleVersionMessage(sender, message);
                break;
                
            case MessageType::PING:
                handlePingMessage(sender, message);
                break;
                
            case MessageType::PONG:
                handlePongMessage(sender, message);
                break;
                
            case MessageType::BLOCK:
                handleBlockMessage(sender, message);
                break;
                
            case MessageType::TRANSACTION:
                handleTransactionMessage(sender, message);
                break;
                
            case MessageType::GET_BLOCKS:
                handleGetBlocksMessage(sender, message);
                break;
                
            case MessageType::GET_PEERS:
                handleGetPeersMessage(sender, message);
                break;
                
            case MessageType::PEERS:
                handlePeersMessage(sender, message);
                break;
                
            default:
                Logger::warning("Unknown message type received: " + std::to_string(static_cast<int>(message.type)));
                break;
        }
        
        // Update message statistics
        updateMessageStats(message.type, "received");
        
    } catch (const std::exception& e) {
        Logger::error("Error processing message: " + std::string(e.what()));
    }
}

void Network::handleVersionMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        json data = json::parse(message.payload);
        
        std::string version = data["version"];
        std::string userAgent = data["user_agent"];
        uint64_t height = data["height"];
        
        sender->setVersion(version);
        sender->setUserAgent(userAgent);
        sender->setHeight(height);
        
        Logger::debug("Received version from " + sender->getRemoteAddress() + 
                     ": " + version + " (" + userAgent + "), height: " + std::to_string(height));
        
        // Send version ack
        NetworkMessage response;
        response.type = MessageType::VERSION_ACK;
        response.payload = "OK";
        sender->sendMessage(response);
        
    } catch (const std::exception& e) {
        Logger::error("Error handling version message: " + std::string(e.what()));
    }
}

void Network::handlePingMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        // Send pong response
        NetworkMessage response;
        response.type = MessageType::PONG;
        response.payload = message.payload; // Echo the ping data
        sender->sendMessage(response);
        
    } catch (const std::exception& e) {
        Logger::error("Error handling ping message: " + std::string(e.what()));
    }
}

void Network::handlePongMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        // Calculate round-trip time if we sent the original ping
        auto now = std::chrono::steady_clock::now();
        // Implementation would store ping timestamps and calculate RTT
        
        Logger::debug("Received pong from " + sender->getRemoteAddress());
        
    } catch (const std::exception& e) {
        Logger::error("Error handling pong message: " + std::string(e.what()));
    }
}

void Network::handleBlockMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        // Forward block to blockchain for processing
        if (onBlockReceived_) {
            onBlockReceived_(message.payload, sender->getId());
        }
        
        // Relay block to other peers (except sender)
        relayMessage(message, sender->getId());
        
    } catch (const std::exception& e) {
        Logger::error("Error handling block message: " + std::string(e.what()));
    }
}

void Network::handleTransactionMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        // Forward transaction to blockchain for processing
        if (onTransactionReceived_) {
            onTransactionReceived_(message.payload, sender->getId());
        }
        
        // Relay transaction to other peers (except sender)
        relayMessage(message, sender->getId());
        
    } catch (const std::exception& e) {
        Logger::error("Error handling transaction message: " + std::string(e.what()));
    }
}

void Network::handleGetBlocksMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        json data = json::parse(message.payload);
        std::string startHash = data["start_hash"];
        std::string endHash = data["end_hash"];
        int limit = data.value("limit", 500);
        
        // Request blocks from blockchain
        if (onBlocksRequested_) {
            onBlocksRequested_(startHash, endHash, limit, sender->getId());
        }
        
    } catch (const std::exception& e) {
        Logger::error("Error handling get blocks message: " + std::string(e.what()));
    }
}

void Network::handleGetPeersMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        // Send list of known peers
        json peersData = json::array();
        
        {
            std::lock_guard<std::mutex> lock(peersMutex_);
            for (const auto& [id, peer] : peers_) {
                if (peer->getId() != sender->getId() && peer->isConnected()) {
                    json peerInfo;
                    peerInfo["address"] = peer->getRemoteAddress();
                    peerInfo["port"] = peer->getRemotePort();
                    peerInfo["version"] = peer->getVersion();
                    peersData.push_back(peerInfo);
                }
            }
        }
        
        NetworkMessage response;
        response.type = MessageType::PEERS;
        response.payload = peersData.dump();
        sender->sendMessage(response);
        
    } catch (const std::exception& e) {
        Logger::error("Error handling get peers message: " + std::string(e.what()));
    }
}

void Network::handlePeersMessage(std::shared_ptr<NetworkPeer> sender, const NetworkMessage& message) {
    try {
        json peersData = json::parse(message.payload);
        
        for (const auto& peerInfo : peersData) {
            std::string address = peerInfo["address"];
            int port = peerInfo["port"];
            
            // Add to known peers for future connections
            addKnownPeer(address, port);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Error handling peers message: " + std::string(e.what()));
    }
}

void Network::broadcastBlock(const std::string& blockData) {
    NetworkMessage message;
    message.type = MessageType::BLOCK;
    message.payload = blockData;
    
    broadcastMessage(message);
}

void Network::broadcastTransaction(const std::string& transactionData) {
    NetworkMessage message;
    message.type = MessageType::TRANSACTION;
    message.payload = transactionData;
    
    broadcastMessage(message);
}

void Network::broadcastMessage(const NetworkMessage& message) {
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    for (const auto& [id, peer] : peers_) {
        if (peer->isConnected()) {
            peer->sendMessage(message);
        }
    }
    
    updateMessageStats(message.type, "sent");
}

void Network::relayMessage(const NetworkMessage& message, const std::string& excludePeerId) {
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    for (const auto& [id, peer] : peers_) {
        if (peer->isConnected() && peer->getId() != excludePeerId) {
            peer->sendMessage(message);
        }
    }
    
    updateMessageStats(message.type, "relayed");
}

void Network::sendVersionMessage(std::shared_ptr<NetworkPeer> peer) {
    try {
        json versionData;
        versionData["version"] = "2.0.0";
        versionData["user_agent"] = "GXC-Node/2.0.0";
        versionData["height"] = getCurrentBlockHeight();
        versionData["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        NetworkMessage message;
        message.type = MessageType::VERSION;
        message.payload = versionData.dump();
        
        peer->sendMessage(message);
        
    } catch (const std::exception& e) {
        Logger::error("Error sending version message: " + std::string(e.what()));
    }
}

void Network::startPeerDiscovery() {
    discoveryThread_ = std::thread(&Network::peerDiscoveryLoop, this);
}

void Network::peerDiscoveryLoop() {
    while (isRunning_) {
        try {
            // Connect to new peers if we don't have enough
            if (getActivePeerCount() < maxPeers_ / 2) {
                connectToNewPeers();
            }
            
            // Send ping messages to maintain connections
            sendPingMessages();
            
            // Clean up disconnected peers
            cleanupDisconnectedPeers();
            
            // Sleep before next discovery cycle
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            Logger::error("Peer discovery loop error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }
}

void Network::connectToNewPeers() {
    // Implementation would connect to known peers from database or seed nodes
    // For now, we'll log the intention
    Logger::debug("Attempting to discover and connect to new peers");
}

void Network::sendPingMessages() {
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    NetworkMessage ping;
    ping.type = MessageType::PING;
    ping.payload = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    
    for (const auto& [id, peer] : peers_) {
        if (peer->isConnected()) {
            peer->sendMessage(ping);
        }
    }
}

void Network::cleanupDisconnectedPeers() {
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    auto it = peers_.begin();
    while (it != peers_.end()) {
        if (!it->second->isConnected()) {
            Logger::debug("Removing disconnected peer: " + it->second->getId());
            it = peers_.erase(it);
        } else {
            ++it;
        }
    }
}

int Network::getActivePeerCount() const {
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    int count = 0;
    for (const auto& [id, peer] : peers_) {
        if (peer->isConnected()) {
            count++;
        }
    }
    return count;
}

std::vector<PeerInfo> Network::getConnectedPeers() const {
    std::vector<PeerInfo> peerList;
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    for (const auto& [id, peer] : peers_) {
        if (peer->isConnected()) {
            PeerInfo info;
            info.id = peer->getId();
            info.address = peer->getRemoteAddress();
            info.port = peer->getRemotePort();
            info.version = peer->getVersion();
            info.userAgent = peer->getUserAgent();
            info.height = peer->getHeight();
            info.connectionTime = peer->getConnectionTime();
            peerList.push_back(info);
        }
    }
    
    return peerList;
}

void Network::banPeer(const std::string& address, const std::string& reason) {
    std::lock_guard<std::mutex> lock(bannedPeersMutex_);
    
    BanInfo banInfo;
    banInfo.reason = reason;
    banInfo.banTime = std::chrono::steady_clock::now();
    banInfo.duration = std::chrono::hours(24); // 24 hour ban
    
    bannedPeers_[address] = banInfo;
    
    Logger::warning("Banned peer " + address + ": " + reason);
    
    // Disconnect if currently connected
    disconnectPeer(address);
}

bool Network::isBanned(const std::string& address) const {
    std::lock_guard<std::mutex> lock(bannedPeersMutex_);
    
    auto it = bannedPeers_.find(address);
    if (it != bannedPeers_.end()) {
        auto now = std::chrono::steady_clock::now();
        if (now - it->second.banTime < it->second.duration) {
            return true;
        }
    }
    
    return false;
}

void Network::disconnectPeer(const std::string& address) {
    std::lock_guard<std::mutex> lock(peersMutex_);
    
    for (const auto& [id, peer] : peers_) {
        if (peer->getRemoteAddress() == address) {
            peer->disconnect();
            break;
        }
    }
}

std::string Network::generatePeerId() {
    static int counter = 0;
    return "peer_" + std::to_string(++counter) + "_" + 
           std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count());
}

void Network::addKnownPeer(const std::string& address, int port) {
    // Implementation would add peer to database of known peers
    Logger::debug("Added known peer: " + address + ":" + std::to_string(port));
}

void Network::updatePeerStats(std::shared_ptr<NetworkPeer> peer, const std::string& event) {
    // Implementation would update peer statistics in database
    Logger::debug("Peer stats updated: " + peer->getId() + " - " + event);
}

void Network::updateMessageStats(MessageType type, const std::string& direction) {
    // Implementation would update message statistics
    std::string typeName = getMessageTypeName(type);
    Logger::debug("Message stats: " + typeName + " - " + direction);
}

std::string Network::getMessageTypeName(MessageType type) const {
    switch (type) {
        case MessageType::VERSION: return "VERSION";
        case MessageType::VERSION_ACK: return "VERSION_ACK";
        case MessageType::PING: return "PING";
        case MessageType::PONG: return "PONG";
        case MessageType::BLOCK: return "BLOCK";
        case MessageType::TRANSACTION: return "TRANSACTION";
        case MessageType::GET_BLOCKS: return "GET_BLOCKS";
        case MessageType::GET_PEERS: return "GET_PEERS";
        case MessageType::PEERS: return "PEERS";
        default: return "UNKNOWN";
    }
}

uint64_t Network::getCurrentBlockHeight() const {
    // This would be implemented to get current height from blockchain
    return 0;
}

void Network::processMessages() {
    // Called by external threads to trigger message processing
    messageQueueCondition_.notify_all();
}