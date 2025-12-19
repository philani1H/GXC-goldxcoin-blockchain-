# GXC Blockchain Explorer API Documentation

## Complete Transparency - Users Can See Everything

**Version**: 1.0  
**Status**: Production Ready ✅  
**Transparency**: 100% - All data is public

---

## Overview

### What Users Can See

**EVERYTHING!** ✅

The GXC blockchain is **completely transparent**. Anyone can see:

1. ✅ **All Blocks** - Every block ever mined
2. ✅ **All Transactions** - Every transaction ever made
3. ✅ **All Addresses** - All addresses and their balances
4. ✅ **All UTXOs** - All unspent transaction outputs
5. ✅ **All Validators** - All validators and their stakes
6. ✅ **All Stock Contracts** - All deployed stocks
7. ✅ **All Stock Transactions** - All buy/sell operations
8. ✅ **Complete Traceability** - Every coin traceable to genesis

**No Hidden Data** - Everything is public and verifiable!

---

## Block Explorer APIs

### 1. Get Latest Blocks

**Endpoint**: `GET /api/v1/explorer/blocks/latest?limit=10`

**Authorization**: Public (no auth required) ✅

**Response**:
```json
{
  "blocks": [
    {
      "height": 12345,
      "hash": "0xabc123...",
      "timestamp": 1703001234,
      "miner": "tGXC_miner_address",
      "transactions": 15,
      "size": 2048,
      "difficulty": 1000.0,
      "block_reward": 50.0,
      "type": "POW_SHA256"
    },
    // ... more blocks
  ]
}
```

**What Users See**:
- Block height
- Block hash
- When it was mined
- Who mined it
- How many transactions
- Block size
- Mining difficulty
- Block reward

---

### 2. Get Block Details

**Endpoint**: `GET /api/v1/explorer/block/{height_or_hash}`

**Authorization**: Public ✅

**Example**: `GET /api/v1/explorer/block/12345`

**Response**:
```json
{
  "height": 12345,
  "hash": "0xabc123...",
  "previous_hash": "0xdef456...",
  "timestamp": 1703001234,
  "miner": "tGXC_miner_address",
  "difficulty": 1000.0,
  "nonce": 123456789,
  "merkle_root": "0xghi789...",
  "block_reward": 50.0,
  "type": "POW_SHA256",
  "size": 2048,
  "transactions": [
    {
      "hash": "0xtx1...",
      "from": "tGXC_sender",
      "to": "tGXC_receiver",
      "amount": 100.0,
      "fee": 0.001,
      "type": "TRANSFER"
    },
    // ... all transactions in block
  ]
}
```

**What Users See**:
- Complete block details
- All transactions in the block
- Miner information
- Block reward
- Everything!

---

## Transaction Explorer APIs

### 3. Get Transaction Details

**Endpoint**: `GET /api/v1/explorer/transaction/{tx_hash}`

**Authorization**: Public ✅

**Response**:
```json
{
  "hash": "0xtx123...",
  "block_height": 12345,
  "block_hash": "0xblock123...",
  "timestamp": 1703001234,
  "from": "tGXC_sender_address",
  "to": "tGXC_receiver_address",
  "amount": 100.0,
  "fee": 0.001,
  "type": "TRANSFER",
  "inputs": [
    {
      "tx_hash": "0xprev_tx...",
      "output_index": 0,
      "amount": 100.001
    }
  ],
  "outputs": [
    {
      "address": "tGXC_receiver",
      "amount": 100.0,
      "output_index": 0
    },
    {
      "address": "tGXC_sender",
      "amount": 0.0,
      "output_index": 1
    }
  ],
  "prev_tx_hash": "0xprev_tx...",
  "referenced_amount": 100.001,
  "traceability_verified": true,
  "confirmations": 6
}
```

**What Users See**:
- Complete transaction details
- Sender and receiver
- Amount and fee
- Input UTXOs
- Output UTXOs
- Traceability information
- Confirmations

---

### 4. Get Transaction Chain (Traceability)

**Endpoint**: `GET /api/v1/explorer/transaction/{tx_hash}/chain`

**Authorization**: Public ✅

