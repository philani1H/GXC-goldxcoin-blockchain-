#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <cstdint>
#include "transaction.h"

enum class BlockType {
    POW_SHA256,  // Bitcoin-style SHA-256 mining
    POW_ETHASH,  // Ethereum-style Ethash mining
    POS          // Proof-of-Stake validation
};

class Block {
private:
    uint32_t index;
    std::string previousHash;
    std::string hash;
    std::string merkleRoot;
    std::time_t timestamp;
    uint64_t nonce;
    BlockType blockType;
    std::string minerAddress;
    std::vector<Transaction> transactions;
    double difficulty;
    std::string validatorSignature; // For PoS blocks

    // Hybrid consensus specific
    std::string powHash;  // Hash for PoW validation
    std::string posHash;  // Hash for PoS validation

    // Adaptive monetary policy
    double blockReward;
    double feeBurnRate;
    std::string popReference; // Reference to latest PoP oracle data

public:
    // Constructors
    Block(); // Default constructor
    Block(uint32_t indexIn, const std::string& previousHashIn, BlockType type);

    // Mining and validation
    void mineBlock(double difficultyIn);
    bool validateBlock(const std::string& validatorSignature = "") const;

    // Transaction management
    bool addTransaction(const Transaction& transaction);
    void calculateMerkleRoot();
    std::string calculateMerkleRoot() const; // Const version for validation

    // Hash calculations
    std::string calculateHash() const;
    std::string calculatePowHash() const;
    std::string calculatePosHash() const;

    // Getters
    uint32_t getIndex() const { return index; }
    std::string getHash() const { return hash; }
    std::string getPreviousHash() const { return previousHash; }
    std::string getMerkleRoot() const { return merkleRoot; }
    std::time_t getTimestamp() const { return timestamp; }
    uint64_t getNonce() const { return nonce; }
    BlockType getBlockType() const { return blockType; }
    std::string getMinerAddress() const { return minerAddress; }
    const std::vector<Transaction>& getTransactions() const { return transactions; }
    double getDifficulty() const { return difficulty; }
    std::string getValidatorSignature() const { return validatorSignature; }
    double getBlockReward() const { return blockReward; }
    double getFeeBurnRate() const { return feeBurnRate; }
    std::string getPopReference() const { return popReference; }

    // Setters
    void setIndex(uint32_t idx) { index = idx; }
    void setHash(const std::string& h) { hash = h; }
    void setPreviousHash(const std::string& prevHash) { previousHash = prevHash; }
    void setMerkleRoot(const std::string& root) { merkleRoot = root; }
    void setTimestamp(std::time_t ts) { timestamp = ts; }
    void setDifficulty(double diff) { difficulty = diff; }
    void setNonce(uint64_t n) { nonce = n; }
    void incrementNonce() { nonce++; }
    void setMinerAddress(const std::string& address) { minerAddress = address; }
    void setValidatorSignature(const std::string& signature) { validatorSignature = signature; }
    void setBlockReward(double reward) { blockReward = reward; }
    void setFeeBurnRate(double rate) { feeBurnRate = rate; }
    void setPopReference(const std::string& popRef) { popReference = popRef; }

    // Serialization
    std::string serialize() const;
    static Block deserialize(const std::string& data);
};

// Helper functions
std::string sha256(const std::string& data);
std::string sha256d(const std::string& data);
std::string ethash(const std::string& data, uint64_t nonce);
std::string keccak256(const std::string& data);
std::string calculateMerkleRoot(const std::vector<std::string>& txHashes);
bool meetsTarget(const std::string& hash, double difficulty);