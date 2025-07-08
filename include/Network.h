#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <cstdint>
#include <memory>

// Network message types
enum class MessageType : uint8_t {
    VERSION = 0x01,
    VERACK = 0x02,
    PING = 0x03,
    PONG = 0x04,
    GETADDR = 0x05,
    ADDR = 0x06,
    GETBLOCKS = 0x07,
    BLOCKS = 0x08,
    GETDATA = 0x09,
    BLOCK = 0x0A,
    TX = 0x0B,
    MEMPOOL = 0x0C,
    GETMEMPOOL = 0x0D,
    REJECT = 0x0E,
    ALERT = 0x0F,
    MINING_WORK = 0x10,
    MINING_SUBMIT = 0x11,
    PEER_LIST = 0x12,
    SYNC_REQUEST = 0x13,
    SYNC_RESPONSE = 0x14
};

// Network configuration
struct NetworkConfig {
    bool isTestnet = false;
    uint16_t defaultPort = 8333;      // Mainnet port
    uint16_t testnetPort = 18333;     // Testnet port
    uint16_t rpcPort = 8332;          // RPC port
    uint16_t testnetRpcPort = 18332;  // Testnet RPC port
    
    std::string networkMagic = "GXC\x01";     // Mainnet magic
    std::string testnetMagic = "GXCT";        // Testnet magic
    
    std::vector<std::string> seedNodes = {
        "seed1.gxc.network:8333",
        "seed2.gxc.network:8333",
        "seed3.gxc.network:8333"
    };
    
    std::vector<std::string> testnetSeeds = {
        "testseed1.gxc.network:18333",
        "testseed2.gxc.network:18333"
    };
    
    uint32_t maxConnections = 125;
    uint32_t maxOutbound = 8;
    uint32_t protocolVersion = 70015;
    std::string userAgent = "/GXC:2.0.0/";
};

// Peer information
struct PeerInfo {
    std::string address;
    uint16_t port;
    uint32_t version;
    std::string userAgent;
    int64_t lastSeen;
    bool isOutbound;
    bool isConnected;
    double pingTime;
    uint32_t height;
    std::string nodeId;
};

// Network message structure
struct NetworkMessage {
    MessageType type;
    std::vector<uint8_t> payload;
    uint32_t checksum;
    std::time_t timestamp;
    std::string fromPeer;
};

// Mining work for pools
struct MiningWork {
    std::string jobId;
    std::string blockTemplate;
    std::string target;
    std::string extraNonce1;
    std::string extraNonce2;
    uint32_t difficulty;
    std::time_t timestamp;
    bool cleanJobs;
};

// Mining submission
struct MiningSubmission {
    std::string jobId;
    std::string extraNonce2;
    std::string nonce;
    std::string timestamp;
    std::string workerName;
};

class NetworkNode {
private:
    NetworkConfig config;
    std::unordered_map<std::string, PeerInfo> peers;
    std::vector<std::thread> connectionThreads;
    std::mutex peersMutex;
    std::mutex messageMutex;
    std::atomic<bool> running{false};
    std::atomic<uint32_t> bestHeight{0};
    
    // Blockchain reference
    class Blockchain* blockchain;
    
    // Message handlers
    std::unordered_map<MessageType, std::function<void(const NetworkMessage&, const std::string&)>> messageHandlers;
    
    // Connection management
    int serverSocket;
    std::thread serverThread;
    std::thread peerManagerThread;
    
    // Mining pool support
    std::unordered_map<std::string, MiningWork> activeMiningWork;
    std::mutex miningMutex;
    uint32_t nextJobId = 1;
    
public:
    NetworkNode(const NetworkConfig& configIn, Blockchain* blockchainIn);
    ~NetworkNode();
    
    // Network lifecycle
    bool start();
    void stop();
    bool isRunning() const { return running; }
    
    // Peer management
    bool connectToPeer(const std::string& address, uint16_t port);
    void disconnectPeer(const std::string& peerId);
    std::vector<PeerInfo> getConnectedPeers() const;
    void addSeedNodes();
    
    // Message handling
    void broadcastMessage(const NetworkMessage& message);
    void sendMessageToPeer(const std::string& peerId, const NetworkMessage& message);
    void registerMessageHandler(MessageType type, std::function<void(const NetworkMessage&, const std::string&)> handler);
    
