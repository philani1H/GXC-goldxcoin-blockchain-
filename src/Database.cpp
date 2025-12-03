
#include "../include/Database.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/Config.h"
#include <sqlite3.h>
#include <sstream>
#include <fstream>

// Static instance
std::unique_ptr<Database> Database::instance = nullptr;
std::mutex Database::instanceMutex;

Database::Database() : db(nullptr) {
    LOG_DATABASE(LogLevel::INFO, "Database instance created");
}

Database::~Database() {
    close();
}

bool Database::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(instanceMutex);
    
    if (instance) {
        LOG_DATABASE(LogLevel::WARNING, "Database already initialized");
        return true;
    }
    
    instance = std::make_unique<Database>();
    return instance->open(dbPath);
}

void Database::shutdown() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    
    if (instance) {
        instance->close();
        instance.reset();
        LOG_DATABASE(LogLevel::INFO, "Database shutdown complete");
    }
}

Database& Database::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    
    if (!instance) {
        throw std::runtime_error("Database not initialized");
    }
    
    return *instance;
}

bool Database::open(const std::string& dbPath) {
    try {
        // Validate database path based on network mode
        bool isTestnet = Config::isTestnet();
        
        // CRITICAL: Ensure testnet and mainnet databases are completely separate
        if (isTestnet) {
            // Testnet mode - database MUST contain "testnet" in path
            if (dbPath.find("testnet") == std::string::npos) {
                LOG_DATABASE(LogLevel::ERROR, "TESTNET mode but database path doesn't contain 'testnet': " + dbPath);
                LOG_DATABASE(LogLevel::ERROR, "REFUSING to open database - risk of mainnet data corruption!");
                LOG_DATABASE(LogLevel::ERROR, "Database path must contain 'testnet' for testnet mode");
                return false;
            }
            LOG_DATABASE(LogLevel::INFO, "✓ Testnet database path validated");
        } else {
            // Mainnet mode - database MUST NOT contain "testnet"
            if (dbPath.find("testnet") != std::string::npos) {
                LOG_DATABASE(LogLevel::ERROR, "MAINNET mode but database path contains 'testnet': " + dbPath);
                LOG_DATABASE(LogLevel::ERROR, "REFUSING to open testnet database in mainnet mode!");
                LOG_DATABASE(LogLevel::ERROR, "This would corrupt testnet data!");
                return false;
            }
            
            // Additional safety: Check if mainnet database already exists
            // If it exists and we're still testing, warn loudly
            std::ifstream dbFile(dbPath);
            if (dbFile.good()) {
                LOG_DATABASE(LogLevel::WARNING, "========================================");
                LOG_DATABASE(LogLevel::WARNING, "WARNING: MAINNET DATABASE ALREADY EXISTS!");
                LOG_DATABASE(LogLevel::WARNING, "Path: " + dbPath);
                LOG_DATABASE(LogLevel::WARNING, "If you're still testing, use --testnet flag!");
                LOG_DATABASE(LogLevel::WARNING, "========================================");
            }
            LOG_DATABASE(LogLevel::INFO, "✓ Mainnet database path validated");
        }
        
        LOG_DATABASE(LogLevel::INFO, "Opening database: " + dbPath);
        LOG_DATABASE(LogLevel::INFO, "Network mode: " + std::string(isTestnet ? "TESTNET" : "MAINNET"));
        
        int rc = sqlite3_open(dbPath.c_str(), &db);
        
        if (rc != SQLITE_OK) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to open database: " + std::string(sqlite3_errmsg(db)));
            sqlite3_close(db);
            db = nullptr;
            return false;
        }
        
        // Enable WAL mode for better concurrency
        executeSQL("PRAGMA journal_mode=WAL;");
        
        // Enable foreign keys
        executeSQL("PRAGMA foreign_keys=ON;");
        
        // Create tables
        if (!createTables()) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to create database tables");
            close();
            return false;
        }
        
        LOG_DATABASE(LogLevel::INFO, "Database opened successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Exception opening database: " + std::string(e.what()));
        close();
        return false;
    }
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
        LOG_DATABASE(LogLevel::INFO, "Database closed");
    }
}

