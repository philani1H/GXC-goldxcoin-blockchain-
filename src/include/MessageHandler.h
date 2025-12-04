#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <thread>
#include <atomic>
#include <vector>
#include <condition_variable>
#include "Network.h"

class Blockchain;
class PeerManager;
class Transaction;
class Block;

class MessageHandler {
private:
    Blockchain* blockchain;
    PeerManager* peerManager;
    std::atomic<bool> isRunning;
    std::thread processingThread;
    std::vector<NetworkMessage> messageQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;

    void processingLoop();
    void processMessage(const NetworkMessage& message);
    void handlePing(const NetworkMessage& message);
    void handlePong(const NetworkMessage& message);
    void handleVersion(const NetworkMessage& message);
    void handleNewTransaction(const NetworkMessage& message);
    void handleNewBlock(const NetworkMessage& message);
    void handleRequestBlocks(const NetworkMessage& message);
    void handleBlockResponse(const NetworkMessage& message);
    void handlePeerList(const NetworkMessage& message);
    void handleMempoolRequest(const NetworkMessage& message);
    void handleMempoolResponse(const NetworkMessage& message);
    void broadcastMessage(const NetworkMessage& message);
    void relayMessage(const NetworkMessage& message, const std::string& excludePeer);
    void sendMessageToPeer(const std::string& peerAddress, const NetworkMessage& message);
    void sendVersionMessage(const std::string& peerAddress);
    void sendBlockResponse(const std::string& peerAddress, const std::vector<Block>& blocks);
    std::string serializeMessage(const NetworkMessage& message);
    NetworkMessage deserializeMessage(const std::string& data);
    std::string serializeTransaction(const Transaction& tx);
    Transaction deserializeTransaction(const std::string& data);
    std::string serializeBlock(const Block& block);
    Block deserializeBlock(const std::string& data);
    std::string serializeBlockList(const std::vector<Block>& blocks);
    std::vector<Block> deserializeBlockList(const std::string& data);
    std::string serializeTransactionList(const std::vector<Transaction>& txs);
    std::vector<Transaction> deserializeTransactionList(const std::string& data);
    std::string createVersionMessage();
    std::pair<std::string, std::string> parseVersionMessage(const std::string& data);
    std::vector<std::pair<std::string, uint16_t>> parsePeerList(const std::string& data);
    std::string messageTypeToString(MessageType type);

public:
    MessageHandler(Blockchain* blockchain, PeerManager* peerManager);
    ~MessageHandler();

    bool start();
    void stop();
    void handleMessage(const NetworkMessage& message);
    void broadcastTransaction(const Transaction& transaction);
    void broadcastBlock(const Block& block);
    void requestBlocks(uint32_t startHeight, uint32_t count);
};
