# Stock Contracts Explained - Complete Guide

## What Are Stock Contracts?

### Simple Explanation

**Stock Contract = Tokenized Stock on GXC Blockchain**

When a market maker deploys a stock contract:
1. A **new token** is created on the blockchain
2. This token represents shares of a real company (e.g., Apple, Microsoft)
3. Each token = 1 share of the company
4. The token can be bought, sold, and traded on the blockchain

**Example**:
```
Market Maker deploys "AAPL" stock contract
→ Creates 1,000,000 AAPL tokens
→ Each token = 1 share of Apple Inc.
→ Users can buy/sell AAPL tokens using GXC coins
```

---

## How Stock Contracts Work

### 1. Stock Contract Creation

**What Happens When You Deploy a Stock**:

```cpp
api.deployStock(
    "tGXC_mm_goldman",      // Market maker address
    "AAPL",                  // Ticker symbol
    "Apple Inc.",            // Company name
    "NASDAQ",                // Exchange
    1000000                  // Total shares (tokens)
);
```

**Behind the Scenes**:
1. ✅ Verify market maker is authorized
2. ✅ Create new token contract on blockchain
3. ✅ Mint 1,000,000 AAPL tokens
4. ✅ Allocate all tokens to market maker initially
5. ✅ Set initial price (e.g., $150.50)
6. ✅ Create order book for trading
7. ✅ Return contract address

**Result**:
- New AAPL token exists on GXC blockchain
- Market maker owns all 1,000,000 tokens
- Users can now buy AAPL tokens using GXC coins

---

### 2. Stock Contract Structure

**Each Stock Contract Contains**:

```cpp
struct StockContract {
    string ticker;              // "AAPL"
    string companyName;         // "Apple Inc."
    string exchange;            // "NASDAQ"
    uint64_t totalShares;       // 1,000,000 tokens
    double currentPrice;        // $150.50
    string priceSource;         // "Bloomberg Terminal"
    string marketMakerAddress;  // Who deployed it
    
    // Token balances
    map<string, uint64_t> balances;  // address -> token count
    
    // Order book
    vector<Order> bids;         // Buy orders
    vector<Order> asks;         // Sell orders
    
    // Trading history
    vector<Trade> trades;       // All trades
    
    // Corporate actions
    vector<CorporateAction> actions;  // Dividends, splits, etc.
};
```

**Example State**:
```json
{
  "ticker": "AAPL",
  "companyName": "Apple Inc.",
  "totalShares": 1000000,
  "currentPrice": 150.50,
  "priceSource": "Bloomberg Terminal",
  "marketMaker": "tGXC_mm_goldman",
  "balances": {
    "tGXC_mm_goldman": 900000,
    "tGXC_user_alice": 50000,
    "tGXC_user_bob": 30000,
    "tGXC_bank_xyz": 20000
  }
}
```

---

## Market Makers - Who They Are and What They Do

### What is a Market Maker?

**Market Maker = Licensed Financial Institution**

Examples:
- Goldman Sachs
- Morgan Stanley
- Citadel Securities
- Jane Street
- Jump Trading

**Their Role**:
1. Deploy stock contracts (create tokens)
2. Provide liquidity (buy/sell orders)
3. Set prices from professional feeds
4. Execute corporate actions
5. Ensure fair trading

---

### Market Maker Authorization Process

**Who Approves Market Makers?**

**Answer: GXC Blockchain Governance**

#### Step 1: Application
```json
{
  "company": "Goldman Sachs Trading Desk",
  "license_number": "MM-12345-US",
  "country": "USA",
  "regulatory_body": "SEC",
  "proof_of_license": "document_hash_abc123",
  "contact": "mm-support@goldmansachs.com"
}
```

#### Step 2: Verification by GXC Team
- ✅ Verify license with regulatory body (SEC, FCA, etc.)
- ✅ Check company reputation
- ✅ Review financial standing
- ✅ Verify technical capabilities
- ✅ KYC/AML compliance check

#### Step 3: Approval
```cpp
// GXC admin approves market maker
marketMakerRegistry.registerMarketMaker(
    "tGXC_mm_goldman",
    "Goldman Sachs Trading Desk",
    "MM-12345-US",
    "USA"
);
```

