# GXC Address System Explained

## How Market Maker Addresses Work

**Version**: 3.0  
**Date**: December 22, 2025

---

## Quick Answer

**Q: Is a market maker address different from a regular address?**

**A: NO - The address format is the same. The difference is in the AUTHORIZATION stored on-chain.**

---

## Address Format

### All Addresses Look the Same

```
Regular User:     tGXC3tz6MsTbP2NCMvc3Qw8K...
Market Maker:     tGXC_mm_goldman123abc456...
Admin:            tGXC_admin_philani789xyz...
```

**Format**: `tGXC` + Base58 encoded public key hash

**Length**: 34-35 characters

**Same for everyone**: Users, market makers, admins all use the same address format.

---

## What Makes Them Different?

### The Difference is AUTHORIZATION, Not Format

```
┌─────────────────────────────────────────────────────────────┐
│                    Blockchain State                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Address: tGXC3tz6MsTbP2NCMvc3...                           │
│  ├─ Type: USER                                              │
│  ├─ Authorized: NO                                          │
│  ├─ Permissions: []                                         │
│  └─ Can Do: Send/receive GXC, buy/sell stocks              │
│                                                              │
│  Address: tGXC_mm_goldman123...                             │
│  ├─ Type: MARKET_MAKER                                      │
│  ├─ Authorized: YES                                         │
│  ├─ Authorized At: Block 12345                             │
│  ├─ Authorized By: tGXC_admin_philani                      │
│  ├─ License: MM-12345-US                                    │
│  ├─ Permissions: [                                          │
│  │    "DEPLOY_STOCK",                                       │
│  │    "UPDATE_PRICE",                                       │
│  │    "ADD_LIQUIDITY",                                      │
│  │    "EXECUTE_CORPORATE_ACTION"                            │
│  │  ]                                                        │
│  └─ Can Do: Everything + deploy stocks + update prices     │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## How Authorization Works

### Step-by-Step Process

#### Step 1: Create Regular Address

```bash
# User creates wallet
gxc-cli getnewaddress

# Output:
tGXC_user_goldman123abc456...
```

**At this point**:
- ✅ Can send/receive GXC
- ✅ Can buy/sell stock tokens
- ❌ Cannot deploy stock contracts
- ❌ Cannot update stock prices

#### Step 2: Apply for Market Maker Status

```bash
# User submits application
curl -X POST https://api.gxc.io/v1/market-maker/apply \
  -d '{
    "address": "tGXC_user_goldman123...",
    "company_name": "Goldman Sachs Trading",
    "license_number": "MM-12345-US",
    "country": "USA",
    "kyc_documents": "..."
  }'
```

**Application includes**:
- Company name
- License number
- KYC/AML documents
- Proof of financial standing
- Technical capabilities

#### Step 3: GXC Admin Reviews

```bash
# Admin reviews application
# Verifies:
# - License is valid
# - Company is legitimate
# - KYC/AML passed
# - Financial standing adequate
```

#### Step 4: Admin Authorizes

```bash
# Admin creates authorization transaction
gxc-cli authorize-market-maker \
  --address=tGXC_user_goldman123... \
  --license=MM-12345-US \
  --permissions=DEPLOY_STOCK,UPDATE_PRICE,ADD_LIQUIDITY

# This creates a transaction:
{
  "tx_type": "AUTHORIZE_MARKET_MAKER",
  "tx_hash": "0xauth123...",
  "address": "tGXC_user_goldman123...",
  "authorized_by": "tGXC_admin_philani",
  "permissions": [
    "DEPLOY_STOCK",
    "UPDATE_PRICE",
    "ADD_LIQUIDITY",
    "EXECUTE_CORPORATE_ACTION"
  ],
  "license": "MM-12345-US",
  "timestamp": 1703001234
}
```

**This transaction is stored on-chain permanently.**

#### Step 5: Address is Now Authorized

```bash
# Check authorization status
curl https://api.gxc.io/v1/market-maker/status \
  -d '{"address": "tGXC_user_goldman123..."}'

