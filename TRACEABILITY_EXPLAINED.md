# GXC Traceability System - Complete Explanation

## Overview

The GXC blockchain has a **built-in traceability system** that allows you to trace every coin back to its origin, even if it was earned from mining. This system works **automatically** for all transactions and ensures complete transparency.

## How Traceability Works

### The Traceability Formula

Every transaction in GXC follows this formula:
```
Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount
```

**Translation**:
- **Ti** = Transaction i (current transaction)
- **Inputs[0].txHash** = Hash of the previous transaction that created the UTXO being spent
- **PrevTxHash** = Previous transaction hash stored in current transaction
- **Inputs[0].amount** = Amount from the previous transaction
- **ReferencedAmount** = Amount referenced from previous transaction (stored in current transaction)

**In Simple Terms**: Every transaction must reference the exact previous transaction it's spending from, and the amounts must match.

## Complete Traceability Chain

### Example: Tracing Coins from Mining

Let's trace coins from mining through multiple transactions:

#### Step 1: Mining Reward (Coinbase Transaction)
```
Block #100 - Coinbase Transaction
├─ Hash: abc123...
├─ Type: coinbase (mining reward)
├─ Output: 50 GXC → Miner Address (tGXC...miner)
├─ PrevTxHash: "0" (coinbase has no previous transaction)
└─ ReferencedAmount: 0 (coinbase creates new coins)
```

**Traceability**: This is the **origin** - coins are created from nothing (mining reward).

#### Step 2: Miner Sends Coins to You
```
Transaction #1: Miner → You
├─ Hash: def456...
├─ Type: transfer
├─ Input: abc123... (spending the 50 GXC from mining)
├─ Output: 50 GXC → Your Address (tGXC...you)
├─ PrevTxHash: "abc123..." (references the coinbase transaction)
└─ ReferencedAmount: 50.0 (amount from coinbase)
```

**Traceability Validation**:
- ✅ `Inputs[0].txHash` (abc123...) == `PrevTxHash` (abc123...) ✓
- ✅ `Inputs[0].amount` (50.0) == `ReferencedAmount` (50.0) ✓
- **Result**: Valid traceability link!

**You can trace**: This 50 GXC came from mining reward in block #100.

#### Step 3: You Send 30 GXC to Someone
```
Transaction #2: You → Recipient
├─ Hash: ghi789...
├─ Type: transfer
├─ Input: def456... (spending the 50 GXC you received)
├─ Output 1: 30 GXC → Recipient Address
├─ Output 2: 20 GXC → Your Address (change)
├─ PrevTxHash: "def456..." (references transaction #1)
└─ ReferencedAmount: 50.0 (amount from transaction #1)
```

**Traceability Validation**:
- ✅ `Inputs[0].txHash` (def456...) == `PrevTxHash` (def456...) ✓
- ✅ `Inputs[0].amount` (50.0) == `ReferencedAmount` (50.0) ✓
- **Result**: Valid traceability link!

**You can trace**: 
- Recipient's 30 GXC came from transaction #1 (def456...)
- Transaction #1 came from mining reward (abc123...)
- **Full chain**: Mining → Miner → You → Recipient

#### Step 4: Recipient Sends 10 GXC to Another Person
```
Transaction #3: Recipient → Another Person
├─ Hash: jkl012...
├─ Type: transfer
├─ Input: ghi789..._0 (spending the 30 GXC output #0)
├─ Output: 10 GXC → Another Person
├─ PrevTxHash: "ghi789..." (references transaction #2)
└─ ReferencedAmount: 30.0 (amount from transaction #2)
```

**Traceability Validation**:
- ✅ `Inputs[0].txHash` (ghi789...) == `PrevTxHash` (ghi789...) ✓
- ✅ `Inputs[0].amount` (30.0) == `ReferencedAmount` (30.0) ✓
- **Result**: Valid traceability link!

**You can trace**:
- Another Person's 10 GXC came from transaction #3 (jkl012...)
- Transaction #3 came from transaction #2 (ghi789...)
- Transaction #2 came from transaction #1 (def456...)
- Transaction #1 came from mining reward (abc123...)
- **Full chain**: Mining → Miner → You → Recipient → Another Person

## Visual Traceability Chain

