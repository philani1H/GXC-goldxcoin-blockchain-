#include "Database.h"
#include "Logger.h"
#include <sqlite3.h>
#include <sstream>
#include <stdexcept>
#include <memory>

Database::Database(const std::string& dbPath) : dbPath_(dbPath), db_(nullptr) {
    open();
    createTables();
}

Database::~Database() {
    close();
}

void Database::open() {
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::string error = "Cannot open database: " + std::string(sqlite3_errmsg(db_));
        Logger::error(error);
        throw std::runtime_error(error);
    }
    
    // Enable foreign keys and WAL mode for better performance
    execute("PRAGMA foreign_keys = ON");
    execute("PRAGMA journal_mode = WAL");
    execute("PRAGMA synchronous = NORMAL");
    execute("PRAGMA cache_size = 10000");
    execute("PRAGMA temp_store = MEMORY");
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void Database::createTables() {
    try {
        // Blocks table with advanced tracking
        execute(R"(
            CREATE TABLE IF NOT EXISTS blocks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                hash TEXT UNIQUE NOT NULL,
                prev_hash TEXT NOT NULL,
                merkle_root TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                nonce INTEGER NOT NULL,
                difficulty REAL NOT NULL,
                height INTEGER UNIQUE NOT NULL,
                validator TEXT NOT NULL,
                signature TEXT NOT NULL,
                size INTEGER NOT NULL,
                tx_count INTEGER NOT NULL,
                total_fees REAL NOT NULL,
                gas_used INTEGER DEFAULT 0,
                gas_limit INTEGER DEFAULT 8000000,
                state_root TEXT,
                extra_data TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(hash),
                INDEX(height),
                INDEX(timestamp),
                INDEX(validator)
            )
        )");

        // Enhanced transactions table with smart contract support
        execute(R"(
            CREATE TABLE IF NOT EXISTS transactions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                hash TEXT UNIQUE NOT NULL,
                block_hash TEXT NOT NULL,
                block_height INTEGER NOT NULL,
                tx_index INTEGER NOT NULL,
                from_address TEXT NOT NULL,
                to_address TEXT,
                amount REAL NOT NULL,
                fee REAL NOT NULL,
                gas_price REAL DEFAULT 0,
                gas_limit INTEGER DEFAULT 21000,
                gas_used INTEGER DEFAULT 0,
                nonce INTEGER NOT NULL,
                data TEXT,
                status INTEGER DEFAULT 1,
                timestamp INTEGER NOT NULL,
                signature TEXT NOT NULL,
                tx_type INTEGER DEFAULT 0,
                contract_address TEXT,
                logs TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(block_hash) REFERENCES blocks(hash),
                INDEX(hash),
                INDEX(block_hash),
                INDEX(from_address),
                INDEX(to_address),
                INDEX(timestamp),
                INDEX(status)
            )
        )");

        // Enhanced wallets table with staking info
        execute(R"(
            CREATE TABLE IF NOT EXISTS wallets (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                address TEXT UNIQUE NOT NULL,
                public_key TEXT NOT NULL,
                balance REAL NOT NULL DEFAULT 0,
                staked_balance REAL NOT NULL DEFAULT 0,
                delegated_balance REAL NOT NULL DEFAULT 0,
                nonce INTEGER NOT NULL DEFAULT 0,
                last_activity INTEGER,
                reputation_score REAL DEFAULT 100.0,
                staking_rewards REAL DEFAULT 0,
                total_fees_paid REAL DEFAULT 0,
                transaction_count INTEGER DEFAULT 0,
                contract_code TEXT,
                storage_root TEXT,
                is_validator BOOLEAN DEFAULT FALSE,
                validator_stake REAL DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(address),
                INDEX(balance),
                INDEX(is_validator)
            )
        )");

        // Advanced mining rewards table (increased from 12 to 100 coins for early miners)
        execute(R"(
            CREATE TABLE IF NOT EXISTS mining_rewards (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                block_hash TEXT NOT NULL,
                miner_address TEXT NOT NULL,
                reward_amount REAL NOT NULL,
                bonus_amount REAL DEFAULT 0,
                early_miner_bonus REAL DEFAULT 0,
                algorithm TEXT NOT NULL,
                difficulty REAL NOT NULL,
                shares_submitted INTEGER DEFAULT 1,
                pool_address TEXT,
                timestamp INTEGER NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(block_hash) REFERENCES blocks(hash),
                INDEX(miner_address),
                INDEX(algorithm),
                INDEX(timestamp)
            )
        )");

        // Advanced validator tracking
        execute(R"(
            CREATE TABLE IF NOT EXISTS validators (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                address TEXT UNIQUE NOT NULL,
                public_key TEXT NOT NULL,
                stake_amount REAL NOT NULL,
                commission_rate REAL DEFAULT 0.05,
                total_delegated REAL DEFAULT 0,
                blocks_validated INTEGER DEFAULT 0,
                blocks_missed INTEGER DEFAULT 0,
                slashing_events INTEGER DEFAULT 0,
                reputation_score REAL DEFAULT 100.0,
                is_active BOOLEAN DEFAULT TRUE,
                last_activity INTEGER,
                rewards_earned REAL DEFAULT 0,
                penalties_incurred REAL DEFAULT 0,
                uptime_percentage REAL DEFAULT 100.0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(address),
                INDEX(stake_amount),
                INDEX(is_active),
                INDEX(reputation_score)
            )
        )");

        // Smart contracts table
        execute(R"(
            CREATE TABLE IF NOT EXISTS contracts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                address TEXT UNIQUE NOT NULL,
                creator_address TEXT NOT NULL,
                bytecode TEXT NOT NULL,
                abi TEXT,
                source_code TEXT,
                compiler_version TEXT,
                creation_tx_hash TEXT,
                creation_block_height INTEGER,
                gas_used INTEGER,
                contract_type TEXT DEFAULT 'general',
                is_verified BOOLEAN DEFAULT FALSE,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(address),
                INDEX(creator_address),
                INDEX(contract_type)
            )
        )");

        // Advanced network peers tracking
        execute(R"(
            CREATE TABLE IF NOT EXISTS peers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                node_id TEXT UNIQUE NOT NULL,
                ip_address TEXT NOT NULL,
                port INTEGER NOT NULL,
                protocol_version TEXT NOT NULL,
                user_agent TEXT,
                last_seen INTEGER NOT NULL,
                connection_count INTEGER DEFAULT 0,
                bytes_sent INTEGER DEFAULT 0,
                bytes_received INTEGER DEFAULT 0,
                reputation_score REAL DEFAULT 50.0,
                is_trusted BOOLEAN DEFAULT FALSE,
                is_banned BOOLEAN DEFAULT FALSE,
                ban_reason TEXT,
                ban_expires INTEGER,
                latitude REAL,
                longitude REAL,
                country TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(node_id),
                INDEX(ip_address),
                INDEX(last_seen),
                INDEX(reputation_score)
            )
        )");

        // Governance proposals
        execute(R"(
            CREATE TABLE IF NOT EXISTS proposals (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                proposal_id TEXT UNIQUE NOT NULL,
                title TEXT NOT NULL,
                description TEXT NOT NULL,
                proposer_address TEXT NOT NULL,
                proposal_type TEXT NOT NULL,
                voting_start INTEGER NOT NULL,
                voting_end INTEGER NOT NULL,
                quorum_threshold REAL NOT NULL,
                approval_threshold REAL NOT NULL,
                status TEXT DEFAULT 'active',
                yes_votes REAL DEFAULT 0,
                no_votes REAL DEFAULT 0,
                abstain_votes REAL DEFAULT 0,
                total_voting_power REAL DEFAULT 0,
                execution_payload TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(proposal_id),
                INDEX(status),
                INDEX(proposer_address)
            )
        )");

        // Token balances for multi-token support
        execute(R"(
            CREATE TABLE IF NOT EXISTS token_balances (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                wallet_address TEXT NOT NULL,
                token_address TEXT NOT NULL,
                balance REAL NOT NULL DEFAULT 0,
                last_updated INTEGER NOT NULL,
                UNIQUE(wallet_address, token_address),
                INDEX(wallet_address),
                INDEX(token_address)
            )
        )");

        // Performance metrics for advanced tracking
        execute(R"(
            CREATE TABLE IF NOT EXISTS performance_metrics (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                metric_type TEXT NOT NULL,
                metric_name TEXT NOT NULL,
                value REAL NOT NULL,
                timestamp INTEGER NOT NULL,
                block_height INTEGER,
                additional_data TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                INDEX(metric_type),
                INDEX(timestamp),
                INDEX(block_height)
            )
        )");

        Logger::info("Database tables created successfully");
    } catch (const std::exception& e) {
        Logger::error("Failed to create database tables: " + std::string(e.what()));
        throw;
    }
}

void Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = "SQL error: " + std::string(errMsg);
        sqlite3_free(errMsg);
        Logger::error(error);
        throw std::runtime_error(error);
    }
}

std::vector<std::map<std::string, std::string>> Database::query(const std::string& sql) {
    sqlite3_stmt* stmt;
    std::vector<std::map<std::string, std::string>> results;
    
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return results;
    }
    
    int columnCount = sqlite3_column_count(stmt);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::map<std::string, std::string> row;
        
        for (int i = 0; i < columnCount; i++) {
            const char* columnName = sqlite3_column_name(stmt, i);
            const char* columnValue = (const char*)sqlite3_column_text(stmt, i);
            
            row[columnName] = columnValue ? columnValue : "";
        }
        
        results.push_back(row);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

bool Database::insertBlock(const Block& block) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO blocks (hash, prev_hash, merkle_root, timestamp, nonce, "
            << "difficulty, height, validator, signature, size, tx_count, total_fees) VALUES ("
            << "'" << block.hash << "', "
            << "'" << block.previousHash << "', "
            << "'" << block.merkleRoot << "', "
            << block.timestamp << ", "
            << block.nonce << ", "
            << block.difficulty << ", "
            << block.height << ", "
            << "'" << block.validator << "', "
            << "'" << block.signature << "', "
            << block.size << ", "
            << block.transactions.size() << ", "
            << calculateTotalFees(block) << ")";
        
        execute(sql.str());
        
        // Insert transactions
        for (size_t i = 0; i < block.transactions.size(); i++) {
            insertTransaction(block.transactions[i], block.hash, block.height, i);
        }
        
        // Update performance metrics
        insertPerformanceMetric("block", "creation_time", 
                               std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::system_clock::now().time_since_epoch()).count(),
                               block.height);
        
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to insert block: " + std::string(e.what()));
        return false;
    }
}

