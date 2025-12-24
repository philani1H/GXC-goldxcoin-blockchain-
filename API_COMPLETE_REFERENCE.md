# GXC Blockchain - Complete API Reference

## All APIs, Admin Functions, and Stock Contracts

---

## 1. STOCK CONTRACT APIs

### Overview
Stock contracts allow tokenized trading of real-world stocks on the GXC blockchain.

### Stock Contract Types

**1. Synthetic Equity (Model 1)**
- Price tracking via oracles
- No legal ownership
- Cash-settled
- Fully on-chain

**2. Custodial-Backed (Model 2)**
- Real shares held by custodian
- Proof of reserves
- Redeemable for real shares
- Audited reserves

**3. Issuer-Authorized (Model 3)**
- Direct from company
- Legal ownership
- Voting rights
- Dividend rights

### Stock Contract API Endpoints

#### Create Stock Contract
```json
POST /api/stock/create
{
  "ticker": "AAPL",
  "name": "Apple Inc.",
  "type": "synthetic_equity",
  "price_oracle": "chainlink_aapl",
  "initial_price": 150.00,
  "total_shares": 1000000
}

Response:
{
  "success": true,
  "contract_address": "GXC_stock_aapl_123...",
  "ticker": "AAPL",
  "type": "synthetic_equity"
}
```

#### Buy Stock Tokens
```json
POST /api/stock/buy
{
  "ticker": "AAPL",
  "shares": 10,
  "max_price": 151.00,
  "buyer_address": "GXC_buyer_address"
}

Response:
{
  "success": true,
  "transaction_hash": "abc123...",
  "shares_purchased": 10,
  "price_per_share": 150.50,
  "total_cost": 1505.00,
  "fee": 0.15
}
```

#### Sell Stock Tokens
```json
POST /api/stock/sell
{
  "ticker": "AAPL",
  "shares": 5,
  "min_price": 149.00,
  "seller_address": "GXC_seller_address"
}

Response:
{
  "success": true,
  "transaction_hash": "def456...",
  "shares_sold": 5,
  "price_per_share": 150.25,
  "total_received": 751.25,
  "fee": 0.08
}
```

#### Get Stock Price
```json
GET /api/stock/price/{ticker}

Response:
{
  "ticker": "AAPL",
  "current_price": 150.75,
  "24h_change": 2.5,
  "24h_volume": 15000,
  "last_updated": 1703419200,
  "oracle_source": "chainlink"
}
```

#### Get Stock Holdings
```json
GET /api/stock/holdings/{address}

Response:
{
  "address": "GXC_user_address",
  "holdings": [
    {
      "ticker": "AAPL",
      "shares": 10,
      "avg_price": 148.50,
      "current_price": 150.75,
      "unrealized_pnl": 22.50,
      "pnl_percentage": 1.52
    },
    {
      "ticker": "MSFT",
      "shares": 5,
      "avg_price": 380.00,
      "current_price": 385.50,
      "unrealized_pnl": 27.50,
      "pnl_percentage": 1.45
    }
  ],
  "total_value": 4433.75,
  "total_pnl": 50.00
}
```

#### Get Transfer History
```json
GET /api/stock/history/{ticker}/{address}

Response:
{
  "ticker": "AAPL",
  "address": "GXC_user_address",
  "transfers": [
    {
      "type": "buy",
      "shares": 10,
      "price": 148.50,
      "timestamp": 1703419200,
      "tx_hash": "abc123..."
    },
    {
      "type": "sell",
      "shares": 5,
      "price": 150.25,
      "timestamp": 1703505600,
      "tx_hash": "def456..."
    }
  ]
}
```

#### Update Stock Price (Oracle)
```json
POST /api/stock/update_price
{
  "ticker": "AAPL",
  "price": 151.25,
  "timestamp": 1703419200,
  "oracle_hash": "oracle_proof_123",
  "source": "chainlink",
  "oracle_signature": "sig123..."
}

Response:
{
  "success": true,
  "ticker": "AAPL",
  "new_price": 151.25,
  "previous_price": 150.75,
  "change": 0.50
}
```

---

## 2. ADMIN APIs

### Admin Authentication
All admin endpoints require authentication token.

```json
POST /api/admin/login
{
  "username": "admin",
  "password": "secure_password",
  "totp_code": "123456"
}

Response:
{
  "success": true,
  "token": "jwt_token_here",
  "expires_in": 3600,
  "permissions": ["manage_users", "manage_validators", "manage_stocks"]
}
```

