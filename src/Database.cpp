#include "../include/Database.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <leveldb/options.h>
#include <leveldb/write_batch.h>
#include <filesystem>
#include <sstream>

Database::Database(const std::string& dataDir)
    : walletDb(nullptr), dataDirectory(dataDir) {
    if (!std::filesystem::exists(dataDirectory)) {
        std::filesystem::create_directories(dataDirectory);
    }
}

Database::~Database() {
    close();
}

bool Database::initialize() {
    LOG_DATABASE(LogLevel::INFO, "Initializing database in directory: " + dataDirectory);
    
    if (!initializeWalletDB()) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to initialize wallet database");
        return false;
    }
    
    if (!initializeBlockchainDB()) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to initialize blockchain database");
        return false;
    }
    
    LOG_DATABASE(LogLevel::INFO, "Database initialization completed successfully");
    return true;
}

bool Database::initializeWalletDB() {
    std::string walletPath = dataDirectory + "/wallet.db";
    
    int result = sqlite3_open(walletPath.c_str(), &walletDb);
    if (result != SQLITE_OK) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to open wallet database: " + std::string(sqlite3_errmsg(walletDb)));
        return false;
    }
    
    // Create wallet tables
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS wallets (
            address TEXT PRIMARY KEY,
            public_key TEXT NOT NULL,
            encrypted_private_key TEXT NOT NULL,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS utxos (
            tx_hash TEXT NOT NULL,
            output_index INTEGER NOT NULL,
            address TEXT NOT NULL,
            amount INTEGER NOT NULL,
            script_pubkey TEXT NOT NULL,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (tx_hash, output_index)
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS validators (
            address TEXT PRIMARY KEY,
            stake INTEGER NOT NULL,
            is_active INTEGER NOT NULL DEFAULT 1,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP,
            updated_at INTEGER DEFAULT CURRENT_TIMESTAMP
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS proposals (
            proposal_id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            proposer TEXT NOT NULL,
            block_height INTEGER NOT NULL,
            status INTEGER DEFAULT 0,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS votes (
            proposal_id TEXT NOT NULL,
            voter TEXT NOT NULL,
            vote_type INTEGER NOT NULL,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (proposal_id, voter)
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS peers (
            ip TEXT NOT NULL,
            port INTEGER NOT NULL,
            last_seen INTEGER NOT NULL,
            PRIMARY KEY (ip, port)
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS price_data (
            asset TEXT NOT NULL,
            price REAL NOT NULL,
            timestamp INTEGER NOT NULL,
            PRIMARY KEY (asset, timestamp)
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS bridge_transfers (
            transfer_id TEXT PRIMARY KEY,
            source_chain TEXT NOT NULL,
            dest_chain TEXT NOT NULL,
            amount REAL NOT NULL,
            recipient TEXT NOT NULL,
            status INTEGER DEFAULT 0,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS pool_shares (
            pool_address TEXT NOT NULL,
            miner_address TEXT NOT NULL,
            share_value REAL NOT NULL,
            timestamp INTEGER NOT NULL,
            PRIMARY KEY (pool_address, miner_address, timestamp)
        );
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS config (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL,
            updated_at INTEGER DEFAULT CURRENT_TIMESTAMP
        );
    )");
    
    // Create indexes for better performance
    executeSQL("CREATE INDEX IF NOT EXISTS idx_utxos_address ON utxos(address);");
    executeSQL("CREATE INDEX IF NOT EXISTS idx_price_data_asset ON price_data(asset);");
    executeSQL("CREATE INDEX IF NOT EXISTS idx_pool_shares_pool ON pool_shares(pool_address);");
    
    return true;
}

bool Database::initializeBlockchainDB() {
    std::string blockchainPath = dataDirectory + "/blockchain";
    
    leveldb::Options options;
    options.create_if_missing = true;
    options.compression = leveldb::kSnappyCompression;
    options.write_buffer_size = 64 * 1024 * 1024; // 64MB
    options.max_file_size = 32 * 1024 * 1024; // 32MB
    
    leveldb::DB* db;
    leveldb::Status status = leveldb::DB::Open(options, blockchainPath, &db);
    
    if (!status.ok()) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to open blockchain database: " + status.ToString());
        return false;
    }
    
    blockchainDb.reset(db);
    return true;
}

void Database::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    int result = sqlite3_exec(walletDb, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (result != SQLITE_OK) {
        std::string error = errMsg ? std::string(errMsg) : "Unknown SQL error";
        sqlite3_free(errMsg);
        LOG_DATABASE(LogLevel::ERROR, "SQL execution failed: " + error);
        throw std::runtime_error("SQL execution failed: " + error);
    }
}

bool Database::storeBlock(const Block& block) {
    if (!blockchainDb) return false;
    
    // Serialize block data (simplified - in real implementation use proper serialization)
    std::string key = "block:" + std::to_string(block.getIndex());
    std::string value = block.getHash(); // Simplified
    
    leveldb::Status status = blockchainDb->Put(leveldb::WriteOptions(), key, value);
    if (!status.ok()) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to store block: " + status.ToString());
        return false;
    }
    
    // Update latest block info
    blockchainDb->Put(leveldb::WriteOptions(), "latest_block_index", std::to_string(block.getIndex()));
    blockchainDb->Put(leveldb::WriteOptions(), "latest_block_hash", block.getHash());
    
    return true;
}

bool Database::storeTransaction(const Transaction& tx) {
    if (!blockchainDb) return false;
    
    std::string key = "tx:" + tx.getHash();
    std::string value = tx.getHash(); // Simplified
    
    leveldb::Status status = blockchainDb->Put(leveldb::WriteOptions(), key, value);
    return status.ok();
}

double Database::getBalance(const std::string& address) const {
    if (!walletDb) return 0.0;
    
    std::string sql = "SELECT SUM(amount) FROM utxos WHERE address = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(walletDb, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return 0.0;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_STATIC);
    
    double balance = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        balance = sqlite3_column_double(stmt, 0) / Utils::SATOSHIS_PER_COIN;
    }
    
    sqlite3_finalize(stmt);
    return balance;
}

bool Database::storeWallet(const std::string& address, const std::string& publicKey, const std::string& encryptedPrivateKey) {
    if (!walletDb) return false;
    
    std::string sql = "INSERT OR REPLACE INTO wallets (address, public_key, encrypted_private_key) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(walletDb, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, publicKey.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, encryptedPrivateKey.c_str(), -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::storeValidator(const std::string& address, double stake, bool isActive) {
    if (!walletDb) return false;
    
    std::string sql = "INSERT OR REPLACE INTO validators (address, stake, is_active) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(walletDb, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, static_cast<int64_t>(stake * Utils::SATOSHIS_PER_COIN));
    sqlite3_bind_int(stmt, 3, isActive ? 1 : 0);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::storePriceData(const std::string& asset, double price, uint32_t timestamp) {
    if (!walletDb) return false;
    
    std::string sql = "INSERT OR REPLACE INTO price_data (asset, price, timestamp) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(walletDb, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, asset.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, price);
    sqlite3_bind_int(stmt, 3, timestamp);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

uint32_t Database::getLatestBlockIndex() const {
    if (!blockchainDb) return 0;
    
    std::string value;
    leveldb::Status status = blockchainDb->Get(leveldb::ReadOptions(), "latest_block_index", &value);
    
    if (status.ok()) {
        return static_cast<uint32_t>(std::stoul(value));
    }
    
    return 0;
}

std::string Database::getLatestBlockHash() const {
    if (!blockchainDb) return "";
    
    std::string value;
    leveldb::Status status = blockchainDb->Get(leveldb::ReadOptions(), "latest_block_hash", &value);
    
    if (status.ok()) {
        return value;
    }
    
    return "";
}

bool Database::setConfigValue(const std::string& key, const std::string& value) {
    if (!walletDb) return false;
    
    std::string sql = "INSERT OR REPLACE INTO config (key, value) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(walletDb, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::getConfigValue(const std::string& key, std::string& value) const {
    if (!walletDb) return false;
    
    std::string sql = "SELECT value FROM config WHERE key = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(walletDb, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::vacuum() {
    if (!walletDb) return false;
    
    try {
        executeSQL("VACUUM;");
        LOG_DATABASE(LogLevel::INFO, "Database vacuum completed successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Database vacuum failed: " + std::string(e.what()));
        return false;
    }
}

uint64_t Database::getDatabaseSize() const {
    try {
        uint64_t size = 0;
        
        // SQLite database size
        if (std::filesystem::exists(dataDirectory + "/wallet.db")) {
            size += std::filesystem::file_size(dataDirectory + "/wallet.db");
        }
        
        // LevelDB size
        if (std::filesystem::exists(dataDirectory + "/blockchain")) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dataDirectory + "/blockchain")) {
                if (entry.is_regular_file()) {
                    size += entry.file_size();
                }
            }
        }
        
        return size;
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to calculate database size: " + std::string(e.what()));
        return 0;
    }
}

void Database::close() {
    if (walletDb) {
        sqlite3_close(walletDb);
        walletDb = nullptr;
    }
    
    if (blockchainDb) {
        blockchainDb.reset();
    }
    
    LOG_DATABASE(LogLevel::INFO, "Database closed successfully");
}

bool Database::isInitialized() const {
    return walletDb != nullptr && blockchainDb != nullptr;
}

std::string Database::getLastError() const {
    if (walletDb) {
        return std::string(sqlite3_errmsg(walletDb));
    }
    return "Database not initialized";
}

Database::DatabaseError Database::getLastErrorCode() const {
    if (!walletDb) {
        return DB_ERROR_INIT;
    }
    
    int code = sqlite3_errcode(walletDb);
    switch (code) {
        case SQLITE_OK: return DB_SUCCESS;
        case SQLITE_NOTFOUND: return DB_ERROR_NOT_FOUND;
        case SQLITE_CORRUPT: return DB_ERROR_CORRUPT;
        case SQLITE_FULL: return DB_ERROR_DISK_FULL;
        case SQLITE_PERM: return DB_ERROR_PERMISSION;
        default: return DB_ERROR_SQL;
    }
}

bool Database::isHealthy() const {
    if (!isInitialized()) {
        return false;
    }
    
    // Quick health check
    try {
        std::string testKey = "health_check";
        std::string testValue = "ok";
        
        if (setConfigValue(testKey, testValue)) {
            std::string retrievedValue;
            if (getConfigValue(testKey, retrievedValue)) {
                return retrievedValue == testValue;
            }
        }
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Database health check failed: " + std::string(e.what()));
    }
    
    return false;
}

void Database::repairDatabase() {
    LOG_DATABASE(LogLevel::INFO, "Starting database repair");
    
    try {
        // SQLite integrity check and repair
        if (walletDb) {
            executeSQL("PRAGMA integrity_check;");
            executeSQL("REINDEX;");
            vacuum();
        }
        
        LOG_DATABASE(LogLevel::INFO, "Database repair completed");
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Database repair failed: " + std::string(e.what()));
    }
}

// Placeholder implementations for interface completeness
bool Database::getBlock(uint32_t index, Block& block) const { return false; }
bool Database::getBlock(const std::string& hash, Block& block) const { return false; }
bool Database::deleteBlock(uint32_t index) { return false; }
bool Database::getTransaction(const std::string& hash, Transaction& tx) const { return false; }
bool Database::deleteTransaction(const std::string& hash) { return false; }
std::vector<Transaction> Database::getTransactionsByAddress(const std::string& address) const { return {}; }
std::vector<Transaction> Database::getPendingTransactions() const { return {}; }
bool Database::storeUTXO(const std::string& txHash, uint32_t outputIndex, const TransactionOutput& output) { return false; }
bool Database::getUTXO(const std::string& txHash, uint32_t outputIndex, TransactionOutput& output) const { return false; }
bool Database::deleteUTXO(const std::string& txHash, uint32_t outputIndex) { return false; }
std::vector<TransactionOutput> Database::getUTXOsByAddress(const std::string& address) const { return {}; }
bool Database::getWallet(const std::string& address, std::string& publicKey, std::string& encryptedPrivateKey) const { return false; }
bool Database::deleteWallet(const std::string& address) { return false; }
std::vector<std::string> Database::getAllWalletAddresses() const { return {}; }
bool Database::getValidator(const std::string& address, double& stake, bool& isActive) const { return false; }
bool Database::updateValidatorStake(const std::string& address, double newStake) { return false; }
bool Database::setValidatorActive(const std::string& address, bool isActive) { return false; }
std::vector<std::string> Database::getActiveValidators() const { return {}; }
bool Database::storeProposal(const std::string& proposalId, const std::string& title, const std::string& description, const std::string& proposer, uint32_t blockHeight) { return false; }
bool Database::storeVote(const std::string& proposalId, const std::string& voter, int voteType) { return false; }
bool Database::getProposal(const std::string& proposalId, std::string& title, std::string& description, std::string& proposer, uint32_t& blockHeight) const { return false; }
std::vector<std::string> Database::getActiveProposals() const { return {}; }
bool Database::storePeer(const std::string& ip, uint16_t port, uint32_t lastSeen) { return false; }
bool Database::deletePeer(const std::string& ip, uint16_t port) { return false; }
std::vector<std::pair<std::string, uint16_t>> Database::getActivePeers() const { return {}; }
uint64_t Database::getTotalTransactions() const { return 0; }
uint64_t Database::getTotalBlocks() const { return 0; }
double Database::getTotalSupply() const { return 0.0; }
double Database::getTotalBurned() const { return 0.0; }
bool Database::backup(const std::string& backupPath) { return false; }
bool Database::restore(const std::string& backupPath) { return false; }
bool Database::getLatestPrice(const std::string& asset, double& price, uint32_t& timestamp) const { return false; }
std::vector<std::pair<double, uint32_t>> Database::getPriceHistory(const std::string& asset, uint32_t count) const { return {}; }
bool Database::storeBridgeTransfer(const std::string& transferId, const std::string& sourceChain, const std::string& destChain, double amount, const std::string& recipient) { return false; }
bool Database::updateBridgeTransferStatus(const std::string& transferId, int status) { return false; }
bool Database::getBridgeTransfer(const std::string& transferId, std::string& sourceChain, std::string& destChain, double& amount, std::string& recipient, int& status) const { return false; }
bool Database::storePoolShare(const std::string& poolAddress, const std::string& minerAddress, double shareValue, uint32_t timestamp) { return false; }
double Database::getPoolShares(const std::string& poolAddress, const std::string& minerAddress) const { return 0.0; }
std::vector<std::pair<std::string, double>> Database::getPoolContributors(const std::string& poolAddress) const { return {}; }
bool Database::deleteConfigValue(const std::string& key) { return false; }