#include "../include/RPCAPI.h"
#include "../include/blockchain.h"
#include "../include/Block.h"
#include "../include/Validator.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/Network.h"
#include "../include/Config.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

RPCAPI::RPCAPI(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), network(nullptr), serverPort(port), isRunning(false) {
    registerMethods();
}

RPCAPI::RPCAPI(Blockchain* blockchain, Network* network, uint16_t port)
    : blockchain(blockchain), network(network), serverPort(port), isRunning(false) {
    
    // Register RPC methods
    registerMethods();
    
    LOG_API(LogLevel::INFO, "RPC API initialized on port " + std::to_string(port));
}

RPCAPI::~RPCAPI() {
    stop();
}

bool RPCAPI::start() {
    return start(serverPort);
}

bool RPCAPI::start(uint16_t port) {
    serverPort = port;
    if (isRunning) {
        LOG_API(LogLevel::WARNING, "RPC API is already running");
        return true;
    }
    
    LOG_API(LogLevel::INFO, "Starting RPC API server");
    
    try {
        isRunning = true;
        serverThread = std::thread(&RPCAPI::serverLoop, this);
        
        LOG_API(LogLevel::INFO, "RPC API server started successfully on port " + std::to_string(serverPort));
        return true;
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Failed to start RPC API server: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void RPCAPI::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_API(LogLevel::INFO, "Stopping RPC API server");
    
    isRunning = false;
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    LOG_API(LogLevel::INFO, "RPC API server stopped");
}

void RPCAPI::registerMethods() {
    // Blockchain information methods
    rpcMethods["getblockchaininfo"] = [this](const JsonValue& params) { return getBlockchainInfo(params); };
    rpcMethods["getbestblockhash"] = [this](const JsonValue& params) { return getBestBlockHash(params); };
    rpcMethods["getblockcount"] = [this](const JsonValue& params) { return getBlockCount(params); };
    rpcMethods["getdifficulty"] = [this](const JsonValue& params) { return getDifficulty(params); };
    rpcMethods["getblock"] = [this](const JsonValue& params) { return getBlock(params); };
    rpcMethods["getblockbynumber"] = [this](const JsonValue& params) { return getBlock(params); };
    rpcMethods["gxc_getBlockByNumber"] = [this](const JsonValue& params) { return getBlock(params); };
    rpcMethods["getlatestblock"] = [this](const JsonValue& params) { 
        JsonValue latestParams = JsonValue(JsonValue::array());
        latestParams.push_back("latest");
        if (params.size() > 0) {
            latestParams.push_back(params[0]);
        }
        return getBlock(latestParams);
    };
    rpcMethods["gxc_getLatestBlock"] = [this](const JsonValue& params) { 
        JsonValue latestParams = JsonValue(JsonValue::array());
        latestParams.push_back("latest");
        if (params.size() > 0) {
            latestParams.push_back(params[0]);
        }
        return getBlock(latestParams);
    };
    rpcMethods["getblockhash"] = [this](const JsonValue& params) { return getBlockHash(params); };
    
    // Transaction methods
    rpcMethods["getrawtransaction"] = [this](const JsonValue& params) { return getRawTransaction(params); };
    rpcMethods["sendrawtransaction"] = [this](const JsonValue& params) { return sendRawTransaction(params); };
    rpcMethods["gettransaction"] = [this](const JsonValue& params) { return getTransaction(params); };
    rpcMethods["gxc_getTransaction"] = [this](const JsonValue& params) { return getTransaction(params); };
    rpcMethods["gettx"] = [this](const JsonValue& params) { return getTransaction(params); };
    rpcMethods["listtransactions"] = [this](const JsonValue& params) { return listTransactions(params); };
    rpcMethods["getaddresstransactions"] = [this](const JsonValue& params) { return listTransactions(params); };
    rpcMethods["getaddresshistory"] = [this](const JsonValue& params) { return listTransactions(params); };
    rpcMethods["getaddresstxids"] = [this](const JsonValue& params) {
        // Return just transaction IDs for an address
        JsonValue txs = listTransactions(params);
        JsonValue txids(JsonValue::array());
        for (const auto& tx : txs) {
            if (tx.is_object() && tx.contains("hash")) {
                txids.push_back(tx["hash"]);
            }
        }
        return txids;
    };
    rpcMethods["searchrawtransactions"] = [this](const JsonValue& params) {
        // Alias for listTransactions
        return listTransactions(params);
    };
    rpcMethods["gettransactions"] = [this](const JsonValue& params) {
        // Get transactions for a block or address
        if (params.size() > 0 && params[0].is_number()) {
            // Block number
            JsonValue blockParams = JsonValue(JsonValue::array());
            blockParams.push_back(params[0]);
            if (params.size() > 1) {
                blockParams.push_back(params[1]);
            }
            JsonValue block = getBlock(blockParams);
            return block.value("transactions", JsonValue(JsonValue::array()));
        } else {
            // Address
            return listTransactions(params);
        }
    };
    rpcMethods["getblocktransactions"] = [this](const JsonValue& params) { 
        if (params.size() < 1) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block number parameter");
        }
        JsonValue blockParams = JsonValue(JsonValue::array());
        blockParams.push_back(params[0]);
        if (params.size() > 1) {
            blockParams.push_back(params[1]);
        }
        JsonValue block = getBlock(blockParams);
        return block.value("transactions", JsonValue(JsonValue::array()));
    };
    rpcMethods["getblocktxs"] = [this](const JsonValue& params) { 
        if (params.size() < 1) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block number parameter");
        }
        JsonValue blockParams = JsonValue(JsonValue::array());
        blockParams.push_back(params[0]);
        if (params.size() > 1) {
            blockParams.push_back(params[1]);
        }
        JsonValue block = getBlock(blockParams);
        return block.value("transactions", JsonValue(JsonValue::array()));
    };
    rpcMethods["gxc_getBlockTransactions"] = [this](const JsonValue& params) { 
        if (params.size() < 1) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block number parameter");
        }
        JsonValue blockParams = JsonValue(JsonValue::array());
        blockParams.push_back(params[0]);
        if (params.size() > 1) {
            blockParams.push_back(params[1]);
        }
        JsonValue block = getBlock(blockParams);
        return block.value("transactions", JsonValue(JsonValue::array()));
    };
    
    // Wallet methods
    rpcMethods["getbalance"] = [this](const JsonValue& params) { return getBalance(params); };
    rpcMethods["gxc_getBalance"] = [this](const JsonValue& params) { return getBalance(params); };
    rpcMethods["getaddressbalance"] = [this](const JsonValue& params) { return getBalance(params); };
    rpcMethods["getaccountbalance"] = [this](const JsonValue& params) { return getBalance(params); };
    
    // Transaction listing methods
    rpcMethods["listtransactions"] = [this](const JsonValue& params) { return listTransactions(params); };
    rpcMethods["getaddresstransactions"] = [this](const JsonValue& params) { return listTransactions(params); };
    rpcMethods["gettransactions"] = [this](const JsonValue& params) { return listTransactions(params); };
    rpcMethods["getaddresshistory"] = [this](const JsonValue& params) { return listTransactions(params); };
    
    // UTXO methods
    rpcMethods["listunspent"] = [this](const JsonValue& params) {
        std::string address = params.size() > 2 && params[2].is_array() && params[2].size() > 0 
                             ? params[2][0].get<std::string>() 
                             : (params.size() > 0 ? params[0].get<std::string>() : "");
        
        if (address.empty()) {
            return JsonValue(JsonValue::array());
        }
        
        JsonValue utxos(JsonValue::array());
        
        // Get real-time UTXO set from blockchain
        const auto& utxoSet = blockchain->getUtxoSet();
        uint32_t currentHeight = blockchain->getHeight();
        
        // Find all UTXOs for this address in real-time
        for (const auto& [utxoKey, output] : utxoSet) {
            if (output.address == address) {
                // Parse UTXO key to get transaction hash and output index
                // Format: "txHash_outputIndex"
                size_t underscorePos = utxoKey.find('_');
                if (underscorePos != std::string::npos) {
                    std::string txHash = utxoKey.substr(0, underscorePos);
                    uint32_t outputIndex = 0;
                    try {
                        outputIndex = std::stoul(utxoKey.substr(underscorePos + 1));
                    } catch (...) {
                        outputIndex = 0;
                    }
                    
                    // Find the transaction to get block height for confirmations
                    uint32_t blockHeight = 0;
                    for (uint32_t h = 0; h < currentHeight; h++) {
                        Block block = blockchain->getBlock(h);
                        for (const auto& tx : block.getTransactions()) {
                            if (tx.getHash() == txHash) {
                                blockHeight = h;
                                break;
                            }
                        }
                        if (blockHeight > 0) break;
                    }
                    
                    uint32_t confirmations = currentHeight > blockHeight ? currentHeight - blockHeight : 1;
                    
                    JsonValue utxo;
                    utxo["txid"] = txHash;
                    utxo["tx_hash"] = txHash;
                    utxo["vout"] = outputIndex;
                    utxo["output_index"] = outputIndex;
                    utxo["address"] = address;
                    utxo["amount"] = output.amount;
                    utxo["value"] = output.amount;
                    utxo["scriptPubKey"] = output.script;
                    utxo["script"] = output.script;
                    utxo["confirmations"] = confirmations;
                    utxo["block_height"] = blockHeight;
                    utxo["spendable"] = true;
                    utxo["solvable"] = true;
                    utxos.push_back(utxo);
                }
            }
        }
        
        double balance = blockchain->getBalance(address);
        LOG_API(LogLevel::INFO, "listunspent for " + address.substr(0, 16) + "...: " + 
                std::to_string(utxos.size()) + " UTXOs, balance: " + std::to_string(balance) + " GXC (REAL-TIME)");
        
        return utxos;
    };
    // listunspent is already registered above with full UTXO access
    rpcMethods["getnewaddress"] = [this](const JsonValue& params) { return getNewAddress(params); };
    rpcMethods["sendtoaddress"] = [this](const JsonValue& params) { return sendToAddress(params); };
    rpcMethods["estimatefee"] = [this](const JsonValue& params) { return estimateFee(params); };
    rpcMethods["estimateFee"] = [this](const JsonValue& params) { return estimateFee(params); };
    rpcMethods["gxc_estimateFee"] = [this](const JsonValue& params) { return estimateFee(params); };
    rpcMethods["validateaddress"] = [this](const JsonValue& params) { return validateAddress(params); };
    rpcMethods["listaccounts"] = [this](const JsonValue& params) { return listAccounts(params); };
    
    // Mining methods
    rpcMethods["getmininginfo"] = [this](const JsonValue& params) { return getMiningInfo(params); };
    rpcMethods["getnetworkhashps"] = [this](const JsonValue& params) { return getNetworkHashPS(params); };
    rpcMethods["submitblock"] = [this](const JsonValue& params) { return submitBlock(params); };
    rpcMethods["gxc_submitBlock"] = [this](const JsonValue& params) { return submitBlock(params); };
    rpcMethods["getblocktemplate"] = [this](const JsonValue& params) { return getBlockTemplate(params); };
    rpcMethods["gxc_getBlockTemplate"] = [this](const JsonValue& params) { return getBlockTemplate(params); };
    
    // PoS block creation methods
    rpcMethods["createposblock"] = [this](const JsonValue& params) { return createPoSBlock(params); };
    rpcMethods["gxc_createPoSBlock"] = [this](const JsonValue& params) { return createPoSBlock(params); };
    rpcMethods["submitposblock"] = [this](const JsonValue& params) { return submitPoSBlock(params); };
    rpcMethods["gxc_submitPoSBlock"] = [this](const JsonValue& params) { return submitPoSBlock(params); };
    
    // Staking and Validator methods
    rpcMethods["registervalidator"] = [this](const JsonValue& params) { return registerValidator(params); };
    rpcMethods["gxc_registerValidator"] = [this](const JsonValue& params) { return registerValidator(params); };
    rpcMethods["stake"] = [this](const JsonValue& params) { return stake(params); };
    rpcMethods["gxc_stake"] = [this](const JsonValue& params) { return stake(params); };
    rpcMethods["unstake"] = [this](const JsonValue& params) { return unstake(params); };
    rpcMethods["gxc_unstake"] = [this](const JsonValue& params) { return unstake(params); };
    rpcMethods["getvalidators"] = [this](const JsonValue& params) { return getValidators(params); };
    rpcMethods["gxc_getValidators"] = [this](const JsonValue& params) { return getValidators(params); };
    rpcMethods["getvalidatorinfo"] = [this](const JsonValue& params) { return getValidatorInfo(params); };
    rpcMethods["gxc_getValidatorInfo"] = [this](const JsonValue& params) { return getValidatorInfo(params); };
    rpcMethods["addstake"] = [this](const JsonValue& params) { return addStake(params); };
    rpcMethods["gxc_addStake"] = [this](const JsonValue& params) { return addStake(params); };
    
    // Traceability methods
    rpcMethods["tracetransaction"] = [this](const JsonValue& params) {
        if (params.size() < 1) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
        }
        std::string txHash = params[0].get<std::string>();
        std::vector<std::string> lineage = blockchain->traceTransactionLineage(txHash);
        JsonValue result(JsonValue::array());
        for (const auto& hash : lineage) {
            result.push_back(hash);
        }
        return result;
    };
    rpcMethods["verifytransactionlineage"] = [this](const JsonValue& params) {
        if (params.size() < 1) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
        }
        std::string txHash = params[0].get<std::string>();
        return blockchain->verifyTransactionLineage(txHash);
    };
    rpcMethods["gettransactionchain"] = [this](const JsonValue& params) {
        if (params.size() < 1) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address parameter");
        }
        std::string address = params[0].get<std::string>();
        uint32_t depth = params.size() > 1 ? params[1].get<uint32_t>() : 100;
        std::vector<std::string> chain = blockchain->getTransactionChain(address, depth);
        JsonValue result(JsonValue::array());
        for (const auto& hash : chain) {
            result.push_back(hash);
        }
        return result;
    };
    
    // Network methods
    rpcMethods["getpeerinfo"] = [this](const JsonValue& params) { return getPeerInfo(params); };
    rpcMethods["getconnectioncount"] = [this](const JsonValue& params) { return getConnectionCount(params); };
    rpcMethods["addnode"] = [this](const JsonValue& params) { return addNode(params); };
    rpcMethods["disconnectnode"] = [this](const JsonValue& params) { return disconnectNode(params); };
    
    // Utility methods
    rpcMethods["help"] = [this](const JsonValue& params) { return help(params); };
    rpcMethods["stop"] = [this](const JsonValue& params) { return stopNode(params); };
    rpcMethods["getinfo"] = [this](const JsonValue& params) { return getInfo(params); };
    
    LOG_API(LogLevel::INFO, "Registered " + std::to_string(rpcMethods.size()) + " RPC methods");
}

