#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/HashUtils.h"
#include <algorithm>
#include <numeric>
#include <sstream>

Blockchain::Blockchain() : difficulty(1000.0), lastBlock() {
    LOG_BLOCKCHAIN(LogLevel::INFO, "Blockchain instance created");
}

Blockchain::~Blockchain() {
    shutdown();
}

bool Blockchain::initialize() {
    try {
        LOG_BLOCKCHAIN(LogLevel::INFO, "Initializing blockchain");
        
        // Create genesis block if blockchain is empty
        if (chain.empty()) {
            createGenesisBlock();
        }
        
        // Validate existing chain
        if (!isValid()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Existing blockchain is invalid");
            return false;
        }
        
        // Initialize traceability system
        if (!initializeTraceability()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to initialize traceability system");
            return false;
        }
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "Blockchain initialized successfully with " + 
                      std::to_string(chain.size()) + " blocks");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to initialize blockchain: " + std::string(e.what()));
        return false;
    }
}

void Blockchain::createGenesisBlock() {
    LOG_BLOCKCHAIN(LogLevel::INFO, "Creating genesis block");
    
    Block genesis;
    genesis.setIndex(0);
    genesis.setPreviousHash("0000000000000000000000000000000000000000000000000000000000000000");
    genesis.setTimestamp(1640995200); // 2022-01-01 00:00:00 UTC
    genesis.setDifficulty(1000.0);
    genesis.setNonce(0);
    
    // Create coinbase transaction for genesis
    Transaction coinbase;
    coinbase.setHash("genesis_coinbase_tx");
    coinbase.setCoinbaseTransaction(true);
    coinbase.setTimestamp(genesis.getTimestamp());
    
    // Add coinbase output
    TransactionOutput output;
    output.address = "genesis_address";
    output.amount = 50000000.0; // 50M GXC initial supply
    coinbase.addOutput(output);
    
    genesis.addTransaction(coinbase);
    genesis.calculateMerkleRoot();
    
    // Mine genesis block (easy difficulty)
    while (!genesis.getHash().substr(0, 4).empty() && 
           genesis.getHash().substr(0, 4) != "0000") {
        genesis.incrementNonce();
        genesis.calculateHash();
    }
    
    // Add to chain
    std::lock_guard<std::mutex> lock(chainMutex);
    chain.push_back(std::make_shared<Block>(genesis));
    lastBlock = chain.back();
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Genesis block created with hash: " + genesis.getHash());
}

bool Blockchain::addBlock(const Block& block) {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    try {
        // Validate block
        if (!validateBlock(block)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Block validation failed");
            return false;
        }
        
        // Validate traceability for all transactions in block
        if (!validateBlockTraceability(block)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Block traceability validation failed");
            return false;
        }
        
        // Add block to chain
        auto blockPtr = std::make_shared<Block>(block);
        chain.push_back(blockPtr);
        lastBlock = blockPtr;
        
        // Update difficulty
        // Difficulty adjustment handled automatically
        
        // Update transaction pool (remove confirmed transactions)
        updateTransactionPool(block);
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "Block added successfully. Height: " + 
                      std::to_string(chain.size()) + ", Hash: " + block.getHash().substr(0, 16) + "...");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to add block: " + std::string(e.what()));
        return false;
    }
}

bool Blockchain::validateBlock(const Block& block) {
    // Basic block validation
    if (block.getIndex() != getHeight()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid block index");
        return false;
    }
    
    if (!lastBlock) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "No previous block");
        return false;
    }
    
    if (block.getPreviousHash() != lastBlock->getHash()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid previous hash");
        return false;
    }
    
    // Validate proof of work
    if (!validateProofOfWork(block)) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid proof of work");
        return false;
    }
    
    // Validate transactions
    for (const auto& tx : block.getTransactions()) {
        if (!validateTransaction(tx)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid transaction in block");
            return false;
        }
    }
    
    // Validate merkle root
    if (block.getMerkleRoot() != block.calculateMerkleRoot()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid merkle root");
        return false;
    }
    
    return true;
}

bool Blockchain::validateProofOfWork(const Block& block) const {
    std::string hash = block.getHash();
    
    // Count leading zeros
    int leadingZeros = 0;
    for (char c : hash) {
        if (c == '0') {
            leadingZeros++;
        } else {
            break;
        }
    }
    
    // Calculate required zeros based on difficulty
    int requiredZeros = static_cast<int>(std::log2(block.getDifficulty())) + 2;
    
    return leadingZeros >= requiredZeros;
}

