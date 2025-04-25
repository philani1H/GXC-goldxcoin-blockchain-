#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include "Block.h"
#include "Wallet.h"
#include "ProofOfPrice.h"

class Blockchain {
private:
    std::vector<Block> chain;
    std::vector<Transaction> pendingTransactions;
    std::unordered_map<std::string, TransactionOutput> utxoSet; // Unspent transaction outputs
    double difficulty;
    double posThreshold;
    uint64_t blockReward;
    double feeBurnRate;
    
    // Validator selection
    std::vector<Validator> validators;
    
    // Proof of Price oracle
    std::shared_ptr<ProofOfPrice> popOracle;
    
    // Adaptive monetary policy parameters
    double targetInflationRate;
    double targetPriceRatio;
    double k1, k2, k3;
    
    // Private methods
    bool isValidChain() const;
    void updateUtxoSet(const Block& block);
    double calculateObservedInflation(uint32_t window) const;
    
public:
    // Constructor
    Blockchain();
    
    // Add a block to the chain
    bool addBlock(Block& block);
    
    // Create a new block (mining)
    Block createBlock(BlockType type, const std::string& minerAddress);
    
    // Process a transaction
    bool processTransaction(const Transaction& transaction);
    
    // Select a validator for PoS block
    Validator selectValidator() const;
    
    // Calculate new block reward based on adaptive monetary policy
    double calculateBlockReward(uint32_t blockNumber) const;
    
    // Calculate fee burn rate based on current inflation
    double calculateFeeBurnRate(uint32_t blockNumber) const;
    
    // Process transaction fees with partial burning
    void processTransactionFees(const Block& block);
    
    // Getters
    const Block& getLatestBlock() const { return chain.back(); }
    size_t getChainLength() const { return chain.size(); }
    double getDifficulty() const { return difficulty; }
    uint64_t getBlockReward() const { return blockReward; }
    
    // Adjust difficulty
    void adjustDifficulty();
    
    // Register a validator
    void registerValidator(const Validator& validator);
    
    // Update PoP oracle data
    void updatePriceData(const PriceData& priceData);
};
