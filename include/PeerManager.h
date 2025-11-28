#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <atomic>
#include <thread>

struct PeerInfo {
    std::string address;
    uint16_t port;
    uint64_t lastSeen;
    bool connected;
    uint32_t version;
    uint32_t height;
    uint64_t bytesSent;
    uint64_t bytesReceived;
    uint32_t messagesSent;
    uint32_t messagesReceived;
    uint64_t connectTime;
    uint64_t uptime;
    
    // Initialize defaults
    PeerInfo() : port(0), lastSeen(0), connected(false), version(0), height(0),
                 bytesSent(0), bytesReceived(0), messagesSent(0), messagesReceived(0),
                 connectTime(0), uptime(0) {}
};

class PeerManager {
private:
    std::unordered_map<std::string, PeerInfo> peers;
    mutable std::mutex peersMutex;
    std::unordered_map<std::string, PeerInfo> knownPeers;
    mutable std::mutex knownPeersMutex;
    std::vector<std::pair<std::string, uint16_t>> seedNodes;
    uint16_t serverPort;
    std::atomic<bool> isRunning;
    std::thread managementThread;
    std::thread discoveryThread;
    size_t maxPeers;
    uint64_t lastPeerCleanup;
    uint64_t startTime;
    
    void managementLoop();
    void discoveryLoop();
    void updatePeerStatistics();
    bool connectToPeer(const std::string& address, uint16_t port);
    void disconnectPeer(const std::string& address, uint16_t port);
public:
    void broadcastMessage(const std::string& message);
private:
    void sendMessageToPeer(const PeerInfo& peer, const std::string& message);
    uint32_t getConnectedPeerCount() const;
public:
    void addKnownPeer(const std::string& address, uint16_t port);
private:
    std::vector<PeerInfo> getKnownPeers() const;
    void cleanupDisconnectedPeers();
    void checkPeerHealth();
    void maintainPeerConnections();
    void discoverNewPeers();
    void exchangePeerLists();
    void connectToRandomKnownPeers(uint32_t count);
    void disconnectExcessPeers(uint32_t count);
    void addDefaultPeers();
    std::string createPeerListMessage();
    
public:
    PeerManager(uint16_t port = 0);
    ~PeerManager();
    
    bool start();
    void stop();
    
    bool addPeer(const std::string& address, uint16_t port);
    bool removePeer(const std::string& address);
    std::vector<PeerInfo> getPeers() const;
    std::vector<PeerInfo> getConnectedPeers() const;
    size_t getPeerCount() const;
    void updatePeerLastSeen(const std::string& address);
    void setPeerConnected(const std::string& address, bool connected);
    void addSeedNode(const std::string& address, uint16_t port);
    std::vector<std::pair<std::string, uint16_t>> getSeedNodes() const;
};
