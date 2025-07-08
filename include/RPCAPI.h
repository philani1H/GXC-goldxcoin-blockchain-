#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>
#include "json/json.h"

class Blockchain;
class NetworkNode;
class Wallet;

// RPC method result
struct RPCResult {
    bool success = true;
    Json::Value result;
    std::string error;
    int errorCode = 0;
};

// RPC request structure
struct RPCRequest {
    std::string method;
    Json::Value params;
    std::string id;
    std::string jsonrpc = "2.0";
};

// Mining template for getblocktemplate
struct BlockTemplate {
    std::string blockTemplate;
    std::string target;
    uint32_t difficulty;
    uint32_t height;
    std::string previousBlockHash;
    std::vector<std::string> transactions;
    std::string coinbaseValue;
    std::string bits;
    std::time_t curtime;
    uint32_t version;
    std::string merkleRoot;
};

// Transaction info for RPC responses
struct TransactionInfo {
    std::string txid;
    std::string hash;
    uint32_t version;
    uint32_t size;
    uint32_t vsize;
    uint32_t weight;
    uint32_t locktime;
    std::vector<Json::Value> vin;
    std::vector<Json::Value> vout;
    std::string hex;
    std::string blockhash;
    uint32_t confirmations;
    std::time_t time;
    std::time_t blocktime;
};

// Block info for RPC responses
struct BlockInfo {
    std::string hash;
    uint32_t confirmations;
    uint32_t size;
    uint32_t height;
    uint32_t version;
    std::string merkleroot;
    std::vector<std::string> tx;
    std::time_t time;
    std::time_t mediantime;
    uint64_t nonce;
    std::string bits;
    double difficulty;
    std::string chainwork;
    uint32_t nTx;
    std::string previousblockhash;
    std::string nextblockhash;
    std::string miner;
    std::string blockType;
};

// Wallet info for RPC responses
struct WalletInfo {
    std::string address;
    double balance;
    double unconfirmedBalance;
    double stakingBalance;
    uint32_t txcount;
    std::string hdPath;
    bool isEncrypted;
    std::time_t lastActivity;
};

// Mining info for RPC responses
struct MiningInfo {
    uint32_t blocks;
    double difficulty;
    double networkhashps;
    uint32_t pooledtx;
    std::string chain;
    std::string warnings;
    double stakingInfo;
    uint32_t currentblocktx;
    uint32_t currentblocksize;
};

// Staking info
struct StakingInfo {
    bool enabled;
    bool staking;
    std::string errors;
    double currentblocksize;
    double currentblocktx;
    double difficulty;
    double search_interval;
    double weight;
    double netstakeweight;
    double expectedtime;
};

class RPCServer {
private:
    Blockchain* blockchain;
    NetworkNode* networkNode;
    uint16_t port;
    std::atomic<bool> running{false};
    std::thread serverThread;
    std::mutex walletMutex;
    
    // RPC method handlers
    std::unordered_map<std::string, std::function<RPCResult(const Json::Value&)>> methodHandlers;
    
    // Wallet management
    std::unordered_map<std::string, std::shared_ptr<Wallet>> wallets;
    std::shared_ptr<Wallet> currentWallet;
    
    // Authentication
    std::string rpcUser;
    std::string rpcPassword;
    bool requireAuth = false;
    
    // Mining
    std::unordered_map<std::string, BlockTemplate> blockTemplates;
    std::mutex miningMutex;
    
public:
    RPCServer(Blockchain* blockchainIn, NetworkNode* networkNodeIn, uint16_t portIn = 8332);
    ~RPCServer();
    
    // Server lifecycle
    bool start();
    void stop();
    bool isRunning() const { return running; }
    
    // Authentication
    void setCredentials(const std::string& user, const std::string& password);
    void enableAuthentication(bool enable = true) { requireAuth = enable; }
    
    // Request processing
    std::string processRequest(const std::string& request);
    RPCResult executeMethod(const std::string& method, const Json::Value& params);
    
private:
    // Server setup
    void initializeMethodHandlers();
    void handleHttpRequest(int clientSocket);
    bool authenticateRequest(const std::string& auth) const;
    
    // Response formatting
    std::string formatResponse(const RPCResult& result, const std::string& id = "");
    std::string formatError(int code, const std::string& message, const std::string& id = "");
    
    // === BLOCKCHAIN RPC METHODS ===
    
