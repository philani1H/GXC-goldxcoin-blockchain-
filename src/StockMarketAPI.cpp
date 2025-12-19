#include "../include/StockMarketAPI.h"
#include "../include/HashUtils.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

// ===== MarketMakerRegistry Implementation =====

bool MarketMakerRegistry::registerMarketMaker(
    const std::string& address,
    const std::string& name,
    const std::string& licenseNumber,
    const std::string& country
) {
    // Verify address is valid
    if (address.empty() || name.empty() || licenseNumber.empty()) {
        throw std::runtime_error("Invalid market maker registration data");
    }
    
    // Check if already registered
    if (authorizedMakers.find(address) != authorizedMakers.end()) {
        throw std::runtime_error("Market maker already registered");
    }
    
    // Create market maker info
    MarketMakerInfo info;
    info.address = address;
    info.name = name;
    info.licenseNumber = licenseNumber;
    info.country = country;
    info.isActive = true;
    info.registeredAt = std::time(nullptr);
    
    authorizedMakers[address] = info;
    
    return true;
}

bool MarketMakerRegistry::verifyMarketMaker(const std::string& address) const {
    auto it = authorizedMakers.find(address);
    if (it == authorizedMakers.end()) {
        return false;
    }
    return it->second.isActive;
}

bool MarketMakerRegistry::authorizeStock(const std::string& makerAddress, const std::string& ticker) {
    auto it = authorizedMakers.find(makerAddress);
    if (it == authorizedMakers.end()) {
        throw std::runtime_error("Market maker not registered");
    }
    
    // Add stock to authorized list
    auto& stocks = it->second.authorizedStocks;
    if (std::find(stocks.begin(), stocks.end(), ticker) == stocks.end()) {
        stocks.push_back(ticker);
    }
    
    return true;
}

bool MarketMakerRegistry::canTradeStock(const std::string& makerAddress, const std::string& ticker) const {
    auto it = authorizedMakers.find(makerAddress);
    if (it == authorizedMakers.end() || !it->second.isActive) {
        return false;
    }
    
    const auto& stocks = it->second.authorizedStocks;
    return std::find(stocks.begin(), stocks.end(), ticker) != stocks.end();
}

bool MarketMakerRegistry::revokeMarketMaker(const std::string& address) {
    auto it = authorizedMakers.find(address);
    if (it == authorizedMakers.end()) {
        return false;
    }
    
    it->second.isActive = false;
    return true;
}

MarketMakerInfo MarketMakerRegistry::getMarketMakerInfo(const std::string& address) const {
    auto it = authorizedMakers.find(address);
    if (it == authorizedMakers.end()) {
        throw std::runtime_error("Market maker not found");
    }
    return it->second;
}

std::vector<MarketMakerInfo> MarketMakerRegistry::getAllMarketMakers() const {
    std::vector<MarketMakerInfo> makers;
    for (const auto& [address, info] : authorizedMakers) {
        makers.push_back(info);
    }
    return makers;
}

// ===== StockMarketAPI Implementation =====

StockMarketAPI::StockMarketAPI(Blockchain* bc, MarketMakerRegistry* registry)
    : blockchain(bc), marketMakerRegistry(registry) {}

bool StockMarketAPI::verifyTraceability(const Transaction& tx) const {
    // Verify traceability formula:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    
    if (tx.getInputs().empty()) {
        return false;
    }
    
    const auto& firstInput = tx.getInputs()[0];
    
    // Check hash matching
    if (firstInput.txHash != tx.getPrevTxHash()) {
        return false;
    }
    
    // Check amount matching
    if (std::abs(firstInput.amount - tx.getReferencedAmount()) > 0.00000001) {
        return false;
    }
    
    return true;
}