**Response**:
```json
{
  "tx_hash": "0xtx123...",
  "chain": [
    {
      "hash": "0xtx123...",
      "height": 12345,
      "amount": 100.0,
      "from": "tGXC_sender",
      "to": "tGXC_receiver"
    },
    {
      "hash": "0xprev_tx...",
      "height": 12340,
      "amount": 100.001,
      "from": "tGXC_prev_sender",
      "to": "tGXC_sender"
    },
    // ... all the way back to genesis
    {
      "hash": "genesis_tx",
      "height": 0,
      "amount": 50.0,
      "from": "coinbase",
      "to": "tGXC_genesis"
    }
  ],
  "chain_length": 25,
  "traceable_to_genesis": true
}
```

**What Users See**:
- Complete transaction history
- Every transaction in the chain
- Traceable all the way to genesis
- Proof of coin origin

---

### 5. Get Latest Transactions

**Endpoint**: `GET /api/v1/explorer/transactions/latest?limit=20`

**Authorization**: Public ✅

**Response**:
```json
{
  "transactions": [
    {
      "hash": "0xtx1...",
      "block_height": 12345,
      "timestamp": 1703001234,
      "from": "tGXC_sender1",
      "to": "tGXC_receiver1",
      "amount": 100.0,
      "type": "TRANSFER"
    },
    {
      "hash": "0xtx2...",
      "block_height": 12345,
      "timestamp": 1703001234,
      "from": "tGXC_sender2",
      "to": "tGXC_receiver2",
      "amount": 50.0,
      "type": "TRANSFER"
    },
    // ... more transactions
  ]
}
```

---

## Address Explorer APIs

### 6. Get Address Details

**Endpoint**: `GET /api/v1/explorer/address/{address}`

**Authorization**: Public ✅

**Response**:
```json
{
  "address": "tGXC_user_address",
  "balance": 1000.50,
  "transaction_count": 125,
  "first_seen": 1703001234,
  "last_seen": 1703005678,
  "is_validator": true,
  "validator_stake": 500.0,
  "utxo_count": 15,
  "stock_holdings": [
    {
      "ticker": "AAPL",
      "shares": 100,
      "value": 15050.0
    }
  ]
}
```

**What Users See**:
- Address balance
- Transaction count
- When first/last seen
- If it's a validator
- Validator stake
- UTXO count
- Stock holdings

---

### 7. Get Address Transactions

**Endpoint**: `GET /api/v1/explorer/address/{address}/transactions?limit=50`

**Authorization**: Public ✅

**Response**:
```json
{
  "address": "tGXC_user_address",
  "transactions": [
    {
      "hash": "0xtx1...",
      "block_height": 12345,
      "timestamp": 1703001234,
      "type": "TRANSFER",
      "direction": "received",
      "amount": 100.0,
      "from": "tGXC_sender",
      "to": "tGXC_user_address"
    },
    {
      "hash": "0xtx2...",
      "block_height": 12340,
      "timestamp": 1703001000,
      "type": "TRANSFER",
      "direction": "sent",
      "amount": 50.0,
      "from": "tGXC_user_address",
      "to": "tGXC_receiver"
    },
    // ... all transactions
  ]
}
```

---

### 8. Get Address UTXOs

**Endpoint**: `GET /api/v1/explorer/address/{address}/utxos`

**Authorization**: Public ✅

**Response**:
```json
{
  "address": "tGXC_user_address",
  "utxos": [
    {
      "tx_hash": "0xtx1...",
      "output_index": 0,
      "amount": 100.0,
      "confirmations": 6,
      "spendable": true
    },
    {
      "tx_hash": "0xtx2...",
      "output_index": 1,
      "amount": 50.0,
      "confirmations": 3,
      "spendable": true
    }
  ],
  "total_balance": 150.0
}
```

**What Users See**:
- All unspent outputs
- Amount in each UTXO
- Confirmations
- Spendable status

---

### 9. Get Richest Addresses

**Endpoint**: `GET /api/v1/explorer/addresses/richest?limit=100`

**Authorization**: Public ✅

