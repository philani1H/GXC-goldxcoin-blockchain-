# Stock Contract Deployment Guide

## Complete Guide to Deploying and Managing Stock Contracts on GXC

---

## Quick Start

### 1. Deploy a Stock Contract

```cpp
#include "StockContract.h"

// Create contract
StockContract appleStock(
    "AAPL",                      // Ticker symbol
    "Apple Inc.",                // Company name
    "NASDAQ",                    // Exchange
    "chainlink_oracle_address"   // Price oracle
);

// Configure
appleStock.setSector("Technology");
appleStock.setIndustry("Consumer Electronics");
appleStock.addAuthorizedIssuer("your_institution_address");

// Issue shares
appleStock.issueShares("your_address", 1000000);
```

### 2. Update Stock Price

```cpp
// Update from price oracle
appleStock.updatePrice(
    150.50,                    // Price in USD
    std::time(nullptr),        // Current timestamp
    "pop_hash_abc123...",      // Proof of Price hash
    "NASDAQ"                   // Source
);
```

### 3. Transfer Shares

```cpp
// Transfer shares between addresses
appleStock.transferShares(
    "from_address",
    "to_address",
    100                        // Number of shares
);
```

---

## Complete Examples

### Example 1: Technology Stock (Apple)

```cpp
// Deploy Apple stock contract
StockContract aapl("AAPL", "Apple Inc.", "NASDAQ", "oracle_addr");

// Set metadata
aapl.setSector("Technology");
aapl.setIndustry("Consumer Electronics");
aapl.setDividendYield(0.0055);  // 0.55%

// Add authorized parties
aapl.addAuthorizedIssuer("institution_addr");
aapl.addAuthorizedOracle("chainlink_oracle");
aapl.addAuthorizedExecutor("corporate_actions_addr");

// Issue initial shares (1 million)
aapl.issueShares("institution_addr", 1000000);

// Update price
aapl.updatePrice(150.50, time(nullptr), "pop_hash", "NASDAQ");

// Enable trading
aapl.resumeTrading();
```

### Example 2: Financial Stock (JPMorgan)

```cpp
// Deploy JPMorgan stock contract
StockContract jpm("JPM", "JPMorgan Chase & Co.", "NYSE", "oracle_addr");

// Set metadata
jpm.setSector("Financials");
jpm.setIndustry("Banking");
jpm.setDividendYield(0.0275);  // 2.75%

// Configure compliance
jpm.setKYCStatus("investor1_addr", true);
jpm.setKYCStatus("investor2_addr", true);

// Issue shares
jpm.issueShares("institution_addr", 500000);

// Propose quarterly dividend
uint64_t actionId = jpm.proposeCorporateAction(
    ActionType::DIVIDEND,
    1.25,                      // $1.25 per share
    "proof_hash_q4_2025",
    "Q4 2025 Dividend"
);

// Execute dividend
jpm.executeCorporateAction(actionId);
jpm.distributeDividend(1.25, "proof_hash_q4_2025");
```

### Example 3: Stock Split

```cpp
// Tesla 3-for-1 stock split
StockContract tsla("TSLA", "Tesla, Inc.", "NASDAQ", "oracle_addr");

// Before split: 1000 shares at $900 each
tsla.issueShares("investor_addr", 1000);
tsla.updatePrice(900.0, time(nullptr), "pop_hash", "NASDAQ");

// Execute 3-for-1 split
tsla.executeSplit(3.0);

// After split: 3000 shares at $300 each
// balanceOf("investor_addr") == 3000
// getCurrentPrice() == 300.0
```

### Example 4: Index Fund (S&P 500)

```cpp
// Create S&P 500 index contract
StockIndexContract sp500("S&P 500", "SPX");

// Add components with weights
sp500.addComponent("AAPL_contract_addr", 0.07);  // 7% weight
sp500.addComponent("MSFT_contract_addr", 0.06);  // 6% weight
sp500.addComponent("AMZN_contract_addr", 0.03);  // 3% weight
// ... add 497 more components

// Calculate index value
sp500.calculateIndexValue();

// Mint index tokens
sp500.mintIndexTokens("investor_addr", 100);

// Enable auto-rebalancing
sp500.enableRebalancing();
```

---

## Corporate Actions

### Dividends

```cpp
// Propose dividend
uint64_t actionId = stockContract.proposeCorporateAction(
    ActionType::DIVIDEND,
    2.50,                      // $2.50 per share
    "proof_hash_dividend",
    "Q4 2025 Dividend - Record date: Dec 31, 2025"
);

// Vote on action (if required)
stockContract.voteCorporateAction(actionId, "board_member1", true);
stockContract.voteCorporateAction(actionId, "board_member2", true);

// Execute after voting period
stockContract.executeCorporateAction(actionId);

// Distribute to all shareholders
stockContract.distributeDividend(2.50, "proof_hash_dividend");

// Shareholders can claim
stockContract.claimDividend("shareholder_addr");
```

