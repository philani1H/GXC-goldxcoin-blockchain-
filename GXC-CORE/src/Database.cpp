
#include "../include/Database.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/Config.h"
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/cache.h>
#include <leveldb/filter_policy.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Static instance
std::unique_ptr<Database> Database::instance = nullptr;
std::mutex Database::instanceMutex;

// Key prefixes
const std::string Database::PREFIX_BLOCK = "blk:";
const std::string Database::PREFIX_BLOCK_HEIGHT = "blkh:";
const std::string Database::PREFIX_TX = "tx:";
const std::string Database::PREFIX_TX_BLOCK = "txb:";
const std::string Database::PREFIX_UTXO = "utxo:";
const std::string Database::PREFIX_VALIDATOR = "val:";
const std::string Database::PREFIX_PEER = "peer:";
const std::string Database::PREFIX_CONFIG = "cfg:";
const std::string Database::PREFIX_TRACE = "trace:";
const std::string Database::PREFIX_ADDRESS = "addr:";

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
        
        std::filesystem::path pathObj(dbPath);
        std::string filename = pathObj.filename().string();

        // CRITICAL: Ensure testnet and mainnet databases are completely separate
        if (isTestnet) {
            if (filename.find("testnet") == std::string::npos) {
                LOG_DATABASE(LogLevel::ERROR, "TESTNET mode but database path doesn't contain 'testnet': " + dbPath);
                LOG_DATABASE(LogLevel::ERROR, "REFUSING to open database - risk of mainnet data corruption!");
                return false;
            }
            LOG_DATABASE(LogLevel::INFO, "✓ Testnet database path validated");
        } else {
            if (filename.find("testnet") != std::string::npos) {
                LOG_DATABASE(LogLevel::ERROR, "MAINNET mode but database path contains 'testnet': " + dbPath);
                LOG_DATABASE(LogLevel::ERROR, "REFUSING to open testnet database in mainnet mode!");
                return false;
            }
            LOG_DATABASE(LogLevel::INFO, "✓ Mainnet database path validated");
        }
        
        LOG_DATABASE(LogLevel::INFO, "Opening LevelDB database: " + dbPath);
        LOG_DATABASE(LogLevel::INFO, "Network mode: " + std::string(isTestnet ? "TESTNET" : "MAINNET"));
        
        // Configure LevelDB options
        options.create_if_missing = true;
        options.paranoid_checks = true;
        options.max_open_files = 100;
        options.write_buffer_size = 4 * 1024 * 1024;  // 4MB write buffer
        options.block_cache = leveldb::NewLRUCache(8 * 1024 * 1024);  // 8MB cache
        options.filter_policy = leveldb::NewBloomFilterPolicy(10);  // Bloom filter
        options.compression = leveldb::kSnappyCompression;
        
        // Configure write options for better performance
        // sync=false means writes are buffered and flushed asynchronously
        // This is safe because we can rebuild from the blockchain if needed
        writeOptions.sync = false;  // Don't force fsync on every write (much faster)
        
        // Create directory if it doesn't exist
        std::filesystem::create_directories(pathObj.parent_path());
        
        // Open database
        leveldb::DB* rawDb;
        leveldb::Status status = leveldb::DB::Open(options, dbPath, &rawDb);
        
        if (!status.ok()) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to open LevelDB: " + status.ToString());
            return false;
        }
        
        db.reset(rawDb);
        dataDirectory = dbPath;
        
        // Verify network mode in database metadata
        if (!checkDatabaseNetwork(isTestnet)) {
            LOG_DATABASE(LogLevel::ERROR, "Database network mismatch! Refusing to use this database.");
            close();
            return false;
        }

        LOG_DATABASE(LogLevel::INFO, "LevelDB database opened successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Exception opening database: " + std::string(e.what()));
        close();
        return false;
    }
}

void Database::close() {
    if (db) {
        db.reset();
        LOG_DATABASE(LogLevel::INFO, "Database closed");
    }
    
    // Clean up options
    delete options.block_cache;
    delete options.filter_policy;
    options.block_cache = nullptr;
    options.filter_policy = nullptr;
}

