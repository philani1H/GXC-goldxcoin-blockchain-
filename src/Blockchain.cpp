#include "../include/Blockchain.h"
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
    : difficulty(4.0), posThreshold(0.5), blockReward(12.5 * 100000000), // 12.5 GXC in satoshis
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