bool Database::insertTransaction(const Transaction& tx, const std::string& blockHash, 
                                int blockHeight, int txIndex) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO transactions (hash, block_hash, block_height, tx_index, "
            << "from_address, to_address, amount, fee, nonce, timestamp, signature) VALUES ("
            << "'" << tx.hash << "', "
            << "'" << blockHash << "', "
            << blockHeight << ", "
            << txIndex << ", "
            << "'" << tx.fromAddress << "', "
            << "'" << tx.toAddress << "', "
            << tx.amount << ", "
            << tx.fee << ", "
            << tx.nonce << ", "
            << tx.timestamp << ", "
            << "'" << tx.signature << "')";
        
        execute(sql.str());
        
        // Update wallet balances
        updateWalletBalance(tx.fromAddress, -tx.amount - tx.fee);
        updateWalletBalance(tx.toAddress, tx.amount);
        
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to insert transaction: " + std::string(e.what()));
        return false;
    }
}

bool Database::updateWalletBalance(const std::string& address, double amount) {
    try {
        // First check if wallet exists
        std::string checkSql = "SELECT balance FROM wallets WHERE address = '" + address + "'";
        auto results = query(checkSql);
        
        if (results.empty()) {
            // Create new wallet
            std::stringstream sql;
            sql << "INSERT INTO wallets (address, public_key, balance, last_activity) VALUES ("
                << "'" << address << "', '', " << amount << ", " 
                << std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch()).count() << ")";
            execute(sql.str());
        } else {
            // Update existing wallet
            double currentBalance = std::stod(results[0]["balance"]);
            double newBalance = currentBalance + amount;
            
            std::stringstream sql;
            sql << "UPDATE wallets SET balance = " << newBalance 
                << ", last_activity = " 
                << std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch()).count()
                << ", transaction_count = transaction_count + 1"
                << " WHERE address = '" << address << "'";
            execute(sql.str());
        }
        
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update wallet balance: " + std::string(e.what()));
        return false;
    }
}

std::vector<Block> Database::getBlocks(int limit, int offset) {
    std::vector<Block> blocks;
    
    try {
        std::stringstream sql;
        sql << "SELECT * FROM blocks ORDER BY height DESC LIMIT " << limit << " OFFSET " << offset;
        
        auto results = query(sql.str());
        
        for (const auto& row : results) {
            Block block;
            block.hash = row.at("hash");
            block.previousHash = row.at("prev_hash");
            block.merkleRoot = row.at("merkle_root");
            block.timestamp = std::stoll(row.at("timestamp"));
            block.nonce = std::stoll(row.at("nonce"));
            block.difficulty = std::stod(row.at("difficulty"));
            block.height = std::stoi(row.at("height"));
            block.validator = row.at("validator");
            block.signature = row.at("signature");
            
            // Load transactions for this block
            block.transactions = getTransactionsByBlockHash(block.hash);
            
            blocks.push_back(block);
        }
    } catch (const std::exception& e) {
        Logger::error("Failed to get blocks: " + std::string(e.what()));
    }
    
    return blocks;
}

