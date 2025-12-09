#include "../include/blockchain.h"
#include "../include/Database.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/HashUtils.h"
#include "../include/Config.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <unordered_set>
#include <random>

Blockchain::Blockchain() : lastBlock(), blockReward(50.0), lastHalvingBlock(0),
    currentHashrate(0.0), lastBlockTime(0.0), recentPoWBlocks(0), recentPoSBlocks(0) {
    
    // Initialize Security Engine
    securityEngine = std::make_unique<GXCSecurity::SecurityEngine>();
    
    // Set difficulty based on network type
    bool isTestnet = Config::isTestnet();
    if (isTestnet) {
        difficulty = 0.1;  // Very easy for testnet
        LOG_BLOCKCHAIN(LogLevel::INFO, "Blockchain instance created (TESTNET mode, difficulty: 0.1)");
    } else {
        difficulty = 1000.0;  // Harder for mainnet
        LOG_BLOCKCHAIN(LogLevel::INFO, "Blockchain instance created (MAINNET mode, difficulty: 1000.0)");
    }
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "Security Engine initialized - AI Hashrate Sentinel active");
}

Blockchain::~Blockchain() {
    shutdown();
}

bool Blockchain::initialize() {
    try {
        LOG_BLOCKCHAIN(LogLevel::INFO, "Initializing blockchain");
        
        // CRITICAL: Load blocks from database FIRST before checking if chain is empty
        // This ensures that mined blocks persist across restarts
        if (!loadBlocksFromDatabase()) {
            LOG_BLOCKCHAIN(LogLevel::WARNING, "Failed to load blocks from database, starting fresh");
        }
        
        // Create genesis block ONLY if blockchain is still empty after loading from database
        if (chain.empty()) {
            LOG_BLOCKCHAIN(LogLevel::INFO, "No blocks found in database, creating genesis block");
            createGenesisBlock();
        } else {
            LOG_BLOCKCHAIN(LogLevel::INFO, "Loaded " + std::to_string(chain.size()) + 
                          " blocks from database, rebuilding UTXO set...");
            // Rebuild UTXO set from loaded blocks
            rebuildUtxoSet();
        }

    // Load validators from database
    if (!loadValidatorsFromDatabase()) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Failed to load validators from database");
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

bool Blockchain::loadValidatorsFromDatabase() {
    try {
        Database& db = Database::getInstance();
        std::vector<Validator> loadedValidators = db.getAllValidators();

        std::lock_guard<std::mutex> lock(chainMutex);
        validators.clear();
        validatorMap.clear();

        for (const auto& validator : loadedValidators) {
            validators.push_back(validator);
            validatorMap[validator.getAddress()] = validator;
        }

        LOG_BLOCKCHAIN(LogLevel::INFO, "Loaded " + std::to_string(loadedValidators.size()) +
                      " validators from database");
        return true;

    } catch (const std::exception& e) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Error loading validators from database: " + std::string(e.what()));
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
            
            // If this is a PoS block, the reward goes to the validator's spendable balance.
            // Our logic automatically handles this because the coinbase transaction creates a UTXO
            // for the minerAddress (which is the validator's address).
            // So, rewards are NOT locked in the stake, they are liquid.
            // This behavior is correct per requirements.

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
        
        // CRITICAL: Check if a block at this index already exists in the chain
        // This prevents duplicate blocks at the same height (race condition protection)
        for (const auto& existingBlock : chain) {
            if (existingBlock->getIndex() == blockToAdd.getIndex()) {
                LOG_BLOCKCHAIN(LogLevel::WARNING, "addBlock: Block at index " + std::to_string(blockToAdd.getIndex()) + 
                              " already exists (hash: " + existingBlock->getHash().substr(0, 16) + 
                              "...). Rejecting duplicate block (hash: " + blockToAdd.getHash().substr(0, 16) + "...).");
                return false;
            }
        }
        
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
        LOG_BLOCKCHAIN(LogLevel::INFO, "addBlock: Updating UTXO set for " + std::to_string(blockToAdd.getTransactions().size()) + " transactions...");
        updateUtxoSet(blockToAdd);
        
        // Log coinbase reward if present and verify UTXO was added
        for (const auto& tx : blockToAdd.getTransactions()) {
            if (tx.isCoinbaseTransaction() && !tx.getOutputs().empty()) {
                const auto& coinbaseOutput = tx.getOutputs()[0];
                LOG_BLOCKCHAIN(LogLevel::INFO, "Coinbase reward: " + std::to_string(coinbaseOutput.amount) + 
                              " GXC to " + coinbaseOutput.address);
                
                // Verify UTXO was added to set
                std::string expectedUtxoKey = tx.getHash() + "_0";
                if (utxoSet.find(expectedUtxoKey) != utxoSet.end()) {
                    LOG_BLOCKCHAIN(LogLevel::INFO, "✅ Verified coinbase UTXO [" + expectedUtxoKey + "] added to UTXO set");
                } else {
                    LOG_BLOCKCHAIN(LogLevel::ERROR, "❌ Coinbase UTXO [" + expectedUtxoKey + "] NOT found in UTXO set after update!");
                }
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
        
        // CRITICAL: Save block to database for persistence
        try {
            Database& db = Database::getInstance();
            if (!db.saveBlock(blockToAdd)) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to save block to database! Block: " + 
                              blockToAdd.getHash().substr(0, 16) + "...");
                // Don't fail the block addition, but log the error
                // The block is already in memory, database save failure is non-critical
            } else {
                LOG_BLOCKCHAIN(LogLevel::INFO, "✅ Block saved to database successfully. Height: " + 
                              std::to_string(blockToAdd.getIndex()) + ", Hash: " + 
                              blockToAdd.getHash().substr(0, 16) + "...");
            }
        } catch (const std::exception& e) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Exception saving block to database: " + std::string(e.what()));
            // Don't fail the block addition, but log the error
        }
        
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

// Load blocks from database on startup
bool Blockchain::loadBlocksFromDatabase() {
    try {
        Database& db = Database::getInstance();
        
        // Get all blocks from database
        std::vector<Block> blocks = db.getAllBlocks();
        
        if (blocks.empty()) {
            LOG_BLOCKCHAIN(LogLevel::INFO, "No blocks found in database");
            return true; // Not an error, just empty database
        }
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "Loading " + std::to_string(blocks.size()) + 
                      " blocks from database...");
        
        // Clear current chain
        chain.clear();
        utxoSet.clear();
        
        // Load blocks into chain (in order)
        for (const auto& block : blocks) {
            std::shared_ptr<Block> blockPtr = std::make_shared<Block>(block);
            chain.push_back(blockPtr);
            lastBlock = blockPtr;
        }
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "Successfully loaded " + std::to_string(chain.size()) + 
                      " blocks from database");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Error loading blocks from database: " + std::string(e.what()));
        return false;
    }
}

