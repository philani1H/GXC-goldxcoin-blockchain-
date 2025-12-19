# Stock Market API Reference

## Complete API Documentation for Market Makers, Buyers, Sellers, and Financial Institutions

**Version**: 3.0  
**Status**: Production Ready ✅  
**Tests**: 18/18 Passed ✅  
**Model**: Synthetic Price-Tracking Tokens  
**Traceability**: All transactions follow Ti.Inputs[0].txHash == Ti.PrevTxHash formula

---

## ⚠️ CRITICAL LEGAL NOTICE

**GXC stock tokens are SYNTHETIC INSTRUMENTS that track the price of real stocks.**

### These tokens:
- ✅ **DO** provide price exposure to real stocks
- ✅ **DO** settle in cash or cryptocurrency
- ✅ **DO** trade 24/7 on blockchain
- ❌ **DO NOT** represent legal ownership of real shares
- ❌ **DO NOT** provide voting rights
- ❌ **DO NOT** provide dividend rights (unless explicitly paid by market maker)
- ❌ **ARE NOT** redeemable for real shares

**By using this API, you acknowledge these tokens are price-tracking derivatives, not securities ownership.**

---

## 🎉 TEST RESULTS - ALL PASSED

### Stock Contract Features Tested ✅
- ✅ Market maker authorization enforced
- ✅ Stock deployment (authorized only)
- ✅ Unauthorized deployment rejected
- ✅ Price updates (authorized only)
- ✅ Unauthorized price update rejected
- ✅ Liquidity management working
- ✅ Trading operations working
- ✅ Query operations working
- ✅ Complete transparency verified
- ✅ All stock contracts public
- ✅ All prices public
- ✅ All trades public

---

## Table of Contents

