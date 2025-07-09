#include "../include/blockchain.h"
#include <algorithm>
#include <random>
#include <cmath>

// Constants for adaptive monetary policy
const double TARGET_INFLATION_RATE = 0.02; // 2% annual inflation
const double K1 = 0.1;
const double K2 = 0.05;
const double K3 = 0.2;
const double BASE_BURN_RATE = 0.3; // 30% base burn rate
const uint32_t MEASUREMENT_WINDOW = 2016; // ~2 weeks of blocks

Blockchain::Blockchain() 
    : difficulty(4.0), posThreshold(0.5), blockReward(50.0 * 100000000), // 50 GXC in satoshis
      feeBurnRate(BASE_BURN_RATE), targetInflationRate(TARGET_INFLATION_RATE),
      k1(K1), k2(K2), k3(K3) {
    
    // Create the genesis block
    Block genesisBlock(0, "0", BlockType::POW_SHA256);
    genesisBlock.setMinerAddress("GENESIS_MINER");
    genesisBlock.mineBlock(difficulty);
    
    chain.push_back(genesisBlock);
    
    // Initialize PoP oracle
    popOracle = std::make_shared<ProofOfPrice>();
}

bool Blockchain::addBlock(Block& block) {
    // Verify the block
    if (block.getPreviousHash() != getLatestBlock().getHash()) {
        return false;
    }
    
    // For PoW blocks, verify the hash meets the difficulty target
    if (block.getBlockType() == BlockType::POW_SHA256 || 
        block.getBlockType() == BlockType::POW_ETHASH) {
        if (!meetsTarget(block.getHash(), difficulty)) {
            return false;
        }
    }
    // For PoS blocks, verify the validator's signature
    else if (block.getBlockType() == BlockType::POS) {
        // In a real implementation, we would verify the validator's signature here
        // For simplicity, we'll just check if the block has a miner address
        if (block.getMinerAddress().empty()) {
            return false;
        }
    }
    
    // Add the block to the chain
    chain.push_back(block);
    
    // Update the UTXO set
    updateUtxoSet(block);
    
    // Process transaction fees
    processTransactionFees(block);
    
    // Adjust difficulty every 2016 blocks (approximately 2 weeks)
    if (chain.size() % 2016 == 0) {
        adjustDifficulty();
    }
    
    return true;
}

Block Blockchain::createBlock(BlockType type, const std::string& minerAddress) {
    uint32_t index = static_cast<uint32_t>(chain.size());
    Block newBlock(index, getLatestBlock().getHash(), type);
    newBlock.setMinerAddress(minerAddress);
    
    // Add pending transactions to the block
    for (const auto& tx : pendingTransactions) {
        newBlock.addTransaction(tx);
    }
    
    // Clear pending transactions
    pendingTransactions.clear();
    
    return newBlock;
}

bool Blockchain::processTransaction(const Transaction& transaction) {
    // Verify the transaction
    if (!transaction.verifyTransaction()) {
        return false;
    }
    
    // Add the transaction to pending transactions
    pendingTransactions.push_back(transaction);
    
    return true;
}

Validator Blockchain::selectValidator() const {
    // Calculate total weighted stake
    double totalWeightedStake = 0.0;
    for (const auto& validator : validators) {
        if (validator.getIsActive()) {
            totalWeightedStake += validator.getWeightedStake();
        }
    }
    
    // Random selection proportional to weighted stake
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, totalWeightedStake);
    double threshold = dis(gen);
    
    double cumulativeWeight = 0.0;
    for (const auto& validator : validators) {
        if (validator.getIsActive()) {
            cumulativeWeight += validator.getWeightedStake();
            if (cumulativeWeight >= threshold) {
                return validator;
            }
        }
    }
    
    // Fallback (should never reach here)
    return validators.back();
}

double Blockchain::calculateBlockReward(uint32_t blockNumber) const {
    // Get the base reward for the current halving epoch
    uint32_t halvings = blockNumber / 1051200; // 4 years of blocks
    double baseReward = blockReward * std::pow(0.5, halvings);
    
    // Get observed inflation rate
    double observedInflation = calculateObservedInflation(MEASUREMENT_WINDOW);
    
    // Get gold-GXC price ratio (in a real implementation, this would come from the PoP oracle)
    double goldPrice = 2000.0; // Example gold price in USD
    double gxcPrice = 100.0;   // Example GXC price in USD
    double priceRatio = goldPrice / gxcPrice;
    
    // Get target parameters
    double targetPriceRatio = 20.0; // Example target ratio
    
    // Calculate adjustment factors
    double inflationAdjustment = k1 * (targetInflationRate - observedInflation);
    double priceRatioAdjustment = k2 * (targetPriceRatio - priceRatio);
    
    // Calculate new reward
    double adjustmentFactor = 1.0 + inflationAdjustment + priceRatioAdjustment;
    double newReward = baseReward * adjustmentFactor;
    
    // Ensure reward stays within bounds
    return std::max(0.0, std::min(baseReward * 2.0, newReward));
}

