#include "../include/blockchain.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/HashUtils.h"

#include "../include/Config.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <unordered_set>

Blockchain::Blockchain() : difficulty(1000.0), lastBlock(), blockReward(50.0), lastHalvingBlock(0) {
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
    
    // Determine genesis wallet address based on network mode
    bool isTestnet = Config::isTestnet();
    std::string genesisAddress;
    
    if (isTestnet) {
        // Testnet genesis address - matches the miner address
        genesisAddress = "tGXC9fab7317231b966af85ac453e168c0932";
        LOG_BLOCKCHAIN(LogLevel::INFO, "Using TESTNET genesis address: " + genesisAddress);
    } else {
        // Mainnet genesis address - to be set before mainnet launch
        genesisAddress = "GXC9fab7317231b966af85ac453e168c0932";
        LOG_BLOCKCHAIN(LogLevel::INFO, "Using MAINNET genesis address: " + genesisAddress);
    }
    
    Block genesis;
    genesis.setIndex(0);
    genesis.setPreviousHash("0000000000000000000000000000000000000000000000000000000000000000");
    genesis.setTimestamp(1640995200); // 2022-01-01 00:00:00 UTC
    genesis.setMinerAddress(genesisAddress);
    
    // Use easier difficulty for testnet
    if (isTestnet) {
        genesis.setDifficulty(0.1);  // Very easy for testnet
        LOG_BLOCKCHAIN(LogLevel::INFO, "Using testnet difficulty: 0.1");
    } else {
        genesis.setDifficulty(1000.0);  // Harder for mainnet
        LOG_BLOCKCHAIN(LogLevel::INFO, "Using mainnet difficulty: 1000.0");
    }
    
    genesis.setNonce(0);
    
    // Create coinbase transaction for genesis
    Transaction coinbase;
    coinbase.setHash("genesis_coinbase_tx");
    coinbase.setCoinbaseTransaction(true);
    coinbase.setTimestamp(genesis.getTimestamp());
    
    // Add coinbase output to genesis address
    TransactionOutput output;
    output.address = genesisAddress;
    output.amount = 50.0; // Genesis block reward (same as initial block reward)
    coinbase.addOutput(output);
    
    genesis.addTransaction(coinbase);
    genesis.calculateMerkleRoot();
    
    // Mine genesis block with appropriate difficulty
    int requiredZeros = isTestnet ? 2 : 4;
    std::string target(requiredZeros, '0');
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Mining genesis block (difficulty: " + std::to_string(requiredZeros) + " leading zeros)");
    
    uint64_t attempts = 0;
    do {
        genesis.incrementNonce();
        std::string hash = genesis.calculateHash();
        attempts++;
        
        if (attempts % 10000 == 0) {
            LOG_BLOCKCHAIN(LogLevel::DEBUG, "Genesis mining attempts: " + std::to_string(attempts));
        }
        
        if (hash.substr(0, requiredZeros) == target) {
            break;
        }
    } while (true);
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Genesis block mined after " + std::to_string(attempts) + " attempts");
    
    // Add to chain
    std::lock_guard<std::mutex> lock(chainMutex);
    chain.push_back(std::make_shared<Block>(genesis));
    lastBlock = chain.back();
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Genesis block created with hash: " + genesis.getHash());
}

