#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Simple P2P test - demonstrates real peer connections

class SimpleP2PNode {
private:
    int listenSocket;
    int port;
    std::vector<int> peers;
    bool running;
    
public:
    SimpleP2PNode(int p) : port(p), running(false), listenSocket(-1) {}
    
    bool start() {
        // Create socket
        listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // Bind
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
        if (bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            close(listenSocket);
            return false;
        }
        
        // Listen
        if (listen(listenSocket, 10) < 0) {
            std::cerr << "Failed to listen" << std::endl;
            close(listenSocket);
            return false;
        }
        
        running = true;
        std::cout << "✅ Node listening on port " << port << std::endl;
        return true;
    }
    
    void acceptConnections() {
        while (running) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            int clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                if (running) {
                    std::cerr << "Accept failed" << std::endl;
                }
                continue;
            }
            
            char addrStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, INET_ADDRSTRLEN);
            
            std::cout << "✅ New peer connected from " << addrStr << ":" << ntohs(clientAddr.sin_port) << std::endl;
            
            peers.push_back(clientSocket);
            
            // Handle peer in thread
            std::thread([this, clientSocket, addrStr]() {
                handlePeer(clientSocket, addrStr);
            }).detach();
        }
    }
    
    void handlePeer(int socket, const std::string& addr) {
        // Send hello
        const char* hello = "HELLO:GXC:2.0.0\n";
        send(socket, hello, strlen(hello), 0);
        
        // Receive messages
        char buffer[1024];
        while (running) {
            int received = recv(socket, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) {
                std::cout << "❌ Peer " << addr << " disconnected" << std::endl;
                break;
            }
            
            buffer[received] = '\0';
            std::cout << "📨 Received from " << addr << ": " << buffer;
            
            // Echo back
            send(socket, buffer, received, 0);
        }
        
        close(socket);
    }
    
    bool connectToPeer(const std::string& address, int peerPort) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(peerPort);
        
        if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address: " << address << std::endl;
            close(sock);
            return false;
        }
        
        if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to connect to " << address << ":" << peerPort << std::endl;
            close(sock);
            return false;
        }
        
        std::cout << "✅ Connected to peer " << address << ":" << peerPort << std::endl;
        
        peers.push_back(sock);
        
        // Handle peer in thread
        std::thread([this, sock, address]() {
            handlePeer(sock, address);
        }).detach();
        
        return true;
    }
    
    void sendToPeers(const std::string& message) {
        for (int socket : peers) {
            send(socket, message.c_str(), message.length(), 0);
        }
    }
    
    int getPeerCount() const {
        return peers.size();
    }
    
    void stop() {
        running = false;
        if (listenSocket >= 0) {
            close(listenSocket);
        }
        for (int socket : peers) {
            close(socket);
        }
    }
};

int main(int argc, char* argv[]) {
    std::cout << "===========================================================" << std::endl;
    std::cout << "GXC P2P Network Test" << std::endl;
    std::cout << "===========================================================" << std::endl;
    
    if (argc < 2) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  Node 1 (server): " << argv[0] << " 8333" << std::endl;
        std::cout << "  Node 2 (client): " << argv[0] << " 8334 127.0.0.1:8333" << std::endl;
        return 1;
    }
    
    int port = std::stoi(argv[1]);
    SimpleP2PNode node(port);
    
    if (!node.start()) {
        return 1;
    }
    
    // Start accepting connections in background
    std::thread acceptThread([&node]() {
        node.acceptConnections();
    });
    
    // If peer address provided, connect to it
    if (argc >= 3) {
        std::string peerAddr = argv[2];
        size_t colonPos = peerAddr.find(':');
        if (colonPos != std::string::npos) {
            std::string ip = peerAddr.substr(0, colonPos);
            int peerPort = std::stoi(peerAddr.substr(colonPos + 1));
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
            node.connectToPeer(ip, peerPort);
        }
    }
    
    // Main loop
    std::cout << "\nNode running. Commands:" << std::endl;
    std::cout << "  status - Show peer count" << std::endl;
    std::cout << "  send <message> - Send message to all peers" << std::endl;
    std::cout << "  quit - Exit" << std::endl;
    std::cout << std::endl;
    
    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "quit") {
            break;
        } else if (command == "status") {
            std::cout << "Connected peers: " << node.getPeerCount() << std::endl;
        } else if (command.find("send ") == 0) {
            std::string message = command.substr(5) + "\n";
            node.sendToPeers(message);
            std::cout << "Message sent to " << node.getPeerCount() << " peer(s)" << std::endl;
        }
    }
    
    std::cout << "Shutting down..." << std::endl;
    node.stop();
    acceptThread.join();
    
    return 0;
}