# Response:
{
  "address": "tGXC_user_goldman123...",
  "authorized": true,
  "type": "MARKET_MAKER",
  "license": "MM-12345-US",
  "authorized_at": 1703001234,
  "authorized_by": "tGXC_admin_philani",
  "permissions": [
    "DEPLOY_STOCK",
    "UPDATE_PRICE",
    "ADD_LIQUIDITY",
    "EXECUTE_CORPORATE_ACTION"
  ]
}
```

**Now the address can**:
- ✅ Deploy stock contracts
- ✅ Update stock prices
- ✅ Add liquidity
- ✅ Execute corporate actions

---

## Permission Enforcement

### How Blockchain Checks Authorization

```cpp
// When market maker tries to deploy stock
bool StockMarketAPI::deployStock(const std::string& makerAddress, ...) {
    // Step 1: Check if address is authorized
    if (!isAuthorizedMarketMaker(makerAddress)) {
        LOG_ERROR("Unauthorized market maker: " + makerAddress);
        return false;  // REJECT
    }
    
    // Step 2: Check if has DEPLOY_STOCK permission
    if (!hasPermission(makerAddress, "DEPLOY_STOCK")) {
        LOG_ERROR("No DEPLOY_STOCK permission: " + makerAddress);
        return false;  // REJECT
    }
    
    // Step 3: All checks passed, allow deployment
    return deployStockContract(...);  // ALLOW
}

bool isAuthorizedMarketMaker(const std::string& address) {
    // Query blockchain state
    auto auth = blockchain->getAuthorization(address);
    
    // Check if authorized
    return auth.authorized && auth.type == "MARKET_MAKER";
}
```

### Example: Unauthorized Attempt

```bash
# Regular user tries to deploy stock
curl -X POST https://api.gxc.io/v1/stock/deploy \
  -d '{
    "market_maker_address": "tGXC_regular_user123...",
    "ticker": "AAPL",
    ...
  }'

# Response:
{
  "success": false,
  "error": "Unauthorized market maker",
  "details": "Address tGXC_regular_user123... is not authorized to deploy stocks"
}
```

### Example: Authorized Attempt

```bash
# Authorized market maker deploys stock
curl -X POST https://api.gxc.io/v1/stock/deploy \
  -d '{
    "market_maker_address": "tGXC_mm_goldman123...",
    "ticker": "AAPL",
    ...
  }'

# Response:
{
  "success": true,
  "contract_address": "0xabc123...",
  "ticker": "AAPL",
  "message": "Stock contract deployed successfully"
}
```

---

## Address Types

### 1. Regular User Address

```
Address: tGXC3tz6MsTbP2NCMvc3...
Type: USER
Authorized: NO
Permissions: []

Can Do:
✅ Send GXC
✅ Receive GXC
✅ Buy stock tokens
✅ Sell stock tokens
✅ Transfer stock tokens
❌ Deploy stock contracts
❌ Update stock prices
❌ Execute corporate actions
```

### 2. Market Maker Address

```
Address: tGXC_mm_goldman123...
Type: MARKET_MAKER
Authorized: YES
License: MM-12345-US
Permissions: [
  "DEPLOY_STOCK",
  "UPDATE_PRICE",
  "ADD_LIQUIDITY",
  "EXECUTE_CORPORATE_ACTION"
]

Can Do:
✅ Send GXC
✅ Receive GXC
✅ Buy stock tokens
✅ Sell stock tokens
✅ Transfer stock tokens
✅ Deploy stock contracts ⭐
✅ Update stock prices ⭐
✅ Add liquidity ⭐
✅ Execute corporate actions ⭐
```

### 3. Admin Address

```
Address: tGXC_admin_philani...
Type: ADMIN
Role: SUPER_ADMIN
Permissions: [
  "AUTHORIZE_MARKET_MAKER",
  "REVOKE_MARKET_MAKER",
  "CREATE_ADMIN",
  "REMOVE_ADMIN",
  "APPROVE_APPLICATION"
]