bool Database::checkDatabaseNetwork(bool isTestnet) {
    if (!db) return false;

    std::string expectedMode = isTestnet ? "testnet" : "mainnet";
    std::string storedMode;
    
    if (getConfigValue("network_mode", storedMode)) {
        // Mode exists, verify it
        if (storedMode != expectedMode) {
            LOG_DATABASE(LogLevel::ERROR, "CRITICAL ERROR: Database network mismatch!");
            LOG_DATABASE(LogLevel::ERROR, "Expected: " + expectedMode + ", Found: " + storedMode);
            return false;
        }
        LOG_DATABASE(LogLevel::INFO, "Database network mode verified: " + storedMode);
    } else {
        // Mode doesn't exist (new DB), set it
        if (!setConfigValue("network_mode", expectedMode)) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to save network mode");
            return false;
        }
        LOG_DATABASE(LogLevel::INFO, "Initialized database network mode: " + expectedMode);
    }

    return true;
}

std::string Database::makeKey(const std::string& prefix, const std::string& id) const {
    return prefix + id;
}

std::string Database::makeKey(const std::string& prefix, uint32_t id) const {
    // Use zero-padded format for proper sorting
    std::ostringstream oss;
    oss << prefix << std::setw(10) << std::setfill('0') << id;
    return oss.str();
}

bool Database::put(const std::string& key, const std::string& value) {
    if (!db) return false;
    
    leveldb::Status status = db->Put(writeOptions, key, value);
    if (!status.ok()) {
        LOG_DATABASE(LogLevel::ERROR, "LevelDB put failed: " + status.ToString());
        return false;
    }
    return true;
}

bool Database::get(const std::string& key, std::string& value) const {
    if (!db) return false;
    
    leveldb::Status status = db->Get(readOptions, key, &value);
    return status.ok();
}

bool Database::del(const std::string& key) {
    if (!db) return false;
    
    leveldb::Status status = db->Delete(writeOptions, key);
    return status.ok();
}

// Serialization helpers using JSON
std::string Database::serializeBlock(const Block& block) const {
    json j;
    j["index"] = block.getIndex();
    j["hash"] = block.getHash();
    j["previous_hash"] = block.getPreviousHash();
    j["merkle_root"] = block.getMerkleRoot();
    j["timestamp"] = block.getTimestamp();
    j["difficulty"] = block.getDifficulty();
    j["nonce"] = block.getNonce();
    j["miner_address"] = block.getMinerAddress();
    j["block_type"] = static_cast<int>(block.getBlockType());
    j["tx_count"] = block.getTransactions().size();
    
    // Serialize transaction hashes
    json txHashes = json::array();
    for (const auto& tx : block.getTransactions()) {
        txHashes.push_back(tx.getHash());
    }
    j["tx_hashes"] = txHashes;
    
    return j.dump();
}

Block Database::deserializeBlock(const std::string& data) const {
    try {
        json j = json::parse(data);
        
        uint32_t index = j["index"].get<uint32_t>();
        std::string prevHash = j["previous_hash"].get<std::string>();
        BlockType blockType = static_cast<BlockType>(j["block_type"].get<int>());
        
        Block block(index, prevHash, blockType);
        block.setHash(j["hash"].get<std::string>());
        block.setMerkleRoot(j["merkle_root"].get<std::string>());
        block.setTimestamp(j["timestamp"].get<uint64_t>());
        block.setDifficulty(j["difficulty"].get<double>());
        block.setNonce(j["nonce"].get<uint64_t>());
        block.setMinerAddress(j["miner_address"].get<std::string>());
        
        return block;
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to deserialize block: " + std::string(e.what()));
        return Block();
    }
}

std::string Database::serializeTransaction(const Transaction& tx) const {
    json j;
    j["hash"] = tx.getHash();
    j["sender"] = tx.getSenderAddress();
    j["receiver"] = tx.getReceiverAddress();
    j["fee"] = tx.getFee();
    j["timestamp"] = tx.getTimestamp();
    j["nonce"] = tx.getNonce();
    j["is_coinbase"] = tx.isCoinbaseTransaction();
    j["prev_tx_hash"] = tx.getPrevTxHash();
    j["referenced_amount"] = tx.getReferencedAmount();
    j["traceability_valid"] = tx.isTraceabilityValid();
    
    // Serialize inputs
    json inputs = json::array();
    for (const auto& input : tx.getInputs()) {
        json inp;
        inp["tx_hash"] = input.txHash;
        inp["output_index"] = input.outputIndex;
        inp["amount"] = input.amount;
        inp["signature"] = input.signature;
        inputs.push_back(inp);
    }
    j["inputs"] = inputs;
    
    // Serialize outputs
    json outputs = json::array();
    for (const auto& output : tx.getOutputs()) {
        json out;
        out["address"] = output.address;
        out["amount"] = output.amount;
        out["script"] = output.script;
        outputs.push_back(out);
    }
    j["outputs"] = outputs;
    
    return j.dump();
}

