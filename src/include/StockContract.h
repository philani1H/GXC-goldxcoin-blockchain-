#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <ctime>

enum class ActionType {
    DIVIDEND,
    SPLIT,
    MERGE,
    DELISTING,
    SPINOFF,
    OTHER
};

struct CorporateAction {
    uint64_t id;
    ActionType actionType;
    std::time_t timestamp;
    double value;       // e.g., dividend amount per share, split ratio
    std::string proofHash;  // Reference to external proof
    std::string description;
    bool executed;
    std::time_t executedAt;
};

struct StockPrice {
    double price;       // Price in USD (scaled by 10^8 for precision)
    std::time_t timestamp;
    std::string popHash;    // PoP hash of the price data
    std::string source;     // Price source identifier
};

struct StockTransfer {
    std::string from;
    std::string to;
    uint64_t shares;
    std::string txHash;
    std::string prevTxHash;
    double priceAtTransfer;
    std::time_t timestamp;
};

class StockContract {
private:
    // Contract identity
    std::string contractAddress;
    std::string ticker;     // e.g., "AAPL", "MSFT"
    std::string companyName;
    std::string exchange;   // e.g., "NYSE", "NASDAQ"

    // Price feed
    std::string priceOracle;
    StockPrice currentPrice;
    std::vector<StockPrice> priceHistory;

    // Share tracking
    uint64_t totalShares;
    std::unordered_map<std::string, uint64_t> shareBalances;

    // Transaction chaining (similar to GXC-G)
    std::unordered_map<std::string, std::string> userLastStockTx;
    std::vector<StockTransfer> transferHistory;

    // Corporate actions
    std::vector<CorporateAction> corporateActions;
    std::unordered_map<uint64_t, bool> actionVotes;

    // Access control
    std::vector<std::string> authorizedIssuers;
    std::vector<std::string> authorizedExecutors;
    std::vector<std::string> authorizedOracles;

    // Compliance and regulation
    std::unordered_map<std::string, bool> transferRestrictions;
    std::unordered_map<std::string, bool> kycVerified;
    bool tradingHalted;

    // Metadata
    std::string sector;
    std::string industry;
    uint64_t marketCap;
    double dividendYield;

public:
    // Constructor
    StockContract();
    StockContract(const std::string& tickerIn, const std::string& companyNameIn,
                  const std::string& exchangeIn, const std::string& priceOracleIn);

    // Price management
    bool updatePrice(double newPrice, std::time_t timestamp, const std::string& popHash,
                    const std::string& source);
    StockPrice getCurrentPrice() const { return currentPrice; }
    std::vector<StockPrice> getPriceHistory(uint32_t days) const;
    bool isPriceDataFresh(uint32_t maxAgeSeconds = 3600) const;

    // Share management
    bool issueShares(const std::string& to, uint64_t shares);
    bool transferShares(const std::string& from, const std::string& to, uint64_t shares);
    bool burnShares(const std::string& from, uint64_t shares);

    // Corporate actions
    uint64_t proposeCorporateAction(ActionType actionType, double value,
                                   const std::string& proofHash, const std::string& description);
    bool executeCorporateAction(uint64_t actionId);
    bool voteCorporateAction(uint64_t actionId, const std::string& voter, bool approve);
    std::vector<CorporateAction> getPendingActions() const;

    // Dividend handling
    bool distributeDividend(double dividendPerShare, const std::string& proofHash);
    double calculateDividendPayment(const std::string& shareholder) const;
    bool claimDividend(const std::string& shareholder);

    // Stock splits and mergers
    bool executeSplit(double ratio);
    bool executeMerger(double exchangeRatio, const std::string& targetContract);

    // Access control
    void addAuthorizedIssuer(const std::string& issuer);
    void removeAuthorizedIssuer(const std::string& issuer);
    bool isAuthorizedIssuer(const std::string& issuer) const;

    void addAuthorizedExecutor(const std::string& executor);
    void removeAuthorizedExecutor(const std::string& executor);
    bool isAuthorizedExecutor(const std::string& executor) const;

    void addAuthorizedOracle(const std::string& oracle);
    void removeAuthorizedOracle(const std::string& oracle);
    bool isAuthorizedOracle(const std::string& oracle) const;

    // Compliance
    void setTransferRestriction(const std::string& address, bool restricted);
    bool hasTransferRestriction(const std::string& address) const;
    void setKYCStatus(const std::string& address, bool verified);
    bool isKYCVerified(const std::string& address) const;
    void haltTrading();
    void resumeTrading();
    bool isTradingHalted() const { return tradingHalted; }

    // Transaction chaining
    std::string getLastTransactionHash(const std::string& address) const;
    bool verifyTransactionChain(const std::string& address, uint32_t depth = 100) const;
    StockTransfer createChainedTransfer(const std::string& from, const std::string& to,
                                       uint64_t shares, double price);