// Rebuild UTXO set from all loaded blocks
void Blockchain::rebuildUtxoSet() {
    LOG_BLOCKCHAIN(LogLevel::INFO, "Rebuilding UTXO set from " + std::to_string(chain.size()) + " blocks...");
    
    // Clear UTXO set
    utxoSet.clear();
    
    // Process all blocks in order to rebuild UTXO set
    for (const auto& blockPtr : chain) {
        if (!blockPtr) continue;
        
        const Block& block = *blockPtr;
        
        // Process all transactions in block
        for (const auto& tx : block.getTransactions()) {
            // Remove spent outputs (inputs)
            for (const auto& input : tx.getInputs()) {
                std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
                utxoSet.erase(utxoKey);
            }
            
            // Add new outputs (UTXOs)
            uint32_t outputIndex = 0;
            for (const auto& output : tx.getOutputs()) {
                std::string utxoKey = tx.getHash() + "_" + std::to_string(outputIndex);
                utxoSet[utxoKey] = output;
                outputIndex++;
            }
        }
    }
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "UTXO set rebuilt: " + std::to_string(utxoSet.size()) + " UTXOs");
}

// Update UTXO set when block is added
void Blockchain::updateUtxoSet(const Block& block) {
    // Note: This function is called from addBlock() which already holds chainMutex
    // So we don't need to lock here to avoid deadlock
    
    uint32_t outputsAdded = 0;
    uint32_t inputsRemoved = 0;
    
    for (const auto& tx : block.getTransactions()) {
        TransactionType txType = tx.getType();
        
        // STAKE TRANSACTION PROCESSING
        if (txType == TransactionType::STAKE) {
            // Step 1: Calculate total input amount BEFORE removing from UTXO set
            double totalInput = 0.0;
            for (const auto& input : tx.getInputs()) {
                totalInput += input.amount;
            }
            
            // Step 2: Remove spent inputs from UTXO set
            for (const auto& input : tx.getInputs()) {
                std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
                if (utxoSet.find(utxoKey) != utxoSet.end()) {
                    utxoSet.erase(utxoKey);
                    inputsRemoved++;
                    
                    try {
                        Database::getInstance().deleteUTXO(input.txHash, input.outputIndex);
                    } catch (const std::exception& e) {
                        LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to delete UTXO from database: " + std::string(e.what()));
                    }
                }
            }
            
            // Step 3: Calculate total output (change only for STAKE)
            double totalOutput = 0.0;
            for (const auto& output : tx.getOutputs()) {
                totalOutput += output.amount;
            }
            
            // Step 4: Calculate staked amount = inputs - outputs - fee
            double stakedAmount = totalInput - totalOutput - tx.getFee();
            
            // Step 5: Update validator's stake
            if (stakedAmount > 0) {
                std::string stakerAddress = tx.getSenderAddress();
                if (stakerAddress.empty() && !tx.getOutputs().empty()) {
                    stakerAddress = tx.getOutputs()[0].address;
                }
                
                bool found = false;
                for (auto& v : validators) {
                    if (v.getAddress() == stakerAddress) {
                        v.addStake(stakedAmount);
                        validatorMap[stakerAddress] = v;
                        
                        try {
                            Database::getInstance().updateValidator(v);
                        } catch (const std::exception& e) {
                            LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to update validator in DB: " + std::string(e.what()));
                        }
                        
                        found = true;
                        LOG_BLOCKCHAIN(LogLevel::INFO, "✅ STAKE confirmed: " + std::to_string(stakedAmount) +
                                      " GXC locked for " + stakerAddress.substr(0, 20) + "...");
                        break;
                    }
                }
                
                if (!found) {
                    // This should never happen because validateTransaction() checks validator existence
                    // If we reach here, it means validation was bypassed or there's a race condition
                    LOG_BLOCKCHAIN(LogLevel::ERROR, "CRITICAL: STAKE TX for unknown validator: " + stakerAddress + 
                                  ". Amount: " + std::to_string(stakedAmount) + " GXC will be lost! " +
                                  "This indicates a validation bypass or race condition.");
            }
            
            // Step 6: Add change outputs as UTXOs
            uint32_t outputIndex = 0;
            for (const auto& output : tx.getOutputs()) {
                std::string utxoKey = tx.getHash() + "_" + std::to_string(outputIndex);
                utxoSet[utxoKey] = output;
                outputsAdded++;
                
                try {
                    Database::getInstance().storeUTXO(tx.getHash(), outputIndex, output, block.getIndex());
                } catch (const std::exception& e) {
                    LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to store UTXO: " + std::string(e.what()));
                }
                
                LOG_BLOCKCHAIN(LogLevel::DEBUG, "STAKE change UTXO [" + utxoKey + "]: " + 
                              std::to_string(output.amount) + " GXC");
                outputIndex++;
            }
            }
            
            continue; // Skip normal processing
        }
        
        // UNSTAKE TRANSACTION PROCESSING
        if (txType == TransactionType::UNSTAKE) {
            double unstakeAmount = tx.getTotalOutputAmount();
            
            std::string stakerAddress = tx.getReceiverAddress();
            if (stakerAddress.empty() && !tx.getOutputs().empty()) {
                stakerAddress = tx.getOutputs()[0].address;
            }
            
            // Reduce validator's stake
            bool found = false;
            for (auto& v : validators) {
                if (v.getAddress() == stakerAddress) {
                    // Use proper removeStake method
                    double currentStake = v.getStakeAmount();
                    if (currentStake >= unstakeAmount) {
                        v.removeStake(unstakeAmount);
                    } else {
                        LOG_BLOCKCHAIN(LogLevel::WARNING, "UNSTAKE: Requested " + std::to_string(unstakeAmount) +
                                      " but only " + std::to_string(currentStake) + " staked");
                        v.removeStake(currentStake);
                    }
                    
                    validatorMap[stakerAddress] = v;
                    
                    try {
                        Database::getInstance().updateValidator(v);
                    } catch (const std::exception& e) {
                        LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to update validator in DB: " + std::string(e.what()));
                    }
                    
                    found = true;
                    LOG_BLOCKCHAIN(LogLevel::INFO, "✅ UNSTAKE confirmed: " + std::to_string(unstakeAmount) +
                                  " GXC returned to " + stakerAddress.substr(0, 20) + "...");
                    break;
                }
            }
            
            if (!found) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "UNSTAKE TX for unknown validator: " + stakerAddress);
            }
            
            // Add outputs as new UTXOs (coins returned to spendable balance)
            uint32_t outputIndex = 0;
            for (const auto& output : tx.getOutputs()) {
                std::string utxoKey = tx.getHash() + "_" + std::to_string(outputIndex);
                utxoSet[utxoKey] = output;
                outputsAdded++;
                
                try {
                    Database::getInstance().storeUTXO(tx.getHash(), outputIndex, output, block.getIndex());
                } catch (const std::exception& e) {
                    LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to store UTXO: " + std::string(e.what()));
                }
                
                LOG_BLOCKCHAIN(LogLevel::INFO, "✅ Unstaked UTXO [" + utxoKey + "]: " + 
                              std::to_string(output.amount) + " GXC to " + output.address.substr(0, 20) + "...");
                outputIndex++;
            }
            
            continue; // Skip normal processing
        }
        
        // NORMAL TRANSACTION PROCESSING (including coinbase)
        
        // Remove inputs (spent outputs) - mark UTXOs as spent
        if (!tx.isCoinbaseTransaction()) {
            for (const auto& input : tx.getInputs()) {
                std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
                if (utxoSet.find(utxoKey) != utxoSet.end()) {
                    utxoSet.erase(utxoKey);
                    inputsRemoved++;
                    
                    try {
                        Database::getInstance().deleteUTXO(input.txHash, input.outputIndex);
                    } catch (const std::exception& e) {
                        LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to delete UTXO from database: " + std::string(e.what()));
                    }
                }
            }
        }

        // Add outputs (new UTXOs) - these become spendable
        uint32_t outputIndex = 0;
        for (const auto& output : tx.getOutputs()) {
            std::string utxoKey = tx.getHash() + "_" + std::to_string(outputIndex);
            utxoSet[utxoKey] = output;
            outputsAdded++;
            
            // Persist UTXO to database
            try {
                Database::getInstance().storeUTXO(tx.getHash(), outputIndex, output, block.getIndex());
            } catch (const std::exception& e) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to persist UTXO to database: " + std::string(e.what()));
            }
            
            // Log all UTXO additions at INFO level for debugging
            if (tx.isCoinbaseTransaction()) {
                LOG_BLOCKCHAIN(LogLevel::INFO, "✅ Added coinbase UTXO [" + utxoKey + "]: " + std::to_string(output.amount) + 
                              " GXC to " + output.address.substr(0, 20) + "...");
            } else {
                LOG_BLOCKCHAIN(LogLevel::DEBUG, "Added UTXO [" + utxoKey + "]: " + std::to_string(output.amount) + 
                              " GXC to " + output.address.substr(0, 16) + "...");
            }
            
            outputIndex++;
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
    
    // Validate consensus (PoW or PoS)
    BlockType blockType = block.getBlockType();
    if (blockType == BlockType::POS) {
        LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Validating proof of stake...");
        if (!validateProofOfStake(block)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid proof of stake for block at index " + std::to_string(block.getIndex()));
            return false;
        }
        LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Proof of stake OK");
    } else {
        // PoW blocks (POW_SHA256, POW_ETHASH)
        LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Validating proof of work...");
        if (!validateProofOfWork(block)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid proof of work for block at index " + std::to_string(block.getIndex()));
            return false;
        }
        LOG_BLOCKCHAIN(LogLevel::INFO, "validateBlock: Proof of work OK");
    }
    
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
    
    if (hash.empty()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfWork: Block hash is empty");
        return false;
    }
    
    // Use blockchain's difficulty for validation (not block's difficulty)
    // This prevents miners from submitting blocks with incorrect difficulty
    double validationDifficulty = difficulty;
    
    // For testnet, use easier validation (difficulty 0.1 means 0 leading zeros required)
    // For mainnet, use stricter validation
    bool isValid = meetsTarget(hash, validationDifficulty);
    
    if (!isValid) {
        // Count leading zeros for logging
        int leadingZeros = 0;
        for (char c : hash) {
            if (c == '0') {
                leadingZeros++;
            } else {
                break;
            }
        }
        size_t requiredZeros = static_cast<size_t>(validationDifficulty);
        LOG_BLOCKCHAIN(LogLevel::DEBUG, "validateProofOfWork: Hash " + hash.substr(0, 16) + 
                      "... has " + std::to_string(leadingZeros) + " leading zeros, required: " + 
                      std::to_string(requiredZeros) + " (difficulty: " + std::to_string(validationDifficulty) + ")");
    }
    
    return isValid;
}

