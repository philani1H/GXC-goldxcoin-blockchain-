#ifndef P2P_NETWORK_H
#define P2P_NETWORK_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class Blockchain;
class Block;

struct Peer {
    int socket;
    std::string address;
    int port;
    bool inbound;
    time_t connectedAt;
    int syncedBlocks;
    bool isActive;
    
    Peer(int sock, const std::string& addr, int p, bool in)
        : socket(sock), address(addr), port(p), inbound(in), 
          connectedAt(time(nullptr)), syncedBlocks(0), isActive(true) {}
};

class P2PNetwork {
private:
    Blockchain* blockchain;
    std::vector<std::shared_ptr<Peer>> peers;
    std::mutex peersMutex;
    std::atomic<bool> running;
    std::thread listenThread;
    std::vector<std::thread> peerThreads;
    
    int listenSocket;
    int listenPort;
    int maxConnections;
    
    // Seed nodes
    std::vector<std::pair<std::string, int>> seedNodes;
    
    void listenForConnections();
    void handlePeer(std::shared_ptr<Peer> peer);
    void connectToPeer(const std::string& address, int port);
    void sendMessage(int socket, const std::string& message);
    std::string receiveMessage(int socket);
    void broadcastBlock(const Block& block);
    void requestBlocks(std::shared_ptr<Peer> peer, int fromHeight);
    
public:
    P2PNetwork(Blockchain* bc, int port = 8333, int maxConn = 125);
    ~P2PNetwork();
    
    void start();
    void stop();
    void addSeedNode(const std::string& address, int port);
    void connectToSeeds();
    std::vector<std::shared_ptr<Peer>> getPeers();
    int getPeerCount();
    void broadcastNewBlock(const Block& block);
};

#endif // P2P_NETWORK_H