#### Step 4: Market Maker Can Now:
- ✅ Deploy stock contracts
- ✅ Set prices from their own feeds
- ✅ Provide liquidity
- ✅ Execute corporate actions

**Unauthorized Users Cannot**:
- ❌ Deploy stocks
- ❌ Set prices
- ❌ Execute corporate actions

---

### Market Maker API Usage

**Can Market Makers Create Their Own Software?**

**YES! ✅ That's the whole point!**

Market makers can:
1. Build their own trading platforms
2. Integrate with their existing systems
3. Use their own price feeds
4. Create custom trading algorithms
5. Build their own exchanges

---

## Market Maker Software Integration

### Example 1: Goldman Sachs Trading Platform

**Goldman Sachs can build**:

```javascript
// Goldman's proprietary trading software
class GoldmanTradingPlatform {
    constructor() {
        this.gxcAPI = new GXCStockMarketAPI({
            apiKey: "goldman_api_key",
            marketMakerAddress: "tGXC_mm_goldman"
        });
        
        this.bloombergFeed = new BloombergTerminal();
    }
    
    // Deploy new stock
    async deployStock(ticker, company, exchange, shares) {
        // Goldman deploys stock on GXC blockchain
        const contractAddr = await this.gxcAPI.deployStock({
            marketMakerAddress: this.marketMakerAddress,
            ticker: ticker,
            companyName: company,
            exchange: exchange,
            totalShares: shares
        });
        
        console.log(`Stock ${ticker} deployed at ${contractAddr}`);
        return contractAddr;
    }
    
    // Update prices from Bloomberg
    async updatePricesFromBloomberg() {
        // Get prices from Bloomberg Terminal
        const stocks = ["AAPL", "MSFT", "GOOGL"];
        
        for (const ticker of stocks) {
            // Get real-time price from Bloomberg
            const price = await this.bloombergFeed.getPrice(ticker);
            const proofHash = await this.bloombergFeed.getProofHash(ticker);
            
            // Update price on GXC blockchain
            await this.gxcAPI.updateStockPrice({
                marketMakerAddress: this.marketMakerAddress,
                ticker: ticker,
                price: price,
                priceSource: "Bloomberg Terminal",
                proofHash: proofHash
            });
            
            console.log(`Updated ${ticker} price to $${price}`);
        }
    }
    
    // Provide liquidity
    async provideLiquidity(ticker, shares, spread) {
        const currentPrice = await this.gxcAPI.getStockPrice(ticker);
        const bidPrice = currentPrice - (spread / 2);
        const askPrice = currentPrice + (spread / 2);
        
        await this.gxcAPI.addLiquidity({
            marketMakerAddress: this.marketMakerAddress,
            ticker: ticker,
            shares: shares,
            bidPrice: bidPrice,
            askPrice: askPrice
        });
        
        console.log(`Added liquidity for ${ticker}: Bid $${bidPrice}, Ask $${askPrice}`);
    }
}

// Goldman runs this 24/7
const goldman = new GoldmanTradingPlatform();
goldman.deployStock("AAPL", "Apple Inc.", "NASDAQ", 1000000);
setInterval(() => goldman.updatePricesFromBloomberg(), 1000); // Update every second
```

---

### Example 2: Citadel Securities Exchange

**Citadel can build their own exchange**:

```javascript
// Citadel's custom exchange for GXC stocks
class CitadelExchange {
    constructor() {
        this.gxcAPI = new GXCStockMarketAPI({
            apiKey: "citadel_api_key",
            marketMakerAddress: "tGXC_mm_citadel"
        });
        
        this.reutersFeed = new ReutersEikon();
        this.webServer = new ExpressServer();
    }
    
    // Build web interface for trading
    startExchange() {
        // Web UI for users to trade
        this.webServer.get("/stocks", async (req, res) => {
            const stocks = await this.gxcAPI.getAllStocks();
            res.render("stocks", { stocks });
        });
        
        this.webServer.post("/buy", async (req, res) => {
            const { userAddress, ticker, shares } = req.body;
            
            // Execute buy on GXC blockchain
            const txHash = await this.gxcAPI.buyShares({
                buyerAddress: userAddress,
                ticker: ticker,
                shares: shares,
                maxPrice: req.body.maxPrice
            });
            
            res.json({ success: true, txHash });
        });
        
        this.webServer.listen(3000);
        console.log("Citadel Exchange running on port 3000");
    }
    
    // Update prices from Reuters
    async updatePricesFromReuters() {
        const stocks = await this.gxcAPI.getMyStocks();
        
        for (const stock of stocks) {
            const price = await this.reutersFeed.getPrice(stock.ticker);
            
            await this.gxcAPI.updateStockPrice({
                marketMakerAddress: this.marketMakerAddress,
                ticker: stock.ticker,
                price: price,
                priceSource: "Reuters Eikon",
                proofHash: await this.reutersFeed.getProofHash(stock.ticker)
            });
        }
    }
}

// Citadel runs their own exchange
const citadel = new CitadelExchange();
citadel.startExchange();
```