    // Queries
    uint64_t balanceOf(const std::string& address) const;
    uint64_t getTotalShares() const { return totalShares; }
    double getMarketCap() const;
    double getDividendYield() const { return dividendYield; }
    std::vector<std::string> getShareholders() const;
    std::vector<StockTransfer> getTransferHistory(const std::string& address) const;

    // Market data
    double getVolume24h() const;
    double getPriceChange24h() const;
    double getAveragePrice(uint32_t days) const;
    double getVolatility(uint32_t days) const;

    // Getters
    std::string getContractAddress() const { return contractAddress; }
    std::string getTicker() const { return ticker; }
    std::string getCompanyName() const { return companyName; }
    std::string getExchange() const { return exchange; }
    std::string getPriceOracle() const { return priceOracle; }
    std::string getSector() const { return sector; }
    std::string getIndustry() const { return industry; }

    // Setters
    void setSector(const std::string& sectorIn) { sector = sectorIn; }
    void setIndustry(const std::string& industryIn) { industry = industryIn; }
    void setDividendYield(double yield) { dividendYield = yield; }

    // Events (for logging)
    void emitSharesIssued(const std::string& to, uint64_t shares, double price, const std::string& txHash);
    void emitSharesTransferred(const std::string& from, const std::string& to, uint64_t shares, const std::string& txHash);
    void emitPriceUpdated(double price, std::time_t timestamp, const std::string& popHash);
    void emitCorporateActionExecuted(uint64_t actionId, ActionType actionType);
    void emitDividendDistributed(double totalAmount, double perShare);

    // Validation
    bool isValidStockContract() const;
    bool isValidTransfer(const std::string& from, const std::string& to, uint64_t shares) const;

    // Serialization
    std::string serialize() const;
    static StockContract deserialize(const std::string& data);

    // Constants
    static const uint32_t MAX_PRICE_AGE_SECONDS = 3600; // 1 hour
    static constexpr double MAX_PRICE_DEVIATION = 0.15; // 15% max price change
    static const uint32_t MIN_VOTING_PERIOD_DAYS = 7;
    static const uint32_t MAX_VOTING_PERIOD_DAYS = 30;
};

// Stock Index Contract (for ETFs and custom indices)
class StockIndexContract {
private:
    std::string indexName;
    std::string indexSymbol;

    // Component stocks with their weights
    struct IndexComponent {
        std::string stockContract;
        double weight;     // Weight in basis points (e.g., 1000 = 10%)
    };

    std::vector<IndexComponent> components;

    // Index tracking
    double indexValue;
    std::time_t lastCalculation;
    std::vector<double> valueHistory;

    // Token tracking
    uint64_t totalSupply;
    std::unordered_map<std::string, uint64_t> balances;

    // Management
    std::vector<std::string> authorizedManagers;
    bool rebalancingEnabled;

public:
    // Constructor
    StockIndexContract(const std::string& nameIn, const std::string& symbolIn);

    // Component management
    bool addComponent(const std::string& stockContract, double weight);
    bool removeComponent(const std::string& stockContract);
    bool changeComponentWeight(const std::string& stockContract, double newWeight);
    std::vector<IndexComponent> getComponents() const { return components; }

    // Index calculation
    void calculateIndexValue();
    double getIndexValue() const { return indexValue; }
    double getTotalWeight() const;

    // Token operations
    bool mintIndexTokens(const std::string& to, uint64_t amount);
    bool burnIndexTokens(const std::string& from, uint64_t amount);
    bool transferIndexTokens(const std::string& from, const std::string& to, uint64_t amount);

    // Rebalancing
    void enableRebalancing();
    void disableRebalancing();
    bool rebalanceIndex();

    // Queries
    uint64_t balanceOf(const std::string& address) const;
    uint64_t getTotalSupply() const { return totalSupply; }
    double getPerformance(uint32_t days) const;

    // Management
    void addAuthorizedManager(const std::string& manager);
    void removeAuthorizedManager(const std::string& manager);
    bool isAuthorizedManager(const std::string& manager) const;

    // Getters
    std::string getIndexName() const { return indexName; }
    std::string getIndexSymbol() const { return indexSymbol; }
    std::time_t getLastCalculation() const { return lastCalculation; }

    // Validation
    bool isValidComponent(const std::string& stockContract) const;

    // Serialization
    std::string serialize() const;
    static StockIndexContract deserialize(const std::string& data);

    // Constants
    static constexpr double MAX_COMPONENT_WEIGHT = 0.30; // 30% max weight
    static const uint32_t MAX_COMPONENTS = 100;
    static const uint32_t REBALANCE_FREQUENCY_DAYS = 30;
};