std::string RPCAPI::processRequest(const std::string& request) {
    try {
        JsonValue requestJson;
        try {
            requestJson = nlohmann::json::parse(request);
        } catch (const std::exception& e) {
            return createErrorResponse(-32700, "Parse error: " + std::string(e.what()), JsonValue());
        }
        
        // Validate JSON-RPC structure
        if (!requestJson.contains("jsonrpc") || requestJson["jsonrpc"] != "2.0") {
            JsonValue id = requestJson.contains("id") ? requestJson["id"] : JsonValue();
            return createErrorResponse(-32600, "Invalid Request: missing or invalid jsonrpc field", id);
        }
        
        if (!requestJson.contains("method") || !requestJson["method"].is_string()) {
            return createErrorResponse(-32600, "Invalid Request: missing or invalid method field", 
                                     requestJson.value("id", JsonValue()));
        }
        
        std::string method = requestJson["method"].get<std::string>();
        JsonValue params = requestJson.value("params", JsonValue::array());
        JsonValue id = requestJson.value("id", JsonValue());
        
        LOG_API(LogLevel::DEBUG, "Processing RPC request: " + method);
        
        // Find and execute the method
        auto it = rpcMethods.find(method);
        if (it == rpcMethods.end()) {
            return createErrorResponse(-32601, "Method not found: " + method, id);
        }
        
        try {
            JsonValue result = it->second(params);
            return createSuccessResponse(result, id);
        } catch (const RPCException& e) {
            return createErrorResponse(e.getCode(), e.what(), id);
        } catch (const std::exception& e) {
            return createErrorResponse(-32603, "Internal error: " + std::string(e.what()), id);
        }
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error processing RPC request: " + std::string(e.what()));
        return createErrorResponse(-32603, "Internal error", JsonValue());
    }
    
    return createErrorResponse(-32603, "Unexpected error", JsonValue());
}

std::string RPCAPI::createSuccessResponse(const JsonValue& result, const JsonValue& id) {
    JsonValue response;
    response["jsonrpc"] = "2.0";
    response["result"] = result;
    response["id"] = id;
    
    return response.dump();
}

std::string RPCAPI::createErrorResponse(int code, const std::string& message, const JsonValue& id) {
    JsonValue error;
    error["code"] = code;
    error["message"] = message;
    
    JsonValue response;
    response["jsonrpc"] = "2.0";
    response["error"] = error;
    response["id"] = id;
    
    return response.dump();
}