---

### Example 3: Jane Street Algorithmic Trading

**Jane Street can build trading algorithms**:

```python
# Jane Street's algorithmic trading bot
class JaneStreetTradingBot:
    def __init__(self):
        self.gxc_api = GXCStockMarketAPI(
            api_key="janestreet_api_key",
            market_maker_address="tGXC_mm_janestreet"
        )
        self.proprietary_feed = JaneStreetPriceFeed()
    
    def deploy_stocks(self):
        """Deploy multiple stocks"""
        stocks = [
            ("AAPL", "Apple Inc.", "NASDAQ", 1000000),
            ("MSFT", "Microsoft", "NASDAQ", 1000000),
            ("GOOGL", "Google", "NASDAQ", 1000000)
        ]
        
        for ticker, name, exchange, shares in stocks:
            self.gxc_api.deploy_stock(
                market_maker_address=self.market_maker_address,
                ticker=ticker,
                company_name=name,
                exchange=exchange,
                total_shares=shares
            )
            print(f"Deployed {ticker}")
    
    def run_market_making_algorithm(self):
        """Provide liquidity using proprietary algorithm"""
        while True:
            for ticker in ["AAPL", "MSFT", "GOOGL"]:
                # Get price from Jane Street's proprietary feed
                price = self.proprietary_feed.get_price(ticker)
                volatility = self.proprietary_feed.get_volatility(ticker)
                
                # Calculate optimal spread based on volatility
                spread = self.calculate_optimal_spread(volatility)
                
                # Update price on blockchain
                self.gxc_api.update_stock_price(
                    market_maker_address=self.market_maker_address,
                    ticker=ticker,
                    price=price,
                    price_source="Jane Street Proprietary Feed",
                    proof_hash=self.proprietary_feed.get_proof(ticker)
                )
                
                # Provide liquidity
                self.gxc_api.add_liquidity(
                    market_maker_address=self.market_maker_address,
                    ticker=ticker,
                    shares=10000,
                    bid_price=price - (spread / 2),
                    ask_price=price + (spread / 2)
                )
            
            time.sleep(0.1)  # Update 10 times per second

# Jane Street runs algorithmic trading
bot = JaneStreetTradingBot()
bot.deploy_stocks()
bot.run_market_making_algorithm()
```

---

## Stock Contract Lifecycle

### 1. Deployment
```
Market Maker → Deploy Stock → Token Created → Initial Allocation
```

### 2. Price Updates
```
Market Maker → Get Price from Feed → Update on Blockchain → Users See New Price
```

### 3. Trading
```
User → Buy Order → Match with Seller → Transfer Tokens → Update Balances
```

### 4. Corporate Actions
```
Market Maker → Execute Dividend → Distribute to All Holders → Update Balances
```

---

## Token Creation Details

### What Happens When Stock is Deployed?

**Step-by-Step**:

1. **Market Maker Calls API**:
```javascript
const contractAddr = await api.deployStock(
    "tGXC_mm_goldman",
    "AAPL",
    "Apple Inc.",
    "NASDAQ",
    1000000
);
```

2. **Blockchain Creates Token**:
```cpp
// Inside blockchain
StockContract newStock;
newStock.ticker = "AAPL";
newStock.companyName = "Apple Inc.";
newStock.totalShares = 1000000;
newStock.marketMaker = "tGXC_mm_goldman";

// Mint tokens
newStock.balances["tGXC_mm_goldman"] = 1000000;

// Store contract
stockContracts["AAPL"] = newStock;
```