bool Blockchain::validateTransaction(const Transaction& tx) {
    // Basic transaction validation
    if (tx.getHash().empty()) {
        return false;
    }
    
    // Skip validation for coinbase transactions
    if (tx.isCoinbaseTransaction()) {
        return true;
    }
    
    // Validate traceability for non-coinbase transactions
    if (!tx.isTraceabilityValid()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction traceability validation failed: " + tx.getHash());
        return false;
    }
    
    // Validate transaction structure
    if (tx.getInputs().empty() || tx.getOutputs().empty()) {
        return false;
    }
    
    // Validate amounts
    double inputTotal = tx.getTotalInputAmount();
    double outputTotal = tx.getTotalOutputAmount();
    double fee = tx.getFee();
    
    if (inputTotal < outputTotal + fee) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction input/output amount mismatch");
        return false;
    }
    
    return true;
}

bool Blockchain::validateBlockTraceability(const Block& block) {
    for (const auto& tx : block.getTransactions()) {
        if (!tx.isCoinbaseTransaction()) {
            // Apply GXC traceability formula
            if (!validateTransactionTraceability(tx)) {
                return false;
            }
        }
    }
    return true;
}

bool Blockchain::validateTransactionTraceability(const Transaction& tx) {
    // GXC Traceability Formula:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount
    
    if (tx.isCoinbaseTransaction()) {
        return true; // Coinbase transactions don't need traceability
    }
    
    const auto& inputs = tx.getInputs();
    if (inputs.empty()) {
        return false;
    }
    
    const auto& firstInput = inputs[0];
    
    // Check hash match
    bool hashMatch = (firstInput.txHash == tx.getPrevTxHash());
    
    // Check amount match (with small epsilon for floating point comparison)
    bool amountMatch = std::abs(firstInput.amount - tx.getReferencedAmount()) < 0.00000001;
    
    if (!hashMatch || !amountMatch) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, 
                      "Traceability validation failed for transaction " + tx.getHash() +
                      ": hashMatch=" + (hashMatch ? "true" : "false") +
                      ", amountMatch=" + (amountMatch ? "true" : "false"));
        return false;
    }
    
    return true;
}

bool Blockchain::initializeTraceability() {
    LOG_BLOCKCHAIN(LogLevel::INFO, "Initializing traceability system");
    
    // Build traceability index for existing transactions
    for (const auto& block : chain) {
        for (const auto& tx : block->getTransactions()) {
            if (!tx.isCoinbaseTransaction()) {
                TraceabilityEntry entry;
                entry.txHash = tx.getHash();
                entry.prevTxHash = tx.getPrevTxHash();
                entry.amount = tx.getReferencedAmount();
                entry.timestamp = tx.getTimestamp();
                entry.fromAddress = ""; // Will be set from transaction inputs
                entry.toAddress = ""; // Will be set from transaction outputs
                traceabilityIndex[tx.getHash()] = entry;
            }
        }
    }
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Traceability index built with " + 
                  std::to_string(traceabilityIndex.size()) + " entries");
    
    return true;
}

bool Blockchain::validateTraceability() {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Validating complete blockchain traceability");
    
    for (const auto& block : chain) {
        if (!validateBlockTraceability(*block)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Traceability validation failed at block " + 
                          std::to_string(block->getIndex()));
            return false;
        }
    }
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Complete blockchain traceability validation successful");
    return true;
}

// adjustDifficulty removed - difficulty adjustment handled elsewhere

void Blockchain::updateTransactionPool(const Block& block) {
    // Remove confirmed transactions from pending pool
    for (const auto& tx : block.getTransactions()) {
        auto it = std::find_if(pendingTransactions.begin(), pendingTransactions.end(),
                              [&tx](const Transaction& poolTx) {
                                  return poolTx.getHash() == tx.getHash();
                              });
        
        if (it != pendingTransactions.end()) {
            pendingTransactions.erase(it);
        }
    }
}

bool Blockchain::addTransaction(const Transaction& tx) {
    std::lock_guard<std::mutex> lock(transactionMutex);
    
    // Validate transaction
    if (!validateTransaction(tx)) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction validation failed: " + tx.getHash());
        return false;
    }
    
    // Check if transaction already exists
    auto it = std::find_if(pendingTransactions.begin(), pendingTransactions.end(),
                          [&tx](const Transaction& poolTx) {
                              return poolTx.getHash() == tx.getHash();
                          });
    
    if (it != pendingTransactions.end()) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Transaction already in pool: " + tx.getHash());
        return false;
    }
    
    // Add to pending transactions
    pendingTransactions.push_back(tx);
    
    LOG_BLOCKCHAIN(LogLevel::DEBUG, "Transaction added to pool: " + tx.getHash());
    return true;
}

void Blockchain::processTransactions() {
    // Process pending transactions (placeholder for more complex logic)
    std::lock_guard<std::mutex> lock(transactionMutex);
    
    // Remove expired transactions
    auto currentTime = Utils::getCurrentTimestamp();
    pendingTransactions.erase(
        std::remove_if(pendingTransactions.begin(), pendingTransactions.end(),
                      [currentTime](const Transaction& tx) {
                          return currentTime - tx.getTimestamp() > 3600; // 1 hour expiry
                      }),
        pendingTransactions.end()
    );
}