**Response**:
```json
{
  "addresses": [
    {
      "rank": 1,
      "address": "tGXC_richest_address",
      "balance": 1000000.0,
      "percentage": 5.5
    },
    {
      "rank": 2,
      "address": "tGXC_second_richest",
      "balance": 500000.0,
      "percentage": 2.75
    },
    // ... top 100
  ],
  "total_supply": 18000000.0
}
```

**What Users See**:
- Top addresses by balance
- Balance of each address
- Percentage of total supply
- Complete wealth distribution

---

## Validator Explorer APIs

### 10. Get All Validators

**Endpoint**: `GET /api/v1/explorer/validators`

**Authorization**: Public ✅

**Response**:
```json
{
  "validators": [
    {
      "address": "tGXC_validator1",
      "stake": 1000.0,
      "blocks_mined": 125,
      "status": "active",
      "uptime": 99.9,
      "last_block": 12345
    },
    {
      "address": "tGXC_validator2",
      "stake": 500.0,
      "blocks_mined": 75,
      "status": "active",
      "uptime": 98.5,
      "last_block": 12340
    }
  ],
  "total_validators": 25,
  "total_stake": 50000.0
}
```

**What Users See**:
- All validators
- Stake amount
- Blocks mined
- Status
- Uptime
- Last block mined

---

### 11. Get Validator Details

**Endpoint**: `GET /api/v1/explorer/validator/{address}`

**Authorization**: Public ✅

**Response**:
```json
{
  "address": "tGXC_validator1",
  "stake": 1000.0,
  "blocks_mined": 125,
  "status": "active",
  "registered_at": 1703001234,
  "last_block": 12345,
  "uptime": 99.9,
  "rewards_earned": 6250.0,
  "blocks": [
    {
      "height": 12345,
      "hash": "0xblock1...",
      "timestamp": 1703005678,
      "transactions": 15,
      "reward": 50.0
    },
    // ... recent blocks
  ]
}
```

---

## Stock Contract Explorer APIs

### 12. Get All Stocks

**Endpoint**: `GET /api/v1/explorer/stocks`

**Authorization**: Public ✅

**Response**:
```json
{
  "stocks": [
    {
      "ticker": "AAPL",
      "company_name": "Apple Inc.",
      "exchange": "NASDAQ",
      "price": 150.50,
      "market_cap": 150500000.0,
      "total_shares": 1000000,
      "market_maker": "tGXC_mm_goldman",
      "deployed_at": 1703001234
    },
    {
      "ticker": "MSFT",
      "company_name": "Microsoft",
      "exchange": "NASDAQ",
      "price": 380.00,
      "market_cap": 380000000.0,
      "total_shares": 1000000,
      "market_maker": "tGXC_mm_citadel",
      "deployed_at": 1703001235
    }
  ]
}
```

**What Users See**:
- All deployed stocks
- Current prices
- Market cap
- Total shares
- Who deployed it
- When deployed

---

### 13. Get Stock Details

**Endpoint**: `GET /api/v1/explorer/stock/{ticker}`

**Authorization**: Public ✅

**Response**:
```json
{
  "ticker": "AAPL",
  "company_name": "Apple Inc.",
  "exchange": "NASDAQ",
  "price": 150.50,
  "change_24h": 2.50,
  "change_percent_24h": 1.69,
  "volume_24h": 50000,
  "market_cap": 150500000.0,
  "total_shares": 1000000,
  "circulating_shares": 900000,
  "market_maker": "tGXC_mm_goldman",
  "price_source": "Bloomberg Terminal",
  "last_updated": 1703005678,
  "deployed_at": 1703001234,
  "holders": 1250,
  "transactions_24h": 350
}
```

---

### 14. Get Stock Transactions

**Endpoint**: `GET /api/v1/explorer/stock/{ticker}/transactions?limit=50`

**Authorization**: Public ✅