    // Block and blockchain info
    RPCResult getbestblockhash(const Json::Value& params);
    RPCResult getblock(const Json::Value& params);
    RPCResult getblockchaininfo(const Json::Value& params);
    RPCResult getblockcount(const Json::Value& params);
    RPCResult getblockhash(const Json::Value& params);
    RPCResult getblockheader(const Json::Value& params);
    RPCResult getchaintips(const Json::Value& params);
    RPCResult getdifficulty(const Json::Value& params);
    
    // Transaction methods
    RPCResult getrawtransaction(const Json::Value& params);
    RPCResult sendrawtransaction(const Json::Value& params);
    RPCResult gettransaction(const Json::Value& params);
    RPCResult listtransactions(const Json::Value& params);
    RPCResult gettxout(const Json::Value& params);
    RPCResult gettxoutproof(const Json::Value& params);
    RPCResult verifytxoutproof(const Json::Value& params);
    
    // === WALLET RPC METHODS ===
    
    // Wallet management
    RPCResult createwallet(const Json::Value& params);
    RPCResult loadwallet(const Json::Value& params);
    RPCResult unloadwallet(const Json::Value& params);
    RPCResult listwallets(const Json::Value& params);
    RPCResult getwalletinfo(const Json::Value& params);
    RPCResult encryptwallet(const Json::Value& params);
    RPCResult walletpassphrase(const Json::Value& params);
    RPCResult walletlock(const Json::Value& params);
    
    // Address management
    RPCResult getnewaddress(const Json::Value& params);
    RPCResult getaddressinfo(const Json::Value& params);
    RPCResult validateaddress(const Json::Value& params);
    RPCResult listreceivedbyaddress(const Json::Value& params);
    RPCResult listaddressgroupings(const Json::Value& params);
    
    // Balance and funds
    RPCResult getbalance(const Json::Value& params);
    RPCResult getunconfirmedbalance(const Json::Value& params);
    RPCResult listunspent(const Json::Value& params);
    RPCResult lockunspent(const Json::Value& params);
    RPCResult listlockunspent(const Json::Value& params);
    
    // Sending transactions
    RPCResult sendtoaddress(const Json::Value& params);
    RPCResult sendmany(const Json::Value& params);
    RPCResult createrawtransaction(const Json::Value& params);
    RPCResult signrawtransaction(const Json::Value& params);
    RPCResult fundrawtransaction(const Json::Value& params);
    
    // HD wallet methods
    RPCResult getmnemonic(const Json::Value& params);
    RPCResult importmnemonic(const Json::Value& params);
    RPCResult derivenewaddress(const Json::Value& params);
    
    // === MINING RPC METHODS ===
    
    // Mining operations
    RPCResult getblocktemplate(const Json::Value& params);
    RPCResult submitblock(const Json::Value& params);
    RPCResult getmininginfo(const Json::Value& params);
    RPCResult getnetworkhashps(const Json::Value& params);
    RPCResult prioritisetransaction(const Json::Value& params);
    
    // Mining pool methods
    RPCResult getwork(const Json::Value& params);
    RPCResult submitwork(const Json::Value& params);
    RPCResult getauxblock(const Json::Value& params);
    
    // === STAKING RPC METHODS ===
    
    // Staking operations
    RPCResult getstakinginfo(const Json::Value& params);
    RPCResult setstaking(const Json::Value& params);
    RPCResult liststaking(const Json::Value& params);
    RPCResult createstake(const Json::Value& params);
    RPCResult withdrawstake(const Json::Value& params);
    RPCResult getvalidators(const Json::Value& params);
    RPCResult getvalidatorinfo(const Json::Value& params);
    
    // === NETWORK RPC METHODS ===
    
    // Network information
    RPCResult getnetworkinfo(const Json::Value& params);
    RPCResult getpeerinfo(const Json::Value& params);
    RPCResult getconnectioncount(const Json::Value& params);
    RPCResult addnode(const Json::Value& params);
    RPCResult removenode(const Json::Value& params);
    RPCResult getnettotals(const Json::Value& params);
    
    // === ORACLE RPC METHODS ===
    
    // Price oracle methods
    RPCResult getprice(const Json::Value& params);
    RPCResult getpricehistory(const Json::Value& params);
    RPCResult submitprice(const Json::Value& params);
    RPCResult getoracles(const Json::Value& params);
    
    // === GOLD TOKEN RPC METHODS ===
    