Transaction Database::deserializeTransaction(const std::string& data) const {
    try {
        json j = json::parse(data);
        
        Transaction tx;
        tx.setHash(j["hash"].get<std::string>());
        tx.setSenderAddress(j["sender"].get<std::string>());
        tx.setReceiverAddress(j["receiver"].get<std::string>());
        tx.setFee(j["fee"].get<double>());
        tx.setTimestamp(j["timestamp"].get<uint64_t>());
        tx.setNonce(j["nonce"].get<uint64_t>());
        tx.setCoinbaseTransaction(j["is_coinbase"].get<bool>());
        tx.setPrevTxHash(j["prev_tx_hash"].get<std::string>());
        tx.setReferencedAmount(j["referenced_amount"].get<double>());
        
        // Deserialize inputs
        for (const auto& inp : j["inputs"]) {
            TransactionInput input;
            input.txHash = inp["tx_hash"].get<std::string>();
            input.outputIndex = inp["output_index"].get<uint32_t>();
            input.amount = inp["amount"].get<double>();
            input.signature = inp["signature"].get<std::string>();
            tx.addInput(input);
        }
        
        // Deserialize outputs
        for (const auto& out : j["outputs"]) {
            TransactionOutput output;
            output.address = out["address"].get<std::string>();
            output.amount = out["amount"].get<double>();
            output.script = out["script"].get<std::string>();
            tx.addOutput(output);
        }
        
        return tx;
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to deserialize transaction: " + std::string(e.what()));
        return Transaction();
    }
}

std::string Database::serializeValidator(const Validator& validator) const {
    json j;
    j["address"] = validator.getAddress();
    j["stake_amount"] = validator.getStakeAmount();
    j["staking_days"] = validator.getStakingDays();
    j["stake_start_time"] = validator.getStakeStartTime();
    j["is_active"] = validator.getIsActive();
    j["public_key"] = validator.getPublicKey();
    j["blocks_produced"] = validator.getBlocksProduced();
    j["missed_blocks"] = validator.getMissedBlocks();
    j["uptime"] = validator.getUptime();
    j["total_rewards"] = validator.getTotalRewards();
    j["pending_rewards"] = validator.getPendingRewards();
    j["is_slashed"] = validator.getIsSlashed();
    j["slashed_amount"] = validator.getSlashedAmount();
    return j.dump();
}

Validator Database::deserializeValidator(const std::string& data) const {
    try {
        json j = json::parse(data);
        
        std::string addr = j["address"].get<std::string>();
        double stakeAmount = j["stake_amount"].get<double>();
        uint32_t stakingDays = j["staking_days"].get<uint32_t>();
        
        Validator validator(addr, stakeAmount, stakingDays);
        validator.setPublicKey(j["public_key"].get<std::string>());
        validator.setIsActive(j["is_active"].get<bool>());
        
        // Restore metrics
        for (uint32_t i = 0; i < j["blocks_produced"].get<uint32_t>(); i++) {
            validator.recordBlockProduced();
        }
        for (uint32_t i = 0; i < j["missed_blocks"].get<uint32_t>(); i++) {
            validator.recordMissedBlock();
        }
        
        return validator;
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Failed to deserialize validator: " + std::string(e.what()));
        return Validator("", 0, 0);
    }
}

