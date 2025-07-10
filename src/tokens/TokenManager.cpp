#include <unordered_map>
#include <string>
#include <memory>
#include "GoldToken.h"
#include "StockContract.h"

class TokenManager {
public:
    void addGoldToken(const std::string& id, const GoldToken& token) { goldTokens[id] = token; }
    void addStockContract(const std::string& id, const StockContract& contract) { stockContracts[id] = contract; }
    GoldToken getGoldToken(const std::string& id) const { return goldTokens.at(id); }
    StockContract getStockContract(const std::string& id) const { return stockContracts.at(id); }
    void removeGoldToken(const std::string& id) { goldTokens.erase(id); }
    void removeStockContract(const std::string& id) { stockContracts.erase(id); }
private:
    std::unordered_map<std::string, GoldToken> goldTokens;
    std::unordered_map<std::string, StockContract> stockContracts;
};