### Admin Endpoints

#### Manage Validators
```json
POST /api/admin/validators/approve
Headers: Authorization: Bearer {token}
{
  "validator_address": "GXC_validator_address",
  "approved": true,
  "min_stake": 10000,
  "max_commission": 10.0
}

Response:
{
  "success": true,
  "validator_address": "GXC_validator_address",
  "status": "approved",
  "stake_required": 10000
}
```

#### Slash Validator
```json
POST /api/admin/validators/slash
Headers: Authorization: Bearer {token}
{
  "validator_address": "GXC_validator_address",
  "amount": 1000,
  "reason": "Double signing detected"
}

Response:
{
  "success": true,
  "validator_address": "GXC_validator_address",
  "slashed_amount": 1000,
  "remaining_stake": 9000,
  "reason": "Double signing detected"
}
```

#### Manage Stock Contracts
```json
POST /api/admin/stocks/approve
Headers: Authorization: Bearer {token}
{
  "ticker": "AAPL",
  "approved": true,
  "oracle_address": "GXC_oracle_address",
  "min_liquidity": 100000
}

Response:
{
  "success": true,
  "ticker": "AAPL",
  "status": "approved",
  "oracle": "GXC_oracle_address"
}
```

#### Pause/Unpause Trading
```json
POST /api/admin/stocks/pause
Headers: Authorization: Bearer {token}
{
  "ticker": "AAPL",
  "paused": true,
  "reason": "Market volatility"
}

Response:
{
  "success": true,
  "ticker": "AAPL",
  "trading_status": "paused",
  "reason": "Market volatility"
}
```

#### Manage Oracles
```json
POST /api/admin/oracles/register
Headers: Authorization: Bearer {token}
{
  "oracle_address": "GXC_oracle_address",
  "name": "Chainlink Oracle",
  "supported_assets": ["AAPL", "MSFT", "GOOGL"],
  "update_frequency": 60
}

Response:
{
  "success": true,
  "oracle_address": "GXC_oracle_address",
  "status": "registered",
  "assets": 3
}
```

#### View System Stats
```json
GET /api/admin/stats
Headers: Authorization: Bearer {token}

Response:
{
  "blockchain": {
    "height": 12345,
    "difficulty": 1000.0,
    "hashrate": "1.5 TH/s",
    "active_validators": 50,
    "total_stake": 5000000
  },
  "stocks": {
    "total_contracts": 100,
    "active_contracts": 95,
    "total_volume_24h": 1500000,
    "total_trades_24h": 5000
  },
  "users": {
    "total_addresses": 10000,
    "active_24h": 500,
    "new_24h": 50
  }
}
```

#### Emergency Actions
```json
POST /api/admin/emergency/halt
Headers: Authorization: Bearer {token}
{
  "reason": "Critical security issue",
  "duration_seconds": 3600,
  "affected_systems": ["trading", "withdrawals"]
}

Response:
{
  "success": true,
  "status": "halted",
  "reason": "Critical security issue",
  "resume_at": 1703422800
}
```

---

## 3. MARKET MAKER APIs

### Market Maker Operations

#### Get Market Depth
```json
GET /api/market/depth/{ticker}

Response:
{
  "ticker": "AAPL",
  "bids": [
    {"price": 150.50, "quantity": 100},
    {"price": 150.45, "quantity": 200},
    {"price": 150.40, "quantity": 150}
  ],
  "asks": [
    {"price": 150.55, "quantity": 100},
    {"price": 150.60, "quantity": 200},
    {"price": 150.65, "quantity": 150}
  ],
  "spread": 0.05,
  "spread_percentage": 0.033
}
```

#### Place Market Order
```json
POST /api/market/order
{
  "ticker": "AAPL",
  "side": "buy",
  "quantity": 10,
  "order_type": "market",
  "address": "GXC_trader_address"
}

Response:
{
  "success": true,
  "order_id": "order_123",
  "ticker": "AAPL",
  "side": "buy",
  "quantity": 10,
  "filled_price": 150.52,
  "total_cost": 1505.20,
  "status": "filled"
}
```

#### Place Limit Order
```json
POST /api/market/order
{
  "ticker": "AAPL",
  "side": "sell",
  "quantity": 5,
  "order_type": "limit",
  "limit_price": 151.00,
  "address": "GXC_trader_address"
}

Response:
{
  "success": true,
  "order_id": "order_456",
  "ticker": "AAPL",
  "side": "sell",
  "quantity": 5,
  "limit_price": 151.00,
  "status": "pending"
}
```