bool Database::createTables() {
    try {
        // Blocks table
        std::string createBlocks = R"(
            CREATE TABLE IF NOT EXISTS blocks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                height INTEGER UNIQUE NOT NULL,
                hash TEXT UNIQUE NOT NULL,
                previous_hash TEXT NOT NULL,
                merkle_root TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                difficulty REAL NOT NULL,
                nonce INTEGER NOT NULL,
                miner_address TEXT,
                block_type INTEGER NOT NULL,
                transaction_count INTEGER NOT NULL,
                size_bytes INTEGER NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (!executeSQL(createBlocks)) {
            return false;
        }
        
        // Transactions table
        std::string createTransactions = R"(
            CREATE TABLE IF NOT EXISTS transactions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                hash TEXT UNIQUE NOT NULL,
                block_hash TEXT,
                block_height INTEGER,
                sender_address TEXT,
                recipient_address TEXT,
                amount REAL NOT NULL,
                fee REAL NOT NULL,
                timestamp INTEGER NOT NULL,
                nonce INTEGER,
                signature TEXT,
                is_coinbase BOOLEAN NOT NULL DEFAULT 0,
                prev_tx_hash TEXT,
                referenced_amount REAL,
                traceability_valid BOOLEAN NOT NULL DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (block_hash) REFERENCES blocks(hash)
            )
        )";
        
        if (!executeSQL(createTransactions)) {
            return false;
        }
        
        // Transaction inputs table
        std::string createInputs = R"(
            CREATE TABLE IF NOT EXISTS transaction_inputs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                transaction_hash TEXT NOT NULL,
                input_index INTEGER NOT NULL,
                prev_tx_hash TEXT NOT NULL,
                prev_output_index INTEGER NOT NULL,
                amount REAL NOT NULL,
                signature TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (transaction_hash) REFERENCES transactions(hash),
                UNIQUE(transaction_hash, input_index)
            )
        )";
        
        if (!executeSQL(createInputs)) {
            return false;
        }
        
        // Transaction outputs table
        std::string createOutputs = R"(
            CREATE TABLE IF NOT EXISTS transaction_outputs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                transaction_hash TEXT NOT NULL,
                output_index INTEGER NOT NULL,
                address TEXT NOT NULL,
                amount REAL NOT NULL,
                spent BOOLEAN NOT NULL DEFAULT 0,
                spent_by_tx TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (transaction_hash) REFERENCES transactions(hash),
                UNIQUE(transaction_hash, output_index)
            )
        )";
        
        if (!executeSQL(createOutputs)) {
            return false;
        }
        
        // UTXO table for faster lookups
        std::string createUtxo = R"(
            CREATE TABLE IF NOT EXISTS utxo (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                tx_hash TEXT NOT NULL,
                output_index INTEGER NOT NULL,
                address TEXT NOT NULL,
                amount REAL NOT NULL,
                script TEXT,
                block_height INTEGER NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                UNIQUE(tx_hash, output_index)
            )
        )";
        
        if (!executeSQL(createUtxo)) {
            return false;
        }
        
        // Addresses table for wallet management
        std::string createAddresses = R"(
            CREATE TABLE IF NOT EXISTS addresses (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                address TEXT UNIQUE NOT NULL,
                public_key TEXT,
                balance REAL NOT NULL DEFAULT 0,
                transaction_count INTEGER NOT NULL DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (!executeSQL(createAddresses)) {
            return false;
        }
        
        // Peers table for network management
        std::string createPeers = R"(
            CREATE TABLE IF NOT EXISTS peers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                host TEXT NOT NULL,
                port INTEGER NOT NULL,
                version TEXT,
                user_agent TEXT,
                last_seen INTEGER,
                connection_count INTEGER DEFAULT 0,
                is_seed BOOLEAN DEFAULT 0,
                is_banned BOOLEAN DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                UNIQUE(host, port)
            )
        )";
        
        if (!executeSQL(createPeers)) {
            return false;
        }
        
        // Traceability index table
        std::string createTraceability = R"(
            CREATE TABLE IF NOT EXISTS traceability_index (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                tx_hash TEXT UNIQUE NOT NULL,
                prev_tx_hash TEXT NOT NULL,
                referenced_amount REAL NOT NULL,
                timestamp INTEGER NOT NULL,
                block_height INTEGER NOT NULL,
                validation_status BOOLEAN NOT NULL DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (tx_hash) REFERENCES transactions(hash)
            )
        )";
        
        if (!executeSQL(createTraceability)) {
            return false;
        }
        
        // Validators table for staking
        std::string createValidators = R"(
            CREATE TABLE IF NOT EXISTS validators (
                address TEXT PRIMARY KEY,
                stake_amount REAL NOT NULL,
                staking_days INTEGER NOT NULL,
                stake_start_time INTEGER NOT NULL,
                is_active BOOLEAN NOT NULL DEFAULT 1,
                public_key TEXT NOT NULL,
                blocks_produced INTEGER NOT NULL DEFAULT 0,
                missed_blocks INTEGER NOT NULL DEFAULT 0,
                uptime REAL NOT NULL DEFAULT 1.0,
                total_rewards REAL NOT NULL DEFAULT 0.0,
                pending_rewards REAL NOT NULL DEFAULT 0.0,
                is_slashed BOOLEAN NOT NULL DEFAULT 0,
                slashed_amount REAL NOT NULL DEFAULT 0.0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (!executeSQL(createValidators)) {
            return false;
        }
        
        // Create indexes for better performance
        createIndexes();
        
        LOG_DATABASE(LogLevel::INFO, "Database tables created successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Error creating tables: " + std::string(e.what()));
        return false;
    }
}