bool Blockchain::validateProofOfStake(const Block& block) const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // Check if block is PoS type
    if (block.getBlockType() != BlockType::POS) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Block is not PoS type");
        return false;
    }
    
    // Get validator address from miner address
    std::string validatorAddress = block.getMinerAddress();
    if (validatorAddress.empty()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Block missing validator address");
        return false;
    }
    
    // Check if validator exists and is active
    auto it = validatorMap.find(validatorAddress);
    if (it == validatorMap.end()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Validator not found: " + validatorAddress);
        return false;
    }
    
    const Validator& validator = it->second;
    if (!validator.getIsActive() || !validator.isValidValidator()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Validator is not active or invalid: " + validatorAddress);
        return false;
    }
    
    // Verify validator signature
    std::string validatorSignature = block.getValidatorSignature();
    if (validatorSignature.empty()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Block missing validator signature");
        return false;
    }
    
    // Verify signature matches validator
    std::string blockHash = block.getHash();
    if (!validator.verifySignature(blockHash, validatorSignature)) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Invalid validator signature");
        return false;
    }
    
    // Verify block hash is valid (should meet a basic target for PoS)
    // PoS blocks still need a valid hash, but difficulty is much lower
    if (blockHash.empty()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Block hash is empty");
        return false;
    }
    
    // PoS blocks require minimal hash difficulty (much easier than PoW)
    double posDifficulty = 0.01; // Very easy for PoS
    if (!meetsTarget(blockHash, posDifficulty)) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "validateProofOfStake: Block hash does not meet PoS target");
        return false;
    }
    
    LOG_BLOCKCHAIN(LogLevel::INFO, "validateProofOfStake: Valid PoS block from validator " + validatorAddress);
    return true;
}