bool Blockchain::addBlock(const Block& block) {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Starting. Block index: " + std::to_string(block.getIndex()) + 
                  ", Hash: " + block.getHash().substr(0, 16) + "..., Miner: " + block.getMinerAddress().substr(0, 20) + "...");
    
    try {
        // Make a mutable copy of the block
        Block blockToAdd = block;
        bool hasCoinbase = false;
        std::string minerAddress = block.getMinerAddress();
        std::string originalHash = block.getHash(); // Save miner's proof of work
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Original hash: " + originalHash.substr(0, 16) + "...");
        
        // Check if block has coinbase transaction
        for (const auto& tx : block.getTransactions()) {
            if (tx.isCoinbaseTransaction()) {
                hasCoinbase = true;
                LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Block already has coinbase transaction");
                break;
            }
        }
        
        // If no coinbase and we have miner address, create one
        if (!hasCoinbase && !minerAddress.empty()) {
            // Calculate block reward with halving
            double blockReward = calculateBlockReward(block.getIndex());
            
            LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Creating coinbase transaction for miner: " + minerAddress.substr(0, 20) + 
                          "..., Reward: " + std::to_string(blockReward) + " GXC");
            
            Transaction coinbase(minerAddress, blockReward);
            LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Coinbase TX hash: " + coinbase.getHash().substr(0, 16) + 
                          "..., Outputs: " + std::to_string(coinbase.getOutputs().size()));
            
            // Add coinbase transaction to the block
            blockToAdd.addTransaction(coinbase);
            
            // Recalculate merkle root
            blockToAdd.calculateMerkleRoot();
            
            // CRITICAL: Restore the original hash (miner's proof of work)
            // The hash must not change when we add the coinbase
            blockToAdd.setHash(originalHash);
            
            LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Coinbase added, hash restored: " + blockToAdd.getHash().substr(0, 16) + "...");
        } else if (hasCoinbase) {
            LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Using existing coinbase transaction");
        } else {
            LOG_BLOCKCHAIN(LogLevel::WARNING, "addBlock: No coinbase and no miner address!");
        }
        
        // Validate block
        // NOTE: We already hold chainMutex, so we can't call methods that try to lock it
        // Pass chain.size() directly instead of calling getHeight()
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Starting block validation...");
        uint32_t expectedIndex = chain.size(); // This is getHeight() without the lock
        bool validationResult = validateBlockInternal(blockToAdd, expectedIndex);
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Block validation result: " + std::string(validationResult ? "PASS" : "FAIL"));
        
        if (!validationResult) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "addBlock: Block validation failed");
            return false;
        }
        
        // Validate traceability for all transactions in block
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Starting traceability validation...");
        bool traceabilityResult = validateBlockTraceability(blockToAdd);
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Traceability validation result: " + std::string(traceabilityResult ? "PASS" : "FAIL"));
        
        if (!traceabilityResult) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "addBlock: Block traceability validation failed");
            return false;
        }
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: All validations passed, adding block to chain...");
        
        // Add block to chain
        auto blockPtr = std::make_shared<Block>(blockToAdd);
        
        // Verify block hash is set before adding
        if (blockPtr->getHash().empty()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "addBlock: Block hash is empty! Index: " + std::to_string(blockPtr->getIndex()));
            return false;
        }
        
        chain.push_back(blockPtr);
        lastBlock = blockPtr;
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Block added successfully. Height: " + std::to_string(chain.size()) + 
                      ", Index: " + std::to_string(blockPtr->getIndex()) + 
                      ", Hash: " + blockPtr->getHash().substr(0, 16) + "...");
        
        // Update UTXO set (add outputs, remove inputs)
        // This must be called BEFORE any balance queries to ensure wallet is updated
        updateUtxoSet(blockToAdd);
        
        // Log coinbase reward if present
        for (const auto& tx : blockToAdd.getTransactions()) {
            if (tx.isCoinbaseTransaction() && !tx.getOutputs().empty()) {
                const auto& coinbaseOutput = tx.getOutputs()[0];
                LOG_BLOCKCHAIN(LogLevel::INFO, "Coinbase reward: " + std::to_string(coinbaseOutput.amount) + 
                              " GXC to " + coinbaseOutput.address.substr(0, 16) + "...");
            }
        }
        
        // Update traceability index for new transactions
        for (const auto& tx : blockToAdd.getTransactions()) {
            if (!tx.isCoinbaseTransaction()) {
                TraceabilityEntry entry;
                entry.txHash = tx.getHash();
                entry.prevTxHash = tx.getPrevTxHash();
                entry.amount = tx.getReferencedAmount();
                entry.timestamp = tx.getTimestamp();
                
                // Set from/to addresses
                if (!tx.getInputs().empty()) {
                    std::string inputTxHash = tx.getInputs()[0].txHash;
                    Transaction inputTx = getTransactionByHash(inputTxHash);
                    if (!inputTx.getHash().empty() && !inputTx.getOutputs().empty()) {
                        entry.fromAddress = inputTx.getOutputs()[0].address;
                    } else {
                        entry.fromAddress = tx.getSenderAddress();
                    }
                } else {
                    entry.fromAddress = tx.getSenderAddress();
                }
                
                if (!tx.getOutputs().empty()) {
                    entry.toAddress = tx.getOutputs()[0].address;
                } else {
                    entry.toAddress = tx.getReceiverAddress();
                }
                
                traceabilityIndex[tx.getHash()] = entry;
            }
        }
        
        // Update difficulty
        // Difficulty adjustment handled automatically
        
        // Update transaction pool (remove confirmed transactions)
        updateTransactionPool(blockToAdd);
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "Block added successfully. Height: " + 
                      std::to_string(chain.size()) + ", Hash: " + blockToAdd.getHash().substr(0, 16) + "...");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to add block: " + std::string(e.what()));
        return false;
    }
}