// Block operations
bool Database::saveBlock(const Block& block) {
    if (!db) return false;
    
    try {
        // Use a single WriteBatch for all operations to avoid multiple writes
        leveldb::WriteBatch batch;
        
        // Store block by hash
        std::string blockData = serializeBlock(block);
        batch.Put(makeKey(PREFIX_BLOCK, block.getHash()), blockData);
        
        // Store hash by height (for height-based lookups)
        batch.Put(makeKey(PREFIX_BLOCK_HEIGHT, block.getIndex()), block.getHash());
        
        // Update latest block height
        batch.Put(makeKey(PREFIX_CONFIG, "latest_block_height"), std::to_string(block.getIndex()));
        batch.Put(makeKey(PREFIX_CONFIG, "latest_block_hash"), block.getHash());
        
        // Store all transactions in the same batch
        for (const auto& tx : block.getTransactions()) {
            // Store transaction by hash
            std::string txData = serializeTransaction(tx);
            batch.Put(makeKey(PREFIX_TX, tx.getHash()), txData);
            
            // Store tx hash -> block hash mapping
            json mapping;
            mapping["block_hash"] = block.getHash();
            mapping["block_height"] = block.getIndex();
            batch.Put(makeKey(PREFIX_TX_BLOCK, tx.getHash()), mapping.dump());
            
            // Update UTXO set in the same batch
            // Remove spent UTXOs
            for (const auto& input : tx.getInputs()) {
                std::string utxoKey = makeKey(PREFIX_UTXO, input.txHash + ":" + std::to_string(input.outputIndex));
                batch.Delete(utxoKey);
            }
            
            // Add new UTXOs
            const auto& outputs = tx.getOutputs();
            for (size_t i = 0; i < outputs.size(); i++) {
                const auto& output = outputs[i];
                
                json utxo;
                utxo["tx_hash"] = tx.getHash();
                utxo["output_index"] = i;
                utxo["address"] = output.address;
                utxo["amount"] = output.amount;
                utxo["script"] = output.script;
                utxo["block_height"] = block.getIndex();
                
                std::string utxoKey = makeKey(PREFIX_UTXO, tx.getHash() + ":" + std::to_string(i));
                batch.Put(utxoKey, utxo.dump());
                
                // Also index by address for balance lookups
                std::string addrKey = makeKey(PREFIX_ADDRESS, output.address + ":" + tx.getHash() + ":" + std::to_string(i));
                batch.Put(addrKey, utxo.dump());
            }
            
            // Save traceability record in the same batch
            if (!tx.isCoinbaseTransaction()) {
                json trace;
                trace["tx_hash"] = tx.getHash();
                trace["prev_tx_hash"] = tx.getPrevTxHash();
                trace["referenced_amount"] = tx.getReferencedAmount();
                trace["sender"] = tx.getSenderAddress();
                trace["receiver"] = tx.getReceiverAddress();
                trace["block_height"] = block.getIndex();
                trace["timestamp"] = tx.getTimestamp();
                
                batch.Put(makeKey(PREFIX_TRACE, tx.getHash()), trace.dump());
            }
        }
        
        // Single atomic write for everything
        leveldb::Status status = db->Write(writeOptions, &batch);
        if (!status.ok()) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to save block: " + status.ToString());
            return false;
        }
        
        LOG_DATABASE(LogLevel::DEBUG, "Saved block: " + block.getHash().substr(0, 16) + "... with " + 
                    std::to_string(block.getTransactions().size()) + " transactions");
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Exception saving block: " + std::string(e.what()));
        return false;
    }
}

bool Database::storeBlock(const Block& block) {
    return saveBlock(block);
}

bool Database::getBlock(uint32_t index, Block& block) const {
    std::string hash;
    if (!get(makeKey(PREFIX_BLOCK_HEIGHT, index), hash)) {
        return false;
    }
    return getBlock(hash, block);
}

bool Database::getBlock(const std::string& hash, Block& block) const {
    std::string data;
    if (!get(makeKey(PREFIX_BLOCK, hash), data)) {
        return false;
    }
    
    block = deserializeBlock(data);
    
    // Load transactions
    auto transactions = getTransactionsByBlockHash(hash);
    for (const auto& tx : transactions) {
        block.addTransaction(tx);
    }
    
    return true;
}

Block Database::getBlock(const std::string& hash) const {
    Block block;
    getBlock(hash, block);
    return block;
}

bool Database::deleteBlock(uint32_t index) {
    std::string hash;
    if (!get(makeKey(PREFIX_BLOCK_HEIGHT, index), hash)) {
        return false;
    }
    
    leveldb::WriteBatch batch;
    batch.Delete(makeKey(PREFIX_BLOCK, hash));
    batch.Delete(makeKey(PREFIX_BLOCK_HEIGHT, index));
    
    return db->Write(writeOptions, &batch).ok();
}

uint32_t Database::getLatestBlockIndex() const {
    std::string value;
    if (getConfigValue("latest_block_height", value)) {
        return std::stoul(value);
    }
    return 0;
}

std::string Database::getLatestBlockHash() const {
    std::string value;
    getConfigValue("latest_block_hash", value);
    return value;
}

std::vector<Block> Database::getAllBlocks() const {
    std::vector<Block> blocks;
    
    if (!db) return blocks;
    
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    
    for (it->Seek(PREFIX_BLOCK_HEIGHT); it->Valid(); it->Next()) {
        std::string key = it->key().ToString();
        if (key.find(PREFIX_BLOCK_HEIGHT) != 0) break;
        
        std::string hash = it->value().ToString();
        Block block;
        if (getBlock(hash, block)) {
            blocks.push_back(block);
        }
    }
    
    LOG_DATABASE(LogLevel::INFO, "Loaded " + std::to_string(blocks.size()) + " blocks from database");
    return blocks;
}

