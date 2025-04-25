#pragma once
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <cmath>
#include "transaction.h"
#include "pop_oracle.h"

class GXCCoinManager {
    static constexpr double MAX_SUPPLY = 31000000.0;
    static constexpr double INITIAL_BLOCK_REWARD = 50.0;
    static constexpr int HALVING_INTERVAL = 210000;
    
    std::unordered_map<std::string,double> balances;
    double currentSupply = 0.0;
    mutable std::mutex mtx;

public:
    double getBlockReward(size_t blockHeight) const {
        int halvings = blockHeight / HALVING_INTERVAL;
        return INITIAL_BLOCK_REWARD / std::pow(2, halvings);
    }

    bool canMintReward(double amount) const {
        std::lock_guard<std::mutex> lock(mtx);
        return currentSupply + amount <= MAX_SUPPLY;
    }

    void mintMiningReward(const std::string& minerAddr, double amount) {
        if (minerAddr.empty()) {
            throw std::invalid_argument("Miner address cannot be empty");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Reward amount must be positive");
        }

        std::lock_guard<std::mutex> lock(mtx);
        if (currentSupply + amount > MAX_SUPPLY) {
            throw std::runtime_error("Would exceed max supply");
        }

        balances[minerAddr] += amount;
        currentSupply += amount;
    }

    bool transfer(const std::string& from, 
                  const std::string& to, 
                  double amount) 
    {
        if (from.empty() || to.empty()) {
            throw std::invalid_argument("Addresses cannot be empty");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Amount must be positive");
        }

        std::lock_guard<std::mutex> lock(mtx);
        
        auto fromIt = balances.find(from);
        if (fromIt == balances.end() || fromIt->second < amount) {
            throw std::runtime_error("Insufficient GXC balance");
        }

        balances[from] -= amount;
        balances[to] += amount;
        return true;
    }

    double balanceOf(const std::string& addr) const {
        if (addr.empty()) {
            throw std::invalid_argument("Address cannot be empty");
        }

        std::lock_guard<std::mutex> lock(mtx);
        auto it = balances.find(addr);
        return it == balances.end() ? 0.0 : it->second;
    }

    double getTotalSupply() const {
        std::lock_guard<std::mutex> lock(mtx);
        return currentSupply;
    }

    double getMaxSupply() const {
        return MAX_SUPPLY;
    }
};