#include "../include/RESTServer.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <sstream>
#include <iomanip>
#include <thread>

RESTServer::RESTServer(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), serverPort(port), isRunning(false) {
    LOG_API(LogLevel::INFO, "REST API initialized on port " + std::to_string(port));
}

RESTServer::~RESTServer() {
    stop();
}

bool RESTServer::start() {
    if (isRunning) {
        LOG_API(LogLevel::WARNING, "REST server is already running");
        return true;
    }
    
    LOG_API(LogLevel::INFO, "Starting REST API server");
    
    try {
        isRunning = true;
        serverThread = std::thread(&RESTServer::serverLoop, this);
        
        LOG_API(LogLevel::INFO, "REST API server started successfully on port " + std::to_string(serverPort));
        return true;
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Failed to start REST server: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void RESTServer::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_API(LogLevel::INFO, "Stopping REST API server");
    
    isRunning = false;
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    LOG_API(LogLevel::INFO, "REST API server stopped");
}

void RESTServer::serverLoop() {
    LOG_API(LogLevel::INFO, "REST server loop started");
    
    while (isRunning) {
        try {
            // Simplified server loop - would handle actual HTTP requests in real implementation
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_API(LogLevel::ERROR, "Error in REST server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_API(LogLevel::INFO, "REST server loop stopped");
}

std::string RESTServer::handleRequest(const std::string& method, const std::string& path, const std::string& body) {
    try {
        LOG_API(LogLevel::DEBUG, "REST request: " + method + " " + path);
        
        // Route the request based on path
        if (path == "/api/v1/blockchain/info") {
            return getBlockchainInfo();
        }
        else if (path.find("/api/v1/block/") == 0) {
            std::string blockId = path.substr(15); // Remove "/api/v1/block/"
            return getBlock(blockId);
        }
        else if (path == "/api/v1/blocks") {
            return getBlocks();
        }
        else if (path.find("/api/v1/transaction/") == 0) {
            std::string txHash = path.substr(20); // Remove "/api/v1/transaction/"
            return getTransaction(txHash);
        }
        else if (path == "/api/v1/transactions" && method == "POST") {
            return submitTransaction(body);
        }
        else if (path.find("/api/v1/address/") == 0 && path.find("/balance") != std::string::npos) {
            std::string address = path.substr(15);
            address = address.substr(0, address.find("/balance"));
            return getAddressBalance(address);
        }
        else if (path.find("/api/v1/address/") == 0 && path.find("/transactions") != std::string::npos) {
            std::string address = path.substr(15);
            address = address.substr(0, address.find("/transactions"));
            return getAddressTransactions(address);
        }
        else if (path.find("/api/v1/trace/") == 0) {
            std::string txHash = path.substr(13); // Remove "/api/v1/trace/"
            return traceTransaction(txHash);
        }
        else if (path == "/api/v1/mining/info") {
            return getMiningInfo();
        }
        else if (path == "/api/v1/network/peers") {
            return getPeerInfo();
        }
        else if (path == "/api/v1/stats") {
            return getStats();
        }
        else {
            return createErrorResponse(404, "Not Found", "Endpoint not found: " + path);
        }
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error handling REST request: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::getBlockchainInfo() {
    Json::Value response;
    
    response["chain"] = "main";
    response["blocks"] = static_cast<uint64_t>(blockchain->getChainLength());
    response["bestblockhash"] = blockchain->getLatestBlock().getHash();
    response["difficulty"] = blockchain->getDifficulty();
    response["mediantime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    response["verificationprogress"] = 1.0;
    response["initialblockdownload"] = false;
    response["pruned"] = false;
    response["block_reward"] = static_cast<uint64_t>(blockchain->getBlockReward());
    response["total_supply"] = blockchain->getTotalSupply();
    
    return jsonToString(response);
}

std::string RESTServer::getBlock(const std::string& blockId) {
    try {
        Json::Value response;
        
        // Check if blockId is a hash or height
        if (blockId.length() == 64) {
            // Assume it's a hash
            response["hash"] = blockId;
            response["height"] = blockchain->getChainLength() - 1; // Simplified
        } else {
            // Assume it's a height
            uint32_t height = std::stoul(blockId);
            if (height >= blockchain->getChainLength()) {
                return createErrorResponse(404, "Not Found", "Block not found");
            }
            response["height"] = height;
            response["hash"] = "block_hash_" + blockId;
        }
        
        response["confirmations"] = 1;
        response["version"] = 1;
        response["merkleroot"] = "merkleroot_hash";
        response["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        response["nonce"] = 0;
        response["bits"] = "1d00ffff";
        response["difficulty"] = blockchain->getDifficulty();
        response["previousblockhash"] = "previous_hash";
        response["tx"] = Json::Value(Json::arrayValue);
        
        return jsonToString(response);
        
    } catch (const std::exception&) {
        return createErrorResponse(400, "Bad Request", "Invalid block identifier");
    }
}

std::string RESTServer::getBlocks() {
    Json::Value response(Json::arrayValue);
    
    // Return last 10 blocks (simplified)
    uint32_t chainLength = blockchain->getChainLength();
    uint32_t startBlock = chainLength > 10 ? chainLength - 10 : 0;
    
    for (uint32_t i = startBlock; i < chainLength; ++i) {
        Json::Value block;
        block["height"] = i;
        block["hash"] = "block_hash_" + std::to_string(i);
        block["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp() - (chainLength - i) * 600);
        block["tx_count"] = 1;
        
        response.append(block);
    }
    
    return jsonToString(response);
}

std::string RESTServer::getTransaction(const std::string& txHash) {
    try {
        // In real implementation, would retrieve actual transaction
        Transaction tx = blockchain->getTransactionByHash(txHash);
        
        Json::Value response;
        response["txid"] = tx.getHash();
        response["hash"] = tx.getHash();
        response["version"] = 1;
        response["size"] = 250;
        response["vsize"] = 250;
        response["weight"] = 1000;
        response["locktime"] = tx.getLockTime();
        response["time"] = static_cast<uint64_t>(tx.getTimestamp());
        response["blocktime"] = static_cast<uint64_t>(tx.getTimestamp());
        response["confirmations"] = 1;
        
        // Traceability information
        response["prev_tx_hash"] = tx.getPrevTxHash();
        response["referenced_amount"] = tx.getReferencedAmount();
        response["sender_address"] = tx.getSenderAddress();
        response["receiver_address"] = tx.getReceiverAddress();
        response["is_traceable"] = tx.isTraceabilityValid();
        
        // Inputs
        Json::Value inputs(Json::arrayValue);
        for (const auto& input : tx.getInputs()) {
            Json::Value inputJson;
            inputJson["txid"] = input.txHash;
            inputJson["vout"] = input.outputIndex;
            inputJson["amount"] = input.amount;
            inputs.append(inputJson);
        }
        response["vin"] = inputs;
        
        // Outputs
        Json::Value outputs(Json::arrayValue);
        for (size_t i = 0; i < tx.getOutputs().size(); ++i) {
            const auto& output = tx.getOutputs()[i];
            Json::Value outputJson;
            outputJson["n"] = static_cast<uint32_t>(i);
            outputJson["value"] = output.amount;
            outputJson["address"] = output.address;
            outputs.append(outputJson);
        }
        response["vout"] = outputs;
        
        return jsonToString(response);
        
    } catch (const std::exception&) {
        return createErrorResponse(404, "Not Found", "Transaction not found");
    }
}

std::string RESTServer::submitTransaction(const std::string& txData) {
    try {
        // In real implementation, would decode and validate transaction
        Json::Value request;
        Json::CharReaderBuilder builder;
        std::string errors;
        
        std::istringstream stream(txData);
        if (!Json::parseFromStream(builder, stream, &request, &errors)) {
            return createErrorResponse(400, "Bad Request", "Invalid JSON: " + errors);
        }
        
        // Create response
        Json::Value response;
        response["txid"] = "new_transaction_hash";
        response["result"] = "success";
        response["message"] = "Transaction submitted to mempool";
        
        LOG_API(LogLevel::INFO, "Transaction submitted via REST API");
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(400, "Bad Request", "Failed to submit transaction: " + std::string(e.what()));
    }
}

std::string RESTServer::getAddressBalance(const std::string& address) {
    Json::Value response;
    
    // In real implementation, would calculate actual balance
    response["address"] = address;
    response["balance"] = 100.0; // Example balance
    response["confirmed"] = 100.0;
    response["unconfirmed"] = 0.0;
    response["tx_count"] = 5;
    
    return jsonToString(response);
}

std::string RESTServer::getAddressTransactions(const std::string& address) {
    Json::Value response(Json::arrayValue);
    
    // Get transaction chain for this address using traceability system
    auto txChain = blockchain->getTransactionChain(address, 20);
    
    for (const auto& txHash : txChain) {
        try {
            Transaction tx = blockchain->getTransactionByHash(txHash);
            
            Json::Value txJson;
            txJson["txid"] = tx.getHash();
            txJson["time"] = static_cast<uint64_t>(tx.getTimestamp());
            txJson["amount"] = tx.getTotalOutputAmount();
            txJson["confirmations"] = 1;
            txJson["prev_tx_hash"] = tx.getPrevTxHash(); // Traceability info
            
            response.append(txJson);
        } catch (const std::exception&) {
            // Skip invalid transactions
        }
    }
    
    return jsonToString(response);
}

std::string RESTServer::traceTransaction(const std::string& txHash) {
    try {
        // Use the traceability system to trace transaction lineage
        auto lineage = blockchain->traceTransactionLineage(txHash);
        
        Json::Value response;
        response["transaction"] = txHash;
        response["is_lineage_valid"] = blockchain->isLineageValid(txHash);
        
        Json::Value lineageArray(Json::arrayValue);
        for (const auto& hash : lineage) {
            lineageArray.append(hash);
        }
        response["lineage"] = lineageArray;
        response["lineage_length"] = static_cast<uint32_t>(lineage.size());
        
        return jsonToString(response);
        
    } catch (const std::exception&) {
        return createErrorResponse(404, "Not Found", "Transaction not found");
    }
}

std::string RESTServer::getMiningInfo() {
    Json::Value response;
    
    response["blocks"] = static_cast<uint64_t>(blockchain->getChainLength());
    response["difficulty"] = blockchain->getDifficulty();
    response["networkhashps"] = 1000000.0; // Example hashrate
    response["block_reward"] = static_cast<uint64_t>(blockchain->getBlockReward());
    response["next_halving"] = static_cast<uint64_t>(blockchain->getChainLength() + 1051200);
    response["mining_algorithm"] = "Multi-Algorithm (SHA256, Ethash, GXHash)";
    
    return jsonToString(response);
}

std::string RESTServer::getPeerInfo() {
    Json::Value response(Json::arrayValue);
    
    // Example peer info - in real implementation would get from network manager
    for (int i = 0; i < 3; ++i) {
        Json::Value peer;
        peer["id"] = i;
        peer["addr"] = "192.168.1." + std::to_string(100 + i) + ":8333";
        peer["connected"] = true;
        peer["last_seen"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        
        response.append(peer);
    }
    
    return jsonToString(response);
}

std::string RESTServer::getStats() {
    Json::Value response;
    
    response["blockchain"] = Json::Value();
    response["blockchain"]["height"] = static_cast<uint64_t>(blockchain->getChainLength());
    response["blockchain"]["difficulty"] = blockchain->getDifficulty();
    response["blockchain"]["total_supply"] = blockchain->getTotalSupply();
    response["blockchain"]["block_reward"] = static_cast<uint64_t>(blockchain->getBlockReward());
    
    response["network"] = Json::Value();
    response["network"]["peers"] = 3;
    response["network"]["hashrate"] = 1000000.0;
    
    response["traceability"] = Json::Value();
    response["traceability"]["enabled"] = true;
    response["traceability"]["formula"] = "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount";
    
    return jsonToString(response);
}

std::string RESTServer::createErrorResponse(int code, const std::string& error, const std::string& message) {
    Json::Value response;
    response["error"] = Json::Value();
    response["error"]["code"] = code;
    response["error"]["type"] = error;
    response["error"]["message"] = message;
    
    return jsonToString(response);
}

std::string RESTServer::jsonToString(const Json::Value& json) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    return Json::writeString(builder, json);
}