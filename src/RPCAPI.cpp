#include "../include/RPCAPI.h"
#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/Network.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

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
    rpcMethods["getblockhash"] = [this](const JsonValue& params) { return getBlockHash(params); };
    
    // Transaction methods
    rpcMethods["getrawtransaction"] = [this](const JsonValue& params) { return getRawTransaction(params); };
    rpcMethods["sendrawtransaction"] = [this](const JsonValue& params) { return sendRawTransaction(params); };
    rpcMethods["gettransaction"] = [this](const JsonValue& params) { return getTransaction(params); };
    rpcMethods["listtransactions"] = [this](const JsonValue& params) { return listTransactions(params); };
    
    // Wallet methods
    rpcMethods["getbalance"] = [this](const JsonValue& params) { return getBalance(params); };
    rpcMethods["getnewaddress"] = [this](const JsonValue& params) { return getNewAddress(params); };
    rpcMethods["sendtoaddress"] = [this](const JsonValue& params) { return sendToAddress(params); };
    rpcMethods["validateaddress"] = [this](const JsonValue& params) { return validateAddress(params); };
    rpcMethods["listaccounts"] = [this](const JsonValue& params) { return listAccounts(params); };
    
    // Mining methods
    rpcMethods["getmininginfo"] = [this](const JsonValue& params) { return getMiningInfo(params); };
    rpcMethods["getnetworkhashps"] = [this](const JsonValue& params) { return getNetworkHashPS(params); };
    rpcMethods["submitblock"] = [this](const JsonValue& params) { return submitBlock(params); };
    rpcMethods["getblocktemplate"] = [this](const JsonValue& params) { return getBlockTemplate(params); };
    
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

void RPCAPI::processRequests() {
    // RPC requests are processed in the serverLoop thread
    // This method can be used for synchronous processing if needed
    // For now, it's a no-op since serverLoop handles requests asynchronously
}

// Blockchain information methods
JsonValue RPCAPI::getBlockchainInfo(const JsonValue& params) {
    JsonValue result;
    
    uint32_t currentHeight = blockchain->getHeight();
    double blockReward = calculateBlockReward(currentHeight);
    
    result["chain"] = "main"; // or "test" for testnet
    result["blocks"] = static_cast<uint64_t>(currentHeight);
    result["height"] = static_cast<uint64_t>(currentHeight);
    result["bestblockhash"] = blockchain->getLatestBlock().getHash();
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
    return blockchain->getLatestBlock().getHash();
}

JsonValue RPCAPI::getBlockCount(const JsonValue& params) {
    return static_cast<uint64_t>(blockchain->getHeight() - 1); // 0-indexed
}

JsonValue RPCAPI::getDifficulty(const JsonValue& params) {
    return blockchain->getDifficulty();
}

JsonValue RPCAPI::getBlock(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block hash parameter");
    }
    
    std::string blockHash = params[0].get<std::string>();
    bool verbose = params.size() > 1 ? params[1].get<bool>() : true;
    
    // In a real implementation, would retrieve block by hash
    JsonValue result;
    result["hash"] = blockHash;
    result["confirmations"] = 1;
    result["height"] = blockchain->getHeight() - 1;
    result["version"] = 1;
    result["merkleroot"] = "merkleroot_hash";
    result["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["nonce"] = 0;
    result["bits"] = "1d00ffff";
    result["difficulty"] = blockchain->getDifficulty();
    result["previousblockhash"] = "previous_hash";
    result["tx"] = JsonValue(JsonValue::array());
    
    return result;
}

JsonValue RPCAPI::getBlockHash(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing block height parameter");
    }
    
    uint32_t height = params[0].get<uint64_t>();
    
    if (height >= blockchain->getHeight()) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Block height out of range");
    }
    
    // In a real implementation, would get block hash by height
    return "block_hash_at_height_" + std::to_string(height);
}