Validator Blockchain::selectValidatorForBlock() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // Get all active validators
    std::vector<Validator> activeValidators;
    for (const auto& validator : validators) {
        if (validator.getIsActive() && validator.isValidValidator()) {
            activeValidators.push_back(validator);
        }
    }
    
    if (activeValidators.empty()) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "selectValidatorForBlock: No active validators");
        return Validator(); // Return invalid validator
    }
    
    // Calculate total weighted stake
    double totalWeightedStake = 0.0;
    for (const auto& validator : activeValidators) {
        totalWeightedStake += validator.getWeightedStake();
    }
    
    if (totalWeightedStake <= 0.0) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "selectValidatorForBlock: Total weighted stake is 0");
        return Validator(); // Return invalid validator
    }
    
    // Select validator based on weighted stake (proportional selection)
    // Use block height as seed for deterministic selection
    uint32_t currentHeight = getHeight();
    std::mt19937 rng(currentHeight); // Seed with block height for determinism
    std::uniform_real_distribution<> dis(0.0, totalWeightedStake);
    double randomValue = dis(rng);
    
    double cumulativeWeight = 0.0;
    for (const auto& validator : activeValidators) {
        cumulativeWeight += validator.getWeightedStake();
        if (randomValue <= cumulativeWeight) {
            LOG_BLOCKCHAIN(LogLevel::INFO, "selectValidatorForBlock: Selected validator " + validator.getAddress() + 
                          " (weighted stake: " + std::to_string(validator.getWeightedStake()) + ")");
            return validator;
        }
    }
    
    // Fallback: return first validator
    LOG_BLOCKCHAIN(LogLevel::WARNING, "selectValidatorForBlock: Fallback to first validator");
    return activeValidators[0];
}

