#include "../include/MessageHandler.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <sstream>
#include <iomanip>

MessageHandler::MessageHandler(Blockchain* blockchain, PeerManager* peerManager)
    : blockchain(blockchain), peerManager(peerManager), isRunning(false) {
    LOG_NETWORK(LogLevel::INFO, "MessageHandler initialized");
}

MessageHandler::~MessageHandler() {
    stop();
}

bool MessageHandler::start() {
    if (isRunning) {
        LOG_NETWORK(LogLevel::WARNING, "MessageHandler already running");
        return true;
    }
    
    LOG_NETWORK(LogLevel::INFO, "Starting MessageHandler");
    
    try {
        isRunning = true;
        
        // Start message processing thread
        processingThread = std::thread(&MessageHandler::processingLoop, this);
        
        LOG_NETWORK(LogLevel::INFO, "MessageHandler started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Failed to start MessageHandler: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void MessageHandler::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_NETWORK(LogLevel::INFO, "Stopping MessageHandler");
    
    isRunning = false;
    
    // Clear message queue
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.clear();
    }
    
    // Stop processing thread
    if (processingThread.joinable()) {
        processingThread.join();
    }
    
    LOG_NETWORK(LogLevel::INFO, "MessageHandler stopped");
}

void MessageHandler::handleMessage(const NetworkMessage& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push_back(message);
    }
    
    LOG_NETWORK(LogLevel::DEBUG, "Queued message from " + message.senderAddress + 
                                 " type: " + messageTypeToString(message.type));
}

void MessageHandler::broadcastTransaction(const Transaction& transaction) {
    NetworkMessage message;
    message.type = MessageType::NEW_TRANSACTION;
    message.data = serializeTransaction(transaction);
    message.timestamp = Utils::getCurrentTimestamp();
    message.senderAddress = "local";
    
    broadcastMessage(message);
    
    LOG_NETWORK(LogLevel::INFO, "Broadcasted transaction: " + transaction.getHash().substr(0, 16));
}

void MessageHandler::broadcastBlock(const Block& block) {
    NetworkMessage message;
    message.type = MessageType::NEW_BLOCK;
    message.data = serializeBlock(block);
    message.timestamp = Utils::getCurrentTimestamp();
    message.senderAddress = "local";
    
    broadcastMessage(message);
    
    LOG_NETWORK(LogLevel::INFO, "Broadcasted block: " + std::to_string(block.getIndex()));
}

void MessageHandler::requestBlocks(uint32_t startHeight, uint32_t count) {
    NetworkMessage message;
    message.type = MessageType::REQUEST_BLOCKS;
    message.data = std::to_string(startHeight) + ":" + std::to_string(count);
    message.timestamp = Utils::getCurrentTimestamp();
    message.senderAddress = "local";
    
    broadcastMessage(message);
    
    LOG_NETWORK(LogLevel::INFO, "Requested blocks starting from " + std::to_string(startHeight));
}