    // GXC-G token methods
    RPCResult mintgoldtokens(const Json::Value& params);
    RPCResult burngoldtokens(const Json::Value& params);
    RPCResult getgoldbalance(const Json::Value& params);
    RPCResult getgoldreserves(const Json::Value& params);
    RPCResult transfergoldtokens(const Json::Value& params);
    
    // === GOVERNANCE RPC METHODS ===
    
    // Governance methods
    RPCResult submitproposal(const Json::Value& params);
    RPCResult listvotes(const Json::Value& params);
    RPCResult vote(const Json::Value& params);
    RPCResult getproposal(const Json::Value& params);
    RPCResult listproposals(const Json::Value& params);
    
    // === UTILITY RPC METHODS ===
    
    // Utility methods
    RPCResult help(const Json::Value& params);
    RPCResult stop(const Json::Value& params);
    RPCResult uptime(const Json::Value& params);
    RPCResult getmemoryinfo(const Json::Value& params);
    RPCResult ping(const Json::Value& params);
    
    // Helper methods
    Json::Value transactionToJson(const TransactionInfo& tx);
    Json::Value blockToJson(const BlockInfo& block);
    Json::Value walletToJson(const WalletInfo& wallet);
    Json::Value peerToJson(const struct PeerInfo& peer);
    
    // Error codes
    static constexpr int RPC_MISC_ERROR = -1;
    static constexpr int RPC_FORBIDDEN_BY_SAFE_MODE = -2;
    static constexpr int RPC_TYPE_ERROR = -3;
    static constexpr int RPC_INVALID_ADDRESS_OR_KEY = -5;
    static constexpr int RPC_OUT_OF_MEMORY = -7;
    static constexpr int RPC_INVALID_PARAMETER = -8;
    static constexpr int RPC_DATABASE_ERROR = -20;
    static constexpr int RPC_DESERIALIZATION_ERROR = -22;
    static constexpr int RPC_VERIFY_ERROR = -25;
    static constexpr int RPC_VERIFY_REJECTED = -26;
    static constexpr int RPC_VERIFY_ALREADY_IN_CHAIN = -27;
    static constexpr int RPC_IN_WARMUP = -28;
    static constexpr int RPC_WALLET_ERROR = -4;
    static constexpr int RPC_WALLET_INSUFFICIENT_FUNDS = -6;
    static constexpr int RPC_WALLET_INVALID_ACCOUNT_NAME = -11;
    static constexpr int RPC_WALLET_KEYPOOL_RAN_OUT = -12;
    static constexpr int RPC_WALLET_UNLOCK_NEEDED = -13;
    static constexpr int RPC_WALLET_PASSPHRASE_INCORRECT = -14;
    static constexpr int RPC_WALLET_WRONG_ENC_STATE = -15;
    static constexpr int RPC_WALLET_ENCRYPTION_FAILED = -16;
    static constexpr int RPC_WALLET_ALREADY_UNLOCKED = -17;
};

// REST API server for lightweight access
class RESTServer {
private:
    RPCServer* rpcServer;
    uint16_t port;
    std::atomic<bool> running{false};
    std::thread serverThread;
    
public:
    RESTServer(RPCServer* rpcServerIn, uint16_t portIn = 8080);
    ~RESTServer();
    
    bool start();
    void stop();
    
private:
    void handleHttpRequest(int clientSocket);
    std::string processRestRequest(const std::string& path, const std::string& method);
    
    // REST endpoints
    std::string getBlockchainInfo();
    std::string getBlock(const std::string& hash);
    std::string getTransaction(const std::string& txid);
    std::string getAddress(const std::string& address);
    std::string getMempool();
    std::string getPeers();
    std::string getMiningInfo();
    std::string getStakingInfo();
};

// WebSocket server for real-time updates
class WebSocketServer {
private:
    uint16_t port;
    std::atomic<bool> running{false};
    std::thread serverThread;
    std::vector<int> connectedClients;
    std::mutex clientsMutex;
    
public:
    WebSocketServer(uint16_t portIn = 8081);
    ~WebSocketServer();
    
    bool start();
    void stop();
    
    // Broadcasting
    void broadcastNewBlock(const std::string& blockHash);
    void broadcastNewTransaction(const std::string& txHash);
    void broadcastPriceUpdate(const Json::Value& priceData);
    void broadcastMiningUpdate(const Json::Value& miningData);
    
private:
    void handleConnection(int clientSocket);
    void broadcastMessage(const std::string& message);
    std::string createWebSocketFrame(const std::string& data);
};