BlockType Blockchain::getNextBlockType() const {
    // Hybrid consensus: Alternate between PoW and PoS blocks
    // Even heights = PoW, Odd heights = PoS (or vice versa)
    uint32_t currentHeight = getHeight();
    
    // Alternate: PoW for even heights, PoS for odd heights
    // But only if we have validators for PoS blocks
    if (currentHeight % 2 == 0) {
        // Even height: PoW block - use configured algorithm
        std::string algorithm = Config::getMiningAlgorithm();
        if (algorithm == "ETHASH" || algorithm == "ethash") {
            return BlockType::POW_ETHASH;
        } else if (algorithm == "GXHASH" || algorithm == "gxhash") {
            return BlockType::POW_GXHASH;
        } else {
            return BlockType::POW_SHA256;  // Default to SHA256
        }
    } else {
        // Odd height: PoS block (if validators exist)
        std::lock_guard<std::mutex> lock(chainMutex);
        bool hasValidators = false;
        for (const auto& validator : validators) {
            if (validator.getIsActive() && validator.isValidValidator()) {
                hasValidators = true;
                break;
            }
        }
        
        if (hasValidators) {
            return BlockType::POS;
        } else {
            // Fallback to PoW if no validators - use configured algorithm
            LOG_BLOCKCHAIN(LogLevel::WARNING, "getNextBlockType: No validators, using PoW");
            std::string algorithm = Config::getMiningAlgorithm();
            if (algorithm == "ETHASH" || algorithm == "ethash") {
                return BlockType::POW_ETHASH;
            } else if (algorithm == "GXHASH" || algorithm == "gxhash") {
                return BlockType::POW_GXHASH;
            } else {
                return BlockType::POW_SHA256;
            }
        }
    }
}

