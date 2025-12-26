#include "../include/Block.h"
#include "../include/HashUtils.h"
#include <sstream>

Block::Block() 
    : index(0), previousHash(""), blockType(BlockType::POW_SHA256), minerAddress(""), 
      validatorAddress(""), nonce(0), difficulty(0), 
      chainWork("0000000000000000000000000000000000000000000000000000000000000000"), nBits(0x1d00ffff) {
    timestamp = std::time(nullptr);
    hash = calculateHash();
}

Block::Block(uint32_t indexIn, const std::string& previousHashIn, BlockType type) 
    : index(indexIn), previousHash(previousHashIn), blockType(type), minerAddress(""),
      validatorAddress(""), nonce(0), difficulty(0),
      chainWork("0000000000000000000000000000000000000000000000000000000000000000"), nBits(0x1d00ffff) {
    timestamp = std::time(nullptr);
    hash = calculateHash();
}

std::string Block::calculateHash() const {
    std::stringstream ss;
    ss << index << previousHash << timestamp << merkleRoot << nonce;
    
    if (blockType == BlockType::POW_SHA256) {
        return sha256d(ss.str());
    } else if (blockType == BlockType::POW_ETHASH) {
        return ethash(ss.str(), nonce);
    } else if (blockType == BlockType::POW_GXHASH) {
        return gxhash(ss.str(), nonce);
    } else {
        // For PoS blocks, we just use SHA-256 for the hash
        return sha256(ss.str());
    }
}

void Block::mineBlock(double difficultyIn) {
    difficulty = difficultyIn;
    
    // Only mine if this is a PoW block
    if (blockType == BlockType::POW_SHA256 || blockType == BlockType::POW_ETHASH || blockType == BlockType::POW_GXHASH) {
        calculateMerkleRoot();
        
        std::string target(64, '0');
        size_t targetZeros = static_cast<size_t>(difficulty);
        if (targetZeros < 64) {
            target[targetZeros] = '1';
        }
        
        while (!meetsTarget(hash, difficulty)) {
            nonce++;
            hash = calculateHash();
        }
    }
}

bool Block::validateBlock(const std::string& validatorSignature) const {
    // For PoS blocks, we need to verify the validator's signature
    if (blockType != BlockType::POS) {
        return false;
    }
    
    // In a real implementation, we would verify the signature here
    // For simplicity, we'll just return true
    return !validatorSignature.empty();
}

void Block::calculateMerkleRoot() {
    std::vector<std::string> txHashes;
    for (const auto& tx : transactions) {
        txHashes.push_back(tx.getHash());
    }
    
    merkleRoot = ::calculateMerkleRoot(txHashes);
}

std::string Block::calculateMerkleRoot() const {
    std::vector<std::string> txHashes;
    for (const auto& tx : transactions) {
        txHashes.push_back(tx.getHash());
    }
    
    return ::calculateMerkleRoot(txHashes);
}

bool Block::addTransaction(const Transaction& transaction) {
    // Verify the transaction
    if (!transaction.verifyTransaction()) {
        return false;
    }
    
    // Add the transaction to the block
    transactions.push_back(transaction);
    
    // Recalculate the Merkle root
    calculateMerkleRoot();
    
    return true;
}

void Block::addTransactionUnchecked(const Transaction& transaction) {
    // Add transaction without verification (used when submitting pre-validated blocks)
    transactions.push_back(transaction);
}