```
[Mining Block #100]
    │
    └─> Coinbase TX (abc123...)
        │ 50 GXC → Miner
        │ PrevTxHash: "0" (origin)
        │
        └─> TX #1 (def456...)
            │ 50 GXC → You
            │ PrevTxHash: "abc123..." ✓
            │ ReferencedAmount: 50.0 ✓
            │
            └─> TX #2 (ghi789...)
                │ 30 GXC → Recipient
                │ 20 GXC → You (change)
                │ PrevTxHash: "def456..." ✓
                │ ReferencedAmount: 50.0 ✓
                │
                └─> TX #3 (jkl012...)
                    │ 10 GXC → Another Person
                    │ PrevTxHash: "ghi789..." ✓
                    │ ReferencedAmount: 30.0 ✓
```

## How It Works Automatically

### 1. When You Create a Transaction

**Location**: `src/Wallet.cpp` - `createTransaction()`

When you send coins, the wallet automatically:
1. Finds UTXOs to spend (from previous transactions)
2. Sets `prevTxHash` = hash of the transaction that created the UTXO
3. Sets `referencedAmount` = amount from that previous transaction
4. Creates transaction with traceability links

**Example**:
```cpp
// You have a UTXO from transaction "def456..." worth 50 GXC
Transaction tx;
tx.addInput("def456...", 0, 50.0);  // Input references previous TX
tx.setPrevTxHash("def456...");       // Set traceability link
tx.setReferencedAmount(50.0);        // Set referenced amount
```

### 2. When Transaction is Validated

**Location**: `src/Blockchain.cpp` - `validateTransactionTraceability()`

Every transaction is validated using the formula:
```cpp
bool hashMatch = (firstInput.txHash == tx.getPrevTxHash());
bool amountMatch = (firstInput.amount == tx.getReferencedAmount());
```

**If validation fails**: Transaction is **rejected** - cannot be added to blockchain!

### 3. When Block is Added

**Location**: `src/Blockchain.cpp` - `addBlock()`

Before a block is added:
1. All transactions are validated for traceability
2. `validateBlockTraceability()` checks every transaction
3. If any transaction fails traceability, **entire block is rejected**

### 4. Traceability Index Built

**Location**: `src/Blockchain.cpp` - `initializeTraceability()`

A traceability index is built that maps:
- Transaction hash → Previous transaction hash
- Transaction hash → Referenced amount
- Transaction hash → From/To addresses

This allows fast tracing of transaction chains.

## Database Storage

**Location**: `src/Database.cpp` - `saveTraceabilityRecord()`

Every transaction's traceability data is saved:
- **Table**: `traceability_index`
- **Fields**:
  - `tx_hash` - Current transaction hash
  - `prev_tx_hash` - Previous transaction hash
  - `referenced_amount` - Amount from previous transaction
  - `timestamp` - When transaction occurred
  - `block_height` - Block where transaction is confirmed
  - `validation_status` - Whether traceability is valid

## Tracing a Transaction Chain

### API Methods for Tracing

**Location**: `src/RPCAPI.cpp` - `traceTransaction()`

You can trace any transaction back to its origin:

1. **Start with any transaction hash**
2. **Follow `prevTxHash` links** backwards
3. **Continue until you reach coinbase** (prevTxHash = "0")
4. **You've traced the complete chain!**

### Example Trace Query

```json
{
  "method": "tracetransaction",
  "params": ["jkl012..."],
  "id": 1
}
```

**Result**:
```json
{
  "result": {
    "chain": [
      {
        "tx_hash": "jkl012...",
        "prev_tx_hash": "ghi789...",
        "amount": 10.0,
        "from": "recipient_address",
        "to": "another_person_address"
      },
      {
        "tx_hash": "ghi789...",
        "prev_tx_hash": "def456...",
        "amount": 30.0,
        "from": "your_address",
        "to": "recipient_address"
      },
      {
        "tx_hash": "def456...",
        "prev_tx_hash": "abc123...",
        "amount": 50.0,
        "from": "miner_address",
        "to": "your_address"
      },
      {
        "tx_hash": "abc123...",
        "prev_tx_hash": "0",
        "amount": 50.0,
        "from": "coinbase",
        "to": "miner_address",
        "is_coinbase": true,
        "block": 100
      }
    ],
    "origin": "mining_reward_block_100"
  }
}
```

