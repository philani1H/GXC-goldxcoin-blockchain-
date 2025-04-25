#pragma once
#include <unordered_map>
#include <string>
#include "transaction.h"

class TransactionTracker {
    // address → last transaction hash
    std::unordered_map<std::string,std::string> lastTxHash;

public:
    Transaction create(const std::string& sender,
                      const std::string& receiver,
                      double amount)
    {
        // Get sender's previous transaction hash for chaining
        std::string prevHash = "0";
        auto it = lastTxHash.find(sender);
        if (it != lastTxHash.end()) {
            prevHash = it->second;
        }

        // Create new transaction with chained hash
        Transaction tx(sender, receiver, amount, prevHash);
        
        // Update sender's last transaction hash
        lastTxHash[sender] = tx.txHash;
        
        return tx;
    }

    std::string getLastTxHash(const std::string& addr) const {
        auto it = lastTxHash.find(addr);
        return (it == lastTxHash.end() ? "0" : it->second);
    }

    bool verifyTxChain(const std::string& addr) const {
        auto it = lastTxHash.find(addr);
        if (it == lastTxHash.end()) return true; // No transactions yet

        std::string currentHash = it->second;
        while (currentHash != "0") {
            // Verify transaction hash matches its contents
            // This would require access to full transaction history
            // Implementation depends on how transactions are stored
            currentHash = "0"; // Placeholder for actual verification
        }
        return true;
    }
};
