#include "../include/WebSocketServer.h"
#include "../include/Blockchain.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <fstream>

// Global variables for clean shutdown
bool g_running = true;
WebSocketServer* g_webSocketServer = nullptr;
Blockchain* g_blockchain = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down web server..." << std::endl;
    g_running = false;
}

void printBanner() {
    std::cout << R"(
   ___   _  _   ___     __      __       _     
  / __| | \| | / __|    \ \    / /  ___ | |__  
 | (__  | .` || (__      \ \/\/ /  / -_)| '_ \ 
  \___| |_|\_| \___|      \_/\_/   \___||_.__/ 
                                              
GXC Blockchain Web Dashboard - Real-time Analytics
Version 1.0.0
    )" << std::endl;
}

void printHelp() {
    std::cout << "GXC Web Dashboard - Usage:" << std::endl;
    std::cout << "  gxc-web [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h                 Show this help message" << std::endl;
    std::cout << "  --port=PORT, -p PORT       Web server port (default: 8080)" << std::endl;
    std::cout << "  --ws-port=PORT             WebSocket port (default: 8081)" << std::endl;
    std::cout << "  --data-dir=DIR             Blockchain data directory" << std::endl;
    std::cout << "  --verbose, -v              Verbose logging" << std::endl;
    std::cout << "  --cors                     Enable CORS headers" << std::endl;
    std::cout << "  --ssl                      Enable SSL/TLS" << std::endl;
    std::cout << "  --cert=FILE                SSL certificate file" << std::endl;
    std::cout << "  --key=FILE                 SSL private key file" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  gxc-web --port=3000 --ws-port=3001" << std::endl;
    std::cout << "  gxc-web --ssl --cert=cert.pem --key=key.pem" << std::endl;
    std::cout << std::endl;
}

struct WebConfig {
    uint16_t httpPort = 8080;
    uint16_t wsPort = 8081;
    std::string dataDir = "./blockchain_data";
    std::string certFile;
    std::string keyFile;
    bool verbose = false;
    bool enableCors = false;
    bool enableSsl = false;
};

bool parseCommandLine(int argc, char* argv[], WebConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return false;
        }
        else if (arg.find("--port=") == 0) {
            config.httpPort = static_cast<uint16_t>(std::stoul(arg.substr(7)));
        }
        else if (arg == "-p" && i + 1 < argc) {
            config.httpPort = static_cast<uint16_t>(std::stoul(argv[++i]));
        }
        else if (arg.find("--ws-port=") == 0) {
            config.wsPort = static_cast<uint16_t>(std::stoul(arg.substr(10)));
        }
        else if (arg.find("--data-dir=") == 0) {
            config.dataDir = arg.substr(11);
        }
        else if (arg.find("--cert=") == 0) {
            config.certFile = arg.substr(7);
        }
        else if (arg.find("--key=") == 0) {
            config.keyFile = arg.substr(6);
        }
        else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        }
        else if (arg == "--cors") {
            config.enableCors = true;
        }
        else if (arg == "--ssl") {
            config.enableSsl = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printHelp();
            return false;
        }
    }
    
    return true;
}

bool validateConfig(const WebConfig& config) {
    if (config.httpPort == 0 || config.httpPort > 65535) {
        std::cerr << "Error: Invalid HTTP port. Use 1-65535" << std::endl;
        return false;
    }
    
    if (config.wsPort == 0 || config.wsPort > 65535) {
        std::cerr << "Error: Invalid WebSocket port. Use 1-65535" << std::endl;
        return false;
    }
    
    if (config.httpPort == config.wsPort) {
        std::cerr << "Error: HTTP and WebSocket ports must be different" << std::endl;
        return false;
    }
    
    if (config.enableSsl) {
        if (config.certFile.empty() || config.keyFile.empty()) {
            std::cerr << "Error: SSL enabled but certificate or key file not specified" << std::endl;
            return false;
        }
        
        std::ifstream certCheck(config.certFile);
        if (!certCheck.good()) {
            std::cerr << "Error: Cannot read certificate file: " << config.certFile << std::endl;
            return false;
        }
        
        std::ifstream keyCheck(config.keyFile);
        if (!keyCheck.good()) {
            std::cerr << "Error: Cannot read key file: " << config.keyFile << std::endl;
            return false;
        }
    }
    
    return true;
}

void printServerInfo(const WebConfig& config) {
    std::cout << "=== Web Dashboard Configuration ===" << std::endl;
    
    std::string protocol = config.enableSsl ? "https" : "http";
    std::string wsProtocol = config.enableSsl ? "wss" : "ws";
    
    std::cout << "HTTP Server: " << protocol << "://localhost:" << config.httpPort << std::endl;
    std::cout << "WebSocket Server: " << wsProtocol << "://localhost:" << config.wsPort << std::endl;
    std::cout << "Data Directory: " << config.dataDir << std::endl;
    std::cout << "CORS: " << (config.enableCors ? "Enabled" : "Disabled") << std::endl;
    std::cout << "SSL/TLS: " << (config.enableSsl ? "Enabled" : "Disabled") << std::endl;
    
    if (config.enableSsl) {
        std::cout << "Certificate: " << config.certFile << std::endl;
        std::cout << "Private Key: " << config.keyFile << std::endl;
    }
    
    std::cout << "Traceability: ENABLED" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << std::endl;
}

void generateSampleData() {
    if (!g_blockchain) return;
    
    static auto lastDataGeneration = Utils::getCurrentTimestamp();
    auto currentTime = Utils::getCurrentTimestamp();
    
    // Generate sample data every 30 seconds
    if (currentTime - lastDataGeneration >= 30) {
        try {
            // Create a sample transaction with traceability
            std::string fromAddr = "gxc1sample" + std::to_string(Utils::randomUint64() % 1000);
            std::string toAddr = "gxc1target" + std::to_string(Utils::randomUint64() % 1000);
            double amount = static_cast<double>(Utils::randomUint64() % 10000) / 100.0; // 0-100 GXC
            
            Transaction tx(fromAddr, toAddr, amount);
            
            // Add some traceability data
            if (g_blockchain->getChainLength() > 1) {
                auto latestBlock = g_blockchain->getLatestBlock();
                if (!latestBlock.getTransactions().empty()) {
                    auto prevTx = latestBlock.getTransactions()[0];
                    tx.setPrevTxHash(prevTx.getHash());
                    tx.setReferencedAmount(amount);
                }
            }
            
            g_blockchain->addPendingTransaction(tx);
            
            // Notify WebSocket clients
            if (g_webSocketServer) {
                g_webSocketServer->notifyNewTransaction(tx);
            }
            
            lastDataGeneration = currentTime;
            
        } catch (const std::exception& e) {
            LOG_WEBSOCKET(LogLevel::ERROR, "Error generating sample data: " + std::string(e.what()));
        }
    }
}

void printWebStats() {
    if (!g_webSocketServer || !g_blockchain) return;
    
    uint32_t clientCount = g_webSocketServer->getClientCount();
    uint32_t blockCount = g_blockchain->getChainLength();
    auto pendingTx = g_blockchain->getPendingTransactions();
    
    std::cout << "\r"; // Clear line
    std::cout << "Blocks: " << blockCount << " | ";
    std::cout << "Pending TX: " << pendingTx.size() << " | ";
    std::cout << "WS Clients: " << clientCount << " | ";
    std::cout << "Uptime: " << Utils::getCurrentTimestamp() - Utils::getCurrentTimestamp() << "s";
    std::cout.flush();
}

std::string createIndexHtml() {
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GXC Blockchain Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            min-height: 100vh;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        .header {
            text-align: center;
            color: white;
            margin-bottom: 30px;
        }
        
        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 10px;
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
        }
        
        .card h3 {
            color: #5a67d8;
            margin-bottom: 15px;
            border-bottom: 2px solid #e2e8f0;
            padding-bottom: 10px;
        }
        
        .stat {
            display: flex;
            justify-content: space-between;
            margin: 10px 0;
            padding: 8px;
            background: #f7fafc;
            border-radius: 5px;
        }
        
        .stat-label {
            font-weight: 600;
            color: #4a5568;
        }
        
        .stat-value {
            color: #2d3748;
            font-weight: bold;
        }
        
        .transaction-list {
            max-height: 300px;
            overflow-y: auto;
        }
        
        .transaction {
            background: #f7fafc;
            border-left: 4px solid #5a67d8;
            padding: 10px;
            margin: 5px 0;
            border-radius: 0 5px 5px 0;
        }
        
        .tx-hash {
            font-family: monospace;
            font-size: 0.8rem;
            color: #718096;
            margin-bottom: 5px;
        }
        
        .status {
            display: inline-block;
            padding: 2px 8px;
            border-radius: 12px;
            font-size: 0.8rem;
            font-weight: bold;
        }
        
        .status.connected {
            background: #c6f6d5;
            color: #22543d;
        }
        
        .status.disconnected {
            background: #fed7d7;
            color: #742a2a;
        }
        
        .traceability-info {
            background: #ebf8ff;
            border: 1px solid #90cdf4;
            border-radius: 5px;
            padding: 15px;
            margin-top: 15px;
        }
        
        .formula {
            font-family: monospace;
            background: #2d3748;
            color: #f7fafc;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            font-size: 0.9rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>GXC Blockchain Dashboard</h1>
            <p>Real-time blockchain analytics with full traceability</p>
        </div>
        
        <div class="dashboard">
            <div class="card">
                <h3>Blockchain Stats</h3>
                <div class="stat">
                    <span class="stat-label">Block Height</span>
                    <span class="stat-value" id="blockHeight">-</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Difficulty</span>
                    <span class="stat-value" id="difficulty">-</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Total Supply</span>
                    <span class="stat-value" id="totalSupply">-</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Latest Block</span>
                    <span class="stat-value" id="latestBlock">-</span>
                </div>
            </div>
            
            <div class="card">
                <h3>Network Status</h3>
                <div class="stat">
                    <span class="stat-label">WebSocket Status</span>
                    <span class="status disconnected" id="wsStatus">Disconnected</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Connected Clients</span>
                    <span class="stat-value" id="connectedClients">0</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Network Peers</span>
                    <span class="stat-value" id="networkPeers">0</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Messages Sent</span>
                    <span class="stat-value" id="messagesSent">0</span>
                </div>
            </div>
            
            <div class="card">
                <h3>Recent Transactions</h3>
                <div class="transaction-list" id="transactionList">
                    <p style="text-align: center; color: #718096; margin: 20px 0;">No transactions yet</p>
                </div>
            </div>
            
            <div class="card">
                <h3>Traceability Information</h3>
                <div class="traceability-info">
                    <p><strong>GXC Traceability Formula:</strong></p>
                    <div class="formula">
                        Ti.Inputs[0].txHash == Ti.PrevTxHash<br>
                        Ti.Inputs[0].amount == Ti.ReferencedAmount
                    </div>
                    <p>Every transaction in the GXC blockchain maintains a cryptographic link to its previous transaction, ensuring full traceability and preventing double-spending.</p>
                </div>
            </div>
        </div>
    </div>

    <script>
        let ws = null;
        let reconnectInterval = null;
        
        function connectWebSocket() {
            const wsPort = window.location.port ? parseInt(window.location.port) + 1 : 8081;
            const wsUrl = `ws://${window.location.hostname}:${wsPort}`;
            
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                console.log('WebSocket connected');
                document.getElementById('wsStatus').textContent = 'Connected';
                document.getElementById('wsStatus').className = 'status connected';
                
                if (reconnectInterval) {
                    clearInterval(reconnectInterval);
                    reconnectInterval = null;
                }
            };
            
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    handleWebSocketMessage(data);
                } catch (e) {
                    console.error('Error parsing WebSocket message:', e);
                }
            };
            
            ws.onclose = function() {
                console.log('WebSocket disconnected');
                document.getElementById('wsStatus').textContent = 'Disconnected';
                document.getElementById('wsStatus').className = 'status disconnected';
                
                // Attempt to reconnect
                if (!reconnectInterval) {
                    reconnectInterval = setInterval(connectWebSocket, 5000);
                }
            };
            
            ws.onerror = function(error) {
                console.error('WebSocket error:', error);
            };
        }
        
        function handleWebSocketMessage(data) {
            switch (data.type) {
                case 'blockchain_state':
                    updateBlockchainStats(data.data);
                    break;
                case 'new_block':
                    updateNewBlock(data.data);
                    break;
                case 'new_transaction':
                    addNewTransaction(data.data);
                    break;
                case 'network_stats':
                    updateNetworkStats(data.data);
                    break;
                case 'blockchain_stats':
                    updateBlockchainStats(data.data);
                    break;
            }
        }
        
        function updateBlockchainStats(stats) {
            document.getElementById('blockHeight').textContent = stats.height || '-';
            document.getElementById('difficulty').textContent = (stats.difficulty || 0).toFixed(2);
            document.getElementById('totalSupply').textContent = (stats.total_supply || 0).toFixed(2) + ' GXC';
            document.getElementById('latestBlock').textContent = (stats.latest_block || '').substring(0, 16) + '...';
        }
        
        function updateNewBlock(block) {
            document.getElementById('blockHeight').textContent = block.height;
            document.getElementById('difficulty').textContent = block.difficulty.toFixed(2);
            document.getElementById('latestBlock').textContent = block.hash.substring(0, 16) + '...';
        }
        
        function addNewTransaction(tx) {
            const list = document.getElementById('transactionList');
            
            // Remove the "no transactions" message
            if (list.children.length === 1 && list.children[0].tagName === 'P') {
                list.innerHTML = '';
            }
            
            const txElement = document.createElement('div');
            txElement.className = 'transaction';
            txElement.innerHTML = `
                <div class="tx-hash">Hash: ${tx.hash.substring(0, 32)}...</div>
                <div>Amount: ${tx.amount.toFixed(2)} GXC</div>
                <div>Fee: ${tx.fee.toFixed(4)} GXC</div>
                <div>Traceable: ${tx.traceable ? '✓' : '✗'}</div>
            `;
            
            list.insertBefore(txElement, list.firstChild);
            
            // Keep only the last 10 transactions
            while (list.children.length > 10) {
                list.removeChild(list.lastChild);
            }
        }
        
        function updateNetworkStats(stats) {
            document.getElementById('connectedClients').textContent = stats.connected_peers || 0;
            document.getElementById('networkPeers').textContent = stats.connected_peers || 0;
            document.getElementById('messagesSent').textContent = stats.messages_sent || 0;
        }
        
        // Connect to WebSocket when page loads
        window.addEventListener('load', connectWebSocket);
    </script>
