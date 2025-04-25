#pragma once
#include <unordered_map>
#include <iostream>

class GoldTokenManager {
    std::unordered_map<std::string,double> balanceG;

public:
    void mint(const std::string& to, double grams) {
        balanceG[to] += grams;
        std::cout << "Minted " << grams << "g GXC-G to " << to << "\n";
    }

    void burn(const std::string& from, double grams) {
        if (balanceG[from] < grams) {
            std::cerr << "Insufficient GXC-G balance for " << from << "\n";
            return;
        }
        balanceG[from] -= grams;
        std::cout << "Burned " << grams << "g GXC-G from " << from << "\n";
    }

    double balanceOf(const std::string& addr) const {
        auto it = balanceG.find(addr);
        return it == balanceG.end() ? 0.0 : it->second;
    }
};