bool Database::createIndexes() {
    std::vector<std::string> indexes = {
        "CREATE INDEX IF NOT EXISTS idx_blocks_height ON blocks(height)",
        "CREATE INDEX IF NOT EXISTS idx_blocks_hash ON blocks(hash)",
        "CREATE INDEX IF NOT EXISTS idx_blocks_timestamp ON blocks(timestamp)",
        
        "CREATE INDEX IF NOT EXISTS idx_transactions_hash ON transactions(hash)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_block_hash ON transactions(block_hash)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_sender ON transactions(sender_address)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_recipient ON transactions(recipient_address)",
        
        "CREATE INDEX IF NOT EXISTS idx_validators_active ON validators(is_active)",
        "CREATE INDEX IF NOT EXISTS idx_validators_stake ON validators(stake_amount)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_timestamp ON transactions(timestamp)",
        
        "CREATE INDEX IF NOT EXISTS idx_inputs_tx_hash ON transaction_inputs(transaction_hash)",
        "CREATE INDEX IF NOT EXISTS idx_inputs_prev_tx ON transaction_inputs(prev_tx_hash, prev_output_index)",
        
        "CREATE INDEX IF NOT EXISTS idx_outputs_tx_hash ON transaction_outputs(transaction_hash)",
        "CREATE INDEX IF NOT EXISTS idx_outputs_address ON transaction_outputs(address)",
        "CREATE INDEX IF NOT EXISTS idx_outputs_spent ON transaction_outputs(spent)",
        
        "CREATE INDEX IF NOT EXISTS idx_utxo_address ON utxo(address)",
        "CREATE INDEX IF NOT EXISTS idx_utxo_tx_output ON utxo(tx_hash, output_index)",
        
        "CREATE INDEX IF NOT EXISTS idx_addresses_address ON addresses(address)",
        
        "CREATE INDEX IF NOT EXISTS idx_peers_host_port ON peers(host, port)",
        "CREATE INDEX IF NOT EXISTS idx_peers_last_seen ON peers(last_seen)",
        
        "CREATE INDEX IF NOT EXISTS idx_traceability_tx_hash ON traceability_index(tx_hash)",
        "CREATE INDEX IF NOT EXISTS idx_traceability_prev_tx ON traceability_index(prev_tx_hash)"
    };
    
    for (const auto& index : indexes) {
        if (!executeSQL(index)) {
            LOG_DATABASE(LogLevel::WARNING, "Failed to create index: " + index);
        }
    }
    
    return true;
}

