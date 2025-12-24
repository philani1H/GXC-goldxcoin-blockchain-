# GXC Blockchain - Stock Market & Admin API Documentation

## Status: Implementation Exists, Integration Pending

**Implementation**: 1,182 lines of production-ready code  
**Integration Effort**: 4-8 hours  
**Priority**: Non-critical (can be completed during testnet)  
**Version**: 2.0.0

---

## Overview

GXC blockchain includes a complete stock market system with market maker administration. The implementation exists but is not yet integrated into the RPC API.

### Features Available

**Stock Market**:
- Synthetic equity contracts (price-tracking)
- Custodial-backed contracts (1:1 real shares)
- Issuer-authorized contracts (company-issued)
- Stock trading with full traceability
- Portfolio management
- Corporate actions (dividends, splits)

**Market Maker Admin**:
- Admin authentication and authorization
- Market maker application system
- License verification
- Company reputation checks
- Financial standing review
- Technical capability verification
- KYC/AML compliance checks
- Application approval/rejection workflow
- Comprehensive audit logging

---

## Stock Market API

### Implementation Files

- `include/StockMarketAPI.h` (400+ lines)
- `src/StockMarketAPI.cpp` (493 lines)
- `include/StockContract.h` (400+ lines)
- `src/tokens/StockContract.cpp`

### Available Methods

#### deployStock

Deploy a new stock contract.

**Parameters**:
- `ticker` (string) - Stock ticker symbol (e.g., "AAPL")
- `name` (string) - Company name
- `total_supply` (number) - Total shares
- `contract_type` (string) - "synthetic", "custodial", or "issuer"
- `market_maker` (string) - Market maker address
- `initial_price` (number) - Initial price per share

**Returns**: Contract address

**Example** (when integrated):
```python
contract = client.call("deployStock", [
    "AAPL",                    # ticker
    "Apple Inc.",              # name
    1000000,                   # total supply
    "synthetic",               # type
    "tGXCmm123...",           # market maker
    150.0                      # initial price
])
print(f"Contract deployed: {contract}")
```

#### buyShares

Buy shares of a stock.

**Parameters**:
- `buyer_address` (string) - Buyer's GXC address
- `ticker` (string) - Stock ticker
- `quantity` (number) - Number of shares
- `max_price` (number) - Maximum price per share

**Returns**: Transaction hash

**Example** (when integrated):
```python
txid = client.call("buyShares", [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",  # buyer
    "AAPL",                                      # ticker
    10,                                          # quantity
    155.0                                        # max price
])
print(f"Purchase TX: {txid}")
```

#### sellShares

Sell shares of a stock.

**Parameters**:
- `seller_address` (string) - Seller's GXC address
- `ticker` (string) - Stock ticker
- `quantity` (number) - Number of shares
- `min_price` (number) - Minimum price per share

**Returns**: Transaction hash

**Example** (when integrated):
```python
txid = client.call("sellShares", [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",  # seller
    "AAPL",                                      # ticker
    5,                                           # quantity
    145.0                                        # min price
])
print(f"Sale TX: {txid}")
```

#### getStockPrice

Get current price of a stock.

**Parameters**:
- `ticker` (string) - Stock ticker

**Returns**: `number` - Current price

**Example** (when integrated):
```python
price = client.call("getStockPrice", ["AAPL"])
print(f"AAPL price: ${price}")
```

#### getStockBalance

Get stock holdings for an address.

**Parameters**:
- `address` (string) - GXC address
- `ticker` (string) - Stock ticker

**Returns**: `number` - Number of shares owned

**Example** (when integrated):
```python
shares = client.call("getStockBalance", [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "AAPL"
])
print(f"Owns {shares} shares of AAPL")
```

#### getPortfolio

Get complete portfolio for an address.

**Parameters**:
- `address` (string) - GXC address

**Returns**: Array of holdings

**Example** (when integrated):
```python
portfolio = client.call("getPortfolio", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
for holding in portfolio:
    print(f"{holding['ticker']}: {holding['quantity']} shares @ ${holding['price']}")
```

#### getPortfolioValue

Get total portfolio value.

**Parameters**:
- `address` (string) - GXC address

**Returns**: `number` - Total value in GXC

**Example** (when integrated):
```python
value = client.call("getPortfolioValue", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
print(f"Portfolio value: {value} GXC")
```

#### updateStockPrice

Update stock price (market maker only).

