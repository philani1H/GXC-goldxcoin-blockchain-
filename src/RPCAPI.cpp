#include "../include/RPCAPI.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <sstream>
#include <iomanip>

RPCAPI::RPCAPI(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), serverPort(port), isRunning(false) {
    
    // Register RPC methods
    registerMethods();
    
    LOG_API(LogLevel::INFO, "RPC API initialized on port " + std::to_string(port));
}

RPCAPI::~RPCAPI() {
    stop();
}

bool RPCAPI::start() {
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
    rpcMethods["getblockchaininfo"] = [this](const Json::Value& params) { return getBlockchainInfo(params); };
    rpcMethods["getbestblockhash"] = [this](const Json::Value& params) { return getBestBlockHash(params); };
    rpcMethods["getblockcount"] = [this](const Json::Value& params) { return getBlockCount(params); };
    rpcMethods["getdifficulty"] = [this](const Json::Value& params) { return getDifficulty(params); };
    rpcMethods["getblock"] = [this](const Json::Value& params) { return getBlock(params); };
    rpcMethods["getblockhash"] = [this](const Json::Value& params) { return getBlockHash(params); };
    
    // Transaction methods
    rpcMethods["getrawtransaction"] = [this](const Json::Value& params) { return getRawTransaction(params); };
    rpcMethods["sendrawtransaction"] = [this](const Json::Value& params) { return sendRawTransaction(params); };
    rpcMethods["gettransaction"] = [this](const Json::Value& params) { return getTransaction(params); };
    rpcMethods["listtransactions"] = [this](const Json::Value& params) { return listTransactions(params); };
    
    // Wallet methods
    rpcMethods["getbalance"] = [this](const Json::Value& params) { return getBalance(params); };
    rpcMethods["getnewaddress"] = [this](const Json::Value& params) { return getNewAddress(params); };
    rpcMethods["sendtoaddress"] = [this](const Json::Value& params) { return sendToAddress(params); };
    rpcMethods["validateaddress"] = [this](const Json::Value& params) { return validateAddress(params); };
    rpcMethods["listaccounts"] = [this](const Json::Value& params) { return listAccounts(params); };
    
    // Mining methods
    rpcMethods["getmininginfo"] = [this](const Json::Value& params) { return getMiningInfo(params); };
    rpcMethods["getnetworkhashps"] = [this](const Json::Value& params) { return getNetworkHashPS(params); };
    rpcMethods["submitblock"] = [this](const Json::Value& params) { return submitBlock(params); };
    rpcMethods["getblocktemplate"] = [this](const Json::Value& params) { return getBlockTemplate(params); };
    
    // Network methods
    rpcMethods["getpeerinfo"] = [this](const Json::Value& params) { return getPeerInfo(params); };
    rpcMethods["getconnectioncount"] = [this](const Json::Value& params) { return getConnectionCount(params); };
    rpcMethods["addnode"] = [this](const Json::Value& params) { return addNode(params); };
    rpcMethods["disconnectnode"] = [this](const Json::Value& params) { return disconnectNode(params); };
    
    // Utility methods
    rpcMethods["help"] = [this](const Json::Value& params) { return help(params); };
    rpcMethods["stop"] = [this](const Json::Value& params) { return stopNode(params); };
    rpcMethods["getinfo"] = [this](const Json::Value& params) { return getInfo(params); };
    
    LOG_API(LogLevel::INFO, "Registered " + std::to_string(rpcMethods.size()) + " RPC methods");
}

std::string RPCAPI::processRequest(const std::string& request) {
    try {
        Json::Value requestJson;
        Json::CharReaderBuilder builder;
        std::string errors;
        
        std::istringstream stream(request);
        if (!Json::parseFromStream(builder, stream, &requestJson, &errors)) {
            return createErrorResponse(-32700, "Parse error: " + errors, Json::Value::null);
        }
        
        // Validate JSON-RPC structure
        if (!requestJson.isMember("jsonrpc") || requestJson["jsonrpc"].asString() != "2.0") {
            return createErrorResponse(-32600, "Invalid Request: missing or invalid jsonrpc field", 
                                     requestJson.get("id", Json::Value::null));
        }
        
        if (!requestJson.isMember("method") || !requestJson["method"].isString()) {
            return createErrorResponse(-32600, "Invalid Request: missing or invalid method field", 
                                     requestJson.get("id", Json::Value::null));
        }
        
        std::string method = requestJson["method"].asString();
        Json::Value params = requestJson.get("params", Json::Value(Json::arrayValue));
        Json::Value id = requestJson.get("id", Json::Value::null);
        
        LOG_API(LogLevel::DEBUG, "Processing RPC request: " + method);
        
        // Find and execute the method
        auto it = rpcMethods.find(method);
        if (it == rpcMethods.end()) {
            return createErrorResponse(-32601, "Method not found: " + method, id);
        }
        
        try {
            Json::Value result = it->second(params);
            return createSuccessResponse(result, id);
        } catch (const RPCException& e) {
            return createErrorResponse(e.getCode(), e.what(), id);
        } catch (const std::exception& e) {
            return createErrorResponse(-32603, "Internal error: " + std::string(e.what()), id);
        }
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error processing RPC request: " + std::string(e.what()));
        return createErrorResponse(-32603, "Internal error", Json::Value::null);
    }
}