bool Database::executeSQL(const std::string& sql) {
    if (!db) {
        LOG_DATABASE(LogLevel::ERROR, "Database not open");
        return false;
    }
    
    char* errorMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (rc != SQLITE_OK) {
        std::string error = errorMsg ? errorMsg : "Unknown error";
        LOG_DATABASE(LogLevel::ERROR, "SQL execution failed: " + error);
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

bool Database::saveBlock(const Block& block) {
    if (!db) {
        return false;
    }
    
    try {
        // Begin transaction
        if (!executeSQL("BEGIN TRANSACTION;")) {
            return false;
        }
        
        // Insert block
        std::string sql = R"(
            INSERT INTO blocks (height, hash, previous_hash, merkle_root, timestamp, 
                              difficulty, nonce, miner_address, block_type, 
                              transaction_count, size_bytes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to prepare block insert statement");
            executeSQL("ROLLBACK;");
            return false;
        }
        
        sqlite3_bind_int(stmt, 1, static_cast<int>(block.getIndex()));
        sqlite3_bind_text(stmt, 2, block.getHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, block.getPreviousHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, block.getMerkleRoot().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 5, block.getTimestamp());
        sqlite3_bind_double(stmt, 6, block.getDifficulty());
        sqlite3_bind_int64(stmt, 7, block.getNonce());
        sqlite3_bind_text(stmt, 8, block.getMinerAddress().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 9, static_cast<int>(block.getBlockType()));
        sqlite3_bind_int(stmt, 10, static_cast<int>(block.getTransactions().size()));
        // Calculate block size from transactions
        size_t blockSize = block.getTransactions().size() * 256; // Approximate size
        sqlite3_bind_int(stmt, 11, static_cast<int>(blockSize));
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to insert block");
            executeSQL("ROLLBACK;");
            return false;
        }
        
        // Save transactions (this also updates UTXO set)
        for (const auto& tx : block.getTransactions()) {
            if (!saveTransaction(tx, block.getHash(), block.getIndex())) {
                LOG_DATABASE(LogLevel::ERROR, "Failed to save transaction in block");
                executeSQL("ROLLBACK;");
                return false;
            }
        }
        
        LOG_DATABASE(LogLevel::DEBUG, "Saved " + std::to_string(block.getTransactions().size()) + 
                    " transactions and updated UTXO set for block at height " + 
                    std::to_string(block.getIndex()));
        
        // Commit transaction
        if (!executeSQL("COMMIT;")) {
            executeSQL("ROLLBACK;");
            return false;
        }
        
        LOG_DATABASE(LogLevel::DEBUG, "Saved block: " + block.getHash().substr(0, 16) + "...");
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Exception saving block: " + std::string(e.what()));
        executeSQL("ROLLBACK;");
        return false;
    }
}

bool Database::saveTransaction(const Transaction& tx, const std::string& blockHash, size_t blockHeight) {
    if (!db) {
        return false;
    }
    
    try {
        // Insert transaction
        std::string sql = R"(
            INSERT INTO transactions (hash, block_hash, block_height, sender_address, 
                                    recipient_address, amount, fee, timestamp, nonce, 
                                    signature, is_coinbase, prev_tx_hash, referenced_amount, 
                                    traceability_valid)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to prepare transaction insert statement");
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, tx.getHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, blockHash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, static_cast<int>(blockHeight));
        sqlite3_bind_text(stmt, 4, tx.getSenderAddress().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, tx.getReceiverAddress().c_str(), -1, SQLITE_STATIC);
        // Calculate total amount from outputs
        double totalAmount = 0.0;
        for (const auto& output : tx.getOutputs()) {
            totalAmount += output.amount;
        }
        sqlite3_bind_double(stmt, 6, totalAmount);
        sqlite3_bind_double(stmt, 7, tx.getFee());
        sqlite3_bind_int64(stmt, 8, tx.getTimestamp());
        sqlite3_bind_int64(stmt, 9, tx.getNonce());
        // Get signature from first input if available
        std::string signature = "";
        if (!tx.getInputs().empty()) {
            signature = tx.getInputs()[0].signature;
        }
        sqlite3_bind_text(stmt, 10, signature.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 11, tx.isCoinbaseTransaction() ? 1 : 0);
        sqlite3_bind_text(stmt, 12, tx.getPrevTxHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 13, tx.getReferencedAmount());
        sqlite3_bind_int(stmt, 14, tx.isTraceabilityValid() ? 1 : 0);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to insert transaction: " + std::string(sqlite3_errmsg(db)));
            return false;
        }
        
        // Save inputs and outputs
        if (!saveTransactionInputs(tx) || !saveTransactionOutputs(tx)) {
            return false;
        }
        
        // Update UTXO set (pass block height for proper tracking)
        if (!updateUtxoSet(tx, blockHeight)) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to update UTXO set");
            return false;
        }
        
        // Update traceability index
        if (!tx.isCoinbaseTransaction()) {
            if (!saveTraceabilityRecord(tx, blockHeight)) {
                return false;
            }
        }

        // Update address statistics
        // For coinbase, only receiver gets updated
        if (tx.isCoinbaseTransaction()) {
            // Calculate total output amount
            double totalOutput = 0.0;
            for (const auto& output : tx.getOutputs()) {
                totalOutput += output.amount;
            }
            // Use receiver address from first output or explicit receiver
            std::string receiver = tx.getReceiverAddress();
            if (receiver.empty() && !tx.getOutputs().empty()) {
                receiver = tx.getOutputs()[0].address;
            }

            if (!receiver.empty()) {
                updateAddressStats(receiver, totalOutput, true);
            }
        } else {
            // For regular transactions:
            // 1. Sender balance decreases (amount + fee)
            // 2. Receiver balance increases (amount)

            // Sender
            std::string sender = tx.getSenderAddress();
            if (!sender.empty()) {
                // Calculate total input amount (approximate as output + fee for now)
                // ideally we should sum inputs, but let's just subtract outputs + fee
                // actually, getting exact input amount requires looking up previous TXs
                // simpler: just track net change based on outputs + fee
                double totalSent = 0.0;
                for (const auto& output : tx.getOutputs()) {
                    totalSent += output.amount;
                }
                totalSent += tx.getFee();
                updateAddressStats(sender, -totalSent, true);
            }

            // Receivers (handle multiple outputs)
            for (const auto& output : tx.getOutputs()) {
                if (!updateAddressStats(output.address, output.amount, true)) {
                    LOG_DATABASE(LogLevel::WARNING, "Failed to update stats for address: " + output.address);
                }
            }
            // If there's a main receiver not in outputs (legacy format), update them
            // but usually receiver IS in outputs.
        }
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Exception saving transaction: " + std::string(e.what()));
        return false;
    }
}

bool Database::saveTransactionInputs(const Transaction& tx) {
    const auto& inputs = tx.getInputs();
    
    for (size_t i = 0; i < inputs.size(); i++) {
        const auto& input = inputs[i];
        
        std::string sql = R"(
            INSERT INTO transaction_inputs (transaction_hash, input_index, prev_tx_hash, 
                                          prev_output_index, amount, signature)
            VALUES (?, ?, ?, ?, ?, ?)
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, tx.getHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(i));
        sqlite3_bind_text(stmt, 3, input.txHash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, static_cast<int>(input.outputIndex));
        sqlite3_bind_double(stmt, 5, input.amount);
        sqlite3_bind_text(stmt, 6, input.signature.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            return false;
        }
    }
    
    return true;
}

bool Database::saveTransactionOutputs(const Transaction& tx) {
    const auto& outputs = tx.getOutputs();
    
    for (size_t i = 0; i < outputs.size(); i++) {
        const auto& output = outputs[i];
        
        std::string sql = R"(
            INSERT INTO transaction_outputs (transaction_hash, output_index, address, amount)
            VALUES (?, ?, ?, ?)
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, tx.getHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(i));
        sqlite3_bind_text(stmt, 3, output.address.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, output.amount);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            return false;
        }
    }
    
    return true;
}