void RPCAPI::serverLoop() {
    LOG_API(LogLevel::INFO, "RPC server loop started");
    
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        LOG_API(LogLevel::ERROR, "Failed to create socket");
        return;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_API(LogLevel::ERROR, "Failed to set socket options");
        close(serverSocket);
        return;
    }
    
    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPort);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_API(LogLevel::ERROR, "Failed to bind socket to port " + std::to_string(serverPort));
        close(serverSocket);
        return;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        LOG_API(LogLevel::ERROR, "Failed to listen on socket");
        close(serverSocket);
        return;
    }
    
    LOG_API(LogLevel::INFO, "RPC server listening on port " + std::to_string(serverPort));
    
    // Set socket to non-blocking mode
    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
    
    while (isRunning) {
        try {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    // No pending connections, sleep briefly
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                } else {
                    LOG_API(LogLevel::ERROR, "Failed to accept connection");
                    continue;
                }
            }
            
            // Handle client in a separate thread
            std::thread([this, clientSocket]() {
                handleClient(clientSocket);
            }).detach();
            
        } catch (const std::exception& e) {
            LOG_API(LogLevel::ERROR, "Error in RPC server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    close(serverSocket);
    LOG_API(LogLevel::INFO, "RPC server loop stopped");
}

void RPCAPI::handleClient(int clientSocket) {
    try {
        // Read HTTP request
        char buffer[8192];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        
        buffer[bytesRead] = '\0';
        std::string request(buffer);
        
        // Parse HTTP request
        std::string method, path, httpVersion;
        std::istringstream requestStream(request);
        requestStream >> method >> path >> httpVersion;
        
        std::string response;
        
        // Handle GET requests (health checks)
        if (method == "GET") {
            if (path == "/" || path == "/health" || path == "/ping") {
                // Health check endpoint
                JsonValue health;
                health["status"] = "ok";
                health["service"] = "GXC Blockchain Node";
                health["height"] = blockchain->getHeight();
                health["difficulty"] = blockchain->getDifficulty();
                health["timestamp"] = Utils::getCurrentTimestamp();
                
                std::string body = health.dump();
                
                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: application/json\r\n";
                response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
                response += "Access-Control-Allow-Origin: *\r\n";
                response += "Connection: close\r\n";
                response += "\r\n";
                response += body;
            } else {
                // 404 for other GET requests
                std::string body = "{\"error\":\"Not found\"}";
                response = "HTTP/1.1 404 Not Found\r\n";
                response += "Content-Type: application/json\r\n";
                response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
                response += "Connection: close\r\n";
                response += "\r\n";
                response += body;
            }
        }
        // Handle POST requests (RPC calls)
        else if (method == "POST") {
            // Find the body (after \r\n\r\n)
            size_t bodyStart = request.find("\r\n\r\n");
            std::string body;
            
            if (bodyStart != std::string::npos) {
                body = request.substr(bodyStart + 4);
            }
            
            // Process RPC request
            std::string rpcResponse = processRequest(body);
            
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(rpcResponse.length()) + "\r\n";
            response += "Access-Control-Allow-Origin: *\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += rpcResponse;
        }
        // Handle OPTIONS requests (CORS preflight)
        else if (method == "OPTIONS") {
            response = "HTTP/1.1 200 OK\r\n";
            response += "Access-Control-Allow-Origin: *\r\n";
            response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
            response += "Access-Control-Allow-Headers: Content-Type\r\n";
            response += "Content-Length: 0\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
        }
        else {
            // 405 Method Not Allowed
            std::string body = "{\"error\":\"Method not allowed\"}";
            response = "HTTP/1.1 405 Method Not Allowed\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += body;
        }
        
        // Send response
        send(clientSocket, response.c_str(), response.length(), 0);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error handling client: " + std::string(e.what()));
    }
    
    close(clientSocket);
}

void RPCAPI::processRequests() {
    // RPC requests are processed in the serverLoop thread
    // This method can be used for synchronous processing if needed
    // For now, it's a no-op since serverLoop handles requests asynchronously
}

// Blockchain information methods
JsonValue RPCAPI::getBlockchainInfo(const JsonValue& params) {
    JsonValue result;
    
    // Get REAL-TIME data from blockchain (no caching)
    uint32_t currentHeight = blockchain->getHeight();
    Block latestBlock = blockchain->getLatestBlock();
    double blockReward = calculateBlockReward(currentHeight);
    
    result["chain"] = Config::isTestnet() ? "test" : "main";
    result["blocks"] = static_cast<uint64_t>(currentHeight);
    result["height"] = static_cast<uint64_t>(currentHeight);
    result["bestblockhash"] = latestBlock.getHash();
    result["difficulty"] = blockchain->getDifficulty();
    result["mediantime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["verificationprogress"] = 1.0;
    result["initialblockdownload"] = false;
    result["chainwork"] = "0000000000000000000000000000000000000000000000000000000000000000";
    result["size_on_disk"] = 0; // Would calculate actual size
    result["pruned"] = false;
    result["block_reward"] = blockReward; // Current block reward
    result["reward"] = blockReward; // Alternative field name
    
    return result;
}

JsonValue RPCAPI::getBestBlockHash(const JsonValue& params) {
    // REAL-TIME: Returns hash of latest block (always current)
    return blockchain->getLatestBlock().getHash();
}

JsonValue RPCAPI::getBlockCount(const JsonValue& params) {
    // REAL-TIME: Returns current blockchain height (always up-to-date)
    return static_cast<uint64_t>(blockchain->getHeight());
}

JsonValue RPCAPI::getDifficulty(const JsonValue& params) {
    // REAL-TIME: Returns current network difficulty (always up-to-date)
    return blockchain->getDifficulty();
}

JsonValue RPCAPI::getBlock(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block hash/height parameter");
    }
    
    bool verbose = params.size() > 1 ? params[1].get<bool>() : true;
    uint32_t verbosity = params.size() > 1 && params[1].is_number() ? params[1].get<uint32_t>() : (verbose ? 2 : 0);
    
    // REAL-TIME: Get block data directly from blockchain (no caching)
    Block block;
    uint32_t height = 0;
    
    // Handle different parameter types: hash, height (int/string), "latest", -1
    if (params[0].is_string()) {
        std::string param = params[0].get<std::string>();
        if (param == "latest" || param == "best") {
            block = blockchain->getLatestBlock();
            height = blockchain->getHeight() - 1;
        } else if (param == "-1") {
            block = blockchain->getLatestBlock();
            height = blockchain->getHeight() - 1;
        } else {
            // Try as hash first
            block = blockchain->getBlock(param);
            if (block.getHash().empty()) {
                // Try as height
                try {
                    height = std::stoul(param);
                    block = blockchain->getBlock(height);
                } catch (...) {
                    throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid block hash or height");
                }
            } else {
                height = block.getIndex();
            }
        }
    } else if (params[0].is_number()) {
        int64_t paramNum = params[0].get<int64_t>();
        if (paramNum == -1) {
            block = blockchain->getLatestBlock();
            height = blockchain->getHeight() - 1;
        } else {
            height = static_cast<uint32_t>(paramNum);
            block = blockchain->getBlock(height);
        }
    } else {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid block parameter type");
    }
    
    if (block.getHash().empty()) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block not found");
    }
    
    uint32_t currentHeight = blockchain->getHeight();
    uint32_t confirmations = currentHeight > height ? currentHeight - height : 1;
    
    // Build complete block response with ALL fields explorer needs
    JsonValue result;
    result["hash"] = block.getHash();
    result["block_hash"] = block.getHash();
    result["height"] = height;
    result["number"] = height;
    result["block_number"] = height;
    result["confirmations"] = confirmations;
    result["version"] = 1;
    result["merkleroot"] = block.getMerkleRoot();
    result["merkle_root"] = block.getMerkleRoot();
    result["time"] = static_cast<uint64_t>(block.getTimestamp());
    result["timestamp"] = static_cast<uint64_t>(block.getTimestamp());
    result["nonce"] = static_cast<uint64_t>(block.getNonce());
    result["bits"] = "1d00ffff";
    result["difficulty"] = block.getDifficulty();
    result["previousblockhash"] = block.getPreviousHash();
    result["prev_hash"] = block.getPreviousHash();
    result["parent_hash"] = block.getPreviousHash();
    result["miner"] = block.getMinerAddress();
    result["miner_address"] = block.getMinerAddress();
    result["block_reward"] = block.getBlockReward();
    result["reward"] = block.getBlockReward();
    
    // Calculate block size (approximate: hash + merkle root + timestamp + nonce + transactions)
    uint32_t estimatedSize = 64 + 64 + 8 + 8 + (static_cast<uint32_t>(block.getTransactions().size()) * 256);
    result["size"] = estimatedSize;
    
    result["gas_used"] = 0;
    result["gas_limit"] = 0;
    
    // Calculate total difficulty (sum of all block difficulties up to this point)
    // For now, use current difficulty as approximation
    double totalDifficulty = block.getDifficulty() * (height + 1);
    result["total_difficulty"] = totalDifficulty;
    result["totalDifficulty"] = totalDifficulty;
    
    // Block type/consensus
    std::string consensusType = "pow";
    if (block.getBlockType() == BlockType::POW_SHA256) {
        consensusType = "pow_sha256";
    } else if (block.getBlockType() == BlockType::POW_ETHASH) {
        consensusType = "pow_ethash";
    } else if (block.getBlockType() == BlockType::POS) {
        consensusType = "pos";
    }
    result["consensus_type"] = consensusType;
    result["consensusType"] = consensusType;
    result["block_type"] = consensusType;
    
    // PoS and advanced fields
    result["validator_signature"] = block.getValidatorSignature();
    result["validatorSignature"] = block.getValidatorSignature();
    result["fee_burn_rate"] = block.getFeeBurnRate();
    result["feeBurnRate"] = block.getFeeBurnRate();
    result["pop_reference"] = block.getPopReference();
    result["popReference"] = block.getPopReference();
    
    // Get all transactions with full data if verbose
    JsonValue transactions = JsonValue(JsonValue::array());
    if (verbose || verbosity >= 1) {
        for (const auto& tx : block.getTransactions()) {
            JsonValue txJson = transactionToJson(tx, height, block.getHash());
            transactions.push_back(txJson);
        }
    } else {
        // Just transaction hashes
        for (const auto& tx : block.getTransactions()) {
            transactions.push_back(tx.getHash());
        }
    }
    
    result["transactions"] = transactions;
    result["tx"] = transactions; // Alternative field name
    result["transaction_count"] = static_cast<uint32_t>(block.getTransactions().size());
    result["tx_count"] = static_cast<uint32_t>(block.getTransactions().size());
    result["nTx"] = static_cast<uint32_t>(block.getTransactions().size());
    
    return result;
}