**Response**:
```json
{
  "ticker": "AAPL",
  "transactions": [
    {
      "tx_hash": "0xtx1...",
      "timestamp": 1703005678,
      "type": "BUY",
      "buyer": "tGXC_buyer",
      "seller": "tGXC_mm_goldman",
      "shares": 100,
      "price": 150.50,
      "total_value": 15050.0
    },
    {
      "tx_hash": "0xtx2...",
      "timestamp": 1703005600,
      "type": "SELL",
      "buyer": "tGXC_mm_goldman",
      "seller": "tGXC_seller",
      "shares": 50,
      "price": 150.25,
      "total_value": 7512.50
    }
  ]
}
```

**What Users See**:
- All buy/sell transactions
- Buyer and seller
- Shares traded
- Price
- Total value
- Complete trading history

---

### 15. Get Stock Holders

**Endpoint**: `GET /api/v1/explorer/stock/{ticker}/holders`

**Authorization**: Public ✅

**Response**:
```json
{
  "ticker": "AAPL",
  "holders": [
    {
      "address": "tGXC_mm_goldman",
      "shares": 900000,
      "percentage": 90.0,
      "value": 135450000.0
    },
    {
      "address": "tGXC_user1",
      "shares": 50000,
      "percentage": 5.0,
      "value": 7525000.0
    },
    {
      "address": "tGXC_user2",
      "shares": 30000,
      "percentage": 3.0,
      "value": 4515000.0
    }
  ],
  "total_holders": 1250,
  "total_shares": 1000000
}
```

**What Users See**:
- All stock holders
- How many shares each holds
- Percentage ownership
- Current value
- Complete ownership distribution

---

## Statistics APIs

### 16. Get Blockchain Statistics

**Endpoint**: `GET /api/v1/explorer/stats/blockchain`

**Authorization**: Public ✅

**Response**:
```json
{
  "total_blocks": 12345,
  "total_transactions": 125000,
  "total_addresses": 5000,
  "total_validators": 25,
  "total_supply": 18000000.0,
  "circulating_supply": 17500000.0,
  "burned": 500000.0,
  "average_block_time": 60.5,
  "transactions_per_second": 15.5,
  "network_hashrate": 1000000.0
}
```

---

### 17. Get Market Statistics

**Endpoint**: `GET /api/v1/explorer/stats/market`

**Authorization**: Public ✅

**Response**:
```json
{
  "total_stocks": 50,
  "total_market_cap": 5000000000.0,
  "trading_volume_24h": 50000000.0,
  "transactions_24h": 5000,
  "active_traders_24h": 1250,
  "top_stocks": [
    {
      "ticker": "AAPL",
      "volume_24h": 5000000.0,
      "transactions_24h": 500
    }
  ]
}
```

---

## Search API

### 18. Universal Search

**Endpoint**: `GET /api/v1/explorer/search?q={query}`

**Authorization**: Public ✅

**Searches**: Blocks, transactions, addresses, validators, stocks

**Example**: `GET /api/v1/explorer/search?q=AAPL`

**Response**:
```json
{
  "query": "AAPL",
  "results": {
    "stocks": [
      {
        "ticker": "AAPL",
        "company_name": "Apple Inc.",
        "price": 150.50
      }
    ],
    "transactions": [
      {
        "hash": "0xtx1...",
        "type": "STOCK_BUY",
        "ticker": "AAPL"
      }
    ],
    "addresses": []
  }
}
```

---

## Transparency Summary

### What Makes GXC Transparent?

**1. All Blocks Are Public** ✅
- Anyone can see every block
- Complete block history
- No hidden blocks

**2. All Transactions Are Public** ✅
- Anyone can see every transaction
- Complete transaction history
- No hidden transactions

**3. All Addresses Are Public** ✅
- Anyone can see all addresses
- Anyone can see all balances
- No hidden addresses

**4. All UTXOs Are Public** ✅
- Anyone can see all unspent outputs
- Complete UTXO set
- No hidden UTXOs

**5. All Validators Are Public** ✅
- Anyone can see all validators
- Anyone can see all stakes
- No hidden validators

**6. All Stock Contracts Are Public** ✅
- Anyone can see all stocks
- Anyone can see all prices
- Anyone can see all trades
- No hidden stocks

**7. Complete Traceability** ✅
- Every transaction traceable to genesis
- Formula: Ti.Inputs[0].txHash == Ti.PrevTxHash
- No fake coins
- No hidden origins