### Stock Splits

```cpp
// 2-for-1 split
stockContract.executeSplit(2.0);

// 3-for-1 split
stockContract.executeSplit(3.0);

// Reverse split (1-for-2)
stockContract.executeSplit(0.5);

// Reverse split (1-for-10)
stockContract.executeSplit(0.1);
```

### Mergers

```cpp
// Company A merges with Company B
// Exchange ratio: 1 share of A = 1.5 shares of B

StockContract companyA("CMPA", "Company A", "NYSE", "oracle");
StockContract companyB("CMPB", "Company B", "NYSE", "oracle");

// Execute merger
companyA.executeMerger(
    1.5,                       // Exchange ratio
    "companyB_contract_addr"   // Target contract
);

// Shareholders of A receive 1.5 shares of B for each share of A
```

### Spin-offs

```cpp
// Parent company spins off subsidiary
uint64_t actionId = parentCompany.proposeCorporateAction(
    ActionType::SPINOFF,
    0.5,                       // 0.5 shares of subsidiary per parent share
    "proof_hash_spinoff",
    "Spin-off of Subsidiary Corp"
);

parentCompany.executeCorporateAction(actionId);
```

---

## Price Management

### Update Price from Oracle

```cpp
// Chainlink price feed integration
class ChainlinkOracle {
public:
    double getLatestPrice(const std::string& ticker) {
        // Query Chainlink oracle
        return chainlink.getPrice(ticker);
    }
    
    std::string getProofHash() {
        // Get Proof of Price hash
        return chainlink.getProofHash();
    }
};

// Update stock price
ChainlinkOracle oracle;
double price = oracle.getLatestPrice("AAPL");
std::string popHash = oracle.getProofHash();

stockContract.updatePrice(
    price,
    std::time(nullptr),
    popHash,
    "Chainlink"
);
```

### Price History

```cpp
// Get price history for last 30 days
std::vector<StockPrice> history = stockContract.getPriceHistory(30);

for (const auto& price : history) {
    std::cout << "Date: " << price.timestamp 
              << " Price: $" << price.price 
              << " Source: " << price.source << "\n";
}
```

### Price Analytics

```cpp
// Calculate average price
double avg30d = stockContract.getAveragePrice(30);

// Calculate volatility
double volatility = stockContract.getVolatility(30);

// Get 24h price change
double change24h = stockContract.getPriceChange24h();

// Get 24h volume
double volume24h = stockContract.getVolume24h();
```

---

## Compliance & KYC

### KYC Verification

```cpp
// Verify investor KYC
stockContract.setKYCStatus("investor_addr", true);

// Check KYC status
bool isVerified = stockContract.isKYCVerified("investor_addr");

// Require KYC for transfers
if (!stockContract.isKYCVerified(toAddress)) {
    throw std::runtime_error("Recipient must complete KYC");
}
```

### Transfer Restrictions

```cpp
// Restrict specific address
stockContract.setTransferRestriction("sanctioned_addr", true);

// Check before transfer
if (stockContract.hasTransferRestriction(toAddress)) {
    throw std::runtime_error("Transfer to this address is restricted");
}
```

### Trading Halts

```cpp
// Halt trading (e.g., during investigation)
stockContract.haltTrading();

// Check status
if (stockContract.isTradingHalted()) {
    std::cout << "Trading is currently halted\n";
}

// Resume trading
stockContract.resumeTrading();
```

---

## Transaction Chaining

### Create Chained Transfer

```cpp
// Create transfer with traceability
StockTransfer transfer = stockContract.createChainedTransfer(
    "from_addr",
    "to_addr",
    100,                       // shares
    150.50                     // price at transfer
);

// Transfer includes:
// - from/to addresses
// - number of shares
// - transaction hash
// - previous transaction hash (traceability!)
// - price at transfer
// - timestamp
```

### Verify Transaction Chain

```cpp
// Verify complete transaction history
bool isValid = stockContract.verifyTransactionChain(
    "address",
    100                        // depth to verify
);

if (isValid) {
    std::cout << "Transaction chain is valid\n";
}
```

### Get Transaction History

```cpp
// Get all transfers for an address
std::vector<StockTransfer> history = 
    stockContract.getTransferHistory("address");

for (const auto& transfer : history) {
    std::cout << "From: " << transfer.from 
              << " To: " << transfer.to
              << " Shares: " << transfer.shares
              << " Price: $" << transfer.priceAtTransfer
              << " TX: " << transfer.txHash << "\n";
}
```

---

## Queries & Analytics

### Balance Queries

```cpp
// Get share balance
uint64_t balance = stockContract.balanceOf("address");

// Get all shareholders
std::vector<std::string> shareholders = stockContract.getShareholders();

// Get total shares
uint64_t totalShares = stockContract.getTotalShares();
```

