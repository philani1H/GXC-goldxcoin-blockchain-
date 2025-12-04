#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <cstdint>
#include <nlohmann/json.hpp>

class Blockchain;

class RESTServer {
private:
    Blockchain* blockchain;
    uint16_t serverPort;
    std::atomic<bool> isRunning;
    std::thread serverThread;

    void serverLoop();
    std::string handleRequest(const std::string& method, const std::string& path, const std::string& body);
    std::string getBlockchainInfo();
    std::string getBlock(const std::string& blockId);
    std::string getBlocks();
    std::string getTransaction(const std::string& txHash);
    std::string submitTransaction(const std::string& body);
    std::string getAddressBalance(const std::string& address);
    std::string getAddressTransactions(const std::string& address);
    std::string traceTransaction(const std::string& txHash);
    std::string getMiningInfo();
    std::string getPeerInfo();
    std::string getStats();
    std::string createErrorResponse(int code, const std::string& error, const std::string& message);
    std::string jsonToString(const nlohmann::json& json);

public:
    RESTServer(Blockchain* blockchain, uint16_t port);
    ~RESTServer();

    bool start();
    void stop();
    void processRequests();
};