**8. No Private Data** ✅
- Everything is public
- Everything is verifiable
- Everything is transparent

---

## How to Use Explorer APIs

### JavaScript Example

```javascript
// Get latest blocks
const blocks = await fetch('https://api.gxc.io/v1/explorer/blocks/latest?limit=10');
const data = await blocks.json();
console.log("Latest blocks:", data.blocks);

// Get transaction details
const tx = await fetch('https://api.gxc.io/v1/explorer/transaction/0xtx123...');
const txData = await tx.json();
console.log("Transaction:", txData);

// Get address balance
const address = await fetch('https://api.gxc.io/v1/explorer/address/tGXC_user');
const addressData = await address.json();
console.log("Balance:", addressData.balance);

// Get all validators
const validators = await fetch('https://api.gxc.io/v1/explorer/validators');
const validatorsData = await validators.json();
console.log("Validators:", validatorsData.validators);

// Get all stocks
const stocks = await fetch('https://api.gxc.io/v1/explorer/stocks');
const stocksData = await stocks.json();
console.log("Stocks:", stocksData.stocks);
```

### Python Example

```python
import requests

# Get latest blocks
blocks = requests.get('https://api.gxc.io/v1/explorer/blocks/latest?limit=10')
print("Latest blocks:", blocks.json()['blocks'])

# Get transaction details
tx = requests.get('https://api.gxc.io/v1/explorer/transaction/0xtx123...')
print("Transaction:", tx.json())

# Get address balance
address = requests.get('https://api.gxc.io/v1/explorer/address/tGXC_user')
print("Balance:", address.json()['balance'])

# Get all validators
validators = requests.get('https://api.gxc.io/v1/explorer/validators')
print("Validators:", validators.json()['validators'])

# Get all stocks
stocks = requests.get('https://api.gxc.io/v1/explorer/stocks')
print("Stocks:", stocks.json()['stocks'])
```

---

## Building a Block Explorer

### What You Can Build

With these APIs, you can build:

1. **Block Explorer Website** (like Etherscan)
2. **Mobile Explorer App** (iOS/Android)
3. **Transaction Tracker**
4. **Address Monitor**
5. **Validator Dashboard**
6. **Stock Market Explorer**
7. **Analytics Dashboard**

### Example: Simple Block Explorer

```html
<!DOCTYPE html>
<html>
<head>
    <title>GXC Block Explorer</title>
</head>
<body>
    <h1>GXC Block Explorer</h1>
    
    <!-- Latest Blocks -->
    <h2>Latest Blocks</h2>
    <div id="blocks"></div>
    
    <!-- Latest Transactions -->
    <h2>Latest Transactions</h2>
    <div id="transactions"></div>
    
    <script>
        // Load latest blocks
        fetch('https://api.gxc.io/v1/explorer/blocks/latest?limit=10')
            .then(r => r.json())
            .then(data => {
                const html = data.blocks.map(block => `
                    <div>
                        Block ${block.height}: ${block.hash}
                        Mined by: ${block.miner}
                        Transactions: ${block.transactions}
                    </div>
                `).join('');
                document.getElementById('blocks').innerHTML = html;
            });
        
        // Load latest transactions
        fetch('https://api.gxc.io/v1/explorer/transactions/latest?limit=20')
            .then(r => r.json())
            .then(data => {
                const html = data.transactions.map(tx => `
                    <div>
                        ${tx.hash}: ${tx.from} → ${tx.to}
                        Amount: ${tx.amount} GXC
                    </div>
                `).join('');
                document.getElementById('transactions').innerHTML = html;
            });
    </script>
</body>
</html>
```

---

## Summary

### Complete Transparency ✅

**Users can see**:
- ✅ All blocks
- ✅ All transactions
- ✅ All addresses
- ✅ All balances
- ✅ All UTXOs
- ✅ All validators
- ✅ All stocks
- ✅ All trades
- ✅ Everything!

**No hidden data** - Everything is public and verifiable!

**Complete traceability** - Every coin traceable to genesis!

**Anyone can verify** - No trust required!

---

**The GXC blockchain is the most transparent blockchain ever built!** 🎉