// Helper function to convert Transaction to JSON
JsonValue RPCAPI::transactionToJson(const Transaction& tx, uint32_t blockHeight, const std::string& blockHash) {
    JsonValue txJson;
    
    txJson["hash"] = tx.getHash();
    txJson["tx_hash"] = tx.getHash();
    txJson["txid"] = tx.getHash();
    txJson["transactionHash"] = tx.getHash();
    
    txJson["block_number"] = blockHeight;
    txJson["blockNumber"] = blockHeight;
    txJson["blockheight"] = blockHeight;
    txJson["blockHeight"] = blockHeight;
    txJson["blockhash"] = blockHash;
    txJson["block_hash"] = blockHash;
    
    txJson["is_coinbase"] = tx.isCoinbaseTransaction();
    txJson["isCoinbase"] = tx.isCoinbaseTransaction();
    txJson["coinbase"] = tx.isCoinbaseTransaction();
    
    // From/To addresses
    if (tx.isCoinbaseTransaction()) {
        txJson["from"] = "coinbase";
        txJson["from_address"] = "coinbase";
        txJson["fromAddress"] = "coinbase";
    } else {
        txJson["from"] = tx.getSenderAddress();
        txJson["from_address"] = tx.getSenderAddress();
        txJson["fromAddress"] = tx.getSenderAddress();
    }
    
    txJson["to"] = tx.getReceiverAddress();
    txJson["to_address"] = tx.getReceiverAddress();
    txJson["toAddress"] = tx.getReceiverAddress();
    
    // Amount/value
    double totalOutput = 0.0;
    for (const auto& output : tx.getOutputs()) {
        totalOutput += output.amount;
    }
    txJson["value"] = totalOutput;
    txJson["amount"] = totalOutput;
    
    txJson["fee"] = tx.getFee();
    txJson["gas_price"] = 0.0;
    txJson["gasPrice"] = 0.0;
    txJson["gas_used"] = 0;
    txJson["gasUsed"] = 0;
    txJson["status"] = "success";
    txJson["timestamp"] = static_cast<uint64_t>(tx.getTimestamp());
    txJson["time"] = static_cast<uint64_t>(tx.getTimestamp());
    txJson["nonce"] = tx.getNonce();
    txJson["input_data"] = "";
    txJson["input"] = "";
    txJson["data"] = "";
    
    // Traceability fields (GXC specific)
    txJson["prev_tx_hash"] = tx.getPrevTxHash();
    txJson["previousTxHash"] = tx.getPrevTxHash();
    txJson["referenced_amount"] = tx.getReferencedAmount();
    txJson["referencedAmount"] = tx.getReferencedAmount();
    
    // Traceability validation
    bool traceabilityValid = tx.isTraceabilityValid();
    txJson["traceability_valid"] = traceabilityValid;
    txJson["traceabilityValid"] = traceabilityValid;
    
    // Transaction metadata
    txJson["memo"] = tx.getMemo();
    txJson["lock_time"] = tx.getLockTime();
    txJson["lockTime"] = tx.getLockTime();
    
    // Gold-backed and PoP fields
    txJson["is_gold_backed"] = tx.isGoldBackedTransaction();
    txJson["isGoldBacked"] = tx.isGoldBackedTransaction();
    txJson["pop_reference"] = tx.getPopReference();
    txJson["popReference"] = tx.getPopReference();
    
    // Signature (from first input if available)
    std::string signature = "";
    if (!tx.getInputs().empty() && !tx.getInputs()[0].signature.empty()) {
        signature = tx.getInputs()[0].signature;
    }
    txJson["signature"] = signature;
    
    // Contract address (empty for regular transactions)
    txJson["contract_address"] = "";
    txJson["contractAddress"] = "";
    
    // Transaction type (more detailed) - update the base type field too
    std::string txType = tx.isCoinbaseTransaction() ? "coinbase" : 
                        (tx.isGoldBackedTransaction() ? "gold_backed" : "transfer");
    txJson["tx_type"] = txType;
    txJson["transaction_type"] = txType;
    txJson["type"] = txType; // Update the main type field with detailed type
    
    // Inputs
    JsonValue inputs = JsonValue(JsonValue::array());
    for (const auto& input : tx.getInputs()) {
        JsonValue inputJson;
        inputJson["txHash"] = input.txHash;
        inputJson["prev_tx_hash"] = input.txHash;
        inputJson["outputIndex"] = input.outputIndex;
        inputJson["output_index"] = input.outputIndex;
        inputJson["amount"] = input.amount;
        inputJson["signature"] = input.signature;
        inputJson["publicKey"] = input.publicKey;
        inputs.push_back(inputJson);
    }
    txJson["inputs"] = inputs;
    txJson["vin"] = inputs;
    
    // Outputs
    JsonValue outputs = JsonValue(JsonValue::array());
    for (const auto& output : tx.getOutputs()) {
        JsonValue outputJson;
        outputJson["address"] = output.address;
        outputJson["amount"] = output.amount;
        outputJson["value"] = output.amount;
        outputJson["script"] = output.script;
        outputs.push_back(outputJson);
    }
    txJson["outputs"] = outputs;
    txJson["vout"] = outputs;
    
    return txJson;
}

JsonValue RPCAPI::getBlockHash(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block height parameter");
    }
    
    uint32_t height = params[0].get<uint64_t>();
    
    if (height >= blockchain->getHeight()) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block height out of range");
    }
    
    Block block = blockchain->getBlock(height);
    if (block.getHash().empty()) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block not found");
    }
    
    return block.getHash();
}

// Transaction methods
JsonValue RPCAPI::getRawTransaction(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
    }
    
    std::string txHash = params[0].get<std::string>();
    bool verbose = params.size() > 1 ? params[1].get<bool>() : false;
    
    // Use getTransaction to find the transaction
    if (verbose) {
        try {
            JsonValue result = getTransaction(params);
            result["hex"] = ""; // Would serialize to hex
            result["size"] = 250; // Would calculate actual size
            result["vsize"] = 250;
            result["weight"] = 1000;
            result["version"] = 1;
            result["locktime"] = 0;
            return result;
        } catch (...) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Transaction not found");
        }
    } else {
        return "raw_hex_transaction_data";
    }
}

JsonValue RPCAPI::sendRawTransaction(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing raw transaction data parameter");
    }
    
    std::string rawTx = params[0].get<std::string>();
    
    // In a real implementation, would decode and broadcast the transaction
    LOG_API(LogLevel::INFO, "Broadcasting raw transaction");
    
    return "transaction_hash";
}

JsonValue RPCAPI::getTransaction(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
    }
    
    std::string txHash = params[0].get<std::string>();
    bool verbose = params.size() > 1 ? params[1].get<bool>() : true;
    
    // REAL-TIME: Search for transaction in all blocks (no caching)
    // Searches from latest to earliest for fastest results
    uint32_t currentHeight = blockchain->getHeight();
    Transaction foundTx;
    uint32_t blockHeight = 0;
    std::string blockHash = "";
    bool found = false;
    
    // Search from latest to earliest
    for (uint32_t h = currentHeight; h > 0; h--) {
        Block block = blockchain->getBlock(h - 1);
        if (block.getHash().empty()) continue;
        
        for (const auto& tx : block.getTransactions()) {
            if (tx.getHash() == txHash) {
                foundTx = tx;
                blockHeight = h - 1;
                blockHash = block.getHash();
                found = true;
                break;
            }
        }
        if (found) break;
    }
    
    if (!found) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Transaction not found");
    }
    
    // Build complete transaction response
    JsonValue result = transactionToJson(foundTx, blockHeight, blockHash);
    
    // Add additional fields
    uint32_t confirmations = currentHeight > blockHeight ? currentHeight - blockHeight : 1;
    result["confirmations"] = confirmations;
    result["blockindex"] = blockHeight;
    result["blocktime"] = static_cast<uint64_t>(foundTx.getTimestamp());
    result["timereceived"] = static_cast<uint64_t>(foundTx.getTimestamp());
    result["hex"] = ""; // Would serialize transaction
    
    if (verbose) {
        result["details"] = JsonValue(JsonValue::array());
    }
    
    return result;
}