**Parameters**:
- `ticker` (string) - Stock ticker
- `new_price` (number) - New price
- `market_maker` (string) - Market maker address

**Returns**: `boolean` - Success

**Example** (when integrated):
```python
success = client.call("updateStockPrice", [
    "AAPL",
    152.50,
    "tGXCmm123..."
])
print(f"Price updated: {success}")
```

#### addLiquidity

Add liquidity to a stock (market maker only).

**Parameters**:
- `ticker` (string) - Stock ticker
- `quantity` (number) - Number of shares
- `market_maker` (string) - Market maker address

**Returns**: `boolean` - Success

**Example** (when integrated):
```python
success = client.call("addLiquidity", [
    "AAPL",
    1000,
    "tGXCmm123..."
])
print(f"Liquidity added: {success}")
```

#### executeCorporateAction

Execute corporate action (dividends, splits, etc.).

**Parameters**:
- `ticker` (string) - Stock ticker
- `action_type` (string) - "dividend", "split", "reverse_split"
- `parameters` (object) - Action-specific parameters

**Returns**: Transaction hash

**Example** (when integrated):
```python
# Dividend
txid = client.call("executeCorporateAction", [
    "AAPL",
    "dividend",
    {"amount_per_share": 0.25}
])

# Stock split (2:1)
txid = client.call("executeCorporateAction", [
    "AAPL",
    "split",
    {"ratio": 2}
])
```

---

## Market Maker Admin API

### Implementation Files

- `include/MarketMakerAdmin.h` (300+ lines)
- `src/MarketMakerAdmin.cpp` (689 lines)

### Available Methods

#### submitApplication

Submit market maker application.

**Parameters**:
- `company_name` (string) - Company name
- `license_number` (string) - Trading license number
- `jurisdiction` (string) - Regulatory jurisdiction
- `contact_info` (object) - Contact information
- `financial_info` (object) - Financial standing
- `technical_info` (object) - Technical capabilities

**Returns**: Application ID

**Example** (when integrated):
```python
app_id = client.call("submitApplication", [{
    "company_name": "ABC Trading LLC",
    "license_number": "SEC-12345",
    "jurisdiction": "USA",
    "contact_info": {
        "email": "admin@abctrading.com",
        "phone": "+1-555-0100"
    },
    "financial_info": {
        "capital": 10000000,
        "insurance": 5000000
    },
    "technical_info": {
        "api_experience": true,
        "uptime_guarantee": 99.9
    }
}])
print(f"Application submitted: {app_id}")
```

#### getApplicationStatus

Check application status.

**Parameters**:
- `application_id` (string) - Application ID

**Returns**: Application status object

**Example** (when integrated):
```python
status = client.call("getApplicationStatus", ["APP-12345"])
print(f"Status: {status['status']}")
print(f"Review stage: {status['review_stage']}")
```

#### approveApplication

Approve market maker application (admin only).

**Parameters**:
- `application_id` (string) - Application ID
- `admin_address` (string) - Admin address
- `admin_signature` (string) - Admin signature

**Returns**: Market maker ID

**Example** (when integrated):
```python
mm_id = client.call("approveApplication", [
    "APP-12345",
    "tGXCadmin123...",
    "signature_hex"
])
print(f"Market maker approved: {mm_id}")
```

#### rejectApplication

Reject market maker application (admin only).

**Parameters**:
- `application_id` (string) - Application ID
- `reason` (string) - Rejection reason
- `admin_address` (string) - Admin address

**Returns**: `boolean` - Success

**Example** (when integrated):
```python
success = client.call("rejectApplication", [
    "APP-12345",
    "Insufficient capital requirements",
    "tGXCadmin123..."
])
```

#### listMarketMakers

List all approved market makers.

**Parameters**: None

**Returns**: Array of market makers

**Example** (when integrated):
```python
mms = client.call("listMarketMakers")
for mm in mms:
    print(f"{mm['company_name']}: {mm['status']}")
```

#### getMarketMakerInfo

Get detailed market maker information.

**Parameters**:
- `mm_id` (string) - Market maker ID

**Returns**: Market maker info object

**Example** (when integrated):
```python
info = client.call("getMarketMakerInfo", ["MM-12345"])
print(f"Company: {info['company_name']}")
print(f"License: {info['license_number']}")
print(f"Stocks: {len(info['managed_stocks'])}")
```

#### suspendMarketMaker

Suspend market maker (admin only).

**Parameters**:
- `mm_id` (string) - Market maker ID
- `reason` (string) - Suspension reason
- `admin_address` (string) - Admin address