3. **Token is Now Tradeable**:
```javascript
// Users can buy AAPL tokens
await api.buyShares("tGXC_user_alice", "AAPL", 100, 151.00);

// Alice now owns 100 AAPL tokens
// Market maker now owns 999,900 AAPL tokens
```

---

## Real-World Example: Complete Flow

### Scenario: Goldman Sachs Deploys Apple Stock

**Step 1: Goldman Applies to be Market Maker**
```
Goldman → Submit Application → GXC Verifies License → Approved ✅
```

**Step 2: Goldman Deploys AAPL Stock**
```javascript
// Goldman's software
const goldman = new GoldmanTradingPlatform();
await goldman.deployStock("AAPL", "Apple Inc.", "NASDAQ", 1000000);
```

**Result**:
- 1,000,000 AAPL tokens created
- Goldman owns all 1,000,000 tokens
- Contract address: `0xabc123...`

**Step 3: Goldman Sets Initial Price**
```javascript
// Get price from Bloomberg Terminal
const price = await bloomberg.getPrice("AAPL"); // $150.50

// Update on blockchain
await goldman.updatePrice("AAPL", 150.50, "Bloomberg Terminal");
```

**Step 4: Goldman Provides Liquidity**
```javascript
// Add buy/sell orders
await goldman.addLiquidity("AAPL", 10000, 150.25, 150.75);
```

**Order Book Now**:
```
Bids (Buy Orders):
  $150.25 - 10,000 shares (Goldman)

Asks (Sell Orders):
  $150.75 - 10,000 shares (Goldman)

Spread: $0.50
```

**Step 5: User Buys AAPL**
```javascript
// Alice buys 100 AAPL tokens
await api.buyShares("tGXC_user_alice", "AAPL", 100, 151.00);
```

**Transaction**:
```
Alice pays: 100 shares × $150.75 = $15,075 GXC
Alice receives: 100 AAPL tokens
Goldman receives: $15,075 GXC
Goldman's AAPL balance: 999,900 tokens
Alice's AAPL balance: 100 tokens
```

**Step 6: Alice Sells AAPL**
```javascript
// Alice sells 50 AAPL tokens
await api.sellShares("tGXC_user_alice", "AAPL", 50, 150.00);
```

**Transaction**:
```
Alice sells: 50 AAPL tokens
Alice receives: 50 × $150.25 = $7,512.50 GXC
Goldman receives: 50 AAPL tokens
Goldman's AAPL balance: 999,950 tokens
Alice's AAPL balance: 50 tokens
```

**Step 7: Goldman Executes Dividend**
```javascript
// Apple announces $2.50 dividend per share
await goldman.executeCorporateAction(
    "AAPL",
    "DIVIDEND",
    2.50,
    "Q4 2025 Dividend"
);
```

**Result**:
```
Alice owns 50 AAPL tokens
Alice receives: 50 × $2.50 = $125 GXC dividend
Goldman owns 999,950 AAPL tokens
Goldman receives: 999,950 × $2.50 = $2,499,875 GXC dividend
```

---

## Multiple Market Makers

### Can Multiple Market Makers Deploy Same Stock?

**NO** ❌

**Reason**: One ticker = One contract

**Example**:
```
Goldman deploys "AAPL" → Contract created ✅
Citadel tries to deploy "AAPL" → ERROR: Ticker already exists ❌
```

**But**:
```
Goldman deploys "AAPL" → Apple Inc. on NASDAQ ✅
Citadel deploys "MSFT" → Microsoft on NASDAQ ✅
Jane Street deploys "GOOGL" → Google on NASDAQ ✅
```

**Each market maker can**:
- Deploy different stocks
- Provide liquidity for any stock
- Trade any stock
- But only the deployer can set prices for their stock

---

## Market Maker Competition

### How Market Makers Compete

**Scenario**: 3 Market Makers

```
Goldman deployed: AAPL, MSFT, GOOGL
Citadel deployed: TSLA, NVDA, AMD
Jane Street deployed: META, NFLX, AMZN
```

**Competition**:

1. **Tighter Spreads**:
```
Goldman AAPL: Bid $150.25, Ask $150.75 (spread $0.50)
Citadel TSLA: Bid $250.10, Ask $250.40 (spread $0.30) ← Better!
```

2. **More Liquidity**:
```
Goldman AAPL: 10,000 shares available
Jane Street META: 50,000 shares available ← Better!
```

