#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include "Block.h"
#include "transaction.h"
#include "Validator.h"
#include "Wallet.h"
#include "ProofOfPrice.h"
#include "CrossChainBridge.h"
#include "Governance.h"
#include "GoldToken.h"
#include "StockContract.h"
#include <mutex>

class Blockchain {
private:
    std::vector<std::shared_ptr<Block>> chain;
    std::vector<Transaction> pendingTransactions;
    std::unordered_map<std::string, TransactionOutput> utxoSet; // Unspent transaction outputs

    // Consensus parameters
    double difficulty;
    double posThreshold;
    uint64_t blockReward;
    double feeBurnRate;

    // Validator management
    std::vector<Validator> validators;
    std::unordered_map<std::string, Validator> validatorMap;

    // Proof of Price oracle
    std::shared_ptr<ProofOfPrice> popOracle;

    // Adaptive monetary policy parameters
    double targetInflationRate;
    double targetPriceRatio;
    double k1, k2, k3; // Adjustment parameters
    double baseBurnRate;

    // Supply tracking
    double totalSupply;
    double totalBurned;
    uint32_t lastHalvingBlock;

    // Governance
    std::vector<Proposal> proposals;
    std::unordered_map<std::string, Vote> votes;

    // Cross-chain bridge
    std::shared_ptr<CrossChainBridge> bridge;

    // Gold-backed tokens (GXC-G)
    std::unordered_map<std::string, double> goldReserves;
    std::unordered_map<std::string, GoldToken> goldTokens;

    // Stock contracts
    std::unordered_map<std::string, StockContract> stockContracts;

    // Private methods
    bool isValidChain() const;
    void updateUtxoSet(const Block& block);
    double calculateBlockReward(uint32_t height) const;
    double calculateObservedInflation(uint32_t window) const;
    double calculateNextDifficulty() const;
    void processHalving(uint32_t blockNumber);
    bool validateConsensus(const Block& block) const;
    std::shared_ptr<Block> lastBlock;
    mutable std::mutex chainMutex;
    mutable std::mutex transactionMutex;

    // Traceability entry structure
    struct TraceabilityEntry {
        std::string txHash;
        std::string prevTxHash;
        double amount;
        std::string fromAddress;
        std::string toAddress;
        uint64_t timestamp;
    };
    std::unordered_map<std::string, TraceabilityEntry> traceabilityIndex;

public:
    // Constructor
    Blockchain();
    ~Blockchain();
    bool initialize();
    bool loadBlocksFromDatabase();
    bool loadValidatorsFromDatabase();
    void rebuildUtxoSet();
    void shutdown();
    void createGenesisBlock();
    bool addBlock(const Block& block);
    bool validateBlock(const Block& block);
    bool validateBlockInternal(const Block& block, uint32_t expectedIndex);
    bool validateProofOfWork(const Block& block) const;
    bool validateBlockTraceability(const Block& block);
    bool validateTransaction(const Transaction& tx);
    bool validateTraceability();
    bool initializeTraceability();
    bool validateTransactionTraceability(const Transaction& tx);
    void updateTransactionPool(const Block& block);
    bool addTransaction(const Transaction& tx);
    void processTransactions();
    std::vector<Transaction> getPendingTransactions(size_t maxCount);
    double calculateRecommendedFee() const;
    Block getLatestBlock() const;
    Block getBlock(const std::string& hash) const;
    Block getBlock(size_t index) const;
    std::vector<Block> getBlocks(size_t count) const;
    bool isValid() const;
    size_t getHeight() const;
    double getDifficulty() const;
    double getBalance(const std::string& address) const;
    const std::unordered_map<std::string, TransactionOutput>& getUtxoSet() const;
    std::string getStats() const;

    // Hybrid consensus
    bool validateProofOfStake(const Block& block) const;
    Validator selectValidatorForBlock() const;
    BlockType getNextBlockType() const;

    // Adaptive monetary policy
    // double calculateBlockReward(uint32_t blockNumber) const;
    // double calculateFeeBurnRate(uint32_t blockNumber) const;
    // void processTransactionFees(const Block& block);
    // void updateMonetaryPolicy();

    // Transaction chaining
    // std::string getLastTransactionHash(const std::string& address) const;
    // bool verifyTransactionChain(const std::string& address, uint32_t depth = 100) const;

    // Gold-backed tokens (GXC-G)
    // bool mintGoldTokens(const std::string& recipient, double goldGrams, const std::string& vaultAttestation);
    // bool burnGoldTokens(const std::string& holder, double goldGrams, const std::string& redemptionCert);
    // double getGoldReserves(const std::string& vault) const;
    // bool verifyGoldReserves() const;