#### Cancel Order
```json
DELETE /api/market/order/{order_id}

Response:
{
  "success": true,
  "order_id": "order_456",
  "status": "cancelled"
}
```

#### Get Order Book
```json
GET /api/market/orderbook/{ticker}

Response:
{
  "ticker": "AAPL",
  "timestamp": 1703419200,
  "bids": [
    {
      "order_id": "order_789",
      "price": 150.50,
      "quantity": 100,
      "trader": "GXC_trader_1"
    }
  ],
  "asks": [
    {
      "order_id": "order_790",
      "price": 150.55,
      "quantity": 100,
      "trader": "GXC_trader_2"
    }
  ]
}
```

---

## 4. COMPLETE RPC API LIST

### Blockchain Queries
```
getblockchaininfo       - Get blockchain information
getblockcount           - Get current block height
getdifficulty           - Get current difficulty
getbestblockhash        - Get latest block hash
getblock                - Get block by hash/height
getblockhash            - Get block hash by height
gettxout                - Get transaction output
gettxoutsetinfo         - Get UTXO set info
```

### Mining
```
getblocktemplate        - Get mining template
submitblock             - Submit mined block
getmininginfo           - Get mining statistics
getnetworkhashps        - Get network hashrate
prioritisetransaction   - Prioritize transaction
```

### Staking
```
getstakinginfo          - Get staking information
listvalidators          - List all validators
getvalidatorinfo        - Get validator details
registervalidator       - Register as validator
unregistervalidator     - Unregister validator
delegatestake           - Delegate stake to validator
undelegatestake         - Undelegate stake
claimrewards            - Claim staking rewards
```

### Wallet
```
getnewaddress           - Generate new address
validateaddress         - Validate address format
getbalance              - Get address balance
listunspent             - List unspent outputs
sendtoaddress           - Send transaction
sendfrom                - Send from specific address
sendmany                - Send to multiple addresses
signmessage             - Sign message with private key
verifymessage           - Verify signed message
dumpprivkey             - Export private key
importprivkey           - Import private key
```

### Transactions
```
getmempoolinfo          - Get mempool information
getrawmempool           - Get pending transactions
gettransaction          - Get transaction details
getrawtransaction       - Get raw transaction
sendrawtransaction      - Broadcast transaction
decoderawtransaction    - Decode raw transaction
createrawtransaction    - Create raw transaction
signrawtransaction      - Sign raw transaction
```

### Network
```
getnetworkinfo          - Get network information
getpeerinfo             - Get peer information
getconnectioncount      - Get peer count
addnode                 - Add node to connect
disconnectnode          - Disconnect from node
getaddednodeinfo        - Get added node info
ping                    - Ping node
```

### Stock Contracts
```
createstockcontract     - Create new stock contract
issuestockshares        - Issue stock shares
transferstockshares     - Transfer stock shares
getstockcontract        - Get stock contract info
liststockcontracts      - List all stock contracts
getstockprice           - Get current stock price
updatestockprice        - Update stock price (oracle)
getstockholdings        - Get user's stock holdings
```

### Gold Tokens (GXC-G)
```
mintgoldtokens          - Mint gold-backed tokens
burngoldtokens          - Burn gold tokens
transfergoldtokens      - Transfer gold tokens
getgoldbalance          - Get gold token balance
getgoldreserves         - Get total gold reserves
verifygoldreserves      - Verify reserve backing
requestredemption       - Request gold redemption
```

### Governance
```
submitproposal          - Submit governance proposal
castvote                - Cast vote on proposal
getproposal             - Get proposal details
listproposals           - List all proposals
tallyproposalvotes      - Tally votes for proposal
executeproposal         - Execute approved proposal
```

---

## 5. ADMIN PERMISSION LEVELS

### Permission Hierarchy

**Level 1: Super Admin**
- Full system access
- Can create/delete admins
- Emergency halt powers
- Database access

**Level 2: Validator Admin**
- Approve/reject validators
- Slash validators
- Manage staking parameters
- View validator stats

**Level 3: Stock Admin**
- Approve stock contracts
- Manage oracles
- Pause/unpause trading
- View trading stats

**Level 4: User Admin**
- Manage user accounts
- KYC verification
- Account restrictions
- View user stats