### Market Data

```cpp
// Get market cap
double marketCap = stockContract.getMarketCap();

// Get dividend yield
double yield = stockContract.getDividendYield();

// Get current price
StockPrice price = stockContract.getCurrentPrice();

// Check if price is fresh
bool isFresh = stockContract.isPriceDataFresh(3600);  // 1 hour
```

### Corporate Actions

```cpp
// Get pending actions
std::vector<CorporateAction> pending = 
    stockContract.getPendingActions();

for (const auto& action : pending) {
    std::cout << "Action ID: " << action.id
              << " Type: " << (int)action.actionType
              << " Value: $" << action.value
              << " Description: " << action.description << "\n";
}
```

---

## Access Control

### Authorized Issuers

```cpp
// Add issuer
stockContract.addAuthorizedIssuer("institution_addr");

// Check authorization
if (stockContract.isAuthorizedIssuer("addr")) {
    stockContract.issueShares("investor_addr", 1000);
}

// Remove issuer
stockContract.removeAuthorizedIssuer("old_institution_addr");
```

### Authorized Executors

```cpp
// Add executor for corporate actions
stockContract.addAuthorizedExecutor("corporate_actions_addr");

// Check authorization
if (stockContract.isAuthorizedExecutor("addr")) {
    stockContract.executeCorporateAction(actionId);
}
```

### Authorized Oracles

```cpp
// Add price oracle
stockContract.addAuthorizedOracle("chainlink_oracle_addr");

// Only authorized oracles can update prices
if (stockContract.isAuthorizedOracle("addr")) {
    stockContract.updatePrice(price, timestamp, popHash, source);
}
```

---

## Best Practices

### 1. Always Verify KYC

```cpp
bool transferShares(const std::string& from, const std::string& to, uint64_t shares) {
    // Verify KYC
    if (!stockContract.isKYCVerified(from) || !stockContract.isKYCVerified(to)) {
        throw std::runtime_error("KYC verification required");
    }
    
    // Check restrictions
    if (stockContract.hasTransferRestriction(to)) {
        throw std::runtime_error("Transfer restricted");
    }
    
    // Execute transfer
    return stockContract.transferShares(from, to, shares);
}
```

### 2. Validate Price Updates

```cpp
bool updatePrice(double newPrice) {
    // Check price deviation
    double currentPrice = stockContract.getCurrentPrice().price;
    double deviation = std::abs(newPrice - currentPrice) / currentPrice;
    
    if (deviation > StockContract::MAX_PRICE_DEVIATION) {
        throw std::runtime_error("Price deviation too large");
    }
    
    // Check price freshness
    if (!stockContract.isPriceDataFresh(3600)) {
        // Price is stale, update needed
    }
    
    return stockContract.updatePrice(newPrice, time(nullptr), popHash, source);
}
```

### 3. Audit Corporate Actions

```cpp
void executeCorporateAction(uint64_t actionId) {
    // Log action
    auditLog.log("Executing corporate action", {
        {"action_id", actionId},
        {"executor", executorAddress},
        {"timestamp", time(nullptr)}
    });
    
    // Execute
    stockContract.executeCorporateAction(actionId);
    
    // Notify shareholders
    notifyAllShareholders(actionId);
}
```

### 4. Monitor Trading Activity

```cpp
void monitorTrading() {
    // Get 24h volume
    double volume = stockContract.getVolume24h();
    
    // Alert if unusual
    if (volume > averageVolume * 3) {
        alert("Unusual trading volume detected");
    }
    
    // Check price volatility
    double volatility = stockContract.getVolatility(1);
    if (volatility > 0.10) {  // 10%
        alert("High volatility detected");
    }
}
```

---

## Integration with GXC Blockchain

### Deploy Contract

```cpp
// Deploy to blockchain
Blockchain blockchain;
std::string contractAddress = blockchain.deployStockContract(stockContract);

// Verify deployment
StockContract deployed = blockchain.getStockContract(contractAddress);
assert(deployed.getTicker() == stockContract.getTicker());
```

### Execute Transactions

```cpp
// Create transaction for share transfer
Transaction tx = stockContract.createTransferTransaction(
    "from_addr",
    "to_addr",
    100
);

// Sign transaction
tx.sign(privateKey);

// Broadcast to blockchain
std::string txHash = blockchain.broadcastTransaction(tx);

// Wait for confirmation
blockchain.waitForConfirmation(txHash, 6);  // 6 confirmations
```

---

## Support

For questions or issues:
- Email: stock-contracts@gxc.io
- Documentation: https://docs.gxc.io/stock-contracts
- Examples: https://github.com/gxc/stock-contract-examples

---

**Version**: 1.0  
**Last Updated**: December 2025  
**Status**: Production Ready ✅