std::vector<Block> Database::getBlocksByRange(uint32_t startHeight, uint32_t endHeight) const {
    std::vector<Block> blocks;
    
    for (uint32_t i = startHeight; i <= endHeight; i++) {
        Block block;
        if (getBlock(i, block)) {
            blocks.push_back(block);
        }
    }
    
    return blocks;
}

// Transaction operations
bool Database::saveTransaction(const Transaction& tx, const std::string& blockHash, size_t blockIndex) {
    if (!db) return false;
    
    try {
        leveldb::WriteBatch batch;
        
        // Store transaction by hash
        std::string txData = serializeTransaction(tx);
        batch.Put(makeKey(PREFIX_TX, tx.getHash()), txData);
        
        // Store tx hash -> block hash mapping
        json mapping;
        mapping["block_hash"] = blockHash;
        mapping["block_height"] = blockIndex;
        batch.Put(makeKey(PREFIX_TX_BLOCK, tx.getHash()), mapping.dump());
        
        leveldb::Status status = db->Write(writeOptions, &batch);
        if (!status.ok()) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to save transaction: " + status.ToString());
            return false;
        }
        
        // Update UTXO set
        if (!updateUtxoSet(tx, blockIndex)) {
            LOG_DATABASE(LogLevel::ERROR, "Failed to update UTXO set");
            return false;
        }
        
        // Save traceability record
        if (!tx.isCoinbaseTransaction()) {
            if (!saveTraceabilityRecord(tx, blockIndex)) {
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_DATABASE(LogLevel::ERROR, "Exception saving transaction: " + std::string(e.what()));
        return false;
    }
}

bool Database::storeTransaction(const Transaction& tx) {
    return saveTransaction(tx, "", 0);
}

bool Database::saveTransactionInputs(const Transaction& tx) {
    // Inputs are stored as part of the transaction
    return true;
}

bool Database::saveTransactionOutputs(const Transaction& tx) {
    // Outputs are stored as part of the transaction
    return true;
}

bool Database::updateUtxoSet(const Transaction& tx, size_t blockHeight) {
    if (!db) return false;
    
    leveldb::WriteBatch batch;
    
    // Remove spent UTXOs
    for (const auto& input : tx.getInputs()) {
        std::string utxoKey = makeKey(PREFIX_UTXO, input.txHash + ":" + std::to_string(input.outputIndex));
        batch.Delete(utxoKey);
    }
    
    // Add new UTXOs
    const auto& outputs = tx.getOutputs();
    for (size_t i = 0; i < outputs.size(); i++) {
        const auto& output = outputs[i];
        
        json utxo;
        utxo["tx_hash"] = tx.getHash();
        utxo["output_index"] = i;
        utxo["address"] = output.address;
        utxo["amount"] = output.amount;
        utxo["script"] = output.script;
        utxo["block_height"] = blockHeight;
        
        std::string utxoKey = makeKey(PREFIX_UTXO, tx.getHash() + ":" + std::to_string(i));
        batch.Put(utxoKey, utxo.dump());
        
        // Also index by address for balance lookups
        std::string addrKey = makeKey(PREFIX_ADDRESS, output.address + ":" + tx.getHash() + ":" + std::to_string(i));
        batch.Put(addrKey, utxo.dump());
    }
    
    return db->Write(writeOptions, &batch).ok();
}

bool Database::saveTraceabilityRecord(const Transaction& tx, size_t blockHeight) {
    json trace;
    trace["tx_hash"] = tx.getHash();
    trace["prev_tx_hash"] = tx.getPrevTxHash();
    trace["referenced_amount"] = tx.getReferencedAmount();
    trace["timestamp"] = tx.getTimestamp();
    trace["block_height"] = blockHeight;
    trace["validation_status"] = tx.isTraceabilityValid();
    
    return put(makeKey(PREFIX_TRACE, tx.getHash()), trace.dump());
}

std::vector<Transaction> Database::getTransactionsByBlockHash(const std::string& blockHash) const {
    std::vector<Transaction> transactions;
    
    if (!db) return transactions;
    
    // Iterate through all tx_block mappings
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    
    for (it->Seek(PREFIX_TX_BLOCK); it->Valid(); it->Next()) {
        std::string key = it->key().ToString();
        if (key.find(PREFIX_TX_BLOCK) != 0) break;
        
        try {
            json mapping = json::parse(it->value().ToString());
            if (mapping["block_hash"].get<std::string>() == blockHash) {
                std::string txHash = key.substr(PREFIX_TX_BLOCK.length());
                std::string txData;
                if (get(makeKey(PREFIX_TX, txHash), txData)) {
                    transactions.push_back(deserializeTransaction(txData));
                }
            }
        } catch (...) {
            continue;
        }
    }
    
    return transactions;
}

std::vector<TransactionInput> Database::getTransactionInputs(const std::string& txHash) const {
    std::string txData;
    if (get(makeKey(PREFIX_TX, txHash), txData)) {
        Transaction tx = deserializeTransaction(txData);
        return tx.getInputs();
    }
    return {};
}

std::vector<TransactionOutput> Database::getTransactionOutputs(const std::string& txHash) const {
    std::string txData;
    if (get(makeKey(PREFIX_TX, txHash), txData)) {
        Transaction tx = deserializeTransaction(txData);
        return tx.getOutputs();
    }
    return {};
}

// UTXO operations
bool Database::storeUTXO(const std::string& txHash, uint32_t outputIndex, const TransactionOutput& output, uint32_t blockHeight) {
    json utxo;
    utxo["tx_hash"] = txHash;
    utxo["output_index"] = outputIndex;
    utxo["address"] = output.address;
    utxo["amount"] = output.amount;
    utxo["script"] = output.script;
    utxo["block_height"] = blockHeight;
    
    std::string key = makeKey(PREFIX_UTXO, txHash + ":" + std::to_string(outputIndex));
    return put(key, utxo.dump());
}

bool Database::getUTXO(const std::string& txHash, uint32_t outputIndex, TransactionOutput& output) const {
    std::string key = makeKey(PREFIX_UTXO, txHash + ":" + std::to_string(outputIndex));
    std::string data;
    
    if (!get(key, data)) return false;
    
    try {
        json j = json::parse(data);
        output.address = j["address"].get<std::string>();
        output.amount = j["amount"].get<double>();
        output.script = j["script"].get<std::string>();
        return true;
    } catch (...) {
        return false;
    }
}

bool Database::deleteUTXO(const std::string& txHash, uint32_t outputIndex) {
    std::string key = makeKey(PREFIX_UTXO, txHash + ":" + std::to_string(outputIndex));
    return del(key);
}

std::vector<TransactionOutput> Database::getUTXOsByAddress(const std::string& address) const {
    std::vector<TransactionOutput> utxos;
    
    if (!db) return utxos;
    
    std::string prefix = makeKey(PREFIX_ADDRESS, address + ":");
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    
    for (it->Seek(prefix); it->Valid(); it->Next()) {
        std::string key = it->key().ToString();
        if (key.find(prefix) != 0) break;
        
        try {
            json j = json::parse(it->value().ToString());
            TransactionOutput output;
            output.address = j["address"].get<std::string>();
            output.amount = j["amount"].get<double>();
            output.script = j["script"].get<std::string>();
            utxos.push_back(output);
        } catch (...) {
            continue;
        }
    }
    
    return utxos;
}

double Database::getAddressBalance(const std::string& address) const {
    double balance = 0.0;
    auto utxos = getUTXOsByAddress(address);
    for (const auto& utxo : utxos) {
        balance += utxo.amount;
    }
    return balance;
}

size_t Database::getBlockCount() const {
    return getLatestBlockIndex() + 1;
}

bool Database::isConnected() const {
    return db != nullptr;
}

// Validator operations
bool Database::storeValidator(const Validator& validator) {
    std::string data = serializeValidator(validator);
    return put(makeKey(PREFIX_VALIDATOR, validator.getAddress()), data);
}

bool Database::getValidator(const std::string& address, Validator& validator) const {
    std::string data;
    if (!get(makeKey(PREFIX_VALIDATOR, address), data)) return false;
    
    validator = deserializeValidator(data);
    return true;
}

bool Database::updateValidator(const Validator& validator) {
    return storeValidator(validator);
}

bool Database::deleteValidator(const std::string& address) {
    return del(makeKey(PREFIX_VALIDATOR, address));
}

std::vector<Validator> Database::getAllValidators() const {
    std::vector<Validator> validators;
    
    if (!db) return validators;
    
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    
    for (it->Seek(PREFIX_VALIDATOR); it->Valid(); it->Next()) {
        std::string key = it->key().ToString();
        if (key.find(PREFIX_VALIDATOR) != 0) break;
        
        validators.push_back(deserializeValidator(it->value().ToString()));
    }
    
    return validators;
}

std::vector<Validator> Database::getActiveValidators() const {
    std::vector<Validator> active;
    auto all = getAllValidators();
    
    for (const auto& v : all) {
        if (v.getIsActive() && !v.getIsSlashed()) {
            active.push_back(v);
        }
    }
    
    return active;
}

// Config operations
bool Database::setConfigValue(const std::string& key, const std::string& value) {
    return put(makeKey(PREFIX_CONFIG, key), value);
}

bool Database::getConfigValue(const std::string& key, std::string& value) const {
    return get(makeKey(PREFIX_CONFIG, key), value);
}

bool Database::deleteConfigValue(const std::string& key) {
    return del(makeKey(PREFIX_CONFIG, key));
}

// Peer operations
bool Database::storePeer(const std::string& ip, uint16_t port, uint32_t lastSeen) {
    json peer;
    peer["ip"] = ip;
    peer["port"] = port;
    peer["last_seen"] = lastSeen;
    
    return put(makeKey(PREFIX_PEER, ip + ":" + std::to_string(port)), peer.dump());
}

bool Database::deletePeer(const std::string& ip, uint16_t port) {
    return del(makeKey(PREFIX_PEER, ip + ":" + std::to_string(port)));
}

std::vector<std::pair<std::string, uint16_t>> Database::getActivePeers() const {
    std::vector<std::pair<std::string, uint16_t>> peers;
    
    if (!db) return peers;
    
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    
    for (it->Seek(PREFIX_PEER); it->Valid(); it->Next()) {
        std::string key = it->key().ToString();
        if (key.find(PREFIX_PEER) != 0) break;
        
        try {
            json j = json::parse(it->value().ToString());
            peers.emplace_back(j["ip"].get<std::string>(), j["port"].get<uint16_t>());
        } catch (...) {
            continue;
        }
    }
    
    return peers;
}

// Statistics
uint64_t Database::getTotalTransactions() const {
    uint64_t count = 0;
    
    if (!db) return count;
    
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    for (it->Seek(PREFIX_TX); it->Valid(); it->Next()) {
        if (it->key().ToString().find(PREFIX_TX) != 0) break;
        count++;
    }
    
    return count;
}

uint64_t Database::getTotalBlocks() const {
    return getBlockCount();
}

double Database::getTotalSupply() const {
    std::string value;
    if (getConfigValue("total_supply", value)) {
        return std::stod(value);
    }
    return 0.0;
}

double Database::getTotalBurned() const {
    std::string value;
    if (getConfigValue("total_burned", value)) {
        return std::stod(value);
    }
    return 0.0;
}

// Maintenance
bool Database::vacuum() {
    // LevelDB handles compaction automatically
    if (db) {
        db->CompactRange(nullptr, nullptr);
        return true;
    }
    return false;
}

bool Database::backup(const std::string& backupPath) {
    // Simple backup by copying database directory
    try {
        std::filesystem::copy(dataDirectory, backupPath, 
                             std::filesystem::copy_options::recursive | 
                             std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}

bool Database::restore(const std::string& backupPath) {
    close();
    try {
        std::filesystem::remove_all(dataDirectory);
        std::filesystem::copy(backupPath, dataDirectory,
                             std::filesystem::copy_options::recursive);
        return open(dataDirectory);
    } catch (...) {
        return false;
    }
}

uint64_t Database::getDatabaseSize() const {
    uint64_t size = 0;
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dataDirectory)) {
            if (entry.is_regular_file()) {
                size += entry.file_size();
            }
        }
    } catch (...) {}
    return size;
}