double Blockchain::calculateFeeBurnRate(uint32_t blockNumber) const {
    // Get observed inflation rate
    double observedInflation = calculateObservedInflation(MEASUREMENT_WINDOW);
    
    // Calculate adjustment
    double adjustment = k3 * (observedInflation - targetInflationRate);
    
    // Calculate new burn rate
    double newBurnRate = BASE_BURN_RATE * (1.0 + adjustment);
    
    // Ensure burn rate stays within bounds
    return std::max(0.1, std::min(0.9, newBurnRate));
}

void Blockchain::processTransactionFees(const Block& block) {
    // Calculate the current burn rate
    double burnRate = calculateFeeBurnRate(block.getIndex());
    
    // Get the block producer
    std::string blockProducer = block.getMinerAddress();
    
    double totalFees = 0.0;
    
    // Process each transaction
    for (const auto& tx : block.getTransactions()) {
        // Calculate transaction fee (in a real implementation, this would be more complex)
        double fee = 0.001; // Example fee
        totalFees += fee;
    }
    
    // Calculate amount to burn
    double burnAmount = totalFees * burnRate;
    
    // Distribute remaining fees to block producer
    double remainingFees = totalFees - burnAmount;
    
    // In a real implementation, we would update the UTXO set to reflect these changes
}

double Blockchain::calculateObservedInflation(uint32_t window) const {
    // In a real implementation, this would calculate the actual inflation rate
    // based on the change in total supply over the specified window
    
    // For simplicity, we'll just return a fixed value
    return 0.02; // 2% inflation
}

void Blockchain::updateUtxoSet(const Block& block) {
    // Process each transaction in the block
    for (const auto& tx : block.getTransactions()) {
        // Remove spent outputs
        for (const auto& input : tx.getInputs()) {
            utxoSet.erase(input.txHash + ":" + std::to_string(input.outputIndex));
        }
        
        // Add new outputs
        for (size_t i = 0; i < tx.getOutputs().size(); ++i) {
            const auto& output = tx.getOutputs()[i];
            utxoSet[tx.getHash() + ":" + std::to_string(i)] = output;
        }
    }
}

void Blockchain::adjustDifficulty() {
    // Calculate the time taken for the last 2016 blocks
    std::time_t startTime = chain[chain.size() - 2016].getTimestamp();
    std::time_t endTime = chain.back().getTimestamp();
    double actualTimespan = difftime(endTime, startTime);
    
    // Expected timespan: 2016 blocks * 10 minutes = 20160 minutes = 1209600 seconds
    double expectedTimespan = 1209600;
    
    // Apply damping factor to prevent excessive swings
    double dampingFactor = 0.25;
    
    // Calculate new difficulty
    double ratio = (expectedTimespan / actualTimespan) * dampingFactor;
    difficulty = difficulty * ratio;
    
    // Ensure difficulty stays within bounds
    difficulty = std::max(1.0, std::min(difficulty, 100.0));
}

void Blockchain::registerValidator(const Validator& validator) {
    validators.push_back(validator);
}

void Blockchain::updatePriceData(const PriceData& priceData) {
    // Update the PoP oracle with new price data
    // This would be used by the adaptive monetary policy
}

// Transaction Traceability Implementation - Your Formula
std::vector<std::string> Blockchain::traceTransactionLineage(const std::string& startHash) const {
    std::vector<std::string> lineage;
    std::string currentHash = startHash;
    
    while (!currentHash.empty() && currentHash != "0") {
        lineage.push_back(currentHash);
        
        try {
            Transaction tx = getTransactionByHash(currentHash);
            
            // If reached genesis or orphan transaction
            if (tx.getPrevTxHash().empty() || tx.getPrevTxHash() == "0" || tx.isCoinbaseTransaction()) {
                break;
            }
            
            currentHash = tx.getPrevTxHash();
            
            // Prevent infinite loops
            if (lineage.size() > 10000) {
                break;
            }
            
        } catch (const std::exception&) {
            // Transaction not found, break the chain
            break;
        }
    }
    
    return lineage;
}