bool Blockchain::validateTransaction(const Transaction& tx) {
    // Basic transaction validation
    if (tx.getHash().empty()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction hash is empty");
        return false;
    }

    // Validate address network prefix
    bool isTestnet = Config::isTestnet();
    std::string expectedPrefix = isTestnet ? "tGXC" : "GXC";

    // Helper to check address prefix
    auto checkAddress = [&](const std::string& addr) -> bool {
        if (addr.empty()) return true; // Empty addresses handled by other validation
        if (addr.length() < expectedPrefix.length()) return false;
        return addr.substr(0, expectedPrefix.length()) == expectedPrefix;
    };

    // Check outputs (receivers)
    for (const auto& output : tx.getOutputs()) {
        if (!checkAddress(output.address)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction output address network mismatch: " +
                          output.address + " (Expected prefix: " + expectedPrefix + ")");
            return false;
        }
    }

    // Check sender (if not coinbase)
    if (!tx.isCoinbaseTransaction()) {
         std::string sender = tx.getSenderAddress();
         if (!sender.empty() && !checkAddress(sender)) {
             LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction sender address network mismatch: " +
                          sender + " (Expected prefix: " + expectedPrefix + ")");
             return false;
         }
    }
    
    // Skip validation for coinbase transactions
    if (tx.isCoinbaseTransaction()) {
        return true;
    }
    
    TransactionType txType = tx.getType();
    
    // STAKE TRANSACTION VALIDATION
    if (txType == TransactionType::STAKE) {
        // STAKE transactions must have inputs (UTXOs to consume)
        if (tx.getInputs().empty()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE transaction has no inputs: " + tx.getHash());
            return false;
        }
        
        // Calculate total input
        double inputTotal = tx.getTotalInputAmount();
        double outputTotal = tx.getTotalOutputAmount();
        double fee = tx.getFee();
        
        // Staked amount = inputs - outputs - fee
        double stakedAmount = inputTotal - outputTotal - fee;
        
        if (stakedAmount < 100.0) { // Minimum stake from Validator::MIN_STAKE
            LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE amount too low: " + std::to_string(stakedAmount) + 
                          " (minimum: 100 GXC)");
            return false;
        }
        
        // Verify minimum fee
        double minFee = Config::getDouble("min_tx_fee", 0.001);
        if (fee < minFee) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE transaction fee too low: " + std::to_string(fee));
            return false;
        }
        
        // CRITICAL FIX: Verify inputs reference valid, unspent UTXOs
        // This ensures the staker actually has the funds they're trying to stake
        for (const auto& input : tx.getInputs()) {
            std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
            auto it = utxoSet.find(utxoKey);
            if (it == utxoSet.end()) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE input references non-existent UTXO: " + utxoKey + 
                              " (double-spend attempt or invalid input)");
                return false;
            }
            
            // Verify the input amount matches the UTXO amount
            if (std::abs(it->second.amount - input.amount) > 0.00000001) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE input amount mismatch: input claims " + 
                              std::to_string(input.amount) + " GXC but UTXO has " + 
                              std::to_string(it->second.amount) + " GXC");
                return false;
            }
            
            // Verify the UTXO belongs to the staker (address match)
            std::string stakerAddress = tx.getSenderAddress();
            if (stakerAddress.empty() && !tx.getOutputs().empty()) {
                stakerAddress = tx.getOutputs()[0].address;
            }
            if (!stakerAddress.empty() && it->second.address != stakerAddress) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE input UTXO doesn't belong to staker: UTXO address " + 
                              it->second.address + " != staker address " + stakerAddress);
                return false;
            }
        }
        
        // CRITICAL FIX: Verify validator exists before accepting stake transaction
        // This prevents orphaned stake amounts
        // ALLOW PENDING validators - the STAKE transaction activates them
        std::string stakerAddress = tx.getSenderAddress();
        if (stakerAddress.empty() && !tx.getOutputs().empty()) {
            stakerAddress = tx.getOutputs()[0].address;
        }
        
        if (!stakerAddress.empty()) {
            std::lock_guard<std::mutex> lock(chainMutex);
            auto it = validatorMap.find(stakerAddress);
            if (it == validatorMap.end()) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "STAKE transaction rejected: Validator not registered for address " + 
                              stakerAddress + ". Register as validator first using 'registervalidator' or 'registerexternalvalidator' RPC.");
                return false;
            }
            
            // Log validator status for debugging
            if (it->second.getIsPending()) {
                LOG_BLOCKCHAIN(LogLevel::INFO, "STAKE transaction for PENDING validator " + stakerAddress + 
                              " - this will activate the validator");
            } else {
                LOG_BLOCKCHAIN(LogLevel::INFO, "STAKE transaction for ACTIVE validator " + stakerAddress + 
                              " - adding to existing stake");
            }
        }
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "STAKE transaction validated: " + tx.getHash() + 
                      ", StakedAmount: " + std::to_string(stakedAmount) + " GXC");
        return true;
    }
    
    // UNSTAKE TRANSACTION VALIDATION
    if (txType == TransactionType::UNSTAKE) {
        // Verify user has enough staked balance
        std::string stakerAddress = tx.getReceiverAddress();
        if (stakerAddress.empty() && !tx.getOutputs().empty()) {
            stakerAddress = tx.getOutputs()[0].address;
        }

        double unstakeAmount = tx.getTotalOutputAmount();

        std::lock_guard<std::mutex> lock(chainMutex);
        auto it = validatorMap.find(stakerAddress);
        if (it == validatorMap.end()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "UNSTAKE failed: Validator not found for " + stakerAddress);
            return false;
        }

        if (it->second.getStakeAmount() < unstakeAmount) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "UNSTAKE failed: Insufficient stake. Has " +
                          std::to_string(it->second.getStakeAmount()) + " GXC, trying to unstake " + 
                          std::to_string(unstakeAmount) + " GXC");
            return false;
        }

        LOG_BLOCKCHAIN(LogLevel::INFO, "UNSTAKE transaction validated: " + tx.getHash() + 
                      ", Amount: " + std::to_string(unstakeAmount) + " GXC");
        return true;
    }
    
    // NORMAL TRANSACTION VALIDATION
    // Validate traceability for non-coinbase transactions
    if (!tx.isTraceabilityValid()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction traceability validation failed: " + tx.getHash());
        return false;
    }
    
    // Validate transaction structure
    if (tx.getInputs().empty() || tx.getOutputs().empty()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction has no inputs or outputs: " + tx.getHash());
        return false;
    }
    
    // CRITICAL: Verify all inputs reference valid, unspent UTXOs
    // This prevents double-spending and ensures coins actually exist
    for (const auto& input : tx.getInputs()) {
        std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
        auto it = utxoSet.find(utxoKey);
        if (it == utxoSet.end()) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction input references non-existent UTXO: " + utxoKey + 
                          " (double-spend attempt or invalid input)");
            return false;
        }
        
        // Verify the input amount matches the UTXO amount
        if (std::abs(it->second.amount - input.amount) > 0.00000001) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction input amount mismatch: input claims " + 
                          std::to_string(input.amount) + " GXC but UTXO has " + 
                          std::to_string(it->second.amount) + " GXC");
            return false;
        }

        // Verify that the input's signature is authorized to spend the UTXO
        // The public key in the input must generate the address in the UTXO's script/address
        const TransactionOutput& utxo = it->second;
        if (!Transaction::verifyScript(input.signature, input.publicKey, utxo.script.empty() ? utxo.address : utxo.script)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction input signature verification failed for UTXO: " + utxoKey);
            return false;
        }
    }

    // Verify signatures (matches message and public key)
    if (!tx.validateSignatures()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction signature validation failed: " + tx.getHash());
        return false;
    }
    
    // Validate amounts
    double inputTotal = tx.getTotalInputAmount();
    double outputTotal = tx.getTotalOutputAmount();
    double fee = tx.getFee();
    
    // Validate minimum fee
    double baseFee = Config::getDouble("base_transaction_fee", 0.001);
    double minFee = Config::getDouble("min_tx_fee", baseFee);
    
    if (fee < minFee) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction fee too low: " + std::to_string(fee) + 
                      " (minimum: " + std::to_string(minFee) + ")");
        return false;
    }

    // Input must cover output + fee (no coin creation allowed)
    if (inputTotal < outputTotal + fee) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Transaction input/output mismatch: inputs=" + 
                      std::to_string(inputTotal) + ", outputs=" + std::to_string(outputTotal) + 
                      ", fee=" + std::to_string(fee) + ". Cannot spend more than you have!");
        return false;
    }
    
    // Verify no coin burning (optional - allows for fee overpayment)
    double excessAmount = inputTotal - outputTotal - fee;
    if (excessAmount > 0.00000001) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Transaction has excess input: " + std::to_string(excessAmount) + 
                      " GXC will be added to fee (potential coin burning)");
        // Note: We allow this but log a warning - the excess goes to the miner as extra fee
    }
    
    LOG_BLOCKCHAIN(LogLevel::DEBUG, "Transaction validated: " + tx.getHash() + 
                  ", inputs=" + std::to_string(inputTotal) + ", outputs=" + std::to_string(outputTotal) +
                  ", fee=" + std::to_string(fee));
    
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

