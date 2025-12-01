#include "../include/RESTServer.h"
#include "../include/blockchain.h"
#include "../include/transaction.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <set>
#include <algorithm>
#include <nlohmann/json.hpp>

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
    try {
        nlohmann::json response;
        
        uint32_t height = blockchain->getHeight();
        Block latestBlock = blockchain->getLatestBlock();
        
        // Calculate block reward for current height
        double blockReward = 50.0;  // Base reward
        uint32_t halvings = height / 1051200;  // Halving every 1051200 blocks
        for (uint32_t i = 0; i < halvings; i++) {
            blockReward /= 2.0;
        }
        if (blockReward < 0.00000001) {
            blockReward = 0.00000001;  // Minimum
        }
        
        response["chain"] = "main";
        response["blocks"] = static_cast<uint64_t>(height);
        response["height"] = static_cast<uint64_t>(height);  // Alternative field name
        response["bestblockhash"] = latestBlock.getHash().empty() ? "0000000000000000000000000000000000000000000000000000000000000000" : latestBlock.getHash();
        response["best_block_hash"] = latestBlock.getHash().empty() ? "0000000000000000000000000000000000000000000000000000000000000000" : latestBlock.getHash();  // Alternative field name
        response["difficulty"] = blockchain->getDifficulty();
        response["mediantime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        response["verificationprogress"] = 1.0;
        response["initialblockdownload"] = false;
        response["pruned"] = false;
        response["block_reward"] = blockReward;
        response["reward"] = blockReward;  // Alternative field name
        response["total_supply"] = 50000000.0;  // Would calculate from actual supply
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error in getBlockchainInfo: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::getBlock(const std::string& blockId) {
    try {
        Block block;
        uint32_t height = 0;
        
        // Check if blockId is a hash or height
        if (blockId.length() == 64) {
            // Assume it's a hash
            block = blockchain->getBlock(blockId);
            if (block.getHash().empty()) {
                return createErrorResponse(404, "Not Found", "Block not found");
            }
            height = block.getIndex();
        } else {
            // Assume it's a height
            try {
                height = std::stoul(blockId);
                if (height >= blockchain->getHeight()) {
                    return createErrorResponse(404, "Not Found", "Block not found");
                }
                block = blockchain->getBlock(height);
                if (block.getHash().empty()) {
                    return createErrorResponse(404, "Not Found", "Block not found");
                }
            } catch (const std::exception&) {
                return createErrorResponse(400, "Bad Request", "Invalid block identifier");
            }
        }
        
        // Get previous block for previous hash
        Block prevBlock;
        if (height > 0) {
            prevBlock = blockchain->getBlock(height - 1);
        }
        
        nlohmann::json response;
        response["hash"] = block.getHash();
        response["height"] = height;
        response["number"] = height;  // Alternative field name
        response["block_number"] = height;  // Explorer expects this
        response["confirmations"] = blockchain->getHeight() - height;
        response["version"] = 1;
        response["merkleroot"] = block.getMerkleRoot();
        response["merkle_root"] = block.getMerkleRoot();  // Alternative field name
        response["time"] = static_cast<uint64_t>(block.getTimestamp());
        response["timestamp"] = static_cast<uint64_t>(block.getTimestamp());  // Alternative field name
        response["nonce"] = block.getNonce();
        response["bits"] = "1d00ffff";
        response["difficulty"] = block.getDifficulty();
        response["previousblockhash"] = prevBlock.getHash().empty() ? "0000000000000000000000000000000000000000000000000000000000000000" : prevBlock.getHash();
        response["parent_hash"] = prevBlock.getHash().empty() ? "0000000000000000000000000000000000000000000000000000000000000000" : prevBlock.getHash();  // Alternative field name
        response["miner"] = block.getMinerAddress();
        response["miner_address"] = block.getMinerAddress();  // Alternative field name
        response["reward"] = block.getBlockReward();
        response["block_reward"] = block.getBlockReward();  // Alternative field name
        response["size"] = 0;  // Would calculate actual size
        response["gas_used"] = 0;
        response["gas_limit"] = 0;
        response["transaction_count"] = static_cast<uint32_t>(block.getTransactions().size());
        response["tx_count"] = static_cast<uint32_t>(block.getTransactions().size());
        response["nTx"] = static_cast<uint32_t>(block.getTransactions().size());
        
        // Include all transactions with full data
        nlohmann::json transactions(nlohmann::json::array());
        for (size_t i = 0; i < block.getTransactions().size(); ++i) {
            const auto& tx = block.getTransactions()[i];
            nlohmann::json txJson;
            txJson["hash"] = tx.getHash();
            txJson["txid"] = tx.getHash();
            txJson["tx_hash"] = tx.getHash();  // Alternative field name
            txJson["index"] = static_cast<uint32_t>(i);
            txJson["tx_index"] = static_cast<uint32_t>(i);  // Alternative field name
            txJson["time"] = static_cast<uint64_t>(tx.getTimestamp());
            txJson["timestamp"] = static_cast<uint64_t>(tx.getTimestamp());  // Alternative field name
            txJson["is_coinbase"] = tx.isCoinbaseTransaction();
            txJson["coinbase"] = tx.isCoinbaseTransaction();  // Alternative field name
            
            // Inputs
            nlohmann::json inputs(nlohmann::json::array());
            for (const auto& input : tx.getInputs()) {
                nlohmann::json inputJson;
                inputJson["txid"] = input.txHash;
                inputJson["vout"] = input.outputIndex;
                inputJson["amount"] = input.amount;
                inputs.push_back(inputJson);
            }
            txJson["vin"] = inputs;
            txJson["inputs"] = inputs;  // Alternative field name
            
            // Outputs
            nlohmann::json outputs(nlohmann::json::array());
            for (size_t j = 0; j < tx.getOutputs().size(); ++j) {
                const auto& output = tx.getOutputs()[j];
                nlohmann::json outputJson;
                outputJson["n"] = static_cast<uint32_t>(j);
                outputJson["value"] = output.amount;
                outputJson["amount"] = output.amount;  // Alternative field name
                outputJson["address"] = output.address;
                outputs.push_back(outputJson);
            }
            txJson["vout"] = outputs;
            txJson["outputs"] = outputs;  // Alternative field name
            
            // Traceability info
            txJson["prev_tx_hash"] = tx.getPrevTxHash();
            txJson["referenced_amount"] = tx.getReferencedAmount();
            txJson["sender_address"] = tx.getSenderAddress();
            txJson["receiver_address"] = tx.getReceiverAddress();
            txJson["from_address"] = tx.getSenderAddress();  // Alternative field name
            txJson["to_address"] = tx.getReceiverAddress();  // Alternative field name
            
            transactions.push_back(txJson);
        }
        response["tx"] = transactions;
        response["transactions"] = transactions;  // Alternative field name
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error in getBlock: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::getBlocks() {
    nlohmann::json response(nlohmann::json::array());
    
    // Return last 20 blocks with real data
    uint32_t chainLength = blockchain->getHeight();
    uint32_t startBlock = chainLength > 20 ? chainLength - 20 : 0;
    
    for (uint32_t i = startBlock; i < chainLength; ++i) {
        try {
            Block block = blockchain->getBlock(i);
            if (block.getHash().empty()) {
                continue;  // Skip invalid blocks
            }
            
            nlohmann::json blockJson;
            blockJson["height"] = i;
            blockJson["number"] = i;  // Alternative field name
            blockJson["block_number"] = i;  // Explorer expects this
            blockJson["hash"] = block.getHash();
            blockJson["block_hash"] = block.getHash();  // Alternative field name
            blockJson["time"] = static_cast<uint64_t>(block.getTimestamp());
            blockJson["timestamp"] = static_cast<uint64_t>(block.getTimestamp());  // Alternative field name
            blockJson["tx_count"] = static_cast<uint32_t>(block.getTransactions().size());
            blockJson["transaction_count"] = static_cast<uint32_t>(block.getTransactions().size());  // Alternative field name
            blockJson["miner"] = block.getMinerAddress();
            blockJson["miner_address"] = block.getMinerAddress();  // Alternative field name
            blockJson["reward"] = block.getBlockReward();
            blockJson["block_reward"] = block.getBlockReward();  // Alternative field name
            blockJson["difficulty"] = block.getDifficulty();
            
            // Get previous block hash
            if (i > 0) {
                Block prevBlock = blockchain->getBlock(i - 1);
                blockJson["previousblockhash"] = prevBlock.getHash();
                blockJson["parent_hash"] = prevBlock.getHash();  // Alternative field name
            } else {
                blockJson["previousblockhash"] = "0000000000000000000000000000000000000000000000000000000000000000";
                blockJson["parent_hash"] = "0000000000000000000000000000000000000000000000000000000000000000";
            }
            
            response.push_back(blockJson);
        } catch (const std::exception&) {
            // Skip blocks that can't be retrieved
            continue;
        }
    }
    
    return jsonToString(response);
}

std::string RESTServer::getTransaction(const std::string& txHash) {
    try {
        // Search for transaction in all blocks
        Transaction tx;
        uint32_t blockHeight = 0;
        std::string blockHash;
        bool found = false;
        
        uint32_t chainLength = blockchain->getHeight();
        for (uint32_t h = 0; h < chainLength && !found; ++h) {
            Block block = blockchain->getBlock(h);
            if (block.getHash().empty()) continue;
            
            for (const auto& blockTx : block.getTransactions()) {
                if (blockTx.getHash() == txHash) {
                    tx = blockTx;
                    blockHeight = h;
                    blockHash = block.getHash();
                    found = true;
                    break;
                }
            }
        }
        
        if (!found || tx.getHash().empty()) {
            return createErrorResponse(404, "Not Found", "Transaction not found");
        }
        
        nlohmann::json response;
        response["txid"] = tx.getHash();
        response["hash"] = tx.getHash();
        response["tx_hash"] = tx.getHash();  // Alternative field name
        response["version"] = 1;
        response["size"] = 250;  // Would calculate actual size
        response["vsize"] = 250;
        response["weight"] = 1000;
        response["locktime"] = tx.getLockTime();
        response["time"] = static_cast<uint64_t>(tx.getTimestamp());
        response["timestamp"] = static_cast<uint64_t>(tx.getTimestamp());  // Alternative field name
        response["blocktime"] = static_cast<uint64_t>(tx.getTimestamp());
        response["block_height"] = blockHeight;
        response["block_number"] = blockHeight;  // Alternative field name
        response["block_hash"] = blockHash;
        response["confirmations"] = chainLength - blockHeight;
        response["is_coinbase"] = tx.isCoinbaseTransaction();
        response["coinbase"] = tx.isCoinbaseTransaction();  // Alternative field name
        
        // Traceability information
        response["prev_tx_hash"] = tx.getPrevTxHash();
        response["referenced_amount"] = tx.getReferencedAmount();
        response["sender_address"] = tx.getSenderAddress();
        response["receiver_address"] = tx.getReceiverAddress();
        response["from_address"] = tx.getSenderAddress();  // Alternative field name
        response["to_address"] = tx.getReceiverAddress();  // Alternative field name
        response["is_traceable"] = tx.isTraceabilityValid();
        
        // Inputs
        nlohmann::json inputs(nlohmann::json::array());
        for (const auto& input : tx.getInputs()) {
            nlohmann::json inputJson;
            inputJson["txid"] = input.txHash;
            inputJson["vout"] = input.outputIndex;
            inputJson["amount"] = input.amount;
            inputs.push_back(inputJson);
        }
        response["vin"] = inputs;
        response["inputs"] = inputs;  // Alternative field name
        
        // Outputs
        nlohmann::json outputs(nlohmann::json::array());
        double totalValue = 0.0;
        for (size_t i = 0; i < tx.getOutputs().size(); ++i) {
            const auto& output = tx.getOutputs()[i];
            nlohmann::json outputJson;
            outputJson["n"] = static_cast<uint32_t>(i);
            outputJson["value"] = output.amount;
            outputJson["amount"] = output.amount;  // Alternative field name
            outputJson["address"] = output.address;
            outputs.push_back(outputJson);
            totalValue += output.amount;
        }
        response["vout"] = outputs;
        response["outputs"] = outputs;  // Alternative field name
        response["value"] = totalValue;
        response["amount"] = totalValue;  // Alternative field name
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error in getTransaction: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::submitTransaction(const std::string& txData) {
    try {
        // In real implementation, would decode and validate transaction
        nlohmann::json request;
        try {
            request = nlohmann::json::parse(txData);
        } catch (const std::exception& e) {
            return createErrorResponse(400, "Bad Request", "Invalid JSON: " + std::string(e.what()));
        }
        
        // Create response
        nlohmann::json response;
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
    try {
        nlohmann::json response;
        
        // Get real balance from blockchain
        double balance = blockchain->getBalance(address);
        
        // Count transactions for this address
        uint32_t txCount = 0;
        uint32_t chainLength = blockchain->getHeight();
        for (uint32_t h = 0; h < chainLength; ++h) {
            Block block = blockchain->getBlock(h);
            if (block.getHash().empty()) continue;
            
            for (const auto& tx : block.getTransactions()) {
                // Check if address is in outputs (receiving)
                for (const auto& output : tx.getOutputs()) {
                    if (output.address == address) {
                        txCount++;
                        break;
                    }
                }
                // Check if address is in inputs (sending) - would need to trace back
                // For now, just count outputs
            }
        }
        
        response["address"] = address;
        response["balance"] = balance;
        response["confirmed"] = balance;
        response["unconfirmed"] = 0.0;  // Would calculate from mempool
        response["tx_count"] = txCount;
        response["transaction_count"] = txCount;  // Alternative field name
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error in getAddressBalance: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::getAddressTransactions(const std::string& address) {
    try {
        nlohmann::json response = nlohmann::json::array();
        
        // Get transaction chain for this address using traceability system
        std::vector<std::string> txChain = blockchain->getTransactionChain(address, 50);
        
        uint32_t chainLength = blockchain->getHeight();
        
        // Also search all blocks for transactions involving this address
        std::set<std::string> seenTxHashes;
        
        for (uint32_t h = 0; h < chainLength; ++h) {
            Block block = blockchain->getBlock(h);
            if (block.getHash().empty()) continue;
            
            for (const auto& tx : block.getTransactions()) {
                // Skip if already seen
                if (seenTxHashes.find(tx.getHash()) != seenTxHashes.end()) {
                    continue;
                }
                
                bool involvesAddress = false;
                double amount = 0.0;
                
                // Check if address is in outputs (receiving)
                for (const auto& output : tx.getOutputs()) {
                    if (output.address == address) {
                        involvesAddress = true;
                        amount += output.amount;
                    }
                }
                
                // Check if address is sender (would need to trace inputs)
                if (tx.getSenderAddress() == address || tx.getReceiverAddress() == address) {
                    involvesAddress = true;
                }
                
                if (involvesAddress) {
                    seenTxHashes.insert(tx.getHash());
                    
                    nlohmann::json txJson;
                    txJson["txid"] = tx.getHash();
                    txJson["hash"] = tx.getHash();
                    txJson["tx_hash"] = tx.getHash();  // Alternative field name
                    txJson["time"] = static_cast<uint64_t>(tx.getTimestamp());
                    txJson["timestamp"] = static_cast<uint64_t>(tx.getTimestamp());  // Alternative field name
                    txJson["amount"] = amount > 0 ? amount : tx.getTotalOutputAmount();
                    txJson["value"] = amount > 0 ? amount : tx.getTotalOutputAmount();  // Alternative field name
                    txJson["block_height"] = h;
                    txJson["block_number"] = h;  // Alternative field name
                    txJson["block_hash"] = block.getHash();
                    txJson["confirmations"] = chainLength - h;
                    txJson["prev_tx_hash"] = tx.getPrevTxHash();  // Traceability info
                    txJson["is_coinbase"] = tx.isCoinbaseTransaction();
                    
                    // Add input/output info
                    nlohmann::json inputs(nlohmann::json::array());
                    for (const auto& input : tx.getInputs()) {
                        nlohmann::json inputJson;
                        inputJson["txid"] = input.txHash;
                        inputJson["vout"] = input.outputIndex;
                        inputJson["amount"] = input.amount;
                        inputs.push_back(inputJson);
                    }
                    txJson["vin"] = inputs;
                    
                    nlohmann::json outputs(nlohmann::json::array());
                    for (size_t i = 0; i < tx.getOutputs().size(); ++i) {
                        const auto& output = tx.getOutputs()[i];
                        nlohmann::json outputJson;
                        outputJson["n"] = static_cast<uint32_t>(i);
                        outputJson["value"] = output.amount;
                        outputJson["address"] = output.address;
                        outputs.push_back(outputJson);
                    }
                    txJson["vout"] = outputs;
                    
                    response.push_back(txJson);
                }
            }
        }
        
        // Sort by block height descending (newest first)
        std::sort(response.begin(), response.end(), 
            [](const nlohmann::json& a, const nlohmann::json& b) {
                return a.value("block_height", 0) > b.value("block_height", 0);
            });
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error in getAddressTransactions: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::traceTransaction(const std::string& txHash) {
    try {
        // Use the traceability system to trace transaction lineage
        std::vector<std::string> lineage = blockchain->traceTransactionLineage(txHash);
        bool isLineageValid = blockchain->isLineageValid(txHash);
        
        nlohmann::json response;
        response["transaction"] = txHash;
        response["tx_hash"] = txHash;  // Alternative field name
        response["is_lineage_valid"] = isLineageValid;
        response["lineage_valid"] = isLineageValid;  // Alternative field name
        
        nlohmann::json lineageArray(nlohmann::json::array());
        for (const auto& hash : lineage) {
            lineageArray.push_back(hash);
        }
        response["lineage"] = lineageArray;
        response["lineage_length"] = static_cast<uint32_t>(lineage.size());
        response["lineageLength"] = static_cast<uint32_t>(lineage.size());  // Alternative field name
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error in traceTransaction: " + std::string(e.what()));
        return createErrorResponse(500, "Internal Server Error", e.what());
    }
}

std::string RESTServer::getMiningInfo() {
    nlohmann::json response;
    
    response["blocks"] = static_cast<uint64_t>(blockchain->getHeight());
    response["difficulty"] = blockchain->getDifficulty();
    response["networkhashps"] = 1000000.0; // Example hashrate
    response["block_reward"] = static_cast<uint64_t>(5000000.0);
    response["next_halving"] = static_cast<uint64_t>(blockchain->getHeight() + 1051200);
    response["mining_algorithm"] = "Multi-Algorithm (SHA256, Ethash, GXHash)";
    
    return jsonToString(response);
}

std::string RESTServer::getPeerInfo() {
    nlohmann::json response(nlohmann::json::array());
    
    // Example peer info - in real implementation would get from network manager
    for (int i = 0; i < 3; ++i) {
        nlohmann::json peer;
        peer["id"] = i;
        peer["addr"] = "192.168.1." + std::to_string(100 + i) + ":8333";
        peer["connected"] = true;
        peer["last_seen"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
        
        response.push_back(peer);
    }
    
    return jsonToString(response);
}

std::string RESTServer::getStats() {
    nlohmann::json response;
    
    response["blockchain"] = nlohmann::json();
    response["blockchain"]["height"] = static_cast<uint64_t>(blockchain->getHeight());
    response["blockchain"]["difficulty"] = blockchain->getDifficulty();
    response["blockchain"]["total_supply"] = 50000000.0;
    response["blockchain"]["block_reward"] = static_cast<uint64_t>(5000000.0);
    
    response["network"] = nlohmann::json();
    response["network"]["peers"] = 3;
    response["network"]["hashrate"] = 1000000.0;
    
    response["traceability"] = nlohmann::json();
    response["traceability"]["enabled"] = true;
    response["traceability"]["formula"] = "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount";
    
    return jsonToString(response);
}

std::string RESTServer::createErrorResponse(int code, const std::string& error, const std::string& message) {
    nlohmann::json response;
    response["error"] = nlohmann::json();
    response["error"]["code"] = code;
    response["error"]["type"] = error;
    response["error"]["message"] = message;
    
    return jsonToString(response);
}

std::string RESTServer::jsonToString(const nlohmann::json& json) {
    return json.dump(2);  // Pretty print with 2-space indentation
}