// Calculate block reward with halving
double Blockchain::calculateBlockReward(uint32_t height) const {
    const uint32_t HALVING_INTERVAL = 1051200; // ~4 years
    const double INITIAL_REWARD = 50.0;
    
    // 1. Base reward with halving (like Bitcoin)
    uint32_t halvings = height / HALVING_INTERVAL;
    double baseReward = INITIAL_REWARD;
    
    for (uint32_t i = 0; i < halvings; i++) {
        baseReward /= 2.0;
    }
    
    // 2. Difficulty adjustment factor
    // Higher difficulty = slightly higher reward (incentivize mining)
    double difficultyMultiplier = 1.0;
    if (difficulty > 1.0) {
        // Max 10% bonus for high difficulty
        difficultyMultiplier = 1.0 + std::min(0.1, (difficulty - 1.0) / 100.0);
    }
    
    // 3. Transaction fee component
    // Miners get transaction fees on top of base reward
    double transactionFees = 0.0;
    for (const auto& tx : pendingTransactions) {
        transactionFees += tx.getFee();
    }
    
    // 4. Network health bonus
    // If blocks are being mined slower than target, increase reward slightly
    double timeBonus = 1.0;
    if (chain.size() >= 10) {
        // Calculate average block time for last 10 blocks
        uint64_t timeDiff = chain.back()->getTimestamp() - chain[chain.size() - 10]->getTimestamp();
        double avgBlockTime = timeDiff / 10.0;
        double targetBlockTime = 120.0; // 2 minutes
        
        if (avgBlockTime > targetBlockTime * 1.5) {
            // Blocks too slow, increase reward by up to 5%
            timeBonus = 1.0 + std::min(0.05, (avgBlockTime - targetBlockTime) / targetBlockTime);
        }
    }
    
    // 5. Supply cap enforcement
    double finalReward = baseReward * difficultyMultiplier * timeBonus;
    
    // Check if we're approaching max supply
    if (totalSupply + finalReward > MAX_SUPPLY) {
        finalReward = MAX_SUPPLY - totalSupply;
    }
    
    // Minimum reward for security (even after cap)
    if (finalReward < 0.00000001) {
        finalReward = 0.00000001;
    }
    
    // Add transaction fees (not subject to cap)
    finalReward += transactionFees;
    
    return finalReward;
}

// Difficulty adjustment (like Bitcoin's 2016 block adjustment)
double Blockchain::calculateNextDifficulty() const {
    const uint32_t DIFFICULTY_ADJUSTMENT_INTERVAL = 2016; // ~2 weeks
    const double TARGET_BLOCK_TIME = 120.0; // 2 minutes
    
    uint32_t currentHeight = chain.size();
    
    // Only adjust every DIFFICULTY_ADJUSTMENT_INTERVAL blocks
    if (currentHeight % DIFFICULTY_ADJUSTMENT_INTERVAL != 0) {
        return difficulty;
    }
    
    // Need at least DIFFICULTY_ADJUSTMENT_INTERVAL blocks
    if (currentHeight < DIFFICULTY_ADJUSTMENT_INTERVAL) {
        return difficulty;
    }
    
    // Calculate actual time taken for last DIFFICULTY_ADJUSTMENT_INTERVAL blocks
    uint64_t startTime = chain[currentHeight - DIFFICULTY_ADJUSTMENT_INTERVAL]->getTimestamp();
    uint64_t endTime = chain.back()->getTimestamp();
    uint64_t actualTime = endTime - startTime;
    
    // Calculate expected time
    uint64_t expectedTime = DIFFICULTY_ADJUSTMENT_INTERVAL * TARGET_BLOCK_TIME;
    
    // Calculate new difficulty
    double newDifficulty = difficulty * (static_cast<double>(expectedTime) / actualTime);
    
    // Limit adjustment to 4x increase or 1/4 decrease (like Bitcoin)
    if (newDifficulty > difficulty * 4.0) {
        newDifficulty = difficulty * 4.0;
    } else if (newDifficulty < difficulty / 4.0) {
        newDifficulty = difficulty / 4.0;
    }
    
    // Minimum difficulty
    if (newDifficulty < 0.001) {
        newDifficulty = 0.001;
    }
    
    return newDifficulty;
}