## Key Features

### ✅ Automatic Traceability
- **Every transaction** automatically includes traceability links
- **No manual setup** required
- **Works for all transactions** (except coinbase)

### ✅ Validation on Every Block
- **Every block** validates traceability before being added
- **Invalid transactions** are rejected
- **Entire blockchain** maintains traceability integrity

### ✅ Complete Chain Tracking
- **Can trace any coin** back to its origin (mining reward)
- **Shows complete history** of ownership
- **Proves coin origin** (mined vs. received)

### ✅ Database Persistence
- **All traceability data** saved to database
- **Fast lookups** via traceability index
- **Permanent record** of transaction chains

### ✅ Mining Reward Tracking
- **Coinbase transactions** are the origin (prevTxHash = "0")
- **All mined coins** can be traced back to coinbase
- **Shows which block** coins were mined in

## Real-World Example

### Scenario: You Mine 50 GXC, Send 30 GXC to Friend

1. **You mine block #100**:
   - Coinbase TX: 50 GXC → Your address
   - PrevTxHash: "0" (origin)
   - **Traceability**: Origin = Mining Block #100

2. **You send 30 GXC to friend**:
   - TX: 50 GXC input → 30 GXC to friend + 20 GXC change
   - PrevTxHash: "coinbase_tx_hash"
   - **Traceability**: Can trace friend's 30 GXC back to your mining reward

3. **Friend sends 10 GXC to someone else**:
   - TX: 30 GXC input → 10 GXC to someone + 20 GXC change
   - PrevTxHash: "your_tx_to_friend_hash"
   - **Traceability**: Can trace someone's 10 GXC back to your mining reward

**Result**: Anyone can verify that the 10 GXC came from your mining reward in block #100!

## Why This Matters

### 1. **Prove Coin Origin**
- Show that coins came from mining (not stolen/hacked)
- Verify legitimate coin sources
- Audit coin history

### 2. **Prevent Money Laundering**
- Trace suspicious transactions
- Identify coin mixing attempts
- Follow money trails

### 3. **Regulatory Compliance**
- Provide transaction history
- Show coin origin for tax purposes
- Demonstrate legitimate transactions

### 4. **Transparency**
- Public can verify coin origins
- Builds trust in the system
- Prevents fraud

## Technical Implementation

### Traceability Validation Code

**Location**: `src/Blockchain.cpp` - `validateTransactionTraceability()`

```cpp
bool Blockchain::validateTransactionTraceability(const Transaction& tx) {
    // GXC Traceability Formula:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount
    
    if (tx.isCoinbaseTransaction()) {
        return true; // Coinbase transactions don't need traceability
    }
    
    const auto& firstInput = tx.getInputs()[0];
    
    // Check hash match
    bool hashMatch = (firstInput.txHash == tx.getPrevTxHash());
    
    // Check amount match (with small epsilon for floating point comparison)
    bool amountMatch = std::abs(firstInput.amount - tx.getReferencedAmount()) < 0.00000001;
    
    return hashMatch && amountMatch;
}
```

### Traceability Index Structure

**Location**: `include/blockchain.h` - `TraceabilityEntry`

```cpp
struct TraceabilityEntry {
    std::string txHash;        // Current transaction hash
    std::string prevTxHash;    // Previous transaction hash
    double amount;             // Referenced amount
    std::string fromAddress;   // Sender address
    std::string toAddress;     // Recipient address
    uint64_t timestamp;        // Transaction timestamp
};
```

## Summary

### ✅ Traceability Works Automatically
- Every transaction includes traceability links
- No manual configuration needed
- Works for all transactions

### ✅ Complete Chain Tracking
- Can trace any coin back to mining reward
- Shows complete ownership history
- Proves coin origin

### ✅ Validated on Every Block
- Every transaction validated before block addition
- Invalid transactions rejected
- Blockchain maintains integrity

### ✅ Database Persistence
- All traceability data saved
- Fast lookups available
- Permanent record maintained

### ✅ Mining Reward Tracking
- Coinbase transactions are origin (prevTxHash = "0")
- All mined coins traceable to coinbase
- Shows which block coins were mined in

**The traceability system ensures that every coin can be traced back to its origin, whether it was mined, received, or sent. This provides complete transparency and allows you to prove where your coins came from!**
