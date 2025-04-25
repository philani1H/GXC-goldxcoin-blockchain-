#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include "transaction.h"

class Block {
public:
    int index;
    std::time_t timestamp;
    std::vector<Transaction> txs;
    std::string previousHash;
    std::string hash;
    long nonce;

    Block(int idx,
          const std::string& prevHash,
          const std::vector<Transaction>& transactions)
      : index(idx), previousHash(prevHash), txs(transactions), nonce(0)
    {
        timestamp = std::time(nullptr);
        hash = calculateHash();
    }

    std::string calculateHash() const {
        std::ostringstream ss;
        ss << index << timestamp << previousHash << nonce;
        for (auto &tx : txs) ss << tx.txHash;
        return sha256(ss.str());
    }

    void mine(unsigned int difficulty) {
        std::string prefix(difficulty, '0');
        do {
            nonce++;
            hash = calculateHash();
        } while (hash.substr(0, difficulty) != prefix);
        std::cout << "Mined block " << index << ": " << hash << "\n";
    }
};