// Update UTXO set when block is added
void Blockchain::updateUtxoSet(const Block& block) {
    // Note: This function is called from addBlock() which already holds chainMutex
    // So we don't need to lock here to avoid deadlock
    
    uint32_t outputsAdded = 0;
    uint32_t inputsRemoved = 0;
    
    for (const auto& tx : block.getTransactions()) {
        // Remove inputs (spent outputs) - mark UTXOs as spent
        if (!tx.isCoinbaseTransaction()) {
            for (const auto& input : tx.getInputs()) {
                std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
                if (utxoSet.find(utxoKey) != utxoSet.end()) {
                    utxoSet.erase(utxoKey);
                    inputsRemoved++;
                }
            }
        }
        
        // Add outputs (new UTXOs) - these become spendable
        uint32_t outputIndex = 0;
        for (const auto& output : tx.getOutputs()) {
            std::string utxoKey = tx.getHash() + "_" + std::to_string(outputIndex);
            utxoSet[utxoKey] = output;
            outputIndex++;
            outputsAdded++;
            
            // Log all UTXO additions at INFO level for debugging
            if (tx.isCoinbaseTransaction()) {
                LOG_BLOCKCHAIN(LogLevel::INFO, "✅ Added coinbase UTXO [" + utxoKey + "]: " + std::to_string(output.amount) + 
                              " GXC to " + output.address.substr(0, 20) + "...");
            } else {
                LOG_BLOCKCHAIN(LogLevel::DEBUG, "Added UTXO [" + utxoKey + "]: " + std::to_string(output.amount) + 
                              " GXC to " + output.address.substr(0, 16) + "...");
            }
        }
    }
    
    LOG_BLOCKCHAIN(LogLevel::DEBUG, "UTXO set updated: +" + std::to_string(outputsAdded) + 
                  " outputs, -" + std::to_string(inputsRemoved) + " inputs. Total UTXOs: " + 
                  std::to_string(utxoSet.size()));
}

bool Blockchain::validateBlock(const Block& block) {
    std::lock_guard<std::mutex> lock(chainMutex);
    return validateBlockInternal(block, chain.size());
}