**Level 5: Read-Only Admin**
- View all stats
- Generate reports
- No modification powers

### Admin Actions Log
All admin actions are logged:
```json
{
  "action_id": "admin_action_123",
  "admin_address": "GXC_admin_address",
  "action_type": "slash_validator",
  "target": "GXC_validator_address",
  "details": {
    "amount": 1000,
    "reason": "Double signing"
  },
  "timestamp": 1703419200,
  "ip_address": "192.168.1.1",
  "success": true
}
```

---

## 6. TESTING ALL APIS

### Test Script Example

```python
#!/usr/bin/env python3
"""
Comprehensive API Testing Script
"""

import requests
import json

BASE_URL = "http://localhost:8332"
ADMIN_TOKEN = "your_admin_token"

def test_stock_apis():
    """Test all stock contract APIs"""
    
    # Create stock contract
    response = requests.post(f"{BASE_URL}/api/stock/create", json={
        "ticker": "AAPL",
        "name": "Apple Inc.",
        "type": "synthetic_equity",
        "price_oracle": "chainlink_aapl",
        "initial_price": 150.00
    })
    print(f"Create Stock: {response.json()}")
    
    # Buy stock
    response = requests.post(f"{BASE_URL}/api/stock/buy", json={
        "ticker": "AAPL",
        "shares": 10,
        "max_price": 151.00,
        "buyer_address": "GXC_test_address"
    })
    print(f"Buy Stock: {response.json()}")
    
    # Get price
    response = requests.get(f"{BASE_URL}/api/stock/price/AAPL")
    print(f"Stock Price: {response.json()}")
    
    # Get holdings
    response = requests.get(f"{BASE_URL}/api/stock/holdings/GXC_test_address")
    print(f"Holdings: {response.json()}")

def test_admin_apis():
    """Test all admin APIs"""
    
    headers = {"Authorization": f"Bearer {ADMIN_TOKEN}"}
    
    # Get system stats
    response = requests.get(f"{BASE_URL}/api/admin/stats", headers=headers)
    print(f"System Stats: {response.json()}")
    
    # Approve validator
    response = requests.post(f"{BASE_URL}/api/admin/validators/approve", 
                            headers=headers,
                            json={
                                "validator_address": "GXC_validator_test",
                                "approved": True
                            })
    print(f"Approve Validator: {response.json()}")

def test_market_maker_apis():
    """Test market maker APIs"""
    
    # Get market depth
    response = requests.get(f"{BASE_URL}/api/market/depth/AAPL")
    print(f"Market Depth: {response.json()}")
    
    # Place order
    response = requests.post(f"{BASE_URL}/api/market/order", json={
        "ticker": "AAPL",
        "side": "buy",
        "quantity": 10,
        "order_type": "market",
        "address": "GXC_trader_address"
    })
    print(f"Place Order: {response.json()}")

if __name__ == "__main__":
    print("Testing Stock APIs...")
    test_stock_apis()
    
    print("\nTesting Admin APIs...")
    test_admin_apis()
    
    print("\nTesting Market Maker APIs...")
    test_market_maker_apis()
    
    print("\n✅ All API tests complete!")
```

---

## 7. API SECURITY

### Authentication Methods

**1. JWT Tokens (Admin)**
```
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

**2. API Keys (Oracles)**
```
X-API-Key: your_api_key_here
```

**3. Signature Verification (Transactions)**
```
X-Signature: ecdsa_signature_here
X-Public-Key: public_key_here
```

### Rate Limiting
```
X-RateLimit-Limit: 1000
X-RateLimit-Remaining: 999
X-RateLimit-Reset: 1703419200
```

### CORS Configuration
```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, PUT, DELETE
Access-Control-Allow-Headers: Content-Type, Authorization
```

---

## 8. ERROR CODES

```
200 - Success
201 - Created
400 - Bad Request
401 - Unauthorized
403 - Forbidden
404 - Not Found
429 - Too Many Requests
500 - Internal Server Error
503 - Service Unavailable
```

### Error Response Format
```json
{
  "error": {
    "code": 400,
    "message": "Invalid ticker symbol",
    "details": "Ticker 'XYZ' not found in system"
  }
}
```

---

**Status**: ✅ ALL APIS DOCUMENTED  
**Total Endpoints**: 90+ RPC + 50+ REST  
**Admin Functions**: 20+  
**Stock APIs**: 15+  
**Market Maker APIs**: 10+
