#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <sqlite3.h>
#include "transaction.h"
#include "Block.h"

class Database {
private:
    sqlite3* db;
    std::string dataDirectory;
    
    // Static members for singleton pattern
    static std::unique_ptr<Database> instance;
    static std::mutex instanceMutex;
    
    // Database initialization
    bool open(const std::string& dbPath);
    bool createTables();
    bool executeSQL(const std::string& sql);
    std::string getLastError() const;

public:
    // Static methods for singleton-like access
    static bool initialize(const std::string& dataDir);
    static void shutdown();
    static Database& getInstance();
    
    // Constructor and destructor
    Database();
    ~Database();
    
    // Database operations
    bool initialize();
    bool isInitialized() const;
    void close();
    
    // Block storage
    bool storeBlock(const Block& block);
    bool getBlock(uint32_t index, Block& block) const;
    bool getBlock(const std::string& hash, Block& block) const;
    bool deleteBlock(uint32_t index);
    uint32_t getLatestBlockIndex() const;
    std::string getLatestBlockHash() const;
    
    // Transaction storage
    bool storeTransaction(const Transaction& tx);
    bool getTransaction(const std::string& hash, Transaction& tx) const;
    bool deleteTransaction(const std::string& hash);
    std::vector<Transaction> getTransactionsByAddress(const std::string& address) const;
    std::vector<Transaction> getPendingTransactions() const;
    
    // UTXO management
    bool storeUTXO(const std::string& txHash, uint32_t outputIndex, const TransactionOutput& output);
    bool getUTXO(const std::string& txHash, uint32_t outputIndex, TransactionOutput& output) const;
    bool deleteUTXO(const std::string& txHash, uint32_t outputIndex);
    std::vector<TransactionOutput> getUTXOsByAddress(const std::string& address) const;
    double getBalance(const std::string& address) const;
    
    // Wallet operations
    bool storeWallet(const std::string& address, const std::string& publicKey, const std::string& encryptedPrivateKey);
    bool getWallet(const std::string& address, std::string& publicKey, std::string& encryptedPrivateKey) const;
    bool deleteWallet(const std::string& address);
    std::vector<std::string> getAllWalletAddresses() const;
    
    // Validator storage
    bool storeValidator(const std::string& address, double stake, bool isActive);
    bool getValidator(const std::string& address, double& stake, bool& isActive) const;
    bool updateValidatorStake(const std::string& address, double newStake);
    bool setValidatorActive(const std::string& address, bool isActive);
    std::vector<std::string> getActiveValidators() const;
    
    // Governance storage
    bool storeProposal(const std::string& proposalId, const std::string& title, const std::string& description,
                      const std::string& proposer, uint32_t blockHeight);
    bool storeVote(const std::string& proposalId, const std::string& voter, int voteType);
    bool getProposal(const std::string& proposalId, std::string& title, std::string& description,
                    std::string& proposer, uint32_t& blockHeight) const;
    std::vector<std::string> getActiveProposals() const;
    
    // Network peer storage
    bool storePeer(const std::string& ip, uint16_t port, uint32_t lastSeen);
    bool deletePeer(const std::string& ip, uint16_t port);
    std::vector<std::pair<std::string, uint16_t>> getActivePeers() const;
    
    // Statistics and metrics
    uint64_t getTotalTransactions() const;
    uint64_t getTotalBlocks() const;
    double getTotalSupply() const;
    double getTotalBurned() const;
    
    // Database maintenance
    bool vacuum();
    bool backup(const std::string& backupPath);
    bool restore(const std::string& backupPath);
    uint64_t getDatabaseSize() const;
    bool createIndexes();
    
    // Additional block operations
    bool saveBlock(const Block& block);
    Block getBlock(const std::string& hash) const;
    
    // Additional transaction operations
    bool saveTransaction(const Transaction& tx, const std::string& blockHash, size_t blockIndex);
    bool saveTransactionInputs(const Transaction& tx);
    bool saveTransactionOutputs(const Transaction& tx);
    bool updateUtxoSet(const Transaction& tx);
    bool saveTraceabilityRecord(const Transaction& tx, size_t blockIndex);
    std::vector<Transaction> getTransactionsByBlockHash(const std::string& blockHash) const;
    std::vector<TransactionInput> getTransactionInputs(const std::string& txHash) const;
    std::vector<TransactionOutput> getTransactionOutputs(const std::string& txHash) const;
    
    // Additional query operations
    double getAddressBalance(const std::string& address) const;
    size_t getBlockCount() const;
    bool isConnected() const;
    
    // Price data storage (for PoP oracle)
    bool storePriceData(const std::string& asset, double price, uint32_t timestamp);
    bool getLatestPrice(const std::string& asset, double& price, uint32_t& timestamp) const;
    std::vector<std::pair<double, uint32_t>> getPriceHistory(const std::string& asset, uint32_t count) const;
    
    // Cross-chain bridge storage
    bool storeBridgeTransfer(const std::string& transferId, const std::string& sourceChain,
                           const std::string& destChain, double amount, const std::string& recipient);
    bool updateBridgeTransferStatus(const std::string& transferId, int status);
    bool getBridgeTransfer(const std::string& transferId, std::string& sourceChain, std::string& destChain,
                          double& amount, std::string& recipient, int& status) const;
    
    // Mining pool storage
    bool storePoolShare(const std::string& poolAddress, const std::string& minerAddress,
                       double shareValue, uint32_t timestamp);
    double getPoolShares(const std::string& poolAddress, const std::string& minerAddress) const;
    std::vector<std::pair<std::string, double>> getPoolContributors(const std::string& poolAddress) const;
    
    // Configuration storage
    bool setConfigValue(const std::string& key, const std::string& value);
    bool getConfigValue(const std::string& key, std::string& value) const;
    bool deleteConfigValue(const std::string& key);
    
    // Error handling
    enum DatabaseError {
        DB_SUCCESS = 0,
        DB_ERROR_INIT = 1,
        DB_ERROR_NOT_FOUND = 2,
        DB_ERROR_CORRUPT = 3,
        DB_ERROR_DISK_FULL = 4,
        DB_ERROR_PERMISSION = 5,
        DB_ERROR_SQL = 6
    };
    
    DatabaseError getLastErrorCode() const;
    bool isHealthy() const;
    void repairDatabase();
};