void MessageHandler::processingLoop() {
    LOG_NETWORK(LogLevel::INFO, "Message processing loop started");
    
    while (isRunning) {
        try {
            std::vector<NetworkMessage> messagesToProcess;
            
            // Get messages from queue
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (!messageQueue.empty()) {
                    messagesToProcess = messageQueue;
                    messageQueue.clear();
                }
            }
            
            // Process messages
            for (const auto& message : messagesToProcess) {
                processMessage(message);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
        } catch (const std::exception& e) {
            LOG_NETWORK(LogLevel::ERROR, "Error in message processing loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_NETWORK(LogLevel::INFO, "Message processing loop stopped");
}

void MessageHandler::processMessage(const NetworkMessage& message) {
    try {
        switch (message.type) {
            case MessageType::PING:
                handlePing(message);
                break;
                
            case MessageType::PONG:
                handlePong(message);
                break;
                
            case MessageType::VERSION:
                handleVersion(message);
                break;
                
            case MessageType::NEW_TRANSACTION:
                handleNewTransaction(message);
                break;
                
            case MessageType::NEW_BLOCK:
                handleNewBlock(message);
                break;
                
            case MessageType::REQUEST_BLOCKS:
                handleRequestBlocks(message);
                break;
                
            case MessageType::BLOCK_RESPONSE:
                handleBlockResponse(message);
                break;
                
            case MessageType::PEER_LIST:
                handlePeerList(message);
                break;
                
            case MessageType::MEMPOOL_REQUEST:
                handleMempoolRequest(message);
                break;
                
            case MessageType::MEMPOOL_RESPONSE:
                handleMempoolResponse(message);
                break;
                
            default:
                LOG_NETWORK(LogLevel::WARNING, "Unknown message type: " + std::to_string(static_cast<int>(message.type)));
                break;
        }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing message: " + std::string(e.what()));
    }
}

void MessageHandler::handlePing(const NetworkMessage& message) {
    // Respond with pong
    NetworkMessage pong;
    pong.type = MessageType::PONG;
    pong.data = message.data; // Echo back the ping data
    pong.timestamp = Utils::getCurrentTimestamp();
    pong.senderAddress = "local";
    
    sendMessageToPeer(message.senderAddress, pong);
    
    LOG_NETWORK(LogLevel::DEBUG, "Responded to ping from " + message.senderAddress);
}

void MessageHandler::handlePong(const NetworkMessage& message) {
    // Update peer last seen time
    LOG_NETWORK(LogLevel::DEBUG, "Received pong from " + message.senderAddress);
}

void MessageHandler::handleVersion(const NetworkMessage& message) {
    // Parse version information
    auto versionInfo = parseVersionMessage(message.data);
    
    LOG_NETWORK(LogLevel::INFO, "Received version from " + message.senderAddress + 
                                " version: " + versionInfo.version);
    
    // Send our version in response
    sendVersionMessage(message.senderAddress);
}

void MessageHandler::handleNewTransaction(const NetworkMessage& message) {
    try {
        Transaction transaction = deserializeTransaction(message.data);
        
        // Validate transaction
        if (transaction.isValid() && transaction.isTraceabilityValid()) {
            // Add to blockchain pending transactions
            blockchain->addPendingTransaction(transaction);
            
            // Relay to other peers (avoid broadcasting back to sender)
            relayMessage(message, message.senderAddress);
            
            LOG_NETWORK(LogLevel::INFO, "Processed new transaction: " + transaction.getHash().substr(0, 16));
        } else {
            LOG_NETWORK(LogLevel::WARNING, "Invalid transaction received from " + message.senderAddress);
        }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing new transaction: " + std::string(e.what()));
    }
}

void MessageHandler::handleNewBlock(const NetworkMessage& message) {
    try {
        Block block = deserializeBlock(message.data);
        
        // Validate and add block
        if (blockchain->addBlock(block)) {
            // Relay to other peers
            relayMessage(message, message.senderAddress);
            
            LOG_NETWORK(LogLevel::INFO, "Processed new block: " + std::to_string(block.getIndex()));
        } else {
            LOG_NETWORK(LogLevel::WARNING, "Invalid block received from " + message.senderAddress);
        }
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing new block: " + std::string(e.what()));
    }
}

void MessageHandler::handleRequestBlocks(const NetworkMessage& message) {
    try {
        // Parse request parameters
        auto pos = message.data.find(':');
        if (pos == std::string::npos) {
            return;
        }
        
        uint32_t startHeight = std::stoul(message.data.substr(0, pos));
        uint32_t count = std::stoul(message.data.substr(pos + 1));
        
        // Limit the number of blocks that can be requested
        count = std::min(count, static_cast<uint32_t>(500));
        
        // Get blocks from blockchain
        std::vector<Block> blocks;
        for (uint32_t i = 0; i < count; i++) {
            uint32_t height = startHeight + i;
            if (height < blockchain->getChainLength()) {
                Block block = blockchain->getBlockByHeight(height);
                blocks.push_back(block);
            } else {
                break;
            }
        }
        
        // Send block response
        sendBlockResponse(message.senderAddress, blocks);
        
        LOG_NETWORK(LogLevel::INFO, "Sent " + std::to_string(blocks.size()) + 
                                   " blocks to " + message.senderAddress);
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error handling block request: " + std::string(e.what()));
    }
}

void MessageHandler::handleBlockResponse(const NetworkMessage& message) {
    try {
        std::vector<Block> blocks = deserializeBlockList(message.data);
        
        for (const auto& block : blocks) {
            blockchain->addBlock(block);
        }
        
        LOG_NETWORK(LogLevel::INFO, "Received " + std::to_string(blocks.size()) + 
                                   " blocks from " + message.senderAddress);
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing block response: " + std::string(e.what()));
    }
}

void MessageHandler::handlePeerList(const NetworkMessage& message) {
    try {
        auto peers = parsePeerList(message.data);
        
        for (const auto& peer : peers) {
            peerManager->addKnownPeer(peer.first, peer.second);
        }
        
        LOG_NETWORK(LogLevel::DEBUG, "Received " + std::to_string(peers.size()) + 
                                    " peers from " + message.senderAddress);
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing peer list: " + std::string(e.what()));
    }
}

void MessageHandler::handleMempoolRequest(const NetworkMessage& message) {
    // Send mempool (pending transactions)
    auto pendingTx = blockchain->getPendingTransactions();
    
    NetworkMessage response;
    response.type = MessageType::MEMPOOL_RESPONSE;
    response.data = serializeTransactionList(pendingTx);
    response.timestamp = Utils::getCurrentTimestamp();
    response.senderAddress = "local";
    
    sendMessageToPeer(message.senderAddress, response);
    
    LOG_NETWORK(LogLevel::DEBUG, "Sent mempool with " + std::to_string(pendingTx.size()) + 
                                " transactions to " + message.senderAddress);
}

void MessageHandler::handleMempoolResponse(const NetworkMessage& message) {
    try {
        auto transactions = deserializeTransactionList(message.data);
        
        for (const auto& tx : transactions) {
            if (tx.isValid() && tx.isTraceabilityValid()) {
                blockchain->addPendingTransaction(tx);
            }
        }
        
        LOG_NETWORK(LogLevel::INFO, "Received mempool with " + std::to_string(transactions.size()) + 
                                   " transactions from " + message.senderAddress);
        
    } catch (const std::exception& e) {
        LOG_NETWORK(LogLevel::ERROR, "Error processing mempool response: " + std::string(e.what()));
    }
}

void MessageHandler::broadcastMessage(const NetworkMessage& message) {
    std::string serializedMessage = serializeMessage(message);
    
    if (peerManager) {
        peerManager->broadcastMessage(serializedMessage);
    }
}

void MessageHandler::relayMessage(const NetworkMessage& message, const std::string& excludePeer) {
    // Broadcast to all peers except the sender
    auto connectedPeers = peerManager->getConnectedPeers();
    
    for (const auto& peer : connectedPeers) {
        std::string peerAddress = peer.address + ":" + std::to_string(peer.port);
        if (peerAddress != excludePeer) {
            sendMessageToPeer(peerAddress, message);
        }
    }
}

void MessageHandler::sendMessageToPeer(const std::string& peerAddress, const NetworkMessage& message) {
    std::string serializedMessage = serializeMessage(message);
    
    // Extract address and port
    auto pos = peerAddress.find(':');
    if (pos != std::string::npos) {
        std::string address = peerAddress.substr(0, pos);
        uint16_t port = static_cast<uint16_t>(std::stoul(peerAddress.substr(pos + 1)));
        
        // In real implementation, would send actual network message
        LOG_NETWORK(LogLevel::DEBUG, "Sending " + messageTypeToString(message.type) + 
                                    " to " + peerAddress);
    }
}

void MessageHandler::sendVersionMessage(const std::string& peerAddress) {
    NetworkMessage version;
    version.type = MessageType::VERSION;
    version.data = createVersionMessage();
    version.timestamp = Utils::getCurrentTimestamp();
    version.senderAddress = "local";
    
    sendMessageToPeer(peerAddress, version);
}

void MessageHandler::sendBlockResponse(const std::string& peerAddress, const std::vector<Block>& blocks) {
    NetworkMessage response;
    response.type = MessageType::BLOCK_RESPONSE;
    response.data = serializeBlockList(blocks);
    response.timestamp = Utils::getCurrentTimestamp();
    response.senderAddress = "local";
    
    sendMessageToPeer(peerAddress, response);
}

std::string MessageHandler::serializeMessage(const NetworkMessage& message) {
    std::ostringstream oss;
    oss << static_cast<int>(message.type) << "|"
        << message.timestamp << "|"
        << message.senderAddress << "|"
        << message.data.length() << "|"
        << message.data;
    
    return oss.str();
}

NetworkMessage MessageHandler::deserializeMessage(const std::string& data) {
    NetworkMessage message;
    
    std::istringstream iss(data);
    std::string token;
    
    // Parse type
    if (std::getline(iss, token, '|')) {
        message.type = static_cast<MessageType>(std::stoi(token));
    }
    
    // Parse timestamp
    if (std::getline(iss, token, '|')) {
        message.timestamp = std::stoull(token);
    }
    
    // Parse sender address
    if (std::getline(iss, token, '|')) {
        message.senderAddress = token;
    }
    
    // Parse data length
    size_t dataLength = 0;
    if (std::getline(iss, token, '|')) {
        dataLength = std::stoul(token);
    }
    
    // Parse data
    if (dataLength > 0) {
        message.data.resize(dataLength);
        iss.read(&message.data[0], dataLength);
    }
    
    return message;
}

std::string MessageHandler::serializeTransaction(const Transaction& transaction) {
    // Simplified serialization - in real implementation would use proper binary format
    std::ostringstream oss;
    oss << transaction.getHash() << "|"
        << transaction.getTimestamp() << "|"
        << transaction.getTotalOutputAmount() << "|"
        << transaction.getFee() << "|"
        << transaction.getPrevTxHash() << "|"
        << transaction.getReferencedAmount();
    
    return oss.str();
}

Transaction MessageHandler::deserializeTransaction(const std::string& data) {
    // Simplified deserialization
    std::istringstream iss(data);
    std::string token;
    
    std::string hash, prevTxHash;
    std::time_t timestamp;
    double amount, fee, refAmount;
    
    if (std::getline(iss, hash, '|') &&
        std::getline(iss, token, '|') && (timestamp = std::stoull(token), true) &&
        std::getline(iss, token, '|') && (amount = std::stod(token), true) &&
        std::getline(iss, token, '|') && (fee = std::stod(token), true) &&
        std::getline(iss, prevTxHash, '|') &&
        std::getline(iss, token, '|') && (refAmount = std::stod(token), true)) {
        
        // Create transaction (simplified)
        Transaction tx("sender", "receiver", amount);
        tx.setTimestamp(timestamp);
        tx.setPrevTxHash(prevTxHash);
        tx.setReferencedAmount(refAmount);
        
        return tx;
    }
    
    throw std::runtime_error("Invalid transaction data");
}

std::string MessageHandler::serializeBlock(const Block& block) {
    // Simplified serialization
    std::ostringstream oss;
    oss << block.getIndex() << "|"
        << block.getHash() << "|"
        << block.getPreviousHash() << "|"
        << block.getTimestamp() << "|"
        << block.getNonce() << "|"
        << block.getTransactions().size();
    
    return oss.str();
}

Block MessageHandler::deserializeBlock(const std::string& data) {
    // Simplified deserialization - would need full implementation
    std::vector<Transaction> emptyTxs;
    Block block(0, "0", emptyTxs);
    return block;
}

std::string MessageHandler::serializeBlockList(const std::vector<Block>& blocks) {
    std::ostringstream oss;
    oss << blocks.size() << "|";
    
    for (const auto& block : blocks) {
        oss << serializeBlock(block) << "|";
    }
    
    return oss.str();
}

std::vector<Block> MessageHandler::deserializeBlockList(const std::string& data) {
    std::vector<Block> blocks;
    // Simplified implementation
    return blocks;
}

std::string MessageHandler::serializeTransactionList(const std::vector<Transaction>& transactions) {
    std::ostringstream oss;
    oss << transactions.size() << "|";
    
    for (const auto& tx : transactions) {
        oss << serializeTransaction(tx) << "|";
    }
    
    return oss.str();
}

std::vector<Transaction> MessageHandler::deserializeTransactionList(const std::string& data) {
    std::vector<Transaction> transactions;
    // Simplified implementation
    return transactions;
}

std::string MessageHandler::createVersionMessage() {
    std::ostringstream oss;
    oss << "version:1.0.0|"
        << "protocol:1|"
        << "services:1|"
        << "timestamp:" << Utils::getCurrentTimestamp() << "|"
        << "user_agent:GXC/1.0.0|"
        << "start_height:" << blockchain->getChainLength();
    
    return oss.str();
}

VersionInfo MessageHandler::parseVersionMessage(const std::string& data) {
    VersionInfo info;
    
    std::istringstream iss(data);
    std::string token;
    
    while (std::getline(iss, token, '|')) {
        auto pos = token.find(':');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            
            if (key == "version") {
                info.version = value;
            } else if (key == "protocol") {
                info.protocolVersion = std::stoi(value);
            } else if (key == "user_agent") {
                info.userAgent = value;
            } else if (key == "start_height") {
                info.startHeight = std::stoul(value);
            }
        }
    }
    
    return info;
}

std::vector<std::pair<std::string, uint16_t>> MessageHandler::parsePeerList(const std::string& data) {
    std::vector<std::pair<std::string, uint16_t>> peers;
    
    std::istringstream iss(data);
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        auto pos = token.find(':');
        if (pos != std::string::npos) {
            std::string address = token.substr(0, pos);
            uint16_t port = static_cast<uint16_t>(std::stoul(token.substr(pos + 1)));
            peers.emplace_back(address, port);
        }
    }
    
    return peers;
}

std::string MessageHandler::messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::PING: return "PING";
        case MessageType::PONG: return "PONG";
        case MessageType::VERSION: return "VERSION";
        case MessageType::NEW_TRANSACTION: return "NEW_TRANSACTION";
        case MessageType::NEW_BLOCK: return "NEW_BLOCK";
        case MessageType::REQUEST_BLOCKS: return "REQUEST_BLOCKS";
        case MessageType::BLOCK_RESPONSE: return "BLOCK_RESPONSE";
        case MessageType::PEER_LIST: return "PEER_LIST";
        case MessageType::MEMPOOL_REQUEST: return "MEMPOOL_REQUEST";
        case MessageType::MEMPOOL_RESPONSE: return "MEMPOOL_RESPONSE";
        default: return "UNKNOWN";
    }
}