    // Blockchain synchronization
    void requestBlocks(const std::string& fromHash = "", uint32_t count = 500);
    void announceNewBlock(const std::string& blockHash);
    void announceNewTransaction(const std::string& txHash);
    void syncWithPeers();
    
    // Mining pool interface
    MiningWork createMiningWork(const std::string& minerAddress);
    bool submitMiningWork(const MiningSubmission& submission);
    std::vector<MiningWork> getActiveMiningWork() const;
    void notifyMiningClients(const MiningWork& work);
    
    // Network statistics
    uint32_t getPeerCount() const;
    uint32_t getBestHeight() const { return bestHeight; }
    double getAveragePingTime() const;
    uint64_t getTotalBytesSent() const;
    uint64_t getTotalBytesReceived() const;
    
    // Configuration
    void setTestnet(bool testnet);
    NetworkConfig getConfig() const { return config; }
    std::string getNetworkMagic() const;
    uint16_t getDefaultPort() const;
    
    // Address management
    std::vector<std::string> getKnownAddresses() const;
    void addKnownAddress(const std::string& address, uint16_t port);
    void removeKnownAddress(const std::string& address);
    
    // Security
    bool validatePeer(const std::string& peerId) const;
    void banPeer(const std::string& peerId, uint32_t banTime = 86400);
    bool isPeerBanned(const std::string& peerId) const;
    
private:
    // Internal methods
    void handleIncomingConnection(int clientSocket);
    void handlePeerConnection(const std::string& peerId);
    void processIncomingMessage(const NetworkMessage& message, const std::string& fromPeer);
    void managePeers();
    void maintainConnections();
    
    // Message serialization
    std::vector<uint8_t> serializeMessage(const NetworkMessage& message) const;
    NetworkMessage deserializeMessage(const std::vector<uint8_t>& data) const;
    uint32_t calculateChecksum(const std::vector<uint8_t>& data) const;
    
    // Protocol handlers
    void handleVersionMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handleVerackMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handlePingMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handlePongMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handleBlockMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handleTransactionMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handleGetBlocksMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handleGetDataMessage(const NetworkMessage& message, const std::string& fromPeer);
    void handleAddrMessage(const NetworkMessage& message, const std::string& fromPeer);
    
    // Mining protocol handlers
    void handleMiningWorkRequest(const NetworkMessage& message, const std::string& fromPeer);
    void handleMiningSubmission(const NetworkMessage& message, const std::string& fromPeer);
};

// Network utilities
class NetworkUtils {
public:
    static std::string ipToString(uint32_t ip);
    static uint32_t stringToIp(const std::string& ipStr);
    static bool isValidIpAddress(const std::string& ip);
    static bool isValidPort(uint16_t port);
    static std::string generateNodeId();
    static std::vector<uint8_t> encodeVarint(uint64_t value);
    static uint64_t decodeVarint(const std::vector<uint8_t>& data, size_t& offset);
    static std::string bytesToHex(const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> hexToBytes(const std::string& hex);
};

// Mining pool server
class MiningPoolServer {
private:
    NetworkNode* networkNode;
    Blockchain* blockchain;
    uint16_t stratumPort;
    std::atomic<bool> running{false};
    std::thread serverThread;
    std::unordered_map<std::string, std::string> miners; // worker -> address mapping
    
public:
    MiningPoolServer(NetworkNode* nodeIn, Blockchain* blockchainIn, uint16_t port = 3333);
    ~MiningPoolServer();
    
    bool start();
    void stop();
    
    // Worker management
    void registerWorker(const std::string& workerName, const std::string& address);
    void removeWorker(const std::string& workerName);
    std::vector<std::string> getActiveWorkers() const;
    
    // Mining statistics
    double getHashrate(const std::string& workerName = "") const;
    uint32_t getActiveConnections() const;
    double getPoolDifficulty() const;
    
private:
    void handleStratumConnection(int clientSocket);
    void processStratumMessage(const std::string& message, int clientSocket);
    void sendStratumResponse(int clientSocket, const std::string& response);
};

// Constants
constexpr uint32_t PROTOCOL_VERSION = 70015;
constexpr uint32_t MAX_MESSAGE_SIZE = 32 * 1024 * 1024; // 32MB
constexpr uint32_t PING_INTERVAL = 30; // seconds
constexpr uint32_t PEER_TIMEOUT = 90; // seconds
constexpr uint32_t MAX_ADDR_TO_SEND = 1000;
constexpr uint32_t MAX_INV_TO_SEND = 50000;