#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <stdexcept>
#include "transaction.h"

class TransactionTracker {
    // address → last transaction hash
    std::unordered_map<std::string,std::string> lastTxHash;
    mutable std::mutex mtx; // Thread safety

public:
    Transaction create(const std::string& sender,
                      const std::string& receiver,
                      double amount)
    {
        if (sender.empty() || receiver.empty()) {
            throw std::invalid_argument("Sender and receiver addresses cannot be empty");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Transaction amount must be positive");
        }
        std::string prevHash;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = lastTxHash.find(sender);
            prevHash = (it != lastTxHash.end()) ? it->second : "0";

        try {
            Transaction tx(sender, receiver, amount, prevHash);
            {
                std::lock_guard<std::mutex> lock(mtx);
                lastTxHash[sender] = tx.txHash;
            }
            return tx;
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to create transaction: " + std::string(e.what()));
        }
    }

    std::string getLastTxHash(const std::string& addr) const {
        if (addr.empty()) {
            throw std::invalid_argument("Address cannot be empty");
        }
        std::lock_guard<std::mutex> lock(mtx);
        auto it = lastTxHash.find(addr);
        return (it == lastTxHash.end() ? "0" : it->second);
    }

    bool verifyTxChain(const std::string& addr) const {
        if (addr.empty()) {
            throw std::invalid_argument("Address cannot be empty");
        }

        std::lock_guard<std::mutex> lock(mtx);
        auto it = lastTxHash.find(addr);
        if (it == lastTxHash.end()) return true; // No transactions yet

        try {
            std::string currentHash = it->second;
            while (currentHash != "0") {
                // Verify transaction hash matches its contents
                // This would require access to full transaction history
                // Implementation depends on how transactions are stored
                currentHash = "0"; // Placeholder for actual verification
            }
            return true;
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to verify transaction chain: " + std::string(e.what()));
        }
    }
};