Can Do:
✅ Everything users can do
✅ Authorize market makers ⭐
✅ Revoke market makers ⭐
✅ Create other admins ⭐
✅ Approve applications ⭐
```

---

## Traceability of Authorization

### Authorization is Traceable

```
Genesis Block
  └─> Block 100: Admin Created (Philani)
      └─> Block 150: Market Maker Application (Goldman)
          └─> Block 200: Market Maker Authorized (Goldman)
              └─> Block 250: Stock Deployed (AAPL by Goldman)
```

**Every authorization**:
- Creates a transaction on-chain
- References previous transaction
- Cannot be altered
- Permanently recorded

### Verify Authorization

```bash
# Trace authorization chain
curl https://api.gxc.io/v1/market-maker/trace \
  -d '{"address": "tGXC_mm_goldman123..."}'

# Response:
{
  "address": "tGXC_mm_goldman123...",
  "authorization_chain": [
    {
      "block": 100,
      "tx_hash": "0xadmin...",
      "action": "ADMIN_CREATED",
      "admin": "tGXC_admin_philani"
    },
    {
      "block": 150,
      "tx_hash": "0xapply...",
      "action": "MM_APPLICATION",
      "applicant": "tGXC_mm_goldman123...",
      "license": "MM-12345-US"
    },
    {
      "block": 200,
      "tx_hash": "0xauth...",
      "action": "MM_AUTHORIZED",
      "authorized_by": "tGXC_admin_philani",
      "address": "tGXC_mm_goldman123..."
    }
  ],
  "verified": true
}
```

---

## Revoking Authorization

### Admin Can Revoke

```bash
# Admin revokes market maker
gxc-cli revoke-market-maker \
  --address=tGXC_mm_goldman123... \
  --reason="License expired"

# This creates a transaction:
{
  "tx_type": "REVOKE_MARKET_MAKER",
  "tx_hash": "0xrevoke123...",
  "address": "tGXC_mm_goldman123...",
  "revoked_by": "tGXC_admin_philani",
  "reason": "License expired",
  "timestamp": 1703001234
}
```

**After revocation**:
- ❌ Cannot deploy new stocks
- ❌ Cannot update prices
- ✅ Existing stocks still work
- ✅ Can still send/receive GXC

---

## Summary

### Key Points

1. **Same Address Format**
   - All addresses use the same format
   - No special prefix or suffix
   - Just regular GXC addresses

2. **Authorization is On-Chain**
   - Stored in blockchain state
   - Created by admin transaction
   - Permanently recorded
   - Fully traceable

3. **Permission-Based**
   - Different addresses have different permissions
   - Blockchain checks permissions before allowing actions
   - Unauthorized actions are rejected

4. **Traceable**
   - Every authorization creates a transaction
   - Complete chain back to admin
   - Cannot be altered
   - Publicly verifiable

### Visual Summary

```
┌──────────────────────────────────────────────────────────┐
│                    Address Format                         │
│                  (Same for Everyone)                      │
├──────────────────────────────────────────────────────────┤
│                                                           │
│  tGXC + Base58(PublicKeyHash)                            │
│                                                           │
│  Example: tGXC3tz6MsTbP2NCMvc3Qw8K...                    │
│                                                           │
└──────────────────────────────────────────────────────────┘
                          │
                          ▼
┌──────────────────────────────────────────────────────────┐
│              Authorization (On-Chain)                     │
│              (Different for Each Type)                    │
├──────────────────────────────────────────────────────────┤
│                                                           │
│  Regular User:    authorized = false                     │
│  Market Maker:    authorized = true + permissions        │
│  Admin:           authorized = true + admin permissions  │
│                                                           │
└──────────────────────────────────────────────────────────┘
```

---

**🔐 GXC: Same Address Format, Different Permissions 🔐**

**Version**: 3.0  
**Date**: December 22, 2025