bool Database::updateUtxoSet(const Transaction& tx, size_t blockHeight) {
    // Remove spent UTXOs (inputs consume previous outputs)
    for (const auto& input : tx.getInputs()) {
        std::string sql = "DELETE FROM utxo WHERE tx_hash = ? AND output_index = ?";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, input.txHash.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, static_cast<int>(input.outputIndex));
            
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
    
    // Add new UTXOs (outputs become new unspent outputs)
    const auto& outputs = tx.getOutputs();
    for (size_t i = 0; i < outputs.size(); i++) {
        const auto& output = outputs[i];
        
        std::string sql = R"(
            INSERT OR REPLACE INTO utxo (tx_hash, output_index, address, amount, block_height)
            VALUES (?, ?, ?, ?, ?)
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to prepare UTXO insert statement");
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, tx.getHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(i));
        sqlite3_bind_text(stmt, 3, output.address.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, output.amount);
        sqlite3_bind_int(stmt, 5, static_cast<int>(blockHeight)); // Use actual block height
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to insert UTXO: " + std::string(sqlite3_errmsg(db)));
            return false;
        }
    }
    
    LOG_DATABASE(LogLevel::DEBUG, "Updated UTXO set: removed " + std::to_string(tx.getInputs().size()) + 
                " inputs, added " + std::to_string(outputs.size()) + " outputs at block height " + 
                std::to_string(blockHeight));
    
    return true;
}

bool Database::saveTraceabilityRecord(const Transaction& tx, size_t blockHeight) {
    std::string sql = R"(
        INSERT INTO traceability_index (tx_hash, prev_tx_hash, referenced_amount, 
                                       timestamp, block_height, validation_status)
        VALUES (?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, tx.getHash().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, tx.getPrevTxHash().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, tx.getReferencedAmount());
    sqlite3_bind_int64(stmt, 4, tx.getTimestamp());
    sqlite3_bind_int(stmt, 5, static_cast<int>(blockHeight));
    sqlite3_bind_int(stmt, 6, tx.isTraceabilityValid() ? 1 : 0);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

Block Database::getBlock(const std::string& hash) const {
    if (!db) {
        return Block();
    }
    
    std::string sql = R"(
        SELECT height, hash, previous_hash, merkle_root, timestamp, difficulty, 
               nonce, miner_address, block_type, transaction_count, size_bytes
        FROM blocks WHERE hash = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return Block();
    }
    
    sqlite3_bind_text(stmt, 1, hash.c_str(), -1, SQLITE_STATIC);
    
    Block block;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        uint32_t index = sqlite3_column_int(stmt, 0);
        std::string prevHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        BlockType blockType = static_cast<BlockType>(sqlite3_column_int(stmt, 8));
        
        // Construct block with proper type
        block = Block(index, prevHash, blockType);
        block.setTimestamp(sqlite3_column_int64(stmt, 4));
        block.setDifficulty(sqlite3_column_double(stmt, 5));
        block.setNonce(sqlite3_column_int64(stmt, 6));
        block.setMinerAddress(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        
        // Load transactions for this block
        auto transactions = getTransactionsByBlockHash(hash);
        for (const auto& tx : transactions) {
            block.addTransaction(tx);
        }
    }
    
    sqlite3_finalize(stmt);
    return block;
}

