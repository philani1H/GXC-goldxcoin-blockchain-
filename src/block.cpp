#include "../include/Block.h"
#include "../include/HashUtils.h"
#include <sstream>

Block::Block(uint32_t indexIn, const std::string& previousHashIn, BlockType type) 
    : index(indexIn), previousHash(previousHashIn), blockType(type), nonce(0), difficulty(0) {
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
    } else {
        // For PoS blocks, we just use SHA-256 for the hash
        return sha256(ss.str());
    }
}

void Block::mineBlock(double difficultyIn) {
    difficulty = difficultyIn;
    
    // Only mine if this is a PoW block
    if (blockType == BlockType::POW_SHA256 || blockType == BlockType::POW_ETHASH) {
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