**Returns**: `boolean` - Success

**Example** (when integrated):
```python
success = client.call("suspendMarketMaker", [
    "MM-12345",
    "Regulatory violation",
    "tGXCadmin123..."
])
```

#### reinstateMarketMaker

Reinstate suspended market maker (admin only).

**Parameters**:
- `mm_id` (string) - Market maker ID
- `admin_address` (string) - Admin address

**Returns**: `boolean` - Success

**Example** (when integrated):
```python
success = client.call("reinstateMarketMaker", [
    "MM-12345",
    "tGXCadmin123..."
])
```

---

## Integration Checklist

### Required Steps

1. **Initialize in RPCAPI Constructor** (30 minutes)
   ```cpp
   // In RPCAPI::RPCAPI()
   stockMarket = std::make_unique<StockMarketAPI>(blockchain, marketMakerRegistry);
   marketMakerAdmin = std::make_unique<MarketMakerAdmin>();
   ```

2. **Register RPC Methods** (1-2 hours)
   ```cpp
   // In registerMethods()
   rpcMethods["deployStock"] = [this](const JsonValue& params) { 
       return deployStock(params); 
   };
   rpcMethods["buyShares"] = [this](const JsonValue& params) { 
       return buyShares(params); 
   };
   // ... etc for all methods
   ```

3. **Create RPC Wrapper Methods** (1-2 hours)
   ```cpp
   JsonValue RPCAPI::deployStock(const JsonValue& params) {
       // Validate params
       // Call stockMarket->deployStock()
       // Return result as JsonValue
   }
   ```

4. **Test All Endpoints** (1-2 hours)
   - Test stock deployment
   - Test buy/sell operations
   - Test portfolio queries
   - Test admin operations
   - Test error handling

5. **Update Documentation** (1 hour)
   - Add to WALLET_API_DOCUMENTATION
   - Add examples
   - Update test suite

### Total Effort: 4-8 hours

---

## Use Cases

### Synthetic Equity Trading

```python
# 1. Deploy synthetic AAPL contract
contract = client.call("deployStock", [
    "AAPL", "Apple Inc.", 1000000, "synthetic", "tGXCmm123...", 150.0
])

# 2. Buy shares
txid = client.call("buyShares", [
    "tGXCuser123...", "AAPL", 10, 155.0
])

# 3. Check portfolio
portfolio = client.call("getPortfolio", ["tGXCuser123..."])

# 4. Sell shares
txid = client.call("sellShares", [
    "tGXCuser123...", "AAPL", 5, 145.0
])
```

### Market Maker Onboarding

```python
# 1. Submit application
app_id = client.call("submitApplication", [{
    "company_name": "ABC Trading",
    "license_number": "SEC-12345",
    # ... other details
}])

# 2. Check status
status = client.call("getApplicationStatus", [app_id])

# 3. Admin approves
mm_id = client.call("approveApplication", [
    app_id, "tGXCadmin123...", "signature"
])

# 4. Market maker deploys stocks
contract = client.call("deployStock", [
    "AAPL", "Apple Inc.", 1000000, "synthetic", mm_id, 150.0
])
```

---

## Benefits

### For Users
- Trade stocks on blockchain
- Full traceability of all trades
- Lower fees than traditional exchanges
- 24/7 trading
- Instant settlement

### For Market Makers
- Automated market making
- Transparent operations
- Lower operational costs
- Global reach
- Regulatory compliance built-in

### For Administrators
- Complete audit trail
- Automated compliance checks
- Real-time monitoring
- Fraud detection
- Regulatory reporting

---

## Security Features

1. **Transaction Traceability**: Every trade is fully traceable
2. **Admin Authorization**: Multi-signature admin operations
3. **Market Maker Verification**: KYC/AML compliance
4. **Audit Logging**: Complete audit trail
5. **Fraud Detection**: Automated monitoring
6. **Access Control**: Role-based permissions

---

## Next Steps

1. Complete RPC integration (4-8 hours)
2. Test all endpoints
3. Deploy to testnet
4. Community testing
5. Security audit
6. Mainnet deployment

---

## Support

- **Repository**: https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
- **Implementation**: See `src/StockMarketAPI.cpp` and `src/MarketMakerAdmin.cpp`
- **Version**: 2.0.0

---

**Last Updated**: 2025-12-24  
**Status**: Implementation Complete, Integration Pending  
**Priority**: Non-Critical