JsonValue RPCAPI::listTransactions(const JsonValue& params) {
    std::string address = params.size() > 0 ? params[0].get<std::string>() : "";
    int count = params.size() > 1 ? params[1].get<int>() : 10;
    int skip = params.size() > 2 ? params[2].get<int>() : 0;
    
    JsonValue result(JsonValue::array());
    
    // REAL-TIME: Get all transactions for address from blockchain (no caching)
    // Searches all blocks in real-time to find transactions involving this address
    uint32_t currentHeight = blockchain->getHeight();
    std::vector<JsonValue> allTxs;
    
    // Search all blocks for transactions involving this address
    for (uint32_t h = 0; h < currentHeight; h++) {
        Block block = blockchain->getBlock(h);
        if (block.getHash().empty()) continue;
        
        for (const auto& tx : block.getTransactions()) {
            bool involvesAddress = false;
            
            // Check if address is in outputs (receiving)
            for (const auto& output : tx.getOutputs()) {
                if (output.address == address) {
                    involvesAddress = true;
                    break;
                }
            }
            
            // Check if address is in inputs (sending)
            if (!involvesAddress) {
                // For coinbase, check if miner matches
                if (tx.isCoinbaseTransaction() && tx.getReceiverAddress() == address) {
                    involvesAddress = true;
                } else {
                    // For regular transactions, check sender
                    if (tx.getSenderAddress() == address) {
                        involvesAddress = true;
                    }
                }
            }
            
            if (involvesAddress) {
                JsonValue txJson = transactionToJson(tx, h, block.getHash());
                uint32_t confirmations = currentHeight > h ? currentHeight - h : 1;
                txJson["confirmations"] = confirmations;
                allTxs.push_back(txJson);
            }
        }
    }
    
    // Sort by block height (newest first) and apply pagination
    std::sort(allTxs.begin(), allTxs.end(), [](const JsonValue& a, const JsonValue& b) {
        uint32_t heightA = a.value("block_number", 0);
        uint32_t heightB = b.value("block_number", 0);
        return heightA > heightB;
    });
    
    // Apply skip and count
    int start = skip;
    int end = std::min(start + count, static_cast<int>(allTxs.size()));
    for (int i = start; i < end; i++) {
        result.push_back(allTxs[i]);
    }
    
    return result;
}

// Wallet methods
JsonValue RPCAPI::getBalance(const JsonValue& params) {
    std::string address = params.size() > 0 ? params[0].get<std::string>() : "";
    
    if (address.empty()) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address parameter");
    }
    
    // REAL-TIME: Get balance from blockchain using current UTXO set (no caching)
    // Balance is calculated fresh from all unspent transaction outputs
    double balance = blockchain->getBalance(address);
    
    LOG_API(LogLevel::DEBUG, "getBalance(" + address.substr(0, 16) + "...): " + 
            std::to_string(balance) + " GXC (REAL-TIME)");
    
    return balance;
}

JsonValue RPCAPI::getNewAddress(const JsonValue& params) {
    std::string account = params.size() > 0 ? params[0].get<std::string>() : "";
    
    // In a real implementation, would generate a new address
    return "GXC" + Utils::randomString(30);
}

JsonValue RPCAPI::sendToAddress(const JsonValue& params) {
    if (params.size() < 2) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address or amount parameter");
    }
    
    std::string address = params[0].get<std::string>();
    double amount = params[1].get<double>();
    std::string comment = params.size() > 2 ? params[2].get<std::string>() : "";
    std::string commentTo = params.size() > 3 ? params[3].get<std::string>() : "";
    
    if (amount <= 0) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid amount");
    }
    
    // Calculate recommended fee (dynamic fees)
    double recommendedFee = blockchain->calculateRecommendedFee();
    
    // In a real implementation, would create and send transaction with fee
    LOG_API(LogLevel::INFO, "Sending " + std::to_string(amount) + " GXC to " + address + 
            " (recommended fee: " + std::to_string(recommendedFee) + " GXC)");
    
    return "transaction_hash";
}

JsonValue RPCAPI::estimateFee(const JsonValue& params) {
    // Get recommended fee from blockchain
    double recommendedFee = blockchain->calculateRecommendedFee();
    
    // Get pending transaction count for context
    auto pendingTxs = blockchain->getPendingTransactions(1000);
    size_t pendingCount = pendingTxs.size();
    
    // Get configuration values for response
    double baseFee = Config::getDouble("base_transaction_fee", 0.001);
    double maxFee = Config::getDouble("max_transaction_fee", 0.01);
    bool dynamicFeesEnabled = Config::getBool("enable_dynamic_fees", true);
    
    // Build response
    JsonValue result;
    result["fee"] = recommendedFee;
    result["recommended_fee"] = recommendedFee;
    result["base_fee"] = baseFee;
    result["max_fee"] = maxFee;
    result["pending_transactions"] = static_cast<uint64_t>(pendingCount);
    result["dynamic_fees_enabled"] = dynamicFeesEnabled;
    
    // Fee tiers for reference
    JsonValue tiers;
    tiers["low"] = baseFee * Config::getDouble("fee_low_multiplier", 1.0);
    tiers["medium"] = baseFee * Config::getDouble("fee_medium_multiplier", 1.5);
    tiers["high"] = baseFee * Config::getDouble("fee_high_multiplier", 2.0);
    tiers["very_high"] = baseFee * Config::getDouble("fee_very_high_multiplier", 3.0);
    result["fee_tiers"] = tiers;
    
    // Current tier
    std::string currentTier = "low";
    if (pendingCount >= Config::getInt("fee_high_threshold", 100)) {
        currentTier = "very_high";
    } else if (pendingCount >= Config::getInt("fee_medium_threshold", 50)) {
        currentTier = "high";
    } else if (pendingCount >= Config::getInt("fee_low_threshold", 10)) {
        currentTier = "medium";
    }
    result["current_tier"] = currentTier;
    
    LOG_API(LogLevel::DEBUG, "Fee estimation: " + std::to_string(recommendedFee) + 
            " GXC (pending: " + std::to_string(pendingCount) + ", tier: " + currentTier + ")");
    
    return result;
}

JsonValue RPCAPI::validateAddress(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address parameter");
    }
    
    std::string address = params[0].get<std::string>();
    
    JsonValue result;
    result["isvalid"] = Utils::isValidAddress(address);
    
    if (result["isvalid"].get<bool>()) {
        result["address"] = address;
        result["ismine"] = false; // Would check if address belongs to wallet
        result["iswatchonly"] = false;
        result["isscript"] = false;
    }
    
    return result;
}

JsonValue RPCAPI::listAccounts(const JsonValue& params) {
    int minConfirms = params.size() > 0 ? params[0].get<int>() : 1;
    
    JsonValue result;
    result[""] = 100.0; // Default account balance
    result["mining"] = 50.0; // Mining account balance
    
    return result;
}

// Mining methods
JsonValue RPCAPI::getMiningInfo(const JsonValue& params) {
    JsonValue result;
    
    result["blocks"] = static_cast<uint64_t>(blockchain->getHeight());
    result["currentblocksize"] = 1000;
    result["currentblocktx"] = 1;
    result["difficulty"] = blockchain->getDifficulty();
    result["networkhashps"] = 1000000.0; // Example hashrate
    result["pooledtx"] = 0;
    result["testnet"] = false;
    result["generate"] = false; // Would check if mining is enabled
    result["genproclimit"] = 1;
    result["hashespersec"] = 0;
    
    return result;
}

JsonValue RPCAPI::getNetworkHashPS(const JsonValue& params) {
    int blocks = params.size() > 0 ? params[0].get<int>() : 120;
    int height = params.size() > 1 ? params[1].get<int>() : -1;
    
    // In a real implementation, would calculate actual network hashrate
    return 1000000.0; // Example hashrate in hashes per second
}