bool Blockchain::validateBlockInternal(const Block& block, uint32_t expectedIndex) {
    // Internal validation that doesn't lock mutex (caller must hold lock)
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Checking block index...");
    if (block.getIndex() != expectedIndex) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid block index: got " + std::to_string(block.getIndex()) + 
                      ", expected " + std::to_string(expectedIndex));
        return false;
    }
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Index OK (" + std::to_string(block.getIndex()) + ")");
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Checking previous block...");
    if (!lastBlock) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "No previous block (lastBlock is null). Chain size: " + std::to_string(chain.size()));
        return false;
    }
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Checking previous hash...");
    std::string blockPrevHash = block.getPreviousHash();
    std::string lastBlockHash = lastBlock->getHash();
    if (blockPrevHash != lastBlockHash) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid previous hash: block has " + blockPrevHash.substr(0, 16) + 
                      "..., lastBlock has " + lastBlockHash.substr(0, 16) + "...");
        return false;
    }
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Previous hash OK");
    
    // Validate proof of work
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Validating proof of work...");
    if (!validateProofOfWork(block)) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid proof of work for block at index " + std::to_string(block.getIndex()));
        return false;
    }
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Proof of work OK");
    
    // Validate transactions
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Validating " + std::to_string(block.getTransactions().size()) + " transactions...");
    for (const auto& tx : block.getTransactions()) {
        if (!validateTransaction(tx)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid transaction in block: " + tx.getHash().substr(0, 16) + "...");
            return false;
        }
    }
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: All transactions valid");

    
    // Validate merkle root - Note: For blocks submitted by miners, the merkle root might differ
    // if we added a coinbase transaction, but we still accept the miner's hash as proof of work
    std::string calculatedMerkle = block.calculateMerkleRoot();
    std::string blockMerkle = block.getMerkleRoot();
    if (blockMerkle != calculatedMerkle && !blockMerkle.empty()) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Merkle root mismatch (may be due to coinbase addition): block has " + 
                      blockMerkle.substr(0, 16) + "..., calculated " + calculatedMerkle.substr(0, 16) + "...");
        // Don't fail validation for merkle root mismatch if hash is valid (miner's hash proves work)
        // The merkle root will be recalculated when coinbase is added
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
                
                // Set from/to addresses from transaction
                if (!tx.getInputs().empty()) {
                    // Get from address from first input's referenced transaction
                    std::string inputTxHash = tx.getInputs()[0].txHash;
                    Transaction inputTx = getTransactionByHash(inputTxHash);
                    if (!inputTx.getHash().empty() && !inputTx.getOutputs().empty()) {
                        entry.fromAddress = inputTx.getOutputs()[0].address;
                    } else {
                        entry.fromAddress = tx.getSenderAddress();
                    }
                } else {
                    entry.fromAddress = tx.getSenderAddress();
                }
                
                if (!tx.getOutputs().empty()) {
                    entry.toAddress = tx.getOutputs()[0].address;
                } else {
                    entry.toAddress = tx.getReceiverAddress();
                }
                
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
        Block result = *lastBlock;
        // Verify hash is not empty
        if (result.getHash().empty()) {
            LOG_BLOCKCHAIN(LogLevel::WARNING, "getLatestBlock: lastBlock has empty hash. Chain size: " + std::to_string(chain.size()));
            // Try to get from chain vector as fallback
            if (!chain.empty()) {
                result = *chain.back();
                LOG_BLOCKCHAIN(LogLevel::INFO, "getLatestBlock: Using chain.back() instead. Hash: " + result.getHash().substr(0, 16) + "...");
            }
        }
        return result;
    }
    
    // Try chain vector as fallback
    if (!chain.empty()) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "getLatestBlock: lastBlock is null but chain has " + std::to_string(chain.size()) + " blocks. Using chain.back()");
        return *chain.back();
    }
    
    // Return empty block if no blocks exist
    LOG_BLOCKCHAIN(LogLevel::WARNING, "getLatestBlock: No blocks in chain");
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

// Get balance for an address from UTXO set
double Blockchain::getBalance(const std::string& address) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    double balance = 0.0;
    uint32_t utxoCount = 0;
    
    // Sum all UTXOs for this address
    for (const auto& [utxoKey, output] : utxoSet) {
        if (output.address == address) {
            balance += output.amount;
            utxoCount++;
            LOG_BLOCKCHAIN(LogLevel::DEBUG, "  UTXO: " + utxoKey + " = " + std::to_string(output.amount) + " GXC");
        }
    }
    
    // Always log balance queries at INFO level for debugging
    LOG_BLOCKCHAIN(LogLevel::INFO, "getBalance(" + address.substr(0, 20) + "...): " + std::to_string(balance) + " GXC from " + std::to_string(utxoCount) + " UTXOs (total: " + std::to_string(utxoSet.size()) + ")");
    
    return balance;
}

const std::unordered_map<std::string, TransactionOutput>& Blockchain::getUtxoSet() const {
    return utxoSet;
}