bool Blockchain::verifyTransactionLineage(const std::string& txHash) const {
    try {
        Transaction tx = getTransactionByHash(txHash);
        
        // Verify the transaction's traceability formula
        if (!tx.verifyTraceabilityFormula()) {
            return false;
        }
        
        // If it's a genesis or coinbase transaction, it's valid
        if (tx.isGenesis() || tx.isCoinbaseTransaction()) {
            return true;
        }
        
        // Verify that the referenced previous transaction exists
        if (!tx.getPrevTxHash().empty() && tx.getPrevTxHash() != "0") {
            try {
                Transaction prevTx = getTransactionByHash(tx.getPrevTxHash());
                
                // Verify that the amount reference is correct
                if (!tx.getInputs().empty()) {
                    const auto& firstInput = tx.getInputs()[0];
                    
                    // Check if the referenced output exists and has the correct amount
                    if (firstInput.outputIndex < prevTx.getOutputs().size()) {
                        const auto& referencedOutput = prevTx.getOutputs()[firstInput.outputIndex];
                        
                        if (std::abs(firstInput.amount - referencedOutput.amount) > 0.00000001) {
                            return false;
                        }
                    }
                }
                
                return true;
            } catch (const std::exception&) {
                return false; // Previous transaction not found
            }
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false; // Transaction not found
    }
}

Transaction Blockchain::getTransactionByHash(const std::string& hash) const {
    // Search through all blocks for the transaction
    for (const auto& block : chain) {
        for (const auto& tx : block.getTransactions()) {
            if (tx.getHash() == hash) {
                return tx;
            }
        }
    }
    
    // Check pending transactions
    for (const auto& tx : pendingTransactions) {
        if (tx.getHash() == hash) {
            return tx;
        }
    }
    
    // Transaction not found - throw exception or return empty transaction
    throw std::runtime_error("Transaction not found: " + hash);
}

bool Blockchain::isLineageValid(const std::string& startHash) const {
    auto lineage = traceTransactionLineage(startHash);
    
    // Verify each transaction in the lineage
    for (const auto& txHash : lineage) {
        if (!verifyTransactionLineage(txHash)) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> Blockchain::getTransactionChain(const std::string& address, uint32_t depth) const {
    std::vector<std::string> chain;
    
    // Find all transactions involving this address
    for (const auto& block : this->chain) {
        for (const auto& tx : block.getTransactions()) {
            // Check if address is in inputs or outputs
            bool addressInvolved = false;
            
            // Check outputs
            for (const auto& output : tx.getOutputs()) {
                if (output.address == address) {
                    addressInvolved = true;
                    break;
                }
            }
            
            // Check sender address
            if (!addressInvolved && tx.getSenderAddress() == address) {
                addressInvolved = true;
            }
            
            if (addressInvolved) {
                chain.push_back(tx.getHash());
                
                if (chain.size() >= depth) {
                    break;
                }
            }
        }
        
        if (chain.size() >= depth) {
            break;
        }
    }
    
    return chain;
}

bool Blockchain::validateTransactionTraceability(const Transaction& tx) const {
    // Apply your traceability formula validation
    return tx.verifyTraceabilityFormula() && tx.validateInputReference() && 
           tx.isTraceabilityValid() && verifyInputReferences(tx);
}

bool Blockchain::verifyInputReferences(const Transaction& tx) const {
    if (tx.isCoinbaseTransaction() || tx.isGenesis()) {
        return true;
    }
    
    // Verify that all inputs reference valid previous transaction outputs
    for (const auto& input : tx.getInputs()) {
        try {
            Transaction prevTx = getTransactionByHash(input.txHash);
            
            // Check if the output index is valid
            if (input.outputIndex >= prevTx.getOutputs().size()) {
                return false;
            }
            
            // Check if the referenced output amount matches
            const auto& referencedOutput = prevTx.getOutputs()[input.outputIndex];
            if (std::abs(input.amount - referencedOutput.amount) > 0.00000001) {
                return false;
            }
            
            // Check if output hasn't been spent already (simplified UTXO check)
            std::string utxoKey = input.txHash + ":" + std::to_string(input.outputIndex);
            if (utxoSet.find(utxoKey) == utxoSet.end()) {
                return false; // UTXO doesn't exist (already spent)
            }
            
        } catch (const std::exception&) {
            return false; // Referenced transaction not found
        }
    }
    
    return true;
}