    // Stock contracts
    // bool deployStockContract(const std::string& ticker, const std::string& priceOracle);
    // bool issueStockShares(const std::string& ticker, const std::string& recipient, uint64_t shares);
    // bool transferStockShares(const std::string& ticker, const std::string& from, const std::string& to, uint64_t shares);
    // StockContract getStockContract(const std::string& ticker) const;

    // Governance
    // std::string submitProposal(const std::string& proposer, const std::string& title,
    //                           const std::string& description, const std::vector<ParameterChange>& changes);
    // bool castVote(const std::string& voter, const std::string& proposalId, VoteType voteType);
    // void tallyVotes(const std::string& proposalId);
    // bool executeProposal(const std::string& proposalId);

    // Validator management
    void registerValidator(const Validator& validator);
    void unregisterValidator(const std::string& address);
    bool slashValidator(const std::string& address, double amount, const std::string& reason);
    std::vector<Validator> getActiveValidators() const;

    // Cross-chain bridge
    std::string initiateCrossChainTransfer(const std::string& sourceChain, const std::string& asset,
                                          double amount, const std::string& destinationAddress);
    bool processCrossChainTransfer(const std::string& transferId);
    void addBridgeValidator(const BridgeValidator& validator);

    // Difficulty adjustment
    // void adjustDifficulty();
    // void adjustPoSDifficulty();

    // Getters
    // const Block& getLatestBlock() const { return chain.back(); }
    // size_t getChainLength() const { return chain.size(); }
    // double getPoSThreshold() const { return posThreshold; }
    // uint64_t getBlockReward() const { return blockReward; }
    // double getFeeBurnRate() const { return feeBurnRate; }
    // double getTotalSupply() const { return totalSupply; }
    // double getTotalBurned() const { return totalBurned; }
    // double getTargetInflationRate() const { return targetInflationRate; }

    // Network statistics
    // double getHashrate() const;
    // double getStakeRatio() const;
    // uint32_t getActiveValidatorCount() const;
    // double getNetworkStakeWeight() const;

    // Oracle integration
    void updatePriceData(const PriceData& priceData);
    PriceData getCurrentGoldPrice() const;
    std::vector<PriceData> getPriceHistory(uint32_t blocks) const;

    // Wallet integration
    double getGoldBalance(const std::string& address) const;
    std::vector<Transaction> getTransactionHistory(const std::string& address) const;

    // Block validation
    // bool isValidBlock(const Block& block) const;
    // bool isValidTransaction(const Transaction& transaction) const;

    // Chain validation
    // bool validateChain() const;
    // void reorgChain(uint32_t fromBlock);

    // Mining support
    // double getCurrentDifficulty(BlockType type) const;
    // std::string getMiningTarget(BlockType type) const;
    // bool submitPoWShare(const std::string& miner, const std::string& blockHash, uint64_t nonce);

    // Transaction Traceability - Implementing Your Formula
    std::vector<std::string> traceTransactionLineage(const std::string& startHash) const;
    bool verifyTransactionLineage(const std::string& txHash) const;
    Transaction getTransactionByHash(const std::string& hash) const;
    bool isLineageValid(const std::string& startHash) const;
    std::vector<std::string> getTransactionChain(const std::string& address, uint32_t depth = 100) const;

    // Enhanced transaction validation with traceability
    bool validateTransactionTraceability(const Transaction& tx) const;
    bool verifyInputReferences(const Transaction& tx) const;

    // Power-Weighted Difficulty
    double getMinerPower(const std::string& minerAddress) const;
    double calculateMinerDifficulty(const std::string& minerAddress) const;

    // Constants
    static const uint32_t MAX_SUPPLY = 31000000; // 31 million GXC
    static const uint32_t HALVING_INTERVAL = 1051200; // ~4 years
    static const uint32_t DIFFICULTY_ADJUSTMENT_INTERVAL = 2016; // ~2 weeks
    static constexpr double INITIAL_BLOCK_REWARD = 50.0;

    // Power-Weighted Difficulty Constants
    static constexpr double PWD_ALPHA = 5.0;      // Penalty strength
    static constexpr double PWD_BETA = 2.0;       // Non-linearity (exponent)
    static constexpr int PWD_WINDOW = 100;        // Window for measuring miner power
    static constexpr double SCARCITY_GAMMA = 1.0; // Scarcity sensitivity
    static constexpr double REF_DIFFICULTY = 1000.0; // Reference difficulty for scarcity scaling
};