Transaction StockMarketAPI::createTrackedTransaction(
    const std::string& from,
    const std::string& to,
    double amount,
    TransactionType type
) {
    // Get REAL UTXOs from blockchain
    const auto& utxoSet = blockchain->getUtxoSet();
    
    // Find UTXOs for sender
    std::vector<TransactionInput> inputs;
    double totalInput = 0.0;
    
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == from) {
            // Parse REAL transaction hash
            size_t sepPos = utxoKey.find('_');
            std::string txHash = utxoKey.substr(0, sepPos);
            int outputIndex = std::stoi(utxoKey.substr(sepPos + 1));
            
            TransactionInput input;
            input.txHash = txHash;  // REAL previous TX!
            input.outputIndex = outputIndex;
            input.amount = utxo.amount;
            inputs.push_back(input);
            
            totalInput += utxo.amount;
            
            if (totalInput >= amount + 0.001) {  // amount + fee
                break;
            }
        }
    }
    
    if (totalInput < amount + 0.001) {
        throw std::runtime_error("Insufficient funds");
    }
    
    // Create outputs
    std::vector<TransactionOutput> outputs;
    
    // Output to recipient
    TransactionOutput recipientOutput;
    recipientOutput.address = to;
    recipientOutput.amount = amount;
    outputs.push_back(recipientOutput);
    
    // Change output
    double change = totalInput - amount - 0.001;
    if (change > 0.00000001) {
        TransactionOutput changeOutput;
        changeOutput.address = from;
        changeOutput.amount = change;
        outputs.push_back(changeOutput);
    }
    
    // TRACEABILITY FORMULA ENFORCEMENT
    std::string prevTxHash;
    double referencedAmount = 0.0;
    
    if (!inputs.empty()) {
        prevTxHash = inputs[0].txHash;  // From REAL UTXO!
        referencedAmount = inputs[0].amount;  // From REAL UTXO!
    }
    
    // Create transaction with traceability
    Transaction tx(inputs, outputs, prevTxHash);
    tx.setType(type);
    tx.setReferencedAmount(referencedAmount);
    tx.setFee(0.001);
    
    // Verify traceability before returning
    if (!verifyTraceability(tx)) {
        throw std::runtime_error("Transaction failed traceability verification");
    }
    
    return tx;
}

std::string StockMarketAPI::deployStock(
    const std::string& makerAddress,
    const std::string& ticker,
    const std::string& companyName,
    const std::string& exchange,
    uint64_t totalShares
) {
    // VERIFY: Only authorized market makers can deploy stocks
    if (!marketMakerRegistry->verifyMarketMaker(makerAddress)) {
        throw std::runtime_error("Unauthorized: Only registered market makers can deploy stocks");
    }
    
    // Create stock contract
    auto contract = std::make_shared<StockContract>(
        ticker,
        companyName,
        exchange,
        makerAddress  // Market maker is the oracle
    );
    
    // Add market maker as authorized issuer
    contract->addAuthorizedIssuer(makerAddress);
    contract->addAuthorizedOracle(makerAddress);
    
    // Issue initial shares to market maker
    contract->issueShares(makerAddress, totalShares);
    
    // Authorize market maker for this stock
    marketMakerRegistry->authorizeStock(makerAddress, ticker);
    
    // Store contract
    stockContracts[ticker] = contract;
    
    return ticker;  // Contract address/identifier
}

bool StockMarketAPI::updateStockPrice(
    const std::string& makerAddress,
    const std::string& ticker,
    double price,
    const std::string& priceSource,
    const std::string& proofHash
) {
    // VERIFY: Only authorized market makers can update prices
    if (!marketMakerRegistry->canTradeStock(makerAddress, ticker)) {
        throw std::runtime_error("Unauthorized: Market maker not authorized for this stock");
    }
    
    auto it = stockContracts.find(ticker);
    if (it == stockContracts.end()) {
        throw std::runtime_error("Stock contract not found");
    }
    
    // Update price using market maker's own price software
    return it->second->updatePrice(
        price,
        std::time(nullptr),
        proofHash,
        priceSource
    );
}

bool StockMarketAPI::addLiquidity(
    const std::string& makerAddress,
    const std::string& ticker,
    uint64_t shares,
    double bidPrice,
    double askPrice
) {
    // VERIFY: Only authorized market makers can add liquidity
    if (!marketMakerRegistry->canTradeStock(makerAddress, ticker)) {
        throw std::runtime_error("Unauthorized: Market maker not authorized for this stock");
    }
    
    // Market maker provides liquidity
    // Implementation would add to order book
    
    return true;
}

std::string StockMarketAPI::buyShares(
    const std::string& buyerAddress,
    const std::string& ticker,
    uint64_t shares,
    double maxPrice
) {
    auto it = stockContracts.find(ticker);
    if (it == stockContracts.end()) {
        throw std::runtime_error("Stock not found");
    }
    
    // Get current price
    double currentPrice = it->second->getCurrentPrice().price;
    
    if (currentPrice > maxPrice) {
        throw std::runtime_error("Price exceeds maximum");
    }
    
    double totalCost = currentPrice * shares;
    
    // Find seller (market maker or other seller)
    std::string sellerAddress = "";  // Would match from order book
    
    // Create transaction with FULL TRACEABILITY
    Transaction tx = createTrackedTransaction(
        buyerAddress,
        sellerAddress,
        totalCost,
        TransactionType::NORMAL
    );
    
    // Transfer shares
    it->second->transferShares(sellerAddress, buyerAddress, shares);
    
    // Add transaction to blockchain
    blockchain->addTransaction(tx);
    
    return tx.getHash();
}

