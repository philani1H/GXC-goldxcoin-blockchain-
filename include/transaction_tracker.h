#pragma once
#include <unordered_map>
#include "transaction.h"

class TransactionTracker {
    // address → last transaction hash
    std::unordered_map<std::string,std::string> lastHash;

public:
    Transaction create(const std::string& sender,
                       const std::string& receiver,
                       double amount)
    {
        std::string prev = "";
        auto it = lastHash.find(sender);
        if (it != lastHash.end()) prev = it->second;

        Transaction tx(sender, receiver, amount, prev);
        lastHash[sender] = tx.txHash;
        return tx;
    }

    std::string getLastHash(const std::string& addr) const {
        auto it = lastHash.find(addr);
        return (it == lastHash.end() ? "" : it->second);
    }
};