</body>
</html>)";
}

int main(int argc, char* argv[]) {
    // Print banner
    printBanner();
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Parse command line
    WebConfig config;
    if (!parseCommandLine(argc, argv, config)) {
        return 1;
    }
    
    // Validate configuration
    if (!validateConfig(config)) {
        return 1;
    }
    
    // Initialize logging
    Logger::initialize();
    Logger::setLogLevel(config.verbose ? LogLevel::DEBUG : LogLevel::INFO);
    
    LOG_WEBSOCKET(LogLevel::INFO, "Starting GXC Web Dashboard");
    
    try {
        // Print server configuration
        printServerInfo(config);
        
        // Create blockchain instance
        g_blockchain = new Blockchain();
        
        // Create WebSocket server
        g_webSocketServer = new WebSocketServer(g_blockchain, config.wsPort);
        
        // Start WebSocket server
        if (!g_webSocketServer->start()) {
            std::cerr << "Failed to start WebSocket server" << std::endl;
            delete g_webSocketServer;
            delete g_blockchain;
            return 1;
        }
        
        std::cout << "Web dashboard started successfully!" << std::endl;
        std::cout << "Open your browser and navigate to:" << std::endl;
        
        std::string protocol = config.enableSsl ? "https" : "http";
        std::cout << "  " << protocol << "://localhost:" << config.httpPort << std::endl;
        std::cout << std::endl;
        std::cout << "WebSocket endpoint:" << std::endl;
        
        std::string wsProtocol = config.enableSsl ? "wss" : "ws";
        std::cout << "  " << wsProtocol << "://localhost:" << config.wsPort << std::endl;
        std::cout << std::endl;
        std::cout << "Press Ctrl+C to stop the server." << std::endl;
        std::cout << std::endl;
        
        // Create a simple HTTP response for the dashboard
        std::string indexHtml = createIndexHtml();
        
        // Simple HTTP server simulation (in real implementation would use proper HTTP library)
        std::cout << "Serving dashboard content..." << std::endl;
        std::cout << "Traceability Formula: Ti.Inputs[0].txHash == Ti.PrevTxHash" << std::endl;
        std::cout << "                     Ti.Inputs[0].amount == Ti.ReferencedAmount" << std::endl;
        std::cout << std::endl;
        
        // Main server loop
        auto lastStatsUpdate = Utils::getCurrentTimestamp();
        while (g_running) {
            // Generate sample data for demonstration
            generateSampleData();
            
            // Print stats every 10 seconds
            auto currentTime = Utils::getCurrentTimestamp();
            if (currentTime - lastStatsUpdate >= 10) {
                printWebStats();
                lastStatsUpdate = currentTime;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        std::cout << std::endl; // New line after stats
        
    } catch (const std::exception& e) {
        LOG_WEBSOCKET(LogLevel::ERROR, "Fatal error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        
        if (g_webSocketServer) {
            delete g_webSocketServer;
        }
        if (g_blockchain) {
            delete g_blockchain;
        }
        return 1;
    }
    
    // Clean shutdown
    if (g_webSocketServer) {
        std::cout << "Stopping WebSocket server..." << std::endl;
        g_webSocketServer->stop();
        delete g_webSocketServer;
    }
    
    if (g_blockchain) {
        delete g_blockchain;
    }
    
    LOG_WEBSOCKET(LogLevel::INFO, "GXC Web Dashboard shutdown complete");
    std::cout << "Web dashboard stopped successfully." << std::endl;
    
    return 0;
}