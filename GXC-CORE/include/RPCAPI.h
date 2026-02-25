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
#include <nlohmann/json.hpp>
using JsonValue = nlohmann::json;

class Blockchain;
class NetworkNode;
class Network;
class Wallet;
class Transaction;

// RPC Exception class
class RPCException : public std::runtime_error {
private:
    int errorCode;
    
public:
    RPCException(int code, const std::string& message);
    int getCode() const;
    
    // Error code constants
    static constexpr int RPC_INVALID_PARAMETER = -8;
    static constexpr int RPC_METHOD_NOT_FOUND = -32601;
    static constexpr int RPC_INVALID_REQUEST = -32600;
    static constexpr int RPC_INTERNAL_ERROR = -32603;
    static constexpr int RPC_VERIFY_REJECTED = -25;
    static constexpr int RPC_MISC_ERROR = -1;
    static constexpr int RPC_TYPE_ERROR = -3;
    static constexpr int RPC_INVALID_ADDRESS_OR_KEY = -5;
    static constexpr int RPC_WALLET_ERROR = -4;
    static constexpr int RPC_WALLET_INSUFFICIENT_FUNDS = -6;
    static constexpr int RPC_DESERIALIZATION_ERROR = -22;
};

// RPC method result
struct RPCResult {
    bool success = true;
    JsonValue result;
    std::string error;
    int errorCode = 0;
};

// RPC request structure
struct RPCRequest {
    std::string method;
    JsonValue params;
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
    std::vector<JsonValue> vin;
    std::vector<JsonValue> vout;
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
    std::unordered_map<std::string, std::function<RPCResult(const JsonValue&)>> methodHandlers;
    
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
    RPCResult executeMethod(const std::string& method, const JsonValue& params);
    
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
    RPCResult getbestblockhash(const JsonValue& params);
    RPCResult getblock(const JsonValue& params);
    RPCResult getblockchaininfo(const JsonValue& params);
    RPCResult getblockcount(const JsonValue& params);
    RPCResult getblockhash(const JsonValue& params);
    RPCResult getblockheader(const JsonValue& params);
    RPCResult getchaintips(const JsonValue& params);
    RPCResult getdifficulty(const JsonValue& params);
    
    // Transaction methods
    RPCResult getrawtransaction(const JsonValue& params);
    RPCResult sendrawtransaction(const JsonValue& params);
    RPCResult gettransaction(const JsonValue& params);
    RPCResult listtransactions(const JsonValue& params);
    RPCResult gettxout(const JsonValue& params);
    RPCResult gettxoutproof(const JsonValue& params);
    RPCResult verifytxoutproof(const JsonValue& params);
    
    // === WALLET RPC METHODS ===
    
    // Wallet management
    RPCResult createwallet(const JsonValue& params);
    RPCResult loadwallet(const JsonValue& params);
    RPCResult unloadwallet(const JsonValue& params);
    RPCResult listwallets(const JsonValue& params);
    RPCResult getwalletinfo(const JsonValue& params);
    RPCResult encryptwallet(const JsonValue& params);
    RPCResult walletpassphrase(const JsonValue& params);
    RPCResult walletlock(const JsonValue& params);
    
    // Address management
    RPCResult getnewaddress(const JsonValue& params);
    RPCResult getaddressinfo(const JsonValue& params);
    RPCResult validateaddress(const JsonValue& params);
    RPCResult listreceivedbyaddress(const JsonValue& params);
    RPCResult listaddressgroupings(const JsonValue& params);
    
    // Balance and funds
    RPCResult getbalance(const JsonValue& params);
    RPCResult getunconfirmedbalance(const JsonValue& params);
    RPCResult listunspent(const JsonValue& params);
    RPCResult lockunspent(const JsonValue& params);
    RPCResult listlockunspent(const JsonValue& params);
    
    // Sending transactions
    RPCResult sendtoaddress(const JsonValue& params);
    RPCResult sendmany(const JsonValue& params);
    RPCResult createrawtransaction(const JsonValue& params);
    RPCResult signrawtransaction(const JsonValue& params);
    RPCResult fundrawtransaction(const JsonValue& params);
    
    // HD wallet methods
    RPCResult getmnemonic(const JsonValue& params);
    RPCResult importmnemonic(const JsonValue& params);
    RPCResult derivenewaddress(const JsonValue& params);
    
    // === MINING RPC METHODS ===
    
    // Mining operations
    RPCResult getblocktemplate(const JsonValue& params);
    RPCResult submitblock(const JsonValue& params);
    RPCResult getmininginfo(const JsonValue& params);
    RPCResult getnetworkhashps(const JsonValue& params);
    RPCResult prioritisetransaction(const JsonValue& params);
    