std::vector<Transaction> Blockchain::getPendingTransactions(size_t maxCount) {
    std::lock_guard<std::mutex> lock(transactionMutex);
    
    std::vector<Transaction> result;
    size_t count = std::min(maxCount, pendingTransactions.size());
    
    result.reserve(count);
    for (size_t i = 0; i < count; i++) {
        result.push_back(pendingTransactions[i]);
    }
    
    return result;
}

Block Blockchain::getLatestBlock() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    if (lastBlock) {
        return *lastBlock;
    }
    
    // Return empty block if no blocks exist
    return Block();
}

Block Blockchain::getBlock(const std::string& hash) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    for (const auto& block : chain) {
        if (block->getHash() == hash) {
            return *block;
        }
    }
    
    // Return empty block if not found
    return Block();
}

Block Blockchain::getBlock(size_t index) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    if (index < chain.size()) {
        return *chain[index];
    }
    
    // Return empty block if index out of range
    return Block();
}

std::vector<Block> Blockchain::getBlocks(size_t count) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    std::vector<Block> result;
    size_t startIndex = chain.size() > count ? chain.size() - count : 0;
    
    for (size_t i = startIndex; i < chain.size(); i++) {
        result.push_back(*chain[i]);
    }
    
    return result;
}

bool Blockchain::isValid() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    if (chain.empty()) {
        return false;
    }
    
    // Validate each block and its connection to previous block
    for (size_t i = 1; i < chain.size(); i++) {
        const auto& currentBlock = *chain[i];
        const auto& previousBlock = *chain[i - 1];
        
        // Check block linkage
        if (currentBlock.getPreviousHash() != previousBlock.getHash()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Block linkage validation failed at index " + std::to_string(i));
            return false;
        }
        
        // Check block index
        if (currentBlock.getIndex() != previousBlock.getIndex() + 1) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Block index validation failed at index " + std::to_string(i));
            return false;
        }
        
        // Validate proof of work
        if (!validateProofOfWork(currentBlock)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Proof of work validation failed at index " + std::to_string(i));
            return false;
        }
    }
    
    return true;
}

size_t Blockchain::getHeight() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    return chain.size();
}

double Blockchain::getDifficulty() const {
    return difficulty;
}

std::string Blockchain::getStats() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    std::ostringstream oss;
    oss << "Blockchain Statistics:\n";
    oss << "  Height: " << chain.size() << "\n";
    oss << "  Difficulty: " << difficulty << "\n";
    oss << "  Pending Transactions: " << pendingTransactions.size() << "\n";
    oss << "  Traceability Index Size: " << traceabilityIndex.size() << "\n";
    oss << "  Last Block Hash: " << (lastBlock ? lastBlock->getHash().substr(0, 16) + "..." : "none") << "\n";
    oss << "  Traceability Formula: Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount";
    
    return oss.str();
}

void Blockchain::shutdown() {
    LOG_BLOCKCHAIN(LogLevel::INFO, "Shutting down blockchain");
    
    // Clear all data structures
    {
        std::lock_guard<std::mutex> lock(chainMutex);
        chain.clear();
        lastBlock = nullptr;
    }
    
    {
        std::lock_guard<std::mutex> lock(transactionMutex);
        pendingTransactions.clear();
    }
    
    traceabilityIndex.clear();
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Blockchain shutdown complete");
}

double Blockchain::getBalance(const std::string& address) const {
    // Query balance from UTXO set
    double balance = 0.0;
    std::lock_guard<std::mutex> lock(chainMutex);
    
    for (const auto& [txHash, output] : utxoSet) {
        if (output.address == address) {
            balance += output.amount;
        }
    }
    
    return balance;
}

double Blockchain::getGoldBalance(const std::string& address) const {
    // Query gold token balance
    auto it = goldTokens.find("GXC-G");
    if (it != goldTokens.end()) {
        // Would call GoldToken::getBalance(address) in full implementation
        return 0.0; // Placeholder
    }
    return 0.0;
}

std::vector<Transaction> Blockchain::getTransactionHistory(const std::string& address) const {
    std::vector<Transaction> history;
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // Search through all blocks for transactions involving this address
    for (const auto& blockPtr : chain) {
        if (!blockPtr) continue;
        const Block& block = *blockPtr;
        for (const auto& tx : block.getTransactions()) {
            // Check if address is involved in transaction (check outputs)
            bool involved = false;
            for (const auto& output : tx.getOutputs()) {
                if (output.address == address) {
                    involved = true;
                    break;
                }
            }
            if (involved) {
                history.push_back(tx);
            }
        }
    }
    
    return history;
}