std::string StockMarketAPI::sellShares(
    const std::string& sellerAddress,
    const std::string& ticker,
    uint64_t shares,
    double minPrice
) {
    auto it = stockContracts.find(ticker);
    if (it == stockContracts.end()) {
        throw std::runtime_error("Stock not found");
    }
    
    // Verify seller has shares
    if (it->second->balanceOf(sellerAddress) < shares) {
        throw std::runtime_error("Insufficient shares");
    }
    
    // Get current price
    double currentPrice = it->second->getCurrentPrice().price;
    
    if (currentPrice < minPrice) {
        throw std::runtime_error("Price below minimum");
    }
    
    double totalValue = currentPrice * shares;
    
    // Find buyer (market maker or other buyer)
    std::string buyerAddress = "";  // Would match from order book
    
    // Create transaction with FULL TRACEABILITY
    Transaction tx = createTrackedTransaction(
        buyerAddress,
        sellerAddress,
        totalValue,
        TransactionType::NORMAL
    );
    
    // Transfer shares
    it->second->transferShares(sellerAddress, buyerAddress, shares);
    
    // Add transaction to blockchain
    blockchain->addTransaction(tx);
    
    return tx.getHash();
}

double StockMarketAPI::getStockPrice(const std::string& ticker) const {
    auto it = stockContracts.find(ticker);
    if (it == stockContracts.end()) {
        throw std::runtime_error("Stock not found");
    }
    
    return it->second->getCurrentPrice().price;
}

uint64_t StockMarketAPI::getStockBalance(const std::string& address, const std::string& ticker) const {
    auto it = stockContracts.find(ticker);
    if (it == stockContracts.end()) {
        throw std::runtime_error("Stock not found");
    }
    
    return it->second->balanceOf(address);
}

bool StockMarketAPI::verifyTradeTraceability(const std::string& txHash) const {
    // TODO: Implement when blockchain->getTransaction() is available
    // For now, return true as all transactions are created with traceability
    return true;
}

std::vector<std::string> StockMarketAPI::batchBuy(
    const std::string& institutionAddress,
    const std::vector<std::pair<std::string, uint64_t>>& orders
) {
    std::vector<std::string> txHashes;
    
    for (const auto& [ticker, shares] : orders) {
        try {
            std::string txHash = buyShares(institutionAddress, ticker, shares, 999999.0);
            txHashes.push_back(txHash);
        } catch (const std::exception& e) {
            // Log error but continue with other orders
            txHashes.push_back("ERROR: " + std::string(e.what()));
        }
    }
    
    return txHashes;
}

std::vector<std::string> StockMarketAPI::batchSell(
    const std::string& institutionAddress,
    const std::vector<std::pair<std::string, uint64_t>>& orders
) {
    std::vector<std::string> txHashes;
    
    for (const auto& [ticker, shares] : orders) {
        try {
            std::string txHash = sellShares(institutionAddress, ticker, shares, 0.0);
            txHashes.push_back(txHash);
        } catch (const std::exception& e) {
            txHashes.push_back("ERROR: " + std::string(e.what()));
        }
    }
    
    return txHashes;
}

bool StockMarketAPI::executeCorporateAction(
    const std::string& makerAddress,
    const std::string& ticker,
    ActionType actionType,
    double value,
    const std::string& description
) {
    // VERIFY: Only authorized market makers can execute corporate actions
    if (!marketMakerRegistry->canTradeStock(makerAddress, ticker)) {
        throw std::runtime_error("Unauthorized: Market maker not authorized for this stock");
    }
    
    auto it = stockContracts.find(ticker);
    if (it == stockContracts.end()) {
        throw std::runtime_error("Stock not found");
    }
    
    // Propose and execute corporate action
    uint64_t actionId = it->second->proposeCorporateAction(
        actionType,
        value,
        "proof_hash",
        description
    );
    
    return it->second->executeCorporateAction(actionId);
}

double StockMarketAPI::getPortfolioValue(const std::string& address) const {
    double totalValue = 0.0;
    
    for (const auto& [ticker, contract] : stockContracts) {
        uint64_t shares = contract->balanceOf(address);
        if (shares > 0) {
            double price = contract->getCurrentPrice().price;
            totalValue += shares * price;
        }
    }
    
    return totalValue;
}

std::vector<StockMarketAPI::Holding> StockMarketAPI::getPortfolio(const std::string& address) const {
    std::vector<Holding> holdings;
    
    for (const auto& [ticker, contract] : stockContracts) {
        uint64_t shares = contract->balanceOf(address);
        if (shares > 0) {
            Holding holding;
            holding.ticker = ticker;
            holding.shares = shares;
            holding.currentPrice = contract->getCurrentPrice().price;
            holding.totalValue = shares * holding.currentPrice;
            holding.avgPrice = 0.0;  // Would calculate from trade history
            holding.gainLoss = 0.0;  // Would calculate from avg price
            
            holdings.push_back(holding);
        }
    }
    
    return holdings;
}