JsonValue RPCAPI::submitBlock(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block data parameter");
    }
    
    try {
        // Parse block data - can be JSON object or string
        JsonValue blockData;
        if (params[0].is_string()) {
            try {
                // nlohmann::json can be constructed from string
                std::string jsonStr = params[0].get<std::string>();
                blockData = JsonValue::parse(jsonStr);
            } catch (const std::exception& e) {
                // If parse fails, try constructing directly
                try {
                    std::string jsonStr = params[0].get<std::string>();
                    blockData = JsonValue(jsonStr);
                } catch (...) {
                    throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Failed to parse block JSON string");
                }
            }
        } else if (params[0].is_object()) {
            blockData = params[0];
        } else {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid block data format - must be JSON object or string");
        }
        
        // Extract block information
        uint32_t height = blockData.value("height", blockData.value("blockNumber", 0));
        std::string hash = blockData.value("hash", blockData.value("blockHash", ""));
        std::string prevHash = blockData.value("previousblockhash", blockData.value("prevHash", blockData.value("previousHash", "")));
        uint64_t nonce = blockData.value("nonce", 0);
        std::string minerAddress = blockData.value("miner", blockData.value("miner_address", ""));
        uint64_t timestamp = blockData.value("timestamp", Utils::getCurrentTimestamp());
        double difficulty = blockData.value("difficulty", 1.0);
        
        // Input validation for production security
        // Validate hash format (should be 64 hex characters)
        if (hash.empty()) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block hash is required");
        }
        if (hash.length() != 64) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid hash length: expected 64 hex characters, got " + std::to_string(hash.length()));
        }
        // Validate hash contains only hex characters
        for (char c : hash) {
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid hash format: contains non-hexadecimal characters");
            }
        }
        
        // Validate previous hash format
        if (!prevHash.empty() && prevHash.length() != 64) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid previous hash length: expected 64 hex characters");
        }
        
        // Validate block height (reasonable bounds)
        uint32_t currentHeight = blockchain->getHeight();
        if (height > currentHeight + 1000) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block height too far in future: " + std::to_string(height) + " (current: " + std::to_string(currentHeight) + ")");
        }
        
        // Validate timestamp (not too far in past or future)
        uint64_t currentTime = Utils::getCurrentTimestamp();
        if (timestamp > currentTime + 7200) { // 2 hours in future max
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block timestamp too far in future");
        }
        if (timestamp < currentTime - 86400) { // 24 hours in past max
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block timestamp too far in past");
        }
        
        // Validate difficulty (reasonable bounds)
        if (difficulty <= 0 || difficulty > 1000000) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid difficulty: " + std::to_string(difficulty) + " (must be between 0 and 1000000)");
        }
        
        // Validate miner address format if provided
        if (!minerAddress.empty()) {
            if (minerAddress.length() < 26 || minerAddress.length() > 62) {
                throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid miner address length");
            }
        }
        
        // Get transactions from block
        std::vector<Transaction> blockTransactions;
        if (blockData.contains("transactions") && blockData["transactions"].is_array()) {
            // Transactions are already in block (from Python miner)
            // We'll process them below
        }
        
        // Calculate block reward with halving
        double blockReward = calculateBlockReward(height);
        
        // Determine block type from block data or use default
        BlockType blockType = BlockType::POW_SHA256;
        if (blockData.contains("consensus_type")) {
            std::string consensusType = blockData["consensus_type"].get<std::string>();
            if (consensusType == "pos") {
                blockType = BlockType::POS;
            } else if (consensusType == "pow_ethash") {
                blockType = BlockType::POW_ETHASH;
            } else {
                blockType = BlockType::POW_SHA256;
            }
        } else if (blockData.contains("block_type")) {
            std::string blockTypeStr = blockData["block_type"].get<std::string>();
            if (blockTypeStr == "pos") {
                blockType = BlockType::POS;
            } else if (blockTypeStr == "pow_ethash") {
                blockType = BlockType::POW_ETHASH;
            } else {
                blockType = BlockType::POW_SHA256;
            }
        }
        
        // Create Block object
        Block newBlock(height, prevHash, blockType);
        newBlock.setHash(hash);
        newBlock.setNonce(nonce);
        newBlock.setTimestamp(timestamp);
        newBlock.setDifficulty(difficulty);
        newBlock.setMinerAddress(minerAddress);
        newBlock.setBlockReward(blockReward);
        
        // For PoS blocks, set validator signature
        if (blockType == BlockType::POS) {
            std::string validatorSignature = blockData.value("validator_signature", "");
            if (!validatorSignature.empty()) {
                newBlock.setValidatorSignature(validatorSignature);
            }
        }
        
        // Check if coinbase transaction exists in submitted block
        bool hasCoinbase = false;
        if (blockData.contains("transactions") && blockData["transactions"].is_array()) {
            for (const auto& txJson : blockData["transactions"]) {
                if (txJson.is_object()) {
                    bool isCoinbase = txJson.value("is_coinbase", txJson.value("coinbase", txJson.value("type", "") == "coinbase"));
                    if (isCoinbase) {
                        hasCoinbase = true;
                        // Create transaction from JSON
                        Transaction coinbaseTx = createTransactionFromJson(txJson);
                        newBlock.addTransaction(coinbaseTx);
                    } else {
                        // Regular transaction
                        Transaction tx = createTransactionFromJson(txJson);
                        newBlock.addTransaction(tx);
                    }
                }
            }
        }
        
        // If no coinbase transaction, create one automatically
        if (!hasCoinbase && !minerAddress.empty()) {
            LOG_API(LogLevel::INFO, "Creating coinbase transaction for miner: " + minerAddress + 
                    ", Reward: " + std::to_string(blockReward) + " GXC");
            Transaction coinbase(minerAddress, blockReward);
            newBlock.addTransaction(coinbase);
            // Recalculate merkle root after adding coinbase
            newBlock.calculateMerkleRoot();
            // Note: The hash provided by miner is based on block without coinbase
            // We keep the miner's hash for validation, but the merkle root will be different
            // This is acceptable as the miner's hash proves work was done
        } else {
            // Merkle root should already be calculated, but recalculate to ensure it's correct
            newBlock.calculateMerkleRoot();
        }
        
        // Verify hash is still set after all modifications
        if (newBlock.getHash().empty()) {
            LOG_API(LogLevel::ERROR, "submitBlock: Block hash is empty after processing!");
            throw RPCException(RPCException::RPC_INTERNAL_ERROR, "Block hash is empty");
        }
        
        LOG_API(LogLevel::INFO, "submitBlock: Block prepared. Height: " + std::to_string(height) + 
                ", Hash: " + newBlock.getHash().substr(0, 16) + "..., Index: " + std::to_string(newBlock.getIndex()) +
                ", Miner: " + minerAddress.substr(0, 20) + "..., Reward: " + std::to_string(blockReward));
        
        // Log block details before adding
        LOG_API(LogLevel::INFO, "submitBlock: Transactions in block: " + std::to_string(newBlock.getTransactions().size()));
        for (const auto& tx : newBlock.getTransactions()) {
            LOG_API(LogLevel::INFO, "  TX: " + tx.getHash().substr(0, 16) + "..., Coinbase: " + 
                    (tx.isCoinbaseTransaction() ? "YES" : "NO") + ", Outputs: " + std::to_string(tx.getOutputs().size()));
        }
        
        // Add block to blockchain
        LOG_API(LogLevel::INFO, "submitBlock: Calling blockchain->addBlock()...");
        bool addResult = false;
        try {
            addResult = blockchain->addBlock(newBlock);
            LOG_API(LogLevel::INFO, "submitBlock: addBlock returned: " + std::string(addResult ? "true" : "false"));
        } catch (const std::exception& e) {
            LOG_API(LogLevel::ERROR, "submitBlock: Exception in addBlock: " + std::string(e.what()));
            throw;
        }
        
        if (addResult) {
            LOG_API(LogLevel::INFO, "✅ Block submitted and added successfully. Height: " + std::to_string(height) + ", Hash: " + hash.substr(0, 16) + "...");
            return JsonValue(); // Success returns null
        } else {
            LOG_API(LogLevel::ERROR, "❌ Block validation failed. Height: " + std::to_string(height) + 
                    ", Hash: " + hash.substr(0, 16) + "...");
            throw RPCException(RPCException::RPC_VERIFY_REJECTED, "Block validation failed");
        }
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error submitting block: " + std::string(e.what()));
        throw RPCException(RPCException::RPC_INTERNAL_ERROR, "Failed to submit block: " + std::string(e.what()));
    }
    
    // Should never reach here, but satisfy compiler
    return JsonValue();
}

// Helper function to calculate block reward with halving
double RPCAPI::calculateBlockReward(uint32_t height) {
    // Halving every 1051200 blocks (~4 years)
    const uint32_t HALVING_INTERVAL = 1051200;
    const double INITIAL_REWARD = 50.0;
    
    uint32_t halvings = height / HALVING_INTERVAL;
    double reward = INITIAL_REWARD;
    
    // Apply halving
    for (uint32_t i = 0; i < halvings; i++) {
        reward /= 2.0;
    }
    
    // Minimum reward is 0.00000001 GXC
    if (reward < 0.00000001) {
        reward = 0.00000001;
    }
    
    return reward;
}

// Helper function to create Transaction from JSON
Transaction RPCAPI::createTransactionFromJson(const JsonValue& txJson) {
    // This is a simplified version - in production, would fully parse all fields
    std::string txHash = txJson.value("hash", txJson.value("txid", ""));
    bool isCoinbase = txJson.value("is_coinbase", txJson.value("coinbase", false));
    
    if (isCoinbase) {
        // Coinbase transaction
        std::string minerAddress = "";
        double amount = 0.0;
        
        // Get miner address from outputs or block
        if (txJson.contains("outputs") && txJson["outputs"].is_array() && txJson["outputs"].size() > 0) {
            auto output = txJson["outputs"][0];
            minerAddress = output.value("address", "");
            amount = output.value("amount", output.value("value", 0.0));
        } else if (txJson.contains("to")) {
            minerAddress = txJson["to"].get<std::string>();
            amount = txJson.value("value", txJson.value("amount", 0.0));
        }
        
        if (!minerAddress.empty() && amount > 0) {
            Transaction coinbase(minerAddress, amount);
            coinbase.setHash(txHash);
            return coinbase;
        }
    }
    
    // Regular transaction - create empty and populate
    Transaction tx;
    tx.setHash(txHash);
    tx.setCoinbaseTransaction(isCoinbase);
    
    // Parse prevTxHash for traceability
    std::string prevTxHash = txJson.value("prevTxHash", txJson.value("prev_tx_hash", txJson.value("previousTxHash", "")));
    if (!prevTxHash.empty()) {
        tx.setPrevTxHash(prevTxHash);
    }
    
    // Parse referencedAmount for traceability
    double referencedAmount = txJson.value("referencedAmount", txJson.value("referenced_amount", 0.0));
    if (referencedAmount > 0) {
        tx.setReferencedAmount(referencedAmount);
    }
    
    // Parse sender/receiver addresses
    std::string senderAddr = txJson.value("from", txJson.value("from_address", txJson.value("fromAddress", "")));
    if (!senderAddr.empty()) {
        tx.setSenderAddress(senderAddr);
    }
    
    std::string receiverAddr = txJson.value("to", txJson.value("to_address", txJson.value("toAddress", "")));
    if (!receiverAddr.empty()) {
        tx.setReceiverAddress(receiverAddr);
    }
    
    // Parse inputs
    if (txJson.contains("inputs") && txJson["inputs"].is_array()) {
        for (const auto& inputJson : txJson["inputs"]) {
            TransactionInput input;
            input.txHash = inputJson.value("txHash", inputJson.value("prev_tx_hash", ""));
            input.outputIndex = inputJson.value("outputIndex", inputJson.value("output_index", 0));
            input.amount = inputJson.value("amount", 0.0);
            tx.addInput(input);
        }
        
        // If prevTxHash not set, use first input's txHash
        if (prevTxHash.empty() && !tx.getInputs().empty()) {
            tx.setPrevTxHash(tx.getInputs()[0].txHash);
        }
        
        // If referencedAmount not set, use first input's amount
        if (referencedAmount == 0.0 && !tx.getInputs().empty()) {
            tx.setReferencedAmount(tx.getInputs()[0].amount);
        }
    }
    
    // Parse outputs
    if (txJson.contains("outputs") && txJson["outputs"].is_array()) {
        for (const auto& outputJson : txJson["outputs"]) {
            TransactionOutput output;
            output.address = outputJson.value("address", "");
            output.amount = outputJson.value("amount", outputJson.value("value", 0.0));
            tx.addOutput(output);
        }
    } else if (txJson.contains("to") && txJson.contains("value")) {
        // Simple format
        TransactionOutput output;
        output.address = txJson["to"].get<std::string>();
        output.amount = txJson["value"].get<double>();
        tx.addOutput(output);
    }
    
    return tx;
}

