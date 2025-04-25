#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include "transaction.h"
#include "gxc_coin.h"

class Block {
public:
    int index;
    std::time_t timestamp;
    std::vector<Transaction> txs;
    std::string previousHash;
    std::string hash;
    long nonce;
    double totalAmount;  // Cache for transaction total
    std::string minerAddress;
    double miningReward;

    Block(int idx,
          const std::string& prevHash,
          const std::vector<Transaction>& transactions,
          const std::string& miner = "",
          double reward = 0.0)
      : index(idx), previousHash(prevHash), txs(transactions), nonce(0),
        minerAddress(miner), miningReward(reward)
    {
        if (prevHash.empty()) {
            throw std::invalid_argument("Previous hash cannot be empty");
        }
        
        validateTransactions();
        calculateTotalAmount();
        
        timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        hash = calculateHash();
    }

    void validateTransactions() {
        if (txs.empty()) return;
        
        std::unordered_set<std::string> seenTxs;
        for (const auto& tx : txs) {
            if (tx.amount <= 0) {
                throw std::invalid_argument("Transaction amount must be positive");
            }
            if (seenTxs.find(tx.txHash) != seenTxs.end()) {
                throw std::invalid_argument("Duplicate transaction detected");
            }
            seenTxs.insert(tx.txHash);
        }
    }

    void calculateTotalAmount() {
        totalAmount = std::accumulate(txs.begin(), txs.end(), 0.0,
            [](double sum, const Transaction& tx) { return sum + tx.amount; }
        );
    }

    std::string calculateHash() const {
        std::ostringstream ss;
        ss << index << timestamp << previousHash << nonce << totalAmount;
        for (const auto& tx : txs) {
            ss << tx.txHash;
        }
        return sha256(ss.str());
    }

    void mine(unsigned int difficulty) {
        if (difficulty == 0) {
            throw std::invalid_argument("Mining difficulty cannot be zero");
        }

        std::string prefix(difficulty, '0');
        const unsigned int maxNonce = std::numeric_limits<unsigned int>::max();
        
        auto startTime = std::chrono::steady_clock::now();
        
        while (nonce < maxNonce) {
            nonce++;
            hash = calculateHash();
            
            if (hash.substr(0, difficulty) == prefix) {
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
                std::cout << "Block " << index << " mined in " << duration.count() 
                          << " seconds with nonce " << nonce << "\n";
                return;
            }
        }
        throw std::runtime_error("Failed to mine block: nonce overflow");
    }
};