std::vector<Transaction> Database::getTransactionsByBlockHash(const std::string& blockHash) {
    std::vector<Transaction> transactions;
    
    try {
        std::string sql = "SELECT * FROM transactions WHERE block_hash = '" + blockHash + "' ORDER BY tx_index";
        auto results = query(sql);
        
        for (const auto& row : results) {
            Transaction tx;
            tx.hash = row.at("hash");
            tx.fromAddress = row.at("from_address");
            tx.toAddress = row.at("to_address");
            tx.amount = std::stod(row.at("amount"));
            tx.fee = std::stod(row.at("fee"));
            tx.nonce = std::stoll(row.at("nonce"));
            tx.timestamp = std::stoll(row.at("timestamp"));
            tx.signature = row.at("signature");
            
            transactions.push_back(tx);
        }
    } catch (const std::exception& e) {
        Logger::error("Failed to get transactions: " + std::string(e.what()));
    }
    
    return transactions;
}

double Database::getWalletBalance(const std::string& address) {
    try {
        std::string sql = "SELECT balance FROM wallets WHERE address = '" + address + "'";
        auto results = query(sql);
        
        if (!results.empty()) {
            return std::stod(results[0]["balance"]);
        }
    } catch (const std::exception& e) {
        Logger::error("Failed to get wallet balance: " + std::string(e.what()));
    }
    
    return 0.0;
}

bool Database::insertMiningReward(const std::string& blockHash, const std::string& minerAddress,
                                 double rewardAmount, const std::string& algorithm, double difficulty) {
    try {
        // Enhanced reward for early miners (100 coins instead of 12)
        double earlyMinerBonus = 0.0;
        int blockHeight = getBlockHeight(blockHash);
        
        if (blockHeight < 10000) {  // First 10,000 blocks get early miner bonus
            earlyMinerBonus = 100.0;  // Increased from 12 to 100
        } else if (blockHeight < 50000) {  // Next 40,000 blocks get reduced bonus
            earlyMinerBonus = 50.0;
        } else if (blockHeight < 100000) {  // Next 50,000 blocks get smaller bonus
            earlyMinerBonus = 25.0;
        }
        
        double totalReward = rewardAmount + earlyMinerBonus;
        
        std::stringstream sql;
        sql << "INSERT INTO mining_rewards (block_hash, miner_address, reward_amount, "
            << "early_miner_bonus, algorithm, difficulty, timestamp) VALUES ("
            << "'" << blockHash << "', "
            << "'" << minerAddress << "', "
            << totalReward << ", "
            << earlyMinerBonus << ", "
            << "'" << algorithm << "', "
            << difficulty << ", "
            << std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count() << ")";
        
        execute(sql.str());
        
        // Update miner wallet with reward
        updateWalletBalance(minerAddress, totalReward);
        
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to insert mining reward: " + std::string(e.what()));
        return false;
    }
}

bool Database::insertPerformanceMetric(const std::string& metricType, const std::string& metricName,
                                      double value, int blockHeight) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO performance_metrics (metric_type, metric_name, value, timestamp, block_height) VALUES ("
            << "'" << metricType << "', "
            << "'" << metricName << "', "
            << value << ", "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count() << ", "
            << blockHeight << ")";
        
        execute(sql.str());
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to insert performance metric: " + std::string(e.what()));
        return false;
    }
}

double Database::calculateTotalFees(const Block& block) {
    double totalFees = 0.0;
    for (const auto& tx : block.transactions) {
        totalFees += tx.fee;
    }
    return totalFees;
}

int Database::getBlockHeight(const std::string& blockHash) {
    try {
        std::string sql = "SELECT height FROM blocks WHERE hash = '" + blockHash + "'";
        auto results = query(sql);
        
        if (!results.empty()) {
            return std::stoi(results[0]["height"]);
        }
    } catch (const std::exception& e) {
        Logger::error("Failed to get block height: " + std::string(e.what()));
    }
    
    return 0;
}

void Database::beginTransaction() {
    execute("BEGIN TRANSACTION");
}

void Database::commitTransaction() {
    execute("COMMIT");
}

void Database::rollbackTransaction() {
    execute("ROLLBACK");
}