JsonValue RPCAPI::getBlockTemplate(const JsonValue& params) {
    try {
        JsonValue result;
        
        uint32_t currentHeight = blockchain->getHeight();
        Block latestBlock = blockchain->getLatestBlock();
        
        // Check if we have a valid latest block
        if (latestBlock.getHash().empty()) {
            LOG_API(LogLevel::ERROR, "getBlockTemplate: Latest block has empty hash. Chain height: " + std::to_string(currentHeight));
            throw RPCException(RPCException::RPC_INTERNAL_ERROR, "No valid blocks in chain");
        }
        
        // Determine next block type (PoW or PoS)
        BlockType nextBlockType = blockchain->getNextBlockType();
        uint32_t nextBlockHeight = currentHeight;
        double blockReward = calculateBlockReward(nextBlockHeight);
        
        LOG_API(LogLevel::DEBUG, "getBlockTemplate: currentHeight=" + std::to_string(currentHeight) + 
                ", nextBlockHeight=" + std::to_string(nextBlockHeight) +
                ", blockType=" + (nextBlockType == BlockType::POS ? "PoS" : "PoW") +
                ", reward=" + std::to_string(blockReward) + ", prevHash=" + latestBlock.getHash().substr(0, 16) + "...");
        
        result["version"] = 1;
        result["previousblockhash"] = latestBlock.getHash();
        result["transactions"] = JsonValue(JsonValue::array());
        result["coinbaseaux"] = JsonValue(JsonValue::object());
        result["coinbasevalue"] = static_cast<uint64_t>(blockReward * 100000000); // Convert to satoshis
        result["coinbase_value"] = blockReward; // Also provide as GXC
        result["block_reward"] = blockReward; // For compatibility
        result["reward"] = blockReward; // Alternative field name
        result["target"] = "00000000ffff0000000000000000000000000000000000000000000000000000";
        result["mintime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        result["mutable"] = JsonValue(JsonValue::array());
        result["noncerange"] = "00000000ffffffff";
        result["sigoplimit"] = 20000;
        result["sizelimit"] = 1000000;
        result["curtime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        result["bits"] = "1d00ffff";
        result["height"] = static_cast<uint64_t>(nextBlockHeight);
        result["difficulty"] = blockchain->getDifficulty();
        
        // Add block type information
        std::string consensusType = "pow";
        if (nextBlockType == BlockType::POS) {
            consensusType = "pos";
            result["consensus_type"] = "pos";
            result["block_type"] = "pos";
            
            // For PoS blocks, include validator selection info
            Validator selectedValidator = blockchain->selectValidatorForBlock();
            if (selectedValidator.getAddress().empty()) {
                LOG_API(LogLevel::WARNING, "getBlockTemplate: No validators available for PoS block");
                // Fallback to PoW
                result["consensus_type"] = "pow";
                result["block_type"] = "pow";
            } else {
                result["validator_address"] = selectedValidator.getAddress();
                result["validator_stake"] = selectedValidator.getStakeAmount();
                result["validator_weighted_stake"] = selectedValidator.getWeightedStake();
            }
        } else {
            result["consensus_type"] = "pow";
            result["block_type"] = "pow";
        }
        
        return result;
    
    } catch (const RPCException& e) {
        // Re-throw RPC exceptions
        throw;
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "getBlockTemplate error: " + std::string(e.what()));
        throw RPCException(RPCException::RPC_INTERNAL_ERROR, "Failed to get block template: " + std::string(e.what()));
    }
}

// Network methods
JsonValue RPCAPI::getPeerInfo(const JsonValue& params) {
    JsonValue result(JsonValue::array());
    
    // In a real implementation, would get actual peer information
    for (int i = 0; i < 3; ++i) {
        JsonValue peer;
        peer["id"] = i;
        peer["addr"] = "192.168.1." + std::to_string(100 + i) + ":8333";
        peer["services"] = "0000000000000001";
        peer["relaytxes"] = true;
        peer["lastsend"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        peer["lastrecv"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        peer["bytessent"] = 1000 * (i + 1);
        peer["bytesrecv"] = 800 * (i + 1);
        peer["conntime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp() - 3600);
        peer["timeoffset"] = 0;
        peer["pingtime"] = 0.1;
        peer["version"] = 70015;
        peer["subver"] = "/GXC:2.0.0/";
        peer["inbound"] = i % 2 == 0;
        peer["startingheight"] = static_cast<uint64_t>(blockchain->getHeight());
        peer["banscore"] = 0;
        peer["synced_headers"] = static_cast<uint64_t>(blockchain->getHeight());
        peer["synced_blocks"] = static_cast<uint64_t>(blockchain->getHeight());
        
        result.push_back(peer);
    }
    
    return result;
}

JsonValue RPCAPI::getConnectionCount(const JsonValue& params) {
    // In a real implementation, would get actual connection count
    return 3;
}

JsonValue RPCAPI::addNode(const JsonValue& params) {
    if (params.size() < 2) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing node address or command parameter");
    }
    
    std::string node = params[0].get<std::string>();
    std::string command = params[1].get<std::string>();
    
    if (command != "add" && command != "remove" && command != "onetry") {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid command. Use 'add', 'remove', or 'onetry'");
    }
    
    LOG_API(LogLevel::INFO, "Node command: " + command + " " + node);
    
    return JsonValue();
}

JsonValue RPCAPI::disconnectNode(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing node address parameter");
    }
    
    std::string node = params[0].get<std::string>();
    
    LOG_API(LogLevel::INFO, "Disconnecting from node: " + node);
    
    return JsonValue();
}

// Utility methods
JsonValue RPCAPI::help(const JsonValue& params) {
    std::string command = params.size() > 0 ? params[0].get<std::string>() : "";
    
    if (command.empty()) {
        // List all available commands
        std::ostringstream help;
        help << "Available RPC commands:\n";
        for (const auto& method : rpcMethods) {
            help << "  " << method.first << "\n";
        }
        return help.str();
    } else {
        // Return help for specific command
        if (rpcMethods.find(command) != rpcMethods.end()) {
            return "Help for command: " + command + "\n(Command help would be detailed here)";
        } else {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Unknown command: " + command);
        }
    }
    
    return JsonValue(); // Default return
}

JsonValue RPCAPI::stopNode(const JsonValue& params) {
    LOG_API(LogLevel::WARNING, "Node shutdown requested via RPC");
    
    // In a real implementation, would trigger graceful shutdown
    return "GXC node stopping";
}

JsonValue RPCAPI::getInfo(const JsonValue& params) {
    JsonValue result;
    
    result["version"] = 200000; // 2.0.0
    result["protocolversion"] = 70015;
    result["blocks"] = static_cast<uint64_t>(blockchain->getHeight());
    result["timeoffset"] = 0;
    result["connections"] = 3; // Would get actual connection count
    result["difficulty"] = blockchain->getDifficulty();
    result["testnet"] = false;
    result["relayfee"] = 0.00001;
    result["errors"] = "";
    
    return result;
}

// Staking and Validator RPC Methods
JsonValue RPCAPI::registerValidator(const JsonValue& params) {
    if (params.size() < 3) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Missing parameters: address, stakeAmount, stakingDays");
    }
    
    std::string address = params[0].get<std::string>();
    double stakeAmount = params[1].get<double>();
    uint32_t stakingDays = params[2].get<uint32_t>();
    
    // Validate minimum stake
    if (stakeAmount < Validator::MIN_STAKE) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Stake amount must be at least " + std::to_string(Validator::MIN_STAKE) + " GXC");
    }
    
    // Validate staking period
    if (stakingDays < Validator::MIN_STAKING_DAYS || stakingDays > Validator::MAX_STAKING_DAYS) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Staking days must be between " + std::to_string(Validator::MIN_STAKING_DAYS) + 
            " and " + std::to_string(Validator::MAX_STAKING_DAYS));
    }
    
    // Check balance
    double balance = blockchain->getBalance(address);
    if (balance < stakeAmount) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Insufficient balance. Required: " + std::to_string(stakeAmount) + 
            " GXC, Available: " + std::to_string(balance) + " GXC");
    }
    
    // Create validator
    Validator validator(address, stakeAmount, stakingDays);
    
    // Register validator
    blockchain->registerValidator(validator);
    
    LOG_API(LogLevel::INFO, "Validator registered: " + address + ", Stake: " + 
            std::to_string(stakeAmount) + " GXC, Days: " + std::to_string(stakingDays));
    
    JsonValue result;
    result["success"] = true;
    result["address"] = address;
    result["stake_amount"] = stakeAmount;
    result["staking_days"] = stakingDays;
    result["weighted_stake"] = validator.getWeightedStake();
    result["message"] = "Validator registered successfully";
    
    return result;
}