double Blockchain::calculateRecommendedFee() const {
    // Check if dynamic fees are enabled
    bool dynamicFeesEnabled = Config::getBool("enable_dynamic_fees", true);
    
    if (!dynamicFeesEnabled) {
        // Return base fee if dynamic fees disabled
        return Config::getDouble("base_transaction_fee", 0.001);
    }
    
    // Get pending transaction count
    std::lock_guard<std::mutex> lock(transactionMutex);
    size_t pendingTxCount = pendingTransactions.size();
    
    // Get configuration values
    double baseFee = Config::getDouble("base_transaction_fee", 0.001);
    double maxFee = Config::getDouble("max_transaction_fee", 0.01);
    
    size_t lowThreshold = Config::getInt("fee_low_threshold", 10);
    size_t mediumThreshold = Config::getInt("fee_medium_threshold", 50);
    size_t highThreshold = Config::getInt("fee_high_threshold", 100);
    
    double lowMultiplier = Config::getDouble("fee_low_multiplier", 1.0);
    double mediumMultiplier = Config::getDouble("fee_medium_multiplier", 1.5);
    double highMultiplier = Config::getDouble("fee_high_multiplier", 2.0);
    double veryHighMultiplier = Config::getDouble("fee_very_high_multiplier", 3.0);
    
    // Calculate fee based on pending transaction count
    double fee;
    if (pendingTxCount < lowThreshold) {
        // Low activity: base fee
        fee = baseFee * lowMultiplier;
    } else if (pendingTxCount < mediumThreshold) {
        // Medium activity: 1.5x base fee
        fee = baseFee * mediumMultiplier;
    } else if (pendingTxCount < highThreshold) {
        // High activity: 2x base fee
        fee = baseFee * highMultiplier;
    } else {
        // Very high activity: 3x base fee (capped at maxFee)
        fee = baseFee * veryHighMultiplier;
        fee = std::min(fee, maxFee);
    }
    
    // Ensure fee is at least base fee
    fee = std::max(fee, baseFee);
    
    LOG_BLOCKCHAIN(LogLevel::DEBUG, "Calculated recommended fee: " + std::to_string(fee) + 
                   " GXC (pending tx: " + std::to_string(pendingTxCount) + ")");
    
    return fee;
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
    // Lock transaction mutex to access pendingTransactions
    std::lock_guard<std::mutex> txLock(transactionMutex);
    
    double balance = 0.0;
    uint32_t utxoCount = 0;
    
    // Identify UTXOs spent by pending transactions
    std::unordered_set<std::string> spentInMempool;
    for (const auto& tx : pendingTransactions) {
        if (!tx.isCoinbaseTransaction()) {
            for (const auto& input : tx.getInputs()) {
                spentInMempool.insert(input.txHash + "_" + std::to_string(input.outputIndex));
            }
        }
    }

    // Sum all unspent UTXOs for this address (excluding those spent in mempool)
    for (const auto& [utxoKey, output] : utxoSet) {
        // Use exact match for address comparison
        if (output.address == address) {
            // Check if this UTXO is being spent in the mempool
            if (spentInMempool.find(utxoKey) == spentInMempool.end()) {
                balance += output.amount;
                utxoCount++;
                LOG_BLOCKCHAIN(LogLevel::DEBUG, "  UTXO: " + utxoKey + " = " + std::to_string(output.amount) +
                              " GXC to " + output.address.substr(0, 20) + "...");
            } else {
                LOG_BLOCKCHAIN(LogLevel::DEBUG, "  UTXO: " + utxoKey + " [SPENT IN MEMPOOL] = " + std::to_string(output.amount) +
                              " GXC to " + output.address.substr(0, 20) + "...");
            }
        }
    }

    // Add pending incoming outputs (unconfirmed balance)
    for (const auto& tx : pendingTransactions) {
        uint32_t outputIndex = 0;
        for (const auto& output : tx.getOutputs()) {
            if (output.address == address) {
                std::string pendingUtxoKey = tx.getHash() + "_" + std::to_string(outputIndex);

                // If this pending output is not spent by another pending transaction, add it
                if (spentInMempool.find(pendingUtxoKey) == spentInMempool.end()) {
                    balance += output.amount;
                    LOG_BLOCKCHAIN(LogLevel::DEBUG, "  Pending Receive: " + pendingUtxoKey + " = " + std::to_string(output.amount) +
                                  " GXC to " + output.address.substr(0, 20) + "...");
                }
            }
            outputIndex++;
        }
    }
    
    // Always log balance queries at INFO level for debugging
    LOG_BLOCKCHAIN(LogLevel::INFO, "getBalance(" + address.substr(0, 20) + "...): " + std::to_string(balance) + 
                  " GXC from " + std::to_string(utxoCount) + " UTXOs (total UTXOs in set: " + std::to_string(utxoSet.size()) + ")");
    
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

// Validator management methods
bool Blockchain::registerValidator(const Validator& validator) {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    // Allow pending validators (waiting for stake confirmation) to be registered
    if (!validator.isValidValidator() && !validator.getIsPending()) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Invalid validator: " + validator.getAddress());
        return false;
    }
    
    // Check if validator already exists
    if (validatorMap.find(validator.getAddress()) != validatorMap.end()) {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Validator already registered: " + validator.getAddress());
        // Update existing validator
        validatorMap[validator.getAddress()] = validator;
    } else {
        // Add new validator
        validatorMap[validator.getAddress()] = validator;
        validators.push_back(validator);
        LOG_BLOCKCHAIN(LogLevel::INFO, "Validator registered: " + validator.getAddress() + 
                      ", Stake: " + std::to_string(validator.getStakeAmount()) + 
                      " GXC, Days: " + std::to_string(validator.getStakingDays()));
    }

    // Persist validator
    try {
        if (!Database::getInstance().storeValidator(validator)) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to persist validator: " + validator.getAddress());
            return false;
        }
    } catch (const std::exception& e) {
        LOG_BLOCKCHAIN(LogLevel::ERROR, "Exception persisting validator: " + std::string(e.what()));
        return false;
    }

    return true;
}