    // Mining pool methods
    RPCResult getwork(const JsonValue& params);
    RPCResult submitwork(const JsonValue& params);
    RPCResult getauxblock(const JsonValue& params);
    
    // === STAKING RPC METHODS ===
    
    // Staking operations
    RPCResult getstakinginfo(const JsonValue& params);
    RPCResult setstaking(const JsonValue& params);
    RPCResult liststaking(const JsonValue& params);
    RPCResult createstake(const JsonValue& params);
    RPCResult withdrawstake(const JsonValue& params);
    RPCResult getvalidators(const JsonValue& params);
    RPCResult getvalidatorinfo(const JsonValue& params);
    
    // === NETWORK RPC METHODS ===
    
    // Network information
    RPCResult getnetworkinfo(const JsonValue& params);
    RPCResult getpeerinfo(const JsonValue& params);
    RPCResult getconnectioncount(const JsonValue& params);
    RPCResult addnode(const JsonValue& params);
    RPCResult removenode(const JsonValue& params);
    RPCResult getnettotals(const JsonValue& params);
    
    // === ORACLE RPC METHODS ===
    
    // Price oracle methods
    RPCResult getprice(const JsonValue& params);
    RPCResult getpricehistory(const JsonValue& params);
    RPCResult submitprice(const JsonValue& params);
    RPCResult getoracles(const JsonValue& params);
    
    // === GOLD TOKEN RPC METHODS ===
    
    // GXC-G token methods
    RPCResult mintgoldtokens(const JsonValue& params);
    RPCResult burngoldtokens(const JsonValue& params);
    RPCResult getgoldbalance(const JsonValue& params);
    RPCResult getgoldreserves(const JsonValue& params);
    RPCResult transfergoldtokens(const JsonValue& params);
    
    // === GOVERNANCE RPC METHODS ===
    
    // Governance methods
    RPCResult submitproposal(const JsonValue& params);
    RPCResult listvotes(const JsonValue& params);
    RPCResult vote(const JsonValue& params);
    RPCResult getproposal(const JsonValue& params);
    RPCResult listproposals(const JsonValue& params);
    
    // === UTILITY RPC METHODS ===
    
    // Utility methods
    RPCResult help(const JsonValue& params);
    RPCResult stop(const JsonValue& params);
    RPCResult uptime(const JsonValue& params);
    RPCResult getmemoryinfo(const JsonValue& params);
    RPCResult ping(const JsonValue& params);
    
    // Helper methods
    JsonValue transactionToJson(const TransactionInfo& tx);
    JsonValue blockToJson(const BlockInfo& block);
    JsonValue walletToJson(const WalletInfo& wallet);
    JsonValue peerToJson(const struct PeerInfo& peer);
    
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

// RPC API class (simplified interface for node_main)
class P2PNetwork;
class CPUMiner;

class RPCAPI {
private:
    Blockchain* blockchain;
    Network* network;
    P2PNetwork* p2pNetwork;
    CPUMiner* cpuMiner;
    std::unique_ptr<Wallet> wallet; // Node wallet
    uint16_t serverPort;
    std::atomic<bool> isRunning;
    std::thread serverThread;
    std::unordered_map<std::string, std::function<JsonValue(const JsonValue&)>> rpcMethods;
    
    void registerMethods();
    void serverLoop();
    void handleClient(int clientSocket);
    std::string createSuccessResponse(const JsonValue& result, const JsonValue& id);
    std::string createErrorResponse(int code, const std::string& message, const JsonValue& id);
    
    // RPC method implementations
    JsonValue getBlockchainInfo(const JsonValue& params);
    JsonValue getBestBlockHash(const JsonValue& params);
    JsonValue getBlockCount(const JsonValue& params);
    JsonValue getDifficulty(const JsonValue& params);
    JsonValue getBlock(const JsonValue& params);
    JsonValue getBlockHash(const JsonValue& params);
    JsonValue getRawTransaction(const JsonValue& params);
    JsonValue sendRawTransaction(const JsonValue& params);
    JsonValue getTransaction(const JsonValue& params);
    JsonValue listTransactions(const JsonValue& params);
    JsonValue getBalance(const JsonValue& params);
    JsonValue getPendingBalance(const JsonValue& params);
    JsonValue getNewAddress(const JsonValue& params);
    JsonValue sendToAddress(const JsonValue& params);
    JsonValue estimateFee(const JsonValue& params);
    JsonValue validateAddress(const JsonValue& params);
    JsonValue listAccounts(const JsonValue& params);
    JsonValue getMiningInfo(const JsonValue& params);
    JsonValue getNetworkHashPS(const JsonValue& params);
    JsonValue submitBlock(const JsonValue& params);
    JsonValue getBlockTemplate(const JsonValue& params);
    JsonValue createPoSBlock(const JsonValue& params);
    JsonValue submitPoSBlock(const JsonValue& params);
    