bool Database::createIndexes() {
    // LevelDB is inherently indexed by key
    return true;
}

Database::DatabaseError Database::getLastErrorCode() const {
    return db ? DB_SUCCESS : DB_ERROR_INIT;
}

bool Database::isHealthy() const {
    return db != nullptr;
}

void Database::repairDatabase() {
    if (!dataDirectory.empty()) {
        leveldb::Status status = leveldb::RepairDB(dataDirectory, options);
        if (!status.ok()) {
            LOG_DATABASE(LogLevel::ERROR, "Database repair failed: " + status.ToString());
        }
    }
}

// Stub implementations for remaining methods
bool Database::storeWallet(const std::string& address, const std::string& publicKey, const std::string& encryptedPrivateKey) {
    json wallet;
    wallet["public_key"] = publicKey;
    wallet["encrypted_private_key"] = encryptedPrivateKey;
    return put(makeKey("wallet:", address), wallet.dump());
}

bool Database::getWallet(const std::string& address, std::string& publicKey, std::string& encryptedPrivateKey) const {
    std::string data;
    if (!get(makeKey("wallet:", address), data)) return false;
    try {
        json j = json::parse(data);
        publicKey = j["public_key"].get<std::string>();
        encryptedPrivateKey = j["encrypted_private_key"].get<std::string>();
        return true;
    } catch (...) {
        return false;
    }
}

