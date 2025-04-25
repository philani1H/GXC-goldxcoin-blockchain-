#pragma once
#include <vector>
#include "block.h"

class Blockchain {
    std::vector<Block> chain;
    unsigned int difficulty;
    std::string lastTxHash;  // for linear chaining of transactions

    Block createGenesis() {
        return Block(0, "0", {});
    }

public:
    Blockchain(unsigned int diff = 4)
      : difficulty(diff)
    {
        chain.push_back(createGenesis());
        lastTxHash = chain[0].hash;
    }

    void addBlock(const std::vector<Transaction>& txs) {
        Block b(chain.size(), chain.back().hash, txs);
        b.mine(difficulty);
        chain.push_back(b);
        if (!txs.empty())
            lastTxHash = txs.back().txHash;
    }

    bool isValid() const {
        for (size_t i = 1; i < chain.size(); ++i) {
            const Block &curr = chain[i], &prev = chain[i-1];
            if (curr.hash != curr.calculateHash() ||
                curr.previousHash != prev.hash)
            {
                return false;
            }
        }
        return true;
    }

    std::string getLastTxHash() const {
        return lastTxHash;
    }
};