std::vector<Transaction> Database::getTransactionsByBlockHash(const std::string& blockHash) const {
    std::vector<Transaction> transactions;
    
    if (!db) {
        return transactions;
    }
    
    std::string sql = R"(
        SELECT hash, sender_address, recipient_address, amount, fee, timestamp, 
               nonce, signature, is_coinbase, prev_tx_hash, referenced_amount
        FROM transactions WHERE block_hash = ?
        ORDER BY id
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return transactions;
    }
    
    sqlite3_bind_text(stmt, 1, blockHash.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction tx;
        tx.setHash(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        tx.setSenderAddress(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        tx.setReceiverAddress(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        // Amount is calculated from outputs, not stored directly
        tx.setFee(sqlite3_column_double(stmt, 4));
        tx.setTimestamp(sqlite3_column_int64(stmt, 5));
        tx.setNonce(sqlite3_column_int64(stmt, 6));
        // Signature is stored in TransactionInput, not Transaction
        tx.setCoinbaseTransaction(sqlite3_column_int(stmt, 8) == 1);
        tx.setPrevTxHash(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        tx.setReferencedAmount(sqlite3_column_double(stmt, 10));
        
        // Load inputs and outputs
        auto inputs = getTransactionInputs(tx.getHash());
        auto outputs = getTransactionOutputs(tx.getHash());
        
        for (const auto& input : inputs) {
            tx.addInput(input);
        }
        
        for (const auto& output : outputs) {
            tx.addOutput(output);
        }
        
        transactions.push_back(tx);
    }
    
    sqlite3_finalize(stmt);
    return transactions;
}

std::vector<TransactionInput> Database::getTransactionInputs(const std::string& txHash) const {
    std::vector<TransactionInput> inputs;
    
    if (!db) {
        return inputs;
    }
    
    std::string sql = R"(
        SELECT prev_tx_hash, prev_output_index, amount, signature
        FROM transaction_inputs 
        WHERE transaction_hash = ?
        ORDER BY input_index
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return inputs;
    }
    
    sqlite3_bind_text(stmt, 1, txHash.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TransactionInput input;
        input.txHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        input.outputIndex = sqlite3_column_int(stmt, 1);
        input.amount = sqlite3_column_double(stmt, 2);
        input.signature = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        inputs.push_back(input);
    }
    
    sqlite3_finalize(stmt);
    return inputs;
}

std::vector<TransactionOutput> Database::getTransactionOutputs(const std::string& txHash) const {
    std::vector<TransactionOutput> outputs;
    
    if (!db) {
        return outputs;
    }
    
    std::string sql = R"(
        SELECT address, amount
        FROM transaction_outputs 
        WHERE transaction_hash = ?
        ORDER BY output_index
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return outputs;
    }
    
    sqlite3_bind_text(stmt, 1, txHash.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TransactionOutput output;
        output.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        output.amount = sqlite3_column_double(stmt, 1);
        
        outputs.push_back(output);
    }
    
    sqlite3_finalize(stmt);
    return outputs;
}

double Database::getAddressBalance(const std::string& address) const {
    if (!db) {
        return 0.0;
    }
    
    std::string sql = "SELECT SUM(amount) FROM utxo WHERE address = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return 0.0;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_STATIC);
    
    double balance = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        balance = sqlite3_column_double(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return balance;
}

size_t Database::getBlockCount() const {
    if (!db) {
        return 0;
    }
    
    std::string sql = "SELECT COUNT(*) FROM blocks";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    size_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

std::vector<Block> Database::getAllBlocks() const {
    std::vector<Block> blocks;
    
    if (!db) {
        return blocks;
    }
    
    std::string sql = R"(
        SELECT height, hash, previous_hash, merkle_root, timestamp, difficulty, 
               nonce, miner_address, block_type, transaction_count, size_bytes
        FROM blocks
        ORDER BY height ASC
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to prepare getAllBlocks statement");
        return blocks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        uint32_t index = sqlite3_column_int(stmt, 0);
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string prevHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        BlockType blockType = static_cast<BlockType>(sqlite3_column_int(stmt, 8));
        
        // Construct block with proper type
        Block block(index, prevHash, blockType);
        block.setHash(hash);
        block.setMerkleRoot(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        block.setTimestamp(sqlite3_column_int64(stmt, 4));
        block.setDifficulty(sqlite3_column_double(stmt, 5));
        block.setNonce(sqlite3_column_int64(stmt, 6));
        block.setMinerAddress(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        
        // Load transactions for this block
        auto transactions = getTransactionsByBlockHash(hash);
        for (const auto& tx : transactions) {
            block.addTransaction(tx);
        }
        
        blocks.push_back(block);
    }
    
    sqlite3_finalize(stmt);
    LOG_DATABASE(LogLevel::INFO, "Loaded " + std::to_string(blocks.size()) + " blocks from database");
    return blocks;
}

std::vector<Block> Database::getBlocksByRange(uint32_t startHeight, uint32_t endHeight) const {
    std::vector<Block> blocks;
    
    if (!db) {
        return blocks;
    }
    
    std::string sql = R"(
        SELECT height, hash, previous_hash, merkle_root, timestamp, difficulty, 
               nonce, miner_address, block_type, transaction_count, size_bytes
        FROM blocks
        WHERE height >= ? AND height <= ?
        ORDER BY height ASC
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return blocks;
    }
    
    sqlite3_bind_int(stmt, 1, static_cast<int>(startHeight));
    sqlite3_bind_int(stmt, 2, static_cast<int>(endHeight));
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        uint32_t index = sqlite3_column_int(stmt, 0);
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string prevHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        BlockType blockType = static_cast<BlockType>(sqlite3_column_int(stmt, 8));
        
        Block block(index, prevHash, blockType);
        block.setHash(hash);
        block.setMerkleRoot(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        block.setTimestamp(sqlite3_column_int64(stmt, 4));
        block.setDifficulty(sqlite3_column_double(stmt, 5));
        block.setNonce(sqlite3_column_int64(stmt, 6));
        block.setMinerAddress(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        
        auto transactions = getTransactionsByBlockHash(hash);
        for (const auto& tx : transactions) {
            block.addTransaction(tx);
        }
        
        blocks.push_back(block);
    }
    
    sqlite3_finalize(stmt);
    return blocks;
}

bool Database::isConnected() const {
    return db != nullptr;
}

// Validator persistence methods
bool Database::storeValidator(const Validator& validator) {
    std::string sql = R"(
        INSERT OR REPLACE INTO validators (
            address, stake_amount, staking_days, stake_start_time, is_active,
            public_key, blocks_produced, missed_blocks, uptime,
            total_rewards, pending_rewards, is_slashed, slashed_amount, updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to prepare validator insert: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, validator.getAddress().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, validator.getStakeAmount());
    sqlite3_bind_int(stmt, 3, validator.getStakingDays());
    sqlite3_bind_int64(stmt, 4, validator.getStakeStartTime());
    sqlite3_bind_int(stmt, 5, validator.getIsActive() ? 1 : 0);
    sqlite3_bind_text(stmt, 6, validator.getPublicKey().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, validator.getBlocksProduced());
    sqlite3_bind_int(stmt, 8, validator.getMissedBlocks());
    sqlite3_bind_double(stmt, 9, validator.getUptime());
    sqlite3_bind_double(stmt, 10, validator.getTotalRewards());
    sqlite3_bind_double(stmt, 11, validator.getPendingRewards());
    sqlite3_bind_int(stmt, 12, validator.getIsSlashed() ? 1 : 0);
    sqlite3_bind_double(stmt, 13, validator.getSlashedAmount());
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to store validator: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    LOG_DATABASE(LogLevel::INFO, "Stored validator: " + validator.getAddress());
    return true;
}

bool Database::getValidator(const std::string& address, Validator& validator) const {
    std::string sql = R"(
        SELECT address, stake_amount, staking_days, stake_start_time, is_active,
               public_key, blocks_produced, missed_blocks, uptime,
               total_rewards, pending_rewards, is_slashed, slashed_amount
        FROM validators
        WHERE address = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_TRANSIENT);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double stakeAmount = sqlite3_column_double(stmt, 1);
        uint32_t stakingDays = sqlite3_column_int(stmt, 2);
        
        validator = Validator(addr, stakeAmount, stakingDays);
        validator.setPublicKey(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        validator.setIsActive(sqlite3_column_int(stmt, 4) == 1);
        
        // Restore performance metrics
        for (uint32_t i = 0; i < sqlite3_column_int(stmt, 6); i++) {
            validator.recordBlockProduced();
        }
        for (uint32_t i = 0; i < sqlite3_column_int(stmt, 7); i++) {
            validator.recordMissedBlock();
        }
        validator.updateUptime(sqlite3_column_double(stmt, 8));
        
        // Restore rewards
        double totalRewards = sqlite3_column_double(stmt, 9);
        double pendingRewards = sqlite3_column_double(stmt, 10);
        validator.addReward(pendingRewards);
        // Set total rewards by distributing and adding back
        if (totalRewards > 0) {
            validator.distributePendingRewards();
            validator.addReward(totalRewards);
            validator.distributePendingRewards();
            validator.addReward(pendingRewards);
        }
        
        // Restore slashing status
        if (sqlite3_column_int(stmt, 11) == 1) {
            validator.slash(sqlite3_column_double(stmt, 12), "Restored from database");
        }
        
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::updateValidator(const Validator& validator) {
    return storeValidator(validator);
}

bool Database::deleteValidator(const std::string& address) {
    std::string sql = "DELETE FROM validators WHERE address = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<Validator> Database::getAllValidators() const {
    std::vector<Validator> validators;
    
    std::string sql = R"(
        SELECT address, stake_amount, staking_days, stake_start_time, is_active,
               public_key, blocks_produced, missed_blocks, uptime,
               total_rewards, pending_rewards, is_slashed, slashed_amount
        FROM validators
        ORDER BY stake_amount DESC
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return validators;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double stakeAmount = sqlite3_column_double(stmt, 1);
        uint32_t stakingDays = sqlite3_column_int(stmt, 2);
        
        Validator validator(addr, stakeAmount, stakingDays);
        validator.setPublicKey(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        validator.setIsActive(sqlite3_column_int(stmt, 4) == 1);
        
        // Restore performance metrics
        for (uint32_t i = 0; i < sqlite3_column_int(stmt, 6); i++) {
            validator.recordBlockProduced();
        }
        for (uint32_t i = 0; i < sqlite3_column_int(stmt, 7); i++) {
            validator.recordMissedBlock();
        }
        validator.updateUptime(sqlite3_column_double(stmt, 8));
        
        // Restore rewards
        double totalRewards = sqlite3_column_double(stmt, 9);
        double pendingRewards = sqlite3_column_double(stmt, 10);
        validator.addReward(pendingRewards);
        if (totalRewards > 0) {
            validator.distributePendingRewards();
            validator.addReward(totalRewards);
            validator.distributePendingRewards();
            validator.addReward(pendingRewards);
        }
        
        // Restore slashing status
        if (sqlite3_column_int(stmt, 11) == 1) {
            validator.slash(sqlite3_column_double(stmt, 12), "Restored from database");
        }
        
        validators.push_back(validator);
    }
    
    sqlite3_finalize(stmt);
    return validators;
}

std::vector<Validator> Database::getActiveValidators() const {
    std::vector<Validator> validators;
    
    std::string sql = R"(
        SELECT address, stake_amount, staking_days, stake_start_time, is_active,
               public_key, blocks_produced, missed_blocks, uptime,
               total_rewards, pending_rewards, is_slashed, slashed_amount
        FROM validators
        WHERE is_active = 1 AND is_slashed = 0
        ORDER BY stake_amount DESC
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return validators;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double stakeAmount = sqlite3_column_double(stmt, 1);
        uint32_t stakingDays = sqlite3_column_int(stmt, 2);
        
        Validator validator(addr, stakeAmount, stakingDays);
        validator.setPublicKey(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        validator.setIsActive(sqlite3_column_int(stmt, 4) == 1);
        
        // Restore performance metrics
        for (uint32_t i = 0; i < sqlite3_column_int(stmt, 6); i++) {
            validator.recordBlockProduced();
        }
        for (uint32_t i = 0; i < sqlite3_column_int(stmt, 7); i++) {
            validator.recordMissedBlock();
        }
        validator.updateUptime(sqlite3_column_double(stmt, 8));
        
        // Restore rewards
        double totalRewards = sqlite3_column_double(stmt, 9);
        double pendingRewards = sqlite3_column_double(stmt, 10);
        validator.addReward(pendingRewards);
        if (totalRewards > 0) {
            validator.distributePendingRewards();
            validator.addReward(totalRewards);
            validator.distributePendingRewards();
            validator.addReward(pendingRewards);
        }
        
        validators.push_back(validator);
    }
    
    sqlite3_finalize(stmt);
    return validators;
}
// UTXO management functions
bool Database::storeUTXO(const std::string& txHash, uint32_t outputIndex, const TransactionOutput& output, uint32_t blockHeight) {
    std::string sql = R"(
        INSERT OR REPLACE INTO utxo (tx_hash, output_index, address, amount, script, block_height)
        VALUES (?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to prepare UTXO insert: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, txHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, outputIndex);
    sqlite3_bind_text(stmt, 3, output.address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, output.amount);
    sqlite3_bind_text(stmt, 5, output.script.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, blockHeight);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to store UTXO: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    return true;
}

bool Database::getUTXO(const std::string& txHash, uint32_t outputIndex, TransactionOutput& output) const {
    std::string sql = R"(
        SELECT address, amount, script
        FROM utxo
        WHERE tx_hash = ? AND output_index = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, txHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, outputIndex);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        output.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        output.amount = sqlite3_column_double(stmt, 1);
        output.script = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::deleteUTXO(const std::string& txHash, uint32_t outputIndex) {
    std::string sql = "DELETE FROM utxo WHERE tx_hash = ? AND output_index = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to prepare UTXO delete: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, txHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, outputIndex);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to delete UTXO: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    return true;
}

std::vector<TransactionOutput> Database::getUTXOsByAddress(const std::string& address) const {
    std::vector<TransactionOutput> utxos;
    
    std::string sql = R"(
        SELECT address, amount, script
        FROM utxo
        WHERE address = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return utxos;
    }
    
    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_TRANSIENT);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TransactionOutput output;
        output.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        output.amount = sqlite3_column_double(stmt, 1);
        output.script = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        utxos.push_back(output);
    }
    
    sqlite3_finalize(stmt);
    return utxos;
}

bool Database::updateAddressStats(const std::string& address, double amountChange, bool isNewTransaction) {
    if (!db || address.empty()) {
        return false;
    }

    // Check if address exists
    std::string checkSql = "SELECT count(*) FROM addresses WHERE address = ?";
    sqlite3_stmt* checkStmt;
    if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(checkStmt, 1, address.c_str(), -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(checkStmt, 0) > 0;
    }
    sqlite3_finalize(checkStmt);

    if (exists) {
        // Update existing address
        std::string updateSql = "UPDATE addresses SET balance = balance + ?, transaction_count = transaction_count + ? WHERE address = ?";
        sqlite3_stmt* updateStmt;
        if (sqlite3_prepare_v2(db, updateSql.c_str(), -1, &updateStmt, nullptr) != SQLITE_OK) {
            return false;
        }
        sqlite3_bind_double(updateStmt, 1, amountChange);
        sqlite3_bind_int(updateStmt, 2, isNewTransaction ? 1 : 0);
        sqlite3_bind_text(updateStmt, 3, address.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(updateStmt) != SQLITE_DONE) {
            sqlite3_finalize(updateStmt);
            return false;
        }
        sqlite3_finalize(updateStmt);
    } else {
        // Insert new address
        std::string insertSql = "INSERT INTO addresses (address, balance, transaction_count) VALUES (?, ?, ?)";
        sqlite3_stmt* insertStmt;
        if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &insertStmt, nullptr) != SQLITE_OK) {
            return false;
        }
        sqlite3_bind_text(insertStmt, 1, address.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(insertStmt, 2, amountChange > 0 ? amountChange : 0); // Don't start with negative balance
        sqlite3_bind_int(insertStmt, 3, isNewTransaction ? 1 : 0);

        if (sqlite3_step(insertStmt) != SQLITE_DONE) {
            sqlite3_finalize(insertStmt);
            return false;
        }
        sqlite3_finalize(insertStmt);
    }

    return true;
}
