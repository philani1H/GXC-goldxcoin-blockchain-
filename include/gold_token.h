#pragma once
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <iomanip>

class GoldTokenManager {
    std::unordered_map<std::string,double> balanceG;
    mutable std::mutex mtx;
    const double MIN_TRANSACTION = 0.0001; // Minimum transaction amount in grams

public:
    void mint(const std::string& to, double grams) {
        if (to.empty()) {
            throw std::invalid_argument("Recipient address cannot be empty");
        }
        if (grams < MIN_TRANSACTION) {
            throw std::invalid_argument("Amount must be at least " + 
                std::to_string(MIN_TRANSACTION) + " grams");
        }

        std::lock_guard<std::mutex> lock(mtx);
        balanceG[to] += grams;
        
        std::cout << std::fixed << std::setprecision(4)
                  << "Minted " << grams << "g GXC-G to " << to << "\n";
    }

    void burn(const std::string& from, double grams) {
        if (from.empty()) {
            throw std::invalid_argument("Sender address cannot be empty");
        }
        if (grams < MIN_TRANSACTION) {
            throw std::invalid_argument("Amount must be at least " + 
                std::to_string(MIN_TRANSACTION) + " grams");
        }

        std::lock_guard<std::mutex> lock(mtx);
        
        auto it = balanceG.find(from);
        if (it == balanceG.end() || it->second < grams) {
            throw std::runtime_error("Insufficient GXC-G balance for " + from);
        }
        
        balanceG[from] -= grams;
        
        std::cout << std::fixed << std::setprecision(4)
                  << "Burned " << grams << "g GXC-G from " << from << "\n";
    }

    double balanceOf(const std::string& addr) const {
        if (addr.empty()) {
            throw std::invalid_argument("Address cannot be empty");
        }

        std::lock_guard<std::mutex> lock(mtx);
        auto it = balanceG.find(addr);
        return it == balanceG.end() ? 0.0 : it->second;
    }

    bool transfer(const std::string& from, const std::string& to, double grams) {
        if (from.empty() || to.empty()) {
            throw std::invalid_argument("Addresses cannot be empty");
        }
        if (grams < MIN_TRANSACTION) {
            throw std::invalid_argument("Amount must be at least " + 
                std::to_string(MIN_TRANSACTION) + " grams");
        }

        std::lock_guard<std::mutex> lock(mtx);
        
        auto fromIt = balanceG.find(from);
        if (fromIt == balanceG.end() || fromIt->second < grams) {
            throw std::runtime_error("Insufficient GXC-G balance for " + from);
        }

        balanceG[from] -= grams;
        balanceG[to] += grams;

        std::cout << std::fixed << std::setprecision(4)
                  << "Transferred " << grams << "g GXC-G from " 
                  << from << " to " << to << "\n";
        return true;
    }
};
