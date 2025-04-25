#pragma once
#include <vector>
#include <mutex>
#include <chrono>
#include <stdexcept>
#include "block.h"
#include "gxc_coin.h"

class Blockchain {
    std::vector<Block> chain;
    unsigned int difficulty;
    std::string lastTxHash;  // for linear chaining of transactions
    mutable std::mutex mtx;
    const int BLOCK_TIME_TARGET = 10;  // seconds
    const int DIFFICULTY_ADJUSTMENT_INTERVAL = 10;  // blocks
    GXCCoinManager gxcManager;

    Block createGenesis() {
        Block genesis(0, "0", {});
        double reward = gxcManager.getBlockReward(0);
        if (gxcManager.canMintReward(reward)) {
            genesis.minerAddress = "genesis";
            genesis.miningReward = reward;
            gxcManager.mintMiningReward(genesis.minerAddress, reward);
        }
        return genesis;
    }

public:
    Blockchain(unsigned int diff = 4)
      : difficulty(diff)
    {
        chain.push_back(createGenesis());
        lastTxHash = chain[0].hash;
    }

    void adjustDifficulty() {
        if (chain.size() <= DIFFICULTY_ADJUSTMENT_INTERVAL) return;
        
        size_t lastAdjustmentIndex = chain.size() - DIFFICULTY_ADJUSTMENT_INTERVAL;
        auto timeExpected = BLOCK_TIME_TARGET * DIFFICULTY_ADJUSTMENT_INTERVAL;
        auto timeTaken = chain.back().timestamp - chain[lastAdjustmentIndex].timestamp;
        
        if (timeTaken < timeExpected / 2) difficulty++;
        else if (timeTaken > timeExpected * 2) difficulty = std::max(1u, difficulty - 1);
    }

    void addBlock(const std::vector<Transaction>& txs, const std::string& minerAddr) {
        std::lock_guard<std::mutex> lock(mtx);
        
        if (chain.empty()) {
            throw std::runtime_error("Blockchain not initialized");
        }
        if (minerAddr.empty()) {
            throw std::invalid_argument("Miner address cannot be empty");
        }

        double reward = gxcManager.getBlockReward(chain.size());
        Block b(chain.size(), chain.back().hash, txs, minerAddr, reward);
        b.mine(difficulty);
        
        // Verify block before adding
        if (!verifyBlock(b)) {
            throw std::runtime_error("Invalid block");
        }

        // Mint mining reward if within supply cap
        if (gxcManager.canMintReward(reward)) {
            gxcManager.mintMiningReward(minerAddr, reward);
        }

        // Process transactions
        for (const auto& tx : txs) {
            gxcManager.transfer(tx.sender, tx.receiver, tx.amount);
        }

        chain.push_back(b);
        if (!txs.empty()) {
            lastTxHash = txs.back().txHash;
        }

        adjustDifficulty();
    }

    bool verifyBlock(const Block& block) const {
        if (block.index != chain.size()) {
            return false;
        }
        if (block.previousHash != chain.back().hash) {
            return false;
        }
        if (block.hash != block.calculateHash()) {
            return false;
        }
        std::string prefix(difficulty, '0');
        if (block.hash.substr(0, difficulty) != prefix) {
            return false;
        }
        return true;
    }

    bool isValid() const {
        std::lock_guard<std::mutex> lock(mtx);
        
        if (chain.empty()) return false;
        
        for (size_t i = 1; i < chain.size(); ++i) {
            const Block &curr = chain[i], &prev = chain[i-1];
            
            if (curr.index != i) return false;
            if (curr.previousHash != prev.hash) return false;
            if (curr.hash != curr.calculateHash()) return false;
            
            std::string prefix(difficulty, '0');
            if (curr.hash.substr(0, difficulty) != prefix) return false;
        }
        return true;
    }

    std::string getLastTxHash() const {
        std::lock_guard<std::mutex> lock(mtx);
        return lastTxHash;
    }

    size_t getHeight() const {
        std::lock_guard<std::mutex> lock(mtx);
        return chain.size();
    }

    unsigned int getDifficulty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return difficulty;
    }

    Block getLastBlock() const {
        std::lock_guard<std::mutex> lock(mtx);
        if (chain.empty()) {
            throw std::runtime_error("Blockchain is empty");
        }
        return chain.back();
    }
};