std::string RPCAPI::createSuccessResponse(const Json::Value& result, const Json::Value& id) {
    Json::Value response;
    response["jsonrpc"] = "2.0";
    response["result"] = result;
    response["id"] = id;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, response);
}

std::string RPCAPI::createErrorResponse(int code, const std::string& message, const Json::Value& id) {
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    
    Json::Value response;
    response["jsonrpc"] = "2.0";
    response["error"] = error;
    response["id"] = id;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, response);
}

void RPCAPI::serverLoop() {
    LOG_API(LogLevel::INFO, "RPC server loop started");
    
    while (isRunning) {
        try {
            // Simplified server loop - would handle actual HTTP requests in real implementation
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_API(LogLevel::ERROR, "Error in RPC server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_API(LogLevel::INFO, "RPC server loop stopped");
}

// Blockchain information methods
Json::Value RPCAPI::getBlockchainInfo(const Json::Value& params) {
    Json::Value result;
    
    result["chain"] = "main"; // or "test" for testnet
    result["blocks"] = static_cast<uint64_t>(blockchain->getChainLength());
    result["bestblockhash"] = blockchain->getLatestBlock().getHash();
    result["difficulty"] = blockchain->getDifficulty();
    result["mediantime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["verificationprogress"] = 1.0;
    result["initialblockdownload"] = false;
    result["chainwork"] = "0000000000000000000000000000000000000000000000000000000000000000";
    result["size_on_disk"] = 0; // Would calculate actual size
    result["pruned"] = false;
    
    return result;
}

Json::Value RPCAPI::getBestBlockHash(const Json::Value& params) {
    return blockchain->getLatestBlock().getHash();
}

Json::Value RPCAPI::getBlockCount(const Json::Value& params) {
    return static_cast<uint64_t>(blockchain->getChainLength() - 1); // 0-indexed
}

Json::Value RPCAPI::getDifficulty(const Json::Value& params) {
    return blockchain->getDifficulty();
}

Json::Value RPCAPI::getBlock(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing block hash parameter");
    }
    
    std::string blockHash = params[0].asString();
    bool verbose = params.size() > 1 ? params[1].asBool() : true;
    
    // In a real implementation, would retrieve block by hash
    Json::Value result;
    result["hash"] = blockHash;
    result["confirmations"] = 1;
    result["height"] = blockchain->getChainLength() - 1;
    result["version"] = 1;
    result["merkleroot"] = "merkleroot_hash";
    result["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["nonce"] = 0;
    result["bits"] = "1d00ffff";
    result["difficulty"] = blockchain->getDifficulty();
    result["previousblockhash"] = "previous_hash";
    result["tx"] = Json::Value(Json::arrayValue);
    
    return result;
}

Json::Value RPCAPI::getBlockHash(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing block height parameter");
    }
    
    uint32_t height = params[0].asUInt();
    
    if (height >= blockchain->getChainLength()) {
        throw RPCException(RPC_INVALID_PARAMETER, "Block height out of range");
    }
    
    // In a real implementation, would get block hash by height
    return "block_hash_at_height_" + std::to_string(height);
}

// Transaction methods
Json::Value RPCAPI::getRawTransaction(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
    }
    
    std::string txHash = params[0].asString();
    bool verbose = params.size() > 1 ? params[1].asBool() : false;
    
    // In a real implementation, would retrieve transaction by hash
    if (verbose) {
        Json::Value result;
        result["txid"] = txHash;
        result["hash"] = txHash;
        result["version"] = 1;
        result["size"] = 250;
        result["vsize"] = 250;
        result["weight"] = 1000;
        result["locktime"] = 0;
        result["vin"] = Json::Value(Json::arrayValue);
        result["vout"] = Json::Value(Json::arrayValue);
        result["hex"] = "raw_hex_data";
        result["blockhash"] = "block_hash";
        result["confirmations"] = 1;
        result["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        result["blocktime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        
        return result;
    } else {
        return "raw_hex_transaction_data";
    }
}

Json::Value RPCAPI::sendRawTransaction(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing raw transaction data parameter");
    }
    
    std::string rawTx = params[0].asString();
    
    // In a real implementation, would decode and broadcast the transaction
    LOG_API(LogLevel::INFO, "Broadcasting raw transaction");
    
    return "transaction_hash";
}

Json::Value RPCAPI::getTransaction(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
    }
    
    std::string txHash = params[0].asString();
    
    Json::Value result;
    result["amount"] = 0.0;
    result["fee"] = -0.0001;
    result["confirmations"] = 1;
    result["blockhash"] = "block_hash";
    result["blockindex"] = 0;
    result["blocktime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["txid"] = txHash;
    result["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["timereceived"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["details"] = Json::Value(Json::arrayValue);
    result["hex"] = "raw_hex_data";
    
    return result;
}

Json::Value RPCAPI::listTransactions(const Json::Value& params) {
    std::string account = params.size() > 0 ? params[0].asString() : "*";
    int count = params.size() > 1 ? params[1].asInt() : 10;
    int skip = params.size() > 2 ? params[2].asInt() : 0;
    
    Json::Value result(Json::arrayValue);
    
    // In a real implementation, would retrieve actual transactions
    for (int i = 0; i < std::min(count, 5); ++i) {
        Json::Value tx;
        tx["account"] = account;
        tx["address"] = "GXC_address_" + std::to_string(i);
        tx["category"] = "receive";
        tx["amount"] = 1.0 + i;
        tx["fee"] = 0.0001;
        tx["confirmations"] = 10 + i;
        tx["blockhash"] = "block_hash_" + std::to_string(i);
        tx["blockindex"] = i;
        tx["blocktime"] = Utils::getCurrentTimestamp() - (i * 600);
        tx["txid"] = "tx_hash_" + std::to_string(i);
        tx["time"] = Utils::getCurrentTimestamp() - (i * 600);
        tx["timereceived"] = Utils::getCurrentTimestamp() - (i * 600);
        
        result.append(tx);
    }
    
    return result;
}

// Wallet methods
Json::Value RPCAPI::getBalance(const Json::Value& params) {
    std::string account = params.size() > 0 ? params[0].asString() : "";
    int minConfirms = params.size() > 1 ? params[1].asInt() : 1;
    
    // In a real implementation, would calculate actual balance
    return 100.0; // Example balance
}

Json::Value RPCAPI::getNewAddress(const Json::Value& params) {
    std::string account = params.size() > 0 ? params[0].asString() : "";
    
    // In a real implementation, would generate a new address
    return "GXC" + Utils::randomString(30);
}

Json::Value RPCAPI::sendToAddress(const Json::Value& params) {
    if (params.size() < 2) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing address or amount parameter");
    }
    
    std::string address = params[0].asString();
    double amount = params[1].asDouble();
    std::string comment = params.size() > 2 ? params[2].asString() : "";
    std::string commentTo = params.size() > 3 ? params[3].asString() : "";
    
    if (amount <= 0) {
        throw RPCException(RPC_INVALID_PARAMETER, "Invalid amount");
    }
    
    // In a real implementation, would create and send transaction
    LOG_API(LogLevel::INFO, "Sending " + std::to_string(amount) + " GXC to " + address);
    
    return "transaction_hash";
}

Json::Value RPCAPI::validateAddress(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing address parameter");
    }
    
    std::string address = params[0].asString();
    
    Json::Value result;
    result["isvalid"] = Utils::isValidAddress(address);
    
    if (result["isvalid"].asBool()) {
        result["address"] = address;
        result["ismine"] = false; // Would check if address belongs to wallet
        result["iswatchonly"] = false;
        result["isscript"] = false;
    }
    
    return result;
}

Json::Value RPCAPI::listAccounts(const Json::Value& params) {
    int minConfirms = params.size() > 0 ? params[0].asInt() : 1;
    
    Json::Value result;
    result[""] = 100.0; // Default account balance
    result["mining"] = 50.0; // Mining account balance
    
    return result;
}

// Mining methods
Json::Value RPCAPI::getMiningInfo(const Json::Value& params) {
    Json::Value result;
    
    result["blocks"] = static_cast<uint64_t>(blockchain->getChainLength());
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

Json::Value RPCAPI::getNetworkHashPS(const Json::Value& params) {
    int blocks = params.size() > 0 ? params[0].asInt() : 120;
    int height = params.size() > 1 ? params[1].asInt() : -1;
    
    // In a real implementation, would calculate actual network hashrate
    return 1000000.0; // Example hashrate in hashes per second
}

Json::Value RPCAPI::submitBlock(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing block data parameter");
    }
    
    std::string blockData = params[0].asString();
    
    // In a real implementation, would validate and add block to blockchain
    LOG_API(LogLevel::INFO, "Submitting block for validation");
    
    return Json::Value::null; // Success returns null
}

Json::Value RPCAPI::getBlockTemplate(const Json::Value& params) {
    Json::Value result;
    
    result["version"] = 1;
    result["previousblockhash"] = blockchain->getLatestBlock().getHash();
    result["transactions"] = Json::Value(Json::arrayValue);
    result["coinbaseaux"] = Json::Value(Json::objectValue);
    result["coinbasevalue"] = static_cast<uint64_t>(blockchain->getBlockReward());
    result["target"] = "00000000ffff0000000000000000000000000000000000000000000000000000";
    result["mintime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["mutable"] = Json::Value(Json::arrayValue);
    result["noncerange"] = "00000000ffffffff";
    result["sigoplimit"] = 20000;
    result["sizelimit"] = 1000000;
    result["curtime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["bits"] = "1d00ffff";
    result["height"] = static_cast<uint64_t>(blockchain->getChainLength());
    
    return result;
}

// Network methods
Json::Value RPCAPI::getPeerInfo(const Json::Value& params) {
    Json::Value result(Json::arrayValue);
    
    // In a real implementation, would get actual peer information
    for (int i = 0; i < 3; ++i) {
        Json::Value peer;
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
        peer["startingheight"] = static_cast<uint64_t>(blockchain->getChainLength());
        peer["banscore"] = 0;
        peer["synced_headers"] = static_cast<uint64_t>(blockchain->getChainLength());
        peer["synced_blocks"] = static_cast<uint64_t>(blockchain->getChainLength());
        
        result.append(peer);
    }
    
    return result;
}

Json::Value RPCAPI::getConnectionCount(const Json::Value& params) {
    // In a real implementation, would get actual connection count
    return 3;
}

Json::Value RPCAPI::addNode(const Json::Value& params) {
    if (params.size() < 2) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing node address or command parameter");
    }
    
    std::string node = params[0].asString();
    std::string command = params[1].asString();
    
    if (command != "add" && command != "remove" && command != "onetry") {
        throw RPCException(RPC_INVALID_PARAMETER, "Invalid command. Use 'add', 'remove', or 'onetry'");
    }
    
    LOG_API(LogLevel::INFO, "Node command: " + command + " " + node);
    
    return Json::Value::null;
}

Json::Value RPCAPI::disconnectNode(const Json::Value& params) {
    if (params.size() < 1) {
        throw RPCException(RPC_INVALID_PARAMETER, "Missing node address parameter");
    }
    
    std::string node = params[0].asString();
    
    LOG_API(LogLevel::INFO, "Disconnecting from node: " + node);
    
    return Json::Value::null;
}

// Utility methods
Json::Value RPCAPI::help(const Json::Value& params) {
    std::string command = params.size() > 0 ? params[0].asString() : "";
    
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
            throw RPCException(RPC_INVALID_PARAMETER, "Unknown command: " + command);
        }
    }
}

Json::Value RPCAPI::stopNode(const Json::Value& params) {
    LOG_API(LogLevel::WARNING, "Node shutdown requested via RPC");
    
    // In a real implementation, would trigger graceful shutdown
    return "GXC node stopping";
}

Json::Value RPCAPI::getInfo(const Json::Value& params) {
    Json::Value result;
    
    result["version"] = 200000; // 2.0.0
    result["protocolversion"] = 70015;
    result["blocks"] = static_cast<uint64_t>(blockchain->getChainLength());
    result["timeoffset"] = 0;
    result["connections"] = 3; // Would get actual connection count
    result["difficulty"] = blockchain->getDifficulty();
    result["testnet"] = false;
    result["relayfee"] = 0.00001;
    result["errors"] = "";
    
    return result;
}

// RPCException implementation
RPCException::RPCException(int code, const std::string& message)
    : std::runtime_error(message), errorCode(code) {
}

int RPCException::getCode() const {
    return errorCode;
}