3. **Better Prices**:
```
Goldman updates AAPL every 1 second
Citadel updates TSLA every 0.1 seconds ← Better!
```

4. **More Stocks**:
```
Goldman: 3 stocks
Citadel: 3 stocks
Jane Street: 3 stocks + 10 more = 13 stocks ← Better!
```

---

## Building Your Own Exchange

### Can Anyone Build an Exchange?

**YES!** ✅

**You don't need to be a market maker to build an exchange**

**Example**: Coinbase-style Exchange for GXC Stocks

```javascript
// Your exchange software
class MyGXCStockExchange {
    constructor() {
        this.gxcAPI = new GXCStockMarketAPI({
            apiKey: "your_api_key"
            // Note: No market maker address needed!
        });
    }
    
    // Show all available stocks
    async getStocks() {
        return await this.gxcAPI.getAllStocks();
    }
    
    // User buys stock
    async buyStock(userAddress, ticker, shares) {
        return await this.gxcAPI.buyShares({
            buyerAddress: userAddress,
            ticker: ticker,
            shares: shares,
            maxPrice: await this.gxcAPI.getStockPrice(ticker) * 1.01
        });
    }
    
    // User sells stock
    async sellStock(userAddress, ticker, shares) {
        return await this.gxcAPI.sellShares({
            sellerAddress: userAddress,
            ticker: ticker,
            shares: shares,
            minPrice: await this.gxcAPI.getStockPrice(ticker) * 0.99
        });
    }
    
    // Show user's portfolio
    async getPortfolio(userAddress) {
        return await this.gxcAPI.getPortfolio(userAddress);
    }
}

// Run your exchange
const myExchange = new MyGXCStockExchange();
myExchange.startWebServer(3000);
```

**Your exchange can**:
- ✅ Show all stocks from all market makers
- ✅ Let users buy/sell any stock
- ✅ Show portfolios
- ✅ Show order books
- ✅ Show trade history
- ✅ Charge trading fees
- ✅ Build mobile apps
- ✅ Build web apps

**Your exchange CANNOT**:
- ❌ Deploy new stocks (only market makers)
- ❌ Set prices (only market makers)
- ❌ Execute corporate actions (only market makers)

---

## Summary

### Stock Contracts
- **What**: Tokenized stocks on GXC blockchain
- **How**: Market maker deploys → Token created → Users trade
- **Example**: Deploy "AAPL" → Creates 1,000,000 AAPL tokens

### Market Makers
- **Who**: Licensed financial institutions (Goldman, Citadel, etc.)
- **Approval**: GXC governance verifies license
- **Can Do**: Deploy stocks, set prices, provide liquidity
- **Cannot Do**: Unauthorized users cannot deploy stocks

### Market Maker Software
- **YES**: Market makers build their own software ✅
- **YES**: Market makers use their own price feeds ✅
- **YES**: Market makers build their own exchanges ✅
- **Examples**: Bloomberg, Reuters, proprietary systems

### Exchanges
- **Anyone**: Can build exchange for trading ✅
- **No Authorization**: Needed to build exchange ✅
- **Can Do**: Show stocks, execute trades, show portfolios
- **Cannot Do**: Deploy stocks, set prices

### Token Creation
- **When**: Stock deployed → Token created
- **Amount**: Specified by market maker (e.g., 1,000,000)
- **Ownership**: Initially all to market maker
- **Trading**: Users buy/sell tokens using GXC coins

---

## Next Steps

### For Market Makers
1. Apply for authorization
2. Get approved by GXC governance
3. Build integration software
4. Deploy stocks
5. Provide liquidity
6. Update prices from your feeds

### For Exchange Builders
1. Get API key
2. Build web/mobile interface
3. Integrate GXC Stock Market API
4. Let users trade
5. Charge fees
6. Grow user base

### For Developers
1. Read `STOCK_MARKET_API_REFERENCE.md`
2. Review examples
3. Test on testnet
4. Build your application
5. Deploy to production

---

**Questions?**
- Technical: Review `STOCK_MARKET_API_REFERENCE.md`
- Integration: Review `include/StockMarketAPI.h`
- Examples: Review `STOCK_MARKET_IMPLEMENTATION_SUMMARY.md`