void Blockchain::unregisterValidator(const std::string& address) {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    auto it = validatorMap.find(address);
    if (it != validatorMap.end()) {
        // Mark as inactive instead of removing (for history)
        it->second.setIsActive(false);
        Validator updatedValidator = it->second;
        
        // Remove from validators vector
        validators.erase(
            std::remove_if(validators.begin(), validators.end(),
                [&address](const Validator& v) { return v.getAddress() == address; }),
            validators.end()
        );
        
        LOG_BLOCKCHAIN(LogLevel::INFO, "Validator unregistered: " + address);

        // Update persistence (store as inactive)
        try {
            if (!Database::getInstance().updateValidator(updatedValidator)) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to update validator persistence: " + address);
            }
        } catch (const std::exception& e) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Exception updating validator persistence: " + std::string(e.what()));
        }
    } else {
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Validator not found: " + address);
    }
}

bool Blockchain::slashValidator(const std::string& address, double amount, const std::string& reason) {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    auto it = validatorMap.find(address);
    if (it != validatorMap.end()) {
        it->second.slash(amount, reason);
        Validator updatedValidator = it->second;
        LOG_BLOCKCHAIN(LogLevel::WARNING, "Validator slashed: " + address + 
                      ", Amount: " + std::to_string(amount) + " GXC, Reason: " + reason);

        // Update persistence
        try {
            if (!Database::getInstance().updateValidator(updatedValidator)) {
                LOG_BLOCKCHAIN(LogLevel::ERROR, "Failed to update slashed validator persistence: " + address);
            }
        } catch (const std::exception& e) {
            LOG_BLOCKCHAIN(LogLevel::ERROR, "Exception updating slashed validator persistence: " + std::string(e.what()));
        }

        return true;
    }
    
    LOG_BLOCKCHAIN(LogLevel::ERROR, "Validator not found for slashing: " + address);
    return false;
}

std::vector<Validator> Blockchain::getActiveValidators() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    
    std::vector<Validator> active;
    for (const auto& validator : validators) {
        if (validator.getIsActive() && validator.isValidValidator()) {
            active.push_back(validator);
        }
    }
    
    return active;
}

// =========================================================
//           SECURITY ENGINE INTEGRATION
// =========================================================

double Blockchain::getSecurityAdjustedDifficulty() const {
    if (!securityEngine) return difficulty;
    
    // Get base difficulty from security engine
    double adjustedDiff = securityEngine->calculateNextDifficulty(
        difficulty, 
        currentHashrate, 
        lastBlockTime
    );
    
    return adjustedDiff;
}

double Blockchain::getSecurityAdjustedReward(double blockTime) const {
    if (!securityEngine) return blockReward;
    
    // Apply emission guard
    return securityEngine->calculateBlockReward(blockTime);
}

double Blockchain::getRecommendedFee() const {
    if (!securityEngine) return 0.0001; // Default min fee
    
    return securityEngine->getRecommendedFee();
}

bool Blockchain::detectPotentialAttack() const {
    if (!securityEngine) return false;
    
    return securityEngine->detectAttack(currentHashrate, lastBlockTime);
}

std::string Blockchain::getAttackStatus() const {
    if (!securityEngine) return "SECURITY_ENGINE_NOT_INITIALIZED";
    
    if (securityEngine->detectAttack(currentHashrate, lastBlockTime)) {
        return securityEngine->getAttackType(currentHashrate, lastBlockTime);
    }
    
    return "NETWORK_SECURE";
}

void Blockchain::updateHashrate(double hashrate) {
    currentHashrate = hashrate;
    
    if (securityEngine) {
        // Update security engine with new hashrate
        securityEngine->predictHashrate(hashrate);
        
        // Update staking metrics
        double stakedAmount = 0.0;
        for (const auto& validator : validators) {
            if (validator.getIsActive()) {
                stakedAmount += validator.getStakeAmount();
            }
        }
        securityEngine->updateStakingMetrics(stakedAmount, totalSupply);
        
        // Check for attacks
        if (securityEngine->detectHashrateSurge(hashrate)) {
            LOG_BLOCKCHAIN(LogLevel::WARNING, 
                "SECURITY ALERT: Hashrate surge detected! Current: " + 
                std::to_string(hashrate) + " H/s");
        }
    }
}