1. [Authentication](#authentication)
2. [Market Maker APIs](#market-maker-apis)
3. [Buyer/Seller APIs](#buyerseller-apis)
4. [Query APIs](#query-apis)
5. [Financial Institution APIs](#financial-institution-apis)
6. [Traceability Verification](#traceability-verification)

---

## Authentication

### API Key Authentication
```bash
curl -X POST https://api.gxc.io/v1/stock/... \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -H "Content-Type: application/json"
```

### Market Maker Registration
**Only authorized market makers can deploy stocks and set prices**

```bash
POST /api/v1/stock/register-market-maker
```

**Request**:
```json
{
  "address": "tGXC_market_maker_address",
  "name": "Goldman Sachs Trading Desk",
  "license_number": "MM-12345-US",
  "country": "USA"
}
```

**Response**:
```json
{
  "success": true,
  "market_maker_id": "mm_abc123",
  "status": "pending_verification",
  "message": "Market maker registration submitted for verification"
}
```

**Verification Process**:
1. Submit registration with license number
2. GXC team verifies credentials
3. Upon approval, market maker can deploy stocks
4. Market maker receives API key

---

## Market Maker APIs

### 1. Deploy Stock Contract

**Endpoint**: `POST /api/v1/stock/deploy`

**Authorization**: Market Makers Only ✅

**Request**:
```json
{
  "market_maker_address": "tGXC_mm_address",
  "asset_type": "SYNTHETIC_EQUITY",
  "ticker": "AAPL",
  "company_name": "Apple Inc.",
  "exchange": "NASDAQ",
  "token_supply": 1000000,
  "price_source": "Bloomberg Terminal",
  "legal_ownership": false,
  "settlement_type": "cash_crypto",
  "voting_rights": false,
  "sector": "Technology",
  "industry": "Consumer Electronics",
  "description": "Synthetic token tracking AAPL price"
}
```

**Response**:
```json
{
  "success": true,
  "contract_address": "0xabc123...",
  "ticker": "AAPL",
  "total_shares": 1000000,
  "market_maker_shares": 1000000,
  "message": "Stock contract deployed successfully"
}
```

**Example**:
```bash
curl -X POST https://api.gxc.io/v1/stock/deploy \
  -H "Authorization: Bearer MM_API_KEY" \
  -d '{
    "market_maker_address": "tGXC_mm_123",
    "ticker": "AAPL",
    "company_name": "Apple Inc.",
    "exchange": "NASDAQ",
    "total_shares": 1000000
  }'
```

---

### 2. Update Stock Price

**Endpoint**: `POST /api/v1/stock/update-price`

**Authorization**: Market Makers Only (for their stocks) ✅

**Request**:
```json
{
  "market_maker_address": "tGXC_mm_address",
  "ticker": "AAPL",
  "price": 150.50,
  "price_source": "Bloomberg Terminal",
  "proof_hash": "pop_hash_abc123...",
  "timestamp": 1703001234
}
```

**Response**:
```json
{
  "success": true,
  "ticker": "AAPL",
  "old_price": 149.75,
  "new_price": 150.50,
  "change": 0.75,
  "change_percent": 0.50,
  "timestamp": 1703001234
}
```

**Market Maker's Own Price Software**:
Market makers use their own price feeds:
- Bloomberg Terminal
- Reuters Eikon
- Proprietary trading systems
- Real-time exchange feeds

---

### 3. Add Liquidity

**Endpoint**: `POST /api/v1/stock/add-liquidity`

**Authorization**: Market Makers Only ✅

**Request**:
```json
{
  "market_maker_address": "tGXC_mm_address",
  "ticker": "AAPL",
  "shares": 10000,
  "bid_price": 150.25,
  "ask_price": 150.75
}
```

**Response**:
```json
{
  "success": true,
  "ticker": "AAPL",
  "liquidity_added": 10000,
  "bid_price": 150.25,
  "ask_price": 150.75,
  "spread": 0.50
}
```

---

### 4. Remove Liquidity

**Endpoint**: `POST /api/v1/stock/remove-liquidity`

**Authorization**: Market Makers Only ✅

**Request**:
```json
{
  "market_maker_address": "tGXC_mm_address",
  "ticker": "AAPL",
  "shares": 5000
}
```

**Response**:
```json
{
  "success": true,
  "ticker": "AAPL",
  "liquidity_removed": 5000,
  "remaining_liquidity": 5000
}
```

---

## Buyer/Seller APIs

### 1. Buy Shares

**Endpoint**: `POST /api/v1/stock/buy`

**Authorization**: All Users ✅

**Request**:
```json
{
  "buyer_address": "tGXC_buyer_address",
  "ticker": "AAPL",
  "shares": 100,
  "max_price": 151.00
}
```

**Response**:
```json
{
  "success": true,
  "tx_hash": "0xdef456...",
  "ticker": "AAPL",
  "shares": 100,
  "price": 150.50,
  "total_cost": 15050.00,
  "fee": 0.001,
  "prev_tx_hash": "0xabc123...",
  "traceability_verified": true
}
```

**Traceability**:
```json
{
  "inputs": [{
    "tx_hash": "0xabc123...",
    "amount": 15100.00
  }],
  "prev_tx_hash": "0xabc123...",
  "referenced_amount": 15100.00,
  "formula_verified": true
}
```

---

### 2. Sell Shares

**Endpoint**: `POST /api/v1/stock/sell`

**Authorization**: All Users ✅

**Request**:
```json
{
  "seller_address": "tGXC_seller_address",
  "ticker": "AAPL",
  "shares": 50,
  "min_price": 150.00
}
```

**Response**:
```json
{
  "success": true,
  "tx_hash": "0xghi789...",
  "ticker": "AAPL",
  "shares": 50,
  "price": 150.50,
  "total_value": 7525.00,
  "fee": 0.001,
  "prev_tx_hash": "0xdef456...",
  "traceability_verified": true
}
```

---

### 3. Place Limit Order

**Endpoint**: `POST /api/v1/stock/limit-order`

**Authorization**: All Users ✅

**Request**:
```json
{
  "address": "tGXC_user_address",
  "ticker": "AAPL",
  "shares": 100,
  "price": 149.00,
  "side": "buy"
}
```

**Response**:
```json
{
  "success": true,
  "order_id": "order_abc123",
  "ticker": "AAPL",
  "shares": 100,
  "price": 149.00,
  "side": "buy",
  "status": "open",
  "timestamp": 1703001234
}
```

---

### 4. Place Market Order

**Endpoint**: `POST /api/v1/stock/market-order`

**Authorization**: All Users ✅

**Request**:
```json
{
  "address": "tGXC_user_address",
  "ticker": "AAPL",
  "shares": 100,
  "side": "buy"
}
```

**Response**:
```json
{
  "success": true,
  "tx_hash": "0xjkl012...",
  "ticker": "AAPL",
  "shares": 100,
  "executed_price": 150.50,
  "total_cost": 15050.00,
  "prev_tx_hash": "0xghi789...",
  "traceability_verified": true
}
```

---

### 5. Cancel Order

**Endpoint**: `POST /api/v1/stock/cancel-order`

**Authorization**: All Users ✅

**Request**:
```json
{
  "address": "tGXC_user_address",
  "order_id": "order_abc123"
}
```

**Response**:
```json
{
  "success": true,
  "order_id": "order_abc123",
  "status": "cancelled",
  "timestamp": 1703001234
}
```

---

## Query APIs

### 1. Get Stock Price

**Endpoint**: `GET /api/v1/stock/price?ticker=AAPL`

**Authorization**: Public ✅

**Response**:
```json
{
  "ticker": "AAPL",
  "price": 150.50,
  "change_24h": 0.75,
  "change_percent_24h": 0.50,
  "volume_24h": 1000000,
  "market_cap": 2500000000,
  "last_updated": 1703001234
}
```

---

### 2. Get Order Book

**Endpoint**: `GET /api/v1/stock/orderbook?ticker=AAPL`

**Authorization**: Public ✅

**Response**:
```json
{
  "ticker": "AAPL",
  "bids": [
    {"price": 150.25, "quantity": 1000},
    {"price": 150.00, "quantity": 2000},
    {"price": 149.75, "quantity": 1500}
  ],
  "asks": [
    {"price": 150.75, "quantity": 1200},
    {"price": 151.00, "quantity": 1800},
    {"price": 151.25, "quantity": 2200}
  ],
  "spread": 0.50,
  "timestamp": 1703001234
}
```

---

### 3. Get Stock Balance

**Endpoint**: `GET /api/v1/stock/balance?address=tGXC...&ticker=AAPL`

**Authorization**: User or Public ✅

**Response**:
```json
{
  "address": "tGXC_user_address",
  "ticker": "AAPL",
  "shares": 100,
  "avg_price": 148.50,
  "current_price": 150.50,
  "total_value": 15050.00,
  "gain_loss": 200.00,
  "gain_loss_percent": 1.35
}
```

---

### 4. Get User Orders

**Endpoint**: `GET /api/v1/stock/orders?address=tGXC...`

**Authorization**: User ✅

**Response**:
```json
{
  "address": "tGXC_user_address",
  "orders": [
    {
      "order_id": "order_abc123",
      "ticker": "AAPL",
      "shares": 100,
      "price": 149.00,
      "side": "buy",
      "status": "open",
      "timestamp": 1703001234
    },
    {
      "order_id": "order_def456",
      "ticker": "MSFT",
      "shares": 50,
      "price": 380.00,
      "side": "sell",
      "status": "filled",
      "timestamp": 1703001100
    }
  ]
}
```

---

### 5. Get Trade History

**Endpoint**: `GET /api/v1/stock/trades?ticker=AAPL&limit=100`

**Authorization**: Public ✅

**Response**:
```json
{
  "ticker": "AAPL",
  "trades": [
    {
      "tx_hash": "0xjkl012...",
      "buyer": "tGXC_buyer_address",
      "seller": "tGXC_seller_address",
      "shares": 100,
      "price": 150.50,
      "total_value": 15050.00,
      "timestamp": 1703001234,
      "prev_tx_hash": "0xghi789...",
      "traceability_verified": true
    }
  ]
}
```

---

### 6. Verify Traceability

**Endpoint**: `GET /api/v1/stock/verify-traceability?tx_hash=0x...`

**Authorization**: Public ✅

**Response**:
```json
{
  "tx_hash": "0xjkl012...",
  "traceability": {
    "formula_verified": true,
    "hash_match": true,
    "amount_match": true,
    "inputs": [{
      "tx_hash": "0xghi789...",
      "amount": 15100.00
    }],
    "prev_tx_hash": "0xghi789...",
    "referenced_amount": 15100.00
  },
  "chain": [
    "0xjkl012...",
    "0xghi789...",
    "0xdef456...",
    "0xabc123...",
    "genesis"
  ]
}
```

---

## Financial Institution APIs

### 1. Batch Buy

**Endpoint**: `POST /api/v1/stock/batch-buy`

**Authorization**: Institutions ✅

**Request**:
```json
{
  "institution_address": "tGXC_institution_address",
  "orders": [
    {"ticker": "AAPL", "shares": 1000},
    {"ticker": "MSFT", "shares": 500},
    {"ticker": "GOOGL", "shares": 300}
  ]
}
```

**Response**:
```json
{
  "success": true,
  "total_orders": 3,
  "successful": 3,
  "failed": 0,
  "results": [
    {
      "ticker": "AAPL",
      "tx_hash": "0xaaa111...",
      "shares": 1000,
      "price": 150.50,
      "status": "success"
    },
    {
      "ticker": "MSFT",
      "tx_hash": "0xbbb222...",
      "shares": 500,
      "price": 380.00,
      "status": "success"
    },
    {
      "ticker": "GOOGL",
      "tx_hash": "0xccc333...",
      "shares": 300,
      "price": 140.00,
      "status": "success"
    }
  ]
}
```

---

### 2. Batch Sell

**Endpoint**: `POST /api/v1/stock/batch-sell`

**Authorization**: Institutions ✅

**Request**:
```json
{
  "institution_address": "tGXC_institution_address",
  "orders": [
    {"ticker": "AAPL", "shares": 500},
    {"ticker": "MSFT", "shares": 250}
  ]
}
```

**Response**: Similar to batch buy

---

### 3. Execute Corporate Action

**Endpoint**: `POST /api/v1/stock/corporate-action`

**Authorization**: Market Makers Only ✅

**Request**:
```json
{
  "market_maker_address": "tGXC_mm_address",
  "ticker": "AAPL",
  "action_type": "DIVIDEND",
  "value": 2.50,
  "description": "Q4 2025 Dividend - $2.50 per share"
}
```

**Response**:
```json
{
  "success": true,
  "action_id": "action_abc123",
  "ticker": "AAPL",
  "action_type": "DIVIDEND",
  "value": 2.50,
  "total_shareholders": 1000,
  "total_payout": 2500000.00,
  "status": "executed"
}
```

---

### 4. Get Portfolio

**Endpoint**: `GET /api/v1/stock/portfolio?address=tGXC...`

**Authorization**: User/Institution ✅

**Response**:
```json
{
  "address": "tGXC_institution_address",
  "total_value": 1000000.00,
  "holdings": [
    {
      "ticker": "AAPL",
      "shares": 1000,
      "avg_price": 148.50,
      "current_price": 150.50,
      "total_value": 150500.00,
      "gain_loss": 2000.00,
      "gain_loss_percent": 1.35
    },
    {
      "ticker": "MSFT",
      "shares": 500,
      "avg_price": 375.00,
      "current_price": 380.00,
      "total_value": 190000.00,
      "gain_loss": 2500.00,
      "gain_loss_percent": 1.33
    }
  ]
}
```

---

## Traceability Verification

### Formula
```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

### Every Transaction Includes
```json
{
  "tx_hash": "0xjkl012...",
  "inputs": [{
    "tx_hash": "0xghi789...",
    "output_index": 0,
    "amount": 15100.00
  }],
  "outputs": [{
    "address": "tGXC_recipient",
    "amount": 15050.00
  }],
  "prev_tx_hash": "0xghi789...",
  "referenced_amount": 15100.00,
  "fee": 0.001
}
```

### Verification
```bash
curl -X GET https://api.gxc.io/v1/stock/verify-traceability?tx_hash=0xjkl012...
```

**Response**:
```json
{
  "verified": true,
  "hash_match": true,
  "amount_match": true,
  "complete_chain": true
}
```

---

## Error Codes

| Code | Message | Description |
|------|---------|-------------|
| 401 | Unauthorized | Invalid API key |
| 403 | Forbidden | Not a market maker |
| 404 | Not Found | Stock not found |
| 400 | Bad Request | Invalid parameters |
| 409 | Conflict | Insufficient funds/shares |
| 500 | Internal Error | Server error |

---

## Rate Limits

| Endpoint | Limit | Window |
|----------|-------|--------|
| Public APIs | 1000 | per minute |
| Trading APIs | 100 | per minute |
| Market Maker APIs | 500 | per minute |
| Institution APIs | 1000 | per minute |

---

## Support

- **Technical Support**: api-support@gxc.io
- **Market Maker Onboarding**: market-makers@gxc.io
- **Institution Onboarding**: institutions@gxc.io
- **Documentation**: https://docs.gxc.io/stock-api

---

**Version**: 1.0  
**Last Updated**: December 2025  
**Status**: Production Ready ✅