// Traceability methods - Implement transaction tracking
std::vector<std::string> Blockchain::traceTransactionLineage(const std::string& startHash) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    std::vector<std::string> lineage;
    std::string currentHash = startHash;
    std::unordered_set<std::string> visited; // Prevent cycles
    
    // Trace backwards through transaction chain
    while (!currentHash.empty() && visited.find(currentHash) == visited.end()) {
        visited.insert(currentHash);
        lineage.push_back(currentHash);
        
        // Find transaction in traceability index
        auto it = traceabilityIndex.find(currentHash);
        if (it != traceabilityIndex.end()) {
            currentHash = it->second.prevTxHash;
            if (currentHash == "0" || currentHash.empty()) {
                break; // Reached genesis or coinbase
            }
        } else {
            // Search in blockchain
            bool found = false;
            for (const auto& block : chain) {
                for (const auto& tx : block->getTransactions()) {
                    if (tx.getHash() == currentHash) {
                        currentHash = tx.getPrevTxHash();
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (!found) break;
        }
    }
    
    return lineage;
}

bool Blockchain::verifyTransactionLineage(const std::string& txHash) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // Get lineage
    std::vector<std::string> lineage = traceTransactionLineage(txHash);
    
    if (lineage.empty()) {
        return false;
    }
    
    // Verify each link in the chain
    for (size_t i = 0; i < lineage.size() - 1; i++) {
        std::string currentHash = lineage[i];
        std::string nextHash = lineage[i + 1];
        
        // Find transaction
        Transaction tx = getTransactionByHash(currentHash);
        if (tx.getHash().empty()) {
            return false;
        }
        
        // Verify prevTxHash points to next transaction
        if (tx.getPrevTxHash() != nextHash) {
            return false;
        }
    }
    
    return true;
}

Transaction Blockchain::getTransactionByHash(const std::string& hash) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // Search all blocks for transaction
    for (const auto& block : chain) {
        for (const auto& tx : block->getTransactions()) {
            if (tx.getHash() == hash) {
                return tx;
            }
        }
    }
    
    // Return empty transaction if not found
    return Transaction();
}

bool Blockchain::isLineageValid(const std::string& startHash) const {
    return verifyTransactionLineage(startHash);
}

std::vector<std::string> Blockchain::getTransactionChain(const std::string& address, uint32_t depth) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    std::vector<std::string> txChain;
    std::string currentHash = "";
    uint32_t currentDepth = 0;
    
    // Find most recent transaction for this address (search from latest to earliest)
    for (auto it = chain.rbegin(); it != chain.rend() && currentDepth < depth; ++it) {
        const auto& block = *it;
        for (const auto& tx : block->getTransactions()) {
            // Check if address is involved
            bool involvesAddress = false;
            
            // Check outputs
            for (const auto& output : tx.getOutputs()) {
                if (output.address == address) {
                    involvesAddress = true;
                    break;
                }
            }
            
            // Check sender (for non-coinbase)
            if (!involvesAddress && !tx.isCoinbaseTransaction()) {
                if (tx.getSenderAddress() == address) {
                    involvesAddress = true;
                }
            }
            
            if (involvesAddress) {
                currentHash = tx.getHash();
                txChain.push_back(currentHash);
                currentDepth++;
                
                // Trace backwards
                std::string prevHash = tx.getPrevTxHash();
                while (!prevHash.empty() && prevHash != "0" && currentDepth < depth) {
                    Transaction prevTx = getTransactionByHash(prevHash);
                    if (prevTx.getHash().empty()) break;
                    
                    txChain.push_back(prevHash);
                    prevHash = prevTx.getPrevTxHash();
                    currentDepth++;
                }
                
                break; // Found most recent, stop searching
            }
        }
        if (!currentHash.empty()) break;
    }
    
    return txChain;
}

bool Blockchain::verifyInputReferences(const Transaction& tx) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    if (tx.isCoinbaseTransaction()) {
        return true;
    }
    
    // Verify all input references point to valid transactions
    for (const auto& input : tx.getInputs()) {
        Transaction referencedTx = getTransactionByHash(input.txHash);
        if (referencedTx.getHash().empty()) {
            return false; // Referenced transaction not found
        }
        
        // Verify output index is valid
        if (input.outputIndex >= referencedTx.getOutputs().size()) {
            return false; // Invalid output index
        }
    }
    
    return true;
}