// Transaction methods
JsonValue RPCAPI::getRawTransaction(const JsonValue& params) {
    if (params.size() < 1) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing transaction hash parameter");
    }
    
    std::string txHash = params[0].get<std::string>();
    bool verbose = params.size() > 1 ? params[1].get<bool>() : false;
    
    // In a real implementation, would retrieve transaction by hash
    if (verbose) {
        JsonValue result;
        result["txid"] = txHash;
        result["hash"] = txHash;
        result["version"] = 1;
        result["size"] = 250;
        result["vsize"] = 250;
        result["weight"] = 1000;
        result["locktime"] = 0;
        result["vin"] = JsonValue(JsonValue::array());
        result["vout"] = JsonValue(JsonValue::array());
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
    
    JsonValue result;
    result["amount"] = 0.0;
    result["fee"] = -0.0001;
    result["confirmations"] = 1;
    result["blockhash"] = "block_hash";
    result["blockindex"] = 0;
    result["blocktime"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["txid"] = txHash;
    result["time"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["timereceived"] = static_cast<uint64_t>(Utils::getCurrentTimestamp());
    result["details"] = JsonValue(JsonValue::array());
    result["hex"] = "raw_hex_data";
    
    return result;
}

JsonValue RPCAPI::listTransactions(const JsonValue& params) {
    std::string account = params.size() > 0 ? params[0].get<std::string>() : "*";
    int count = params.size() > 1 ? params[1].get<int>() : 10;
    int skip = params.size() > 2 ? params[2].get<int>() : 0;
    
    JsonValue result(JsonValue::array());
    
    // In a real implementation, would retrieve actual transactions
    for (int i = 0; i < std::min(count, 5); ++i) {
        JsonValue tx;
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
        
        result.push_back(tx);
    }
    
    return result;
}

// Wallet methods
JsonValue RPCAPI::getBalance(const JsonValue& params) {
    std::string address = params.size() > 0 ? params[0].get<std::string>() : "";
    
    if (address.empty()) {
        throw RPCException(RPCException::RPC_INVALID_PARAMETER, "Missing address parameter");
    }
    
    // Get balance from blockchain using UTXO set
    double balance = blockchain->getBalance(address);
    
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
    
    // In a real implementation, would create and send transaction
    LOG_API(LogLevel::INFO, "Sending " + std::to_string(amount) + " GXC to " + address);
    
    return "transaction_hash";
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
        
        // Get transactions from block
        std::vector<Transaction> blockTransactions;
        if (blockData.contains("transactions") && blockData["transactions"].is_array()) {
            // Transactions are already in block (from Python miner)
            // We'll process them below
        }
        
        // Calculate block reward with halving
        double blockReward = calculateBlockReward(height);
        
        // Create Block object
        Block newBlock(height, prevHash, BlockType::POW_SHA256);
        newBlock.setHash(hash);
        newBlock.setNonce(nonce);
        newBlock.setTimestamp(timestamp);
        newBlock.setDifficulty(difficulty);
        newBlock.setMinerAddress(minerAddress);
        newBlock.setBlockReward(blockReward);
        
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
        }
        
        // Merkle root already calculated if coinbase was added
        // Recalculate to ensure it's correct
        newBlock.calculateMerkleRoot();
        
        // Add block to blockchain
        if (blockchain->addBlock(newBlock)) {
            LOG_API(LogLevel::INFO, "Block submitted and added successfully. Height: " + std::to_string(height) + ", Hash: " + hash.substr(0, 16) + "...");
            return JsonValue(); // Success returns null
        } else {
            throw RPCException(RPCException::RPC_VERIFY_REJECTED, "Block validation failed");
        }
        
    } catch (const std::exception& e) {
        LOG_API(LogLevel::ERROR, "Error submitting block: " + std::string(e.what()));
        throw RPCException(RPCException::RPC_INTERNAL_ERROR, "Failed to submit block: " + std::string(e.what()));
    }
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
    
    // Parse inputs
    if (txJson.contains("inputs") && txJson["inputs"].is_array()) {
        for (const auto& inputJson : txJson["inputs"]) {
            TransactionInput input;
            input.txHash = inputJson.value("txHash", inputJson.value("prev_tx_hash", ""));
            input.outputIndex = inputJson.value("outputIndex", inputJson.value("output_index", 0));
            input.amount = inputJson.value("amount", 0.0);
            tx.addInput(input);
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
    JsonValue result;
    
    uint32_t currentHeight = blockchain->getHeight();
    double blockReward = calculateBlockReward(currentHeight);
    
    result["version"] = 1;
    result["previousblockhash"] = blockchain->getLatestBlock().getHash();
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
    result["height"] = static_cast<uint64_t>(currentHeight);
    result["difficulty"] = blockchain->getDifficulty();
    
    return result;
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