JsonValue RPCAPI::stake(const JsonValue& params) {
    // Alias for registerValidator
    return registerValidator(params);
}

JsonValue RPCAPI::unstake(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address parameter");
    }
    
    std::string address = params[0].get<std::string>();
    
    // Unregister validator
    blockchain->unregisterValidator(address);
    
    LOG_API(LogLevel::INFO, "Validator unregistered: " + address);
    
    JsonValue result;
    result["success"] = true;
    result["address"] = address;
    result["message"] = "Stake withdrawn successfully";
    
    return result;
}

JsonValue RPCAPI::addStake(const JsonValue& params) {
    if (params.size() < 2) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Missing parameters: address, additionalAmount");
    }
    
    std::string address = params[0].get<std::string>();
    double additionalAmount = params[1].get<double>();
    
    if (additionalAmount <= 0) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Additional stake amount must be positive");
    }
    
    // Check balance
    double balance = blockchain->getBalance(address);
    if (balance < additionalAmount) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Insufficient balance. Required: " + std::to_string(additionalAmount) + 
            " GXC, Available: " + std::to_string(balance) + " GXC");
    }
    
    // Get validators to find this one
    auto validators = blockchain->getActiveValidators();
    bool found = false;
    
    for (auto& validator : validators) {
        if (validator.getAddress() == address) {
            validator.addStake(additionalAmount);
            found = true;
            break;
        }
    }
    
    if (!found) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Validator not found for address: " + address);
    }
    
    LOG_API(LogLevel::INFO, "Additional stake added: " + address + ", Amount: " + 
            std::to_string(additionalAmount) + " GXC");
    
    JsonValue result;
    result["success"] = true;
    result["address"] = address;
    result["additional_amount"] = additionalAmount;
    result["message"] = "Additional stake added successfully";
    
    return result;
}

JsonValue RPCAPI::getValidators(const JsonValue& params) {
    auto validators = blockchain->getActiveValidators();
    
    JsonValue result(JsonValue::array());
    
    for (const auto& validator : validators) {
        JsonValue v;
        v["address"] = validator.getAddress();
        v["stake_amount"] = validator.getStakeAmount();
        v["staking_days"] = validator.getStakingDays();
        v["weighted_stake"] = validator.getWeightedStake();
        v["is_active"] = validator.getIsActive();
        v["blocks_produced"] = validator.getBlocksProduced();
        v["missed_blocks"] = validator.getMissedBlocks();
        v["uptime"] = validator.getUptime();
        v["total_rewards"] = validator.getTotalRewards();
        v["apy"] = validator.calculateAPY();
        v["is_slashed"] = validator.getIsSlashed();
        
        result.push_back(v);
    }
    
    return result;
}

JsonValue RPCAPI::getValidatorInfo(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address parameter");
    }
    
    std::string address = params[0].get<std::string>();
    
    auto validators = blockchain->getActiveValidators();
    
    for (const auto& validator : validators) {
        if (validator.getAddress() == address) {
            JsonValue result;
            result["address"] = validator.getAddress();
            result["stake_amount"] = validator.getStakeAmount();
            result["staking_days"] = validator.getStakingDays();
            result["weighted_stake"] = validator.getWeightedStake();
            result["time_weight"] = validator.getTimeWeight();
            result["is_active"] = validator.getIsActive();
            result["blocks_produced"] = validator.getBlocksProduced();
            result["missed_blocks"] = validator.getMissedBlocks();
            result["uptime"] = validator.getUptime();
            result["total_rewards"] = validator.getTotalRewards();
            result["pending_rewards"] = validator.getPendingRewards();
            result["apy"] = validator.calculateAPY();
            result["is_slashed"] = validator.getIsSlashed();
            result["slashed_amount"] = validator.getSlashedAmount();
            result["is_valid"] = validator.isValidValidator();
            
            return result;
        }
    }
    
    throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
        "Validator not found for address: " + address);
}

// PoS Block Creation Methods
JsonValue RPCAPI::createPoSBlock(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing validator address parameter");
    }
    
    std::string validatorAddress = params[0].get<std::string>();
    
    // Get validator info
    auto validators = blockchain->getActiveValidators();
    Validator validator;
    bool found = false;
    
    for (const auto& v : validators) {
        if (v.getAddress() == validatorAddress) {
            validator = v;
            found = true;
            break;
        }
    }
    
    if (!found) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Validator not found or not active: " + validatorAddress);
    }
    
    // Get blockchain state
    uint32_t currentHeight = blockchain->getHeight();
    Block latestBlock = blockchain->getLatestBlock();
    
    if (latestBlock.getHash().empty()) {
        throw RPCException(RPCException::RPC_INTERNAL_ERROR, "No valid blocks in chain");
    }
    
    // Check if next block should be PoS
    BlockType nextBlockType = blockchain->getNextBlockType();
    if (nextBlockType != BlockType::POS) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, 
            "Next block should be PoW, not PoS. Current height: " + std::to_string(currentHeight));
    }
    
    // Check if this validator should create the block
    Validator selectedValidator = blockchain->selectValidatorForBlock();
    if (selectedValidator.getAddress() != validatorAddress) {
        LOG_API(LogLevel::WARNING, "createPoSBlock: Validator " + validatorAddress + 
                " not selected. Selected: " + selectedValidator.getAddress());
        // Still allow creation, but warn
    }
    
    uint32_t nextBlockHeight = currentHeight;
    double blockReward = calculateBlockReward(nextBlockHeight);
    
    // Create PoS block template
    Block posBlock(nextBlockHeight, latestBlock.getHash(), BlockType::POS);
    posBlock.setTimestamp(Utils::getCurrentTimestamp());
    posBlock.setDifficulty(0.01); // Low difficulty for PoS
    posBlock.setMinerAddress(validatorAddress);
    posBlock.setBlockReward(blockReward);
    
    // Get pending transactions
    auto pendingTxs = blockchain->getPendingTransactions(100);
    for (const auto& tx : pendingTxs) {
        posBlock.addTransaction(tx);
    }
    
    // Create coinbase transaction
    Transaction coinbase(validatorAddress, blockReward);
    posBlock.addTransaction(coinbase);
    
    // Calculate merkle root
    posBlock.calculateMerkleRoot();
    
    // Calculate block hash (PoS blocks need minimal hash work)
    std::string blockHash = posBlock.calculateHash();
    posBlock.setHash(blockHash);
    
    // Sign block with validator (in production, would use actual cryptographic signing)
    // For now, create a signature based on block hash and validator address
    std::string validatorSignature = validator.signBlock(blockHash, ""); // Private key would be used here
    posBlock.setValidatorSignature(validatorSignature);
    
    // Return block template
    JsonValue result;
    result["height"] = nextBlockHeight;
    result["hash"] = blockHash;
    result["previousblockhash"] = latestBlock.getHash();
    result["timestamp"] = static_cast<uint64_t>(posBlock.getTimestamp());
    result["difficulty"] = posBlock.getDifficulty();
    result["validator_address"] = validatorAddress;
    result["validator_signature"] = validatorSignature;
    result["block_reward"] = blockReward;
    result["consensus_type"] = "pos";
    result["block_type"] = "pos";
    result["merkle_root"] = posBlock.getMerkleRoot();
    
    // Include transactions
    JsonValue transactions(JsonValue::array());
    for (const auto& tx : posBlock.getTransactions()) {
        JsonValue txJson = transactionToJson(tx, nextBlockHeight, blockHash);
        transactions.push_back(txJson);
    }
    result["transactions"] = transactions;
    
    LOG_API(LogLevel::INFO, "createPoSBlock: Created PoS block template for validator " + validatorAddress);
    
    return result;
}

JsonValue RPCAPI::submitPoSBlock(const JsonValue& params) {
    // PoS block submission uses the same submitBlock method
    // but with block_type set to "pos"
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block data parameter");
    }
    
    JsonValue blockData;
    if (params[0].is_string()) {
        try {
            blockData = JsonValue::parse(params[0].get<std::string>());
        } catch (...) {
            throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Failed to parse block JSON");
        }
    } else if (params[0].is_object()) {
        blockData = params[0];
    } else {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Invalid block data format");
    }
    
    // Ensure block type is set to PoS
    blockData["consensus_type"] = "pos";
    blockData["block_type"] = "pos";
    
    // Use submitBlock with modified params
    JsonValue submitParams(JsonValue::array());
    submitParams.push_back(blockData);
    
    return submitBlock(submitParams);
}

// RPCException implementation
RPCException::RPCException(int code, const std::string& message)
    : std::runtime_error(message), errorCode(code) {
}

int RPCException::getCode() const {
    return errorCode;
}

constexpr int RPCException::RPCException::RPC_INVALID_PARAMETER;
constexpr int RPCException::RPC_METHOD_NOT_FOUND;
constexpr int RPCException::RPC_INVALID_REQUEST;
constexpr int RPCException::RPC_INTERNAL_ERROR;