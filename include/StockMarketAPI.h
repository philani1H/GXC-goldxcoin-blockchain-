#pragma once

#include "StockContract.h"
#include "blockchain.h"
#include "Wallet.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/**
 * Market Maker Authorization System
 * Only authorized market makers can deploy stocks and set prices
 */
class MarketMakerRegistry {
private:
    std::unordered_map<std::string, MarketMakerInfo> authorizedMakers;
    Blockchain* blockchain;
    
public:
    struct MarketMakerInfo {
        std::string address;
        std::string name;
        std::string licenseNumber;
        std::string country;
        bool isActive;
        std::time_t registeredAt;
        std::vector<std::string> authorizedStocks;
    };
    
    MarketMakerRegistry(Blockchain* bc) : blockchain(bc) {}
    
    // Register market maker (requires verification)
    bool registerMarketMaker(
        const std::string& address,
        const std::string& name,
        const std::string& licenseNumber,
        const std::string& country
    );
    
    // Verify market maker credentials
    bool verifyMarketMaker(const std::string& address) const;
    
    // Authorize market maker for specific stock
    bool authorizeStock(const std::string& makerAddress, const std::string& ticker);
    
    // Check if market maker can trade stock
    bool canTradeStock(const std::string& makerAddress, const std::string& ticker) const;
    
    // Revoke market maker authorization
    bool revokeMarketMaker(const std::string& address);
    
    // Get market maker info
    MarketMakerInfo getMarketMakerInfo(const std::string& address) const;
    
    // List all authorized market makers
    std::vector<MarketMakerInfo> getAllMarketMakers() const;
};

/**
 * Stock Market API
 * Handles all stock trading operations with traceability
 */
class StockMarketAPI {
private:
    Blockchain* blockchain;
    MarketMakerRegistry* marketMakerRegistry;
    std::unordered_map<std::string, std::shared_ptr<StockContract>> stockContracts;
    
    // Verify transaction follows traceability formula
    bool verifyTraceability(const Transaction& tx) const;
    
    // Create transaction with traceability
    Transaction createTrackedTransaction(
        const std::string& from,
        const std::string& to,
        double amount,
        TransactionType type
    );
    
public:
    StockMarketAPI(Blockchain* bc, MarketMakerRegistry* registry);
    
    // ===== MARKET MAKER APIs =====
    
    /**
     * Deploy new stock contract (Market Makers only)
     * Requires: Market maker authorization
     * Returns: Contract address
     */
    std::string deployStock(
        const std::string& makerAddress,
        const std::string& ticker,
        const std::string& companyName,
        const std::string& exchange,
        uint64_t totalShares
    );
    
    /**
     * Update stock price (Market Makers only)
     * Requires: Market maker authorization for this stock
     * Uses: Market maker's own price software/oracle
     */
    bool updateStockPrice(
        const std::string& makerAddress,
        const std::string& ticker,
        double price,
        const std::string& priceSource,
        const std::string& proofHash
    );
    
    /**
     * Add liquidity (Market Makers only)
     * Market makers provide buy/sell liquidity
     */
    bool addLiquidity(
        const std::string& makerAddress,
        const std::string& ticker,
        uint64_t shares,
        double bidPrice,
        double askPrice
    );
    
    /**
     * Remove liquidity (Market Makers only)
     */
    bool removeLiquidity(
        const std::string& makerAddress,
        const std::string& ticker,
        uint64_t shares
    );
    
    // ===== BUYER/SELLER APIs =====
    
    /**
     * Buy stock shares
     * Creates transaction with full traceability
     * Follows formula: Ti.Inputs[0].txHash == Ti.PrevTxHash
     */
    std::string buyShares(
        const std::string& buyerAddress,
        const std::string& ticker,
        uint64_t shares,
        double maxPrice
    );
    
    /**
     * Sell stock shares
     * Creates transaction with full traceability
     */
    std::string sellShares(
        const std::string& sellerAddress,
        const std::string& ticker,
        uint64_t shares,
        double minPrice
    );
    
    /**
     * Place limit order
     */
    std::string placeLimitOrder(
        const std::string& address,
        const std::string& ticker,
        uint64_t shares,
        double price,
        bool isBuy
    );
    
    /**
     * Place market order
     */
    std::string placeMarketOrder(
        const std::string& address,
        const std::string& ticker,
        uint64_t shares,
        bool isBuy
    );
    
    /**
     * Cancel order
     */
    bool cancelOrder(const std::string& address, const std::string& orderId);
    
    // ===== QUERY APIs =====
    
    /**
     * Get stock price
     */
    double getStockPrice(const std::string& ticker) const;
    
    /**
     * Get order book
     */
    struct OrderBook {
        std::vector<std::pair<double, uint64_t>> bids;  // price, quantity
        std::vector<std::pair<double, uint64_t>> asks;  // price, quantity
    };
    OrderBook getOrderBook(const std::string& ticker) const;
    
    /**
     * Get user's stock balance
     */
    uint64_t getStockBalance(const std::string& address, const std::string& ticker) const;
    
    /**
     * Get user's orders
     */
    struct Order {
        std::string orderId;
        std::string ticker;
        uint64_t shares;
        double price;
        bool isBuy;
        std::time_t timestamp;
        std::string status;
    };
    std::vector<Order> getUserOrders(const std::string& address) const;
    
    /**
     * Get trade history
     */
    struct Trade {
        std::string txHash;
        std::string ticker;
        std::string buyer;
        std::string seller;
        uint64_t shares;
        double price;
        std::time_t timestamp;
        std::string prevTxHash;  // Traceability!
    };
    std::vector<Trade> getTradeHistory(const std::string& ticker, uint32_t limit) const;
    
    /**
     * Verify transaction traceability
     */
    bool verifyTradeTraceability(const std::string& txHash) const;
    
    // ===== FINANCIAL INSTITUTION APIs =====
    
    /**
     * Batch buy for institutional investors
     */
    std::vector<std::string> batchBuy(
        const std::string& institutionAddress,
        const std::vector<std::pair<std::string, uint64_t>>& orders  // ticker, shares
    );
    
    /**
     * Batch sell for institutional investors
     */
    std::vector<std::string> batchSell(
        const std::string& institutionAddress,
        const std::vector<std::pair<std::string, uint64_t>>& orders
    );
    
    /**
     * Execute corporate action (dividends, splits)
     */
    bool executeCorporateAction(
        const std::string& makerAddress,
        const std::string& ticker,
        ActionType actionType,
        double value,
        const std::string& description
    );
    
    /**
     * Get portfolio value
     */
    double getPortfolioValue(const std::string& address) const;
    
    /**
     * Get portfolio holdings
     */
    struct Holding {
        std::string ticker;
        uint64_t shares;
        double avgPrice;
        double currentPrice;
        double totalValue;
        double gainLoss;
    };
    std::vector<Holding> getPortfolio(const std::string& address) const;
};

/**
 * Stock Market RPC API
 * JSON-RPC interface for stock trading
 */
class StockMarketRPC {
private:
    StockMarketAPI* api;
    MarketMakerRegistry* registry;
    
public:
    StockMarketRPC(StockMarketAPI* api, MarketMakerRegistry* registry);
    
    // Handle JSON-RPC requests
    std::string handleRequest(const std::string& jsonRequest);
    
    // Market Maker endpoints
    std::string registerMarketMaker(const std::string& params);
    std::string deployStock(const std::string& params);
    std::string updatePrice(const std::string& params);
    std::string addLiquidity(const std::string& params);
    
    // Trading endpoints
    std::string buyShares(const std::string& params);
    std::string sellShares(const std::string& params);
    std::string placeLimitOrder(const std::string& params);
    std::string placeMarketOrder(const std::string& params);
    std::string cancelOrder(const std::string& params);
    
    // Query endpoints
    std::string getPrice(const std::string& params);
    std::string getOrderBook(const std::string& params);
    std::string getBalance(const std::string& params);
    std::string getOrders(const std::string& params);
    std::string getTradeHistory(const std::string& params);
    std::string verifyTraceability(const std::string& params);
    
    // Institutional endpoints
    std::string batchBuy(const std::string& params);
    std::string batchSell(const std::string& params);
    std::string executeCorporateAction(const std::string& params);
    std::string getPortfolio(const std::string& params);
};
