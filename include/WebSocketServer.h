#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <mutex>

class Blockchain;
class Block;
class Transaction;

class WebSocketServer {
private:
    Blockchain* blockchain;
    uint16_t serverPort;
    std::atomic<bool> isRunning;
    std::thread serverThread;
    std::unordered_map<std::string, int> clients;
    mutable std::mutex clientsMutex;
    
    void serverLoop();
    void handleClient(int clientSocket);
    void handleIncomingConnections();
    void broadcastMessage(const std::string& message);
    bool addClient(const std::string& clientId);
    void removeClient(const std::string& clientId);
    void sendBlockchainState(const std::string& clientId);
    void sendMessageToClient(const std::string& clientId, const std::string& message);
    void notifyNewBlock(const Block& block);
    void notifyNewTransaction(const Transaction& tx);
    std::string createBlockNotification(const Block& block);
    std::string createTransactionNotification(const Transaction& tx);
    std::string createBlockchainStatsMessage();
    uint32_t getClientCount() const;
    void sendPeriodicUpdates();
    void cleanupClients();
    
public:
    WebSocketServer(Blockchain* blockchain, uint16_t port);
    ~WebSocketServer();
    
    bool start();
    void stop();
    void processRequests();
};