bool Database::deleteWallet(const std::string& address) {
    return del(makeKey("wallet:", address));
}

std::vector<std::string> Database::getAllWalletAddresses() const {
    std::vector<std::string> addresses;
    if (!db) return addresses;
    
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(readOptions));
    for (it->Seek("wallet:"); it->Valid(); it->Next()) {
        std::string key = it->key().ToString();
        if (key.find("wallet:") != 0) break;
        addresses.push_back(key.substr(7));
    }
    return addresses;
}

bool Database::storeProposal(const std::string& proposalId, const std::string& title, const std::string& description,
                           const std::string& proposer, uint32_t blockHeight) {
    json proposal;
    proposal["title"] = title;
    proposal["description"] = description;
    proposal["proposer"] = proposer;
    proposal["block_height"] = blockHeight;
    proposal["active"] = true;
    return put(makeKey("proposal:", proposalId), proposal.dump());
}

bool Database::storeVote(const std::string& proposalId, const std::string& voter, int voteType) {
    json vote;
    vote["voter"] = voter;
    vote["type"] = voteType;
    return put(makeKey("vote:", proposalId + ":" + voter), vote.dump());
}

bool Database::getProposal(const std::string& proposalId, std::string& title, std::string& description,
                         std::string& proposer, uint32_t& blockHeight) const {
    std::string data;
    if (!get(makeKey("proposal:", proposalId), data)) return false;
    try {
        json j = json::parse(data);
        title = j["title"].get<std::string>();
        description = j["description"].get<std::string>();
        proposer = j["proposer"].get<std::string>();
        blockHeight = j["block_height"].get<uint32_t>();
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> Database::getActiveProposals() const {
    std::vector<std::string> proposals;
    return proposals; // Simplified for now
}

bool Database::storePriceData(const std::string& asset, double price, uint32_t timestamp) {
    json data;
    data["price"] = price;
    data["timestamp"] = timestamp;
    return put(makeKey("price:", asset + ":" + std::to_string(timestamp)), data.dump());
}

bool Database::getLatestPrice(const std::string& asset, double& price, uint32_t& timestamp) const {
    // Simplified implementation
    return false;
}

std::vector<std::pair<double, uint32_t>> Database::getPriceHistory(const std::string& asset, uint32_t count) const {
    return {}; // Simplified
}

bool Database::storeBridgeTransfer(const std::string& transferId, const std::string& sourceChain,
                                 const std::string& destChain, double amount, const std::string& recipient) {
    json transfer;
    transfer["source_chain"] = sourceChain;
    transfer["dest_chain"] = destChain;
    transfer["amount"] = amount;
    transfer["recipient"] = recipient;
    transfer["status"] = 0;
    return put(makeKey("bridge:", transferId), transfer.dump());
}

bool Database::updateBridgeTransferStatus(const std::string& transferId, int status) {
    std::string data;
    if (!get(makeKey("bridge:", transferId), data)) return false;
    try {
        json j = json::parse(data);
        j["status"] = status;
        return put(makeKey("bridge:", transferId), j.dump());
    } catch (...) {
        return false;
    }
}

bool Database::getBridgeTransfer(const std::string& transferId, std::string& sourceChain, std::string& destChain,
                                double& amount, std::string& recipient, int& status) const {
    std::string data;
    if (!get(makeKey("bridge:", transferId), data)) return false;
    try {
        json j = json::parse(data);
        sourceChain = j["source_chain"].get<std::string>();
        destChain = j["dest_chain"].get<std::string>();
        amount = j["amount"].get<double>();
        recipient = j["recipient"].get<std::string>();
        status = j["status"].get<int>();
        return true;
    } catch (...) {
        return false;
    }
}

bool Database::storePoolShare(const std::string& poolAddress, const std::string& minerAddress,
                            double shareValue, uint32_t timestamp) {
    json share;
    share["pool"] = poolAddress;
    share["miner"] = minerAddress;
    share["value"] = shareValue;
    share["timestamp"] = timestamp;
    return put(makeKey("share:", poolAddress + ":" + minerAddress), share.dump());
}

double Database::getPoolShares(const std::string& poolAddress, const std::string& minerAddress) const {
    std::string data;
    if (!get(makeKey("share:", poolAddress + ":" + minerAddress), data)) return 0.0;
    try {
        json j = json::parse(data);
        return j["value"].get<double>();
    } catch (...) {
        return 0.0;
    }
}

std::vector<std::pair<std::string, double>> Database::getPoolContributors(const std::string& poolAddress) const {
    return {}; // Simplified
}