    // Helper functions
    double calculateBlockReward(uint32_t height);
    Transaction createTransactionFromJson(const JsonValue& txJson);
    JsonValue transactionToJson(const Transaction& tx, uint32_t blockHeight, const std::string& blockHash);
    JsonValue getPeerInfo(const JsonValue& params);
    JsonValue getConnectionCount(const JsonValue& params);
    JsonValue getNetworkInfo(const JsonValue& params);
    JsonValue getMempoolInfo(const JsonValue& params);
    JsonValue getRawMempool(const JsonValue& params);
    JsonValue addNode(const JsonValue& params);
    JsonValue disconnectNode(const JsonValue& params);
    JsonValue help(const JsonValue& params);
    JsonValue stopNode(const JsonValue& params);
    JsonValue getInfo(const JsonValue& params);
    
    // Address statistics methods
    JsonValue getAddressCount(const JsonValue& params);
    JsonValue getAddressStats(const JsonValue& params);
    JsonValue listAllAddresses(const JsonValue& params);
    
    // Staking and Validator methods
    JsonValue registerValidator(const JsonValue& params);
    JsonValue registerExternalValidator(const JsonValue& params);  // For third-party wallets
    
    // External wallet helper methods
    JsonValue createTransaction(const JsonValue& params);  // Create unsigned transaction template
    JsonValue getSigningMessage(const JsonValue& params);  // Get message format for signing
    JsonValue listUnspent(const JsonValue& params);  // List UTXOs for address
    JsonValue stake(const JsonValue& params);
    JsonValue unstake(const JsonValue& params);
    JsonValue addStake(const JsonValue& params);
    JsonValue getValidators(const JsonValue& params);
    JsonValue getValidatorInfo(const JsonValue& params);
    JsonValue getStakingInfo(const JsonValue& params);
    
    // Third-party wallet support methods
    JsonValue importPrivKey(const JsonValue& params);
    JsonValue importAddress(const JsonValue& params);
    JsonValue listImportedAddresses(const JsonValue& params);
    JsonValue signMessageWithAddress(const JsonValue& params);
    JsonValue verifySignedMessage(const JsonValue& params);
    
    // ============================================================================
    // NEW HIGH PRIORITY RPC METHODS
    // ============================================================================
    
    // Block methods
    JsonValue getBlockHeader(const JsonValue& params);
    JsonValue getBlockStats(const JsonValue& params);
    
    // UTXO methods
    JsonValue getTxOut(const JsonValue& params);
    JsonValue getTxOutSetInfo(const JsonValue& params);
    
    // Transaction methods
    JsonValue decodeRawTransaction(const JsonValue& params);
    JsonValue createRawTransaction(const JsonValue& params);
    JsonValue signRawTransactionWithKey(const JsonValue& params);
    
    // Address methods
    JsonValue getAddressBalance(const JsonValue& params);
    JsonValue getAddressUtxos(const JsonValue& params);
    
    // Wallet methods
    JsonValue dumpPrivKey(const JsonValue& params);
    
    // Network methods
    JsonValue getNetTotals(const JsonValue& params);
    
    // Mempool methods
    JsonValue getMempoolEntry(const JsonValue& params);
    
    // Address traceability tracking
    JsonValue traceAddress(const JsonValue& params);
    JsonValue trackStolenFunds(const JsonValue& params);
    
public:
    RPCAPI(Blockchain* blockchain, uint16_t port);
    RPCAPI(Blockchain* blockchain, Network* network, uint16_t port);
    ~RPCAPI();
    
    void setP2PNetwork(P2PNetwork* p2p) { p2pNetwork = p2p; }
    void setCPUMiner(CPUMiner* miner) { cpuMiner = miner; }
    
    bool start();
    bool start(uint16_t port);
    void stop();
    void processRequests();
    std::string processRequest(const std::string& request);
};

// REST API server is defined in RESTServer.h

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
    void broadcastPriceUpdate(const JsonValue& priceData);
    void broadcastMiningUpdate(const JsonValue& miningData);
    
private:
    void handleConnection(int clientSocket);
    void broadcastMessage(const std::string& message);
    std::string createWebSocketFrame(const std::string& data);
};