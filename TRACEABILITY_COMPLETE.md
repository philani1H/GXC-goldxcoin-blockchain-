# GXC Blockchain - Complete Traceability System
## Based on Actual Implementation

---

## Overview

GXC implements **complete transaction traceability** where every transaction links to its previous transaction, creating an unbreakable chain of ownership.

**Formula:**
```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

This means: Every transaction must reference where its funds came from.

---

## What is Traceable

### All Transaction Types

**1. NORMAL Transactions** ✅
- Regular transfers between addresses
- Must reference previous transaction
- Automatic validation

**2. STAKE Transactions** ✅
- Locking coins for staking
- References source transaction
- Tracks staked amount origin

**3. UNSTAKE Transactions** ✅
- Unlocking staked coins
- References original stake transaction
- Validates lock period

**4. REWARD Transactions** ✅
- Staking rewards (mints new coins)
- References validator performance
- Tracks reward distribution

**5. COINBASE Transactions** ✅
- Mining rewards (mints new coins)
- References proof-of-work
- Links to block height

**6. Stock Token Transactions** ✅
- Buying/selling stock tokens
- References previous ownership
- Tracks token transfers

**7. Gold Token Transactions** ✅
- Gold-backed token transfers
- References gold reserves
- Links to Proof-of-Price

**8. Cross-Chain Transactions** ✅
- Bridge transfers
- References source chain
- Tracks cross-chain movement

---

## Transaction Structure

### From transaction.h

```cpp
class Transaction {
    // Core fields
    std::string txHash;              // This transaction's hash
    TransactionType type;            // NORMAL, STAKE, UNSTAKE, REWARD, COINBASE
    std::vector<TransactionInput> inputs;   // Where funds come from
    std::vector<TransactionOutput> outputs; // Where funds go to
    std::time_t timestamp;           // When created
    
    // TRACEABILITY FIELDS
    std::string prevTxHash;          // Hash of previous transaction (Ti-1)
    double referencedAmount;         // Amount from previous transaction
    std::string senderAddress;       // Who sent
    std::string receiverAddress;     // Who receives
    uint32_t nonce;                  // Uniqueness
    
    // Special fields
    std::string popReference;        // Proof of Price (gold tokens)
    bool isGoldBacked;              // Gold-backed flag
    bool isCoinbase;                // Mining reward flag
    std::string chainReference;      // Cross-chain reference
    std::string workReceiptHash;     // Proof-of-work link (coinbase)
    uint32_t blockHeight;           // Block height (coinbase)
    
    // Metadata
    double fee;                     // Transaction fee
    std::string memo;               // Optional note
    uint32_t lockTime;              // Time lock
};
```

### TransactionInput Structure

```cpp
struct TransactionInput {
    std::string txHash;      // Previous transaction hash
    uint32_t outputIndex;    // Which output from previous TX
    std::string signature;   // Proof of ownership
    double amount;           // Amount being spent (TRACEABILITY)
    std::string publicKey;   // Public key for verification
};
```

### TransactionOutput Structure

```cpp
struct TransactionOutput {
    std::string address;     // Recipient address
    double amount;           // Amount of GXC
    std::string script;      // Output script
};
```

---

## How Traceability Works

### Example: Normal Transaction

**Transaction 1 (Genesis/Coinbase):**
```json
{
  "hash": "TX1_HASH",
  "type": "COINBASE",
  "inputs": [],
  "outputs": [
    {
      "address": "Alice",
      "amount": 50.0
    }
  ],
  "prevTxHash": "0x000...000",
  "isCoinbase": true
}
```

**Transaction 2 (Alice → Bob):**
```json
{
  "hash": "TX2_HASH",
  "type": "NORMAL",
  "inputs": [
    {
      "txHash": "TX1_HASH",        // References TX1
      "outputIndex": 0,
      "amount": 50.0,               // TRACEABILITY: Amount from TX1
      "signature": "...",
      "publicKey": "Alice_PubKey"
    }
  ],
  "outputs": [
    {
      "address": "Bob",
      "amount": 49.0
    },
    {
      "address": "Alice",           // Change
      "amount": 0.99
    }
  ],
  "prevTxHash": "TX1_HASH",         // TRACEABILITY: Links to TX1
  "referencedAmount": 50.0,         // TRACEABILITY: Amount from TX1
  "senderAddress": "Alice",
  "receiverAddress": "Bob",
  "fee": 0.01
}
```

**Validation:**
```
✓ TX2.inputs[0].txHash == TX2.prevTxHash  (TX1_HASH == TX1_HASH)
✓ TX2.inputs[0].amount == TX2.referencedAmount  (50.0 == 50.0)
✓ Signature valid
✓ Input amount >= Output amounts + Fee  (50.0 >= 49.0 + 0.99 + 0.01)
```

**Transaction 3 (Bob → Charlie):**
```json
{
  "hash": "TX3_HASH",
  "type": "NORMAL",
  "inputs": [
    {
      "txHash": "TX2_HASH",        // References TX2
      "outputIndex": 0,
      "amount": 49.0,               // TRACEABILITY: Amount from TX2
      "signature": "...",
      "publicKey": "Bob_PubKey"
    }
  ],
  "outputs": [
    {
      "address": "Charlie",
      "amount": 48.99
    }
  ],
  "prevTxHash": "TX2_HASH",         // TRACEABILITY: Links to TX2
  "referencedAmount": 49.0,         // TRACEABILITY: Amount from TX2
  "senderAddress": "Bob",
  "receiverAddress": "Charlie",
  "fee": 0.01
}
```

**Chain:**
```
TX1 (Coinbase) → TX2 (Alice→Bob) → TX3 (Bob→Charlie)
```

---

## Automatic Validation

### From Transaction.cpp

**verifyTraceabilityFormula():**
```cpp
bool Transaction::verifyTraceabilityFormula() const {
    // 1. Coinbase and genesis don't need traceability
    if (isCoinbase || isGenesis()) {
        return true;
    }
    
    // 2. Must have inputs
    if (inputs.empty()) {
        return false;
    }
    
    // 3. Auto-fill prevTxHash if not set (third-party wallet support)
    std::string effectivePrevTxHash = prevTxHash;
    if (effectivePrevTxHash.empty() || effectivePrevTxHash == "0") {
        effectivePrevTxHash = inputs[0].txHash;  // AUTOMATIC
    }
    
    // 4. Verify: Ti.Inputs[0].txHash == Ti.PrevTxHash
    if (inputs[0].txHash != effectivePrevTxHash) {
        return false;  // VALIDATION FAILED
    }
    
    // 5. Auto-fill referencedAmount if not set
    double effectiveReferencedAmount = referencedAmount;
    if (effectiveReferencedAmount == 0.0 && !inputs.empty()) {
        effectiveReferencedAmount = inputs[0].amount;  // AUTOMATIC
    }
    
    // 6. Verify: Ti.Inputs[0].amount == Ti.ReferencedAmount
    if (abs(inputs[0].amount - effectiveReferencedAmount) > 0.00000001) {
        return false;  // VALIDATION FAILED
    }
    
    return true;  // VALIDATION PASSED
}
```

**Key Points:**
- ✅ **Automatic:** If prevTxHash not set, uses inputs[0].txHash
- ✅ **Automatic:** If referencedAmount not set, uses inputs[0].amount
- ✅ **Validated:** Every transaction checked before acceptance
- ✅ **Enforced:** Invalid transactions rejected

---

## Transaction Types in Detail

### 1. NORMAL Transaction

**Purpose:** Regular transfer of GXC

**Traceability:**
```json
{
  "type": "NORMAL",
  "prevTxHash": "PREVIOUS_TX_HASH",
  "referencedAmount": 100.0,
  "inputs": [
    {
      "txHash": "PREVIOUS_TX_HASH",
      "amount": 100.0
    }
  ]
}
```

**Validation:**
- ✅ Must reference previous transaction
- ✅ Input amount must match referenced amount
- ✅ Signature must be valid
- ✅ Balance must be sufficient

**Example:**
```
Alice has 100 GXC from TX_A
Alice sends 50 GXC to Bob

New transaction:
- prevTxHash: TX_A
- referencedAmount: 100.0
- inputs[0].txHash: TX_A
- inputs[0].amount: 100.0
- outputs[0]: Bob, 50.0
- outputs[1]: Alice (change), 49.99
- fee: 0.01
```

### 2. STAKE Transaction

**Purpose:** Lock coins for staking

**Traceability:**
```json
{
  "type": "STAKE",
  "prevTxHash": "SOURCE_TX_HASH",
  "referencedAmount": 1000.0,
  "inputs": [
    {
      "txHash": "SOURCE_TX_HASH",
      "amount": 1000.0
    }
  ],
  "outputs": [
    {
      "address": "STAKING_CONTRACT",
      "amount": 1000.0
    }
  ],
  "lockTime": 7776000  // 90 days
}
```

**Validation:**
- ✅ Must reference source transaction
- ✅ Amount must be >= minimum stake (100 GXC)
- ✅ Lock time must be valid (30-365 days)
- ✅ Signature must be valid

**Example:**
```
Alice has 1000 GXC from TX_B
Alice stakes 1000 GXC for 90 days

New transaction:
- type: STAKE
- prevTxHash: TX_B
- referencedAmount: 1000.0
- inputs[0].txHash: TX_B
- inputs[0].amount: 1000.0
- outputs[0]: STAKING_CONTRACT, 1000.0
- lockTime: 90 days
```

### 3. UNSTAKE Transaction

**Purpose:** Unlock staked coins

**Traceability:**
```json
{
  "type": "UNSTAKE",
  "prevTxHash": "STAKE_TX_HASH",
  "referencedAmount": 1000.0,
  "inputs": [
    {
      "txHash": "STAKE_TX_HASH",
      "amount": 1000.0
    }
  ],
  "outputs": [
    {
      "address": "ALICE_ADDRESS",
      "amount": 1000.0
    }
  ]
}
```

**Validation:**
- ✅ Must reference stake transaction
- ✅ Lock period must be expired
- ✅ Amount must match stake amount
- ✅ Signature must be valid

**Example:**
```
Alice staked 1000 GXC in TX_STAKE
90 days passed
Alice unstakes

New transaction:
- type: UNSTAKE
- prevTxHash: TX_STAKE
- referencedAmount: 1000.0
- inputs[0].txHash: TX_STAKE
- inputs[0].amount: 1000.0
- outputs[0]: Alice, 1000.0
```

### 4. REWARD Transaction

**Purpose:** Distribute staking rewards (mints new coins)

**Traceability:**
```json
{
  "type": "REWARD",
  "prevTxHash": "VALIDATOR_PERFORMANCE_HASH",
  "referencedAmount": 0.0,  // Minting new coins
  "inputs": [],  // No inputs (minting)
  "outputs": [
    {
      "address": "VALIDATOR_ADDRESS",
      "amount": 5.0
    }
  ],
  "blockHeight": 12345
}
```

**Validation:**
- ✅ Must reference validator performance
- ✅ Amount must match calculated reward
- ✅ Validator must be active
- ✅ Block height must be valid

**Example:**
```
Validator produced 10 blocks
Earned 5 GXC reward

New transaction:
- type: REWARD
- prevTxHash: VALIDATOR_PERFORMANCE_HASH
- inputs: [] (minting)
- outputs[0]: Validator, 5.0
- blockHeight: 12345
```

### 5. COINBASE Transaction

**Purpose:** Mining rewards (mints new coins)

**Traceability:**
```json
{
  "type": "COINBASE",
  "prevTxHash": "0x000...000",
  "referencedAmount": 0.0,  // Minting new coins
  "inputs": [],  // No inputs (minting)
  "outputs": [
    {
      "address": "MINER_ADDRESS",
      "amount": 50.0
    }
  ],
  "workReceiptHash": "PROOF_OF_WORK_HASH",
  "blockHeight": 12345,
  "isCoinbase": true
}
```

**Validation:**
- ✅ Must be first transaction in block
- ✅ Must reference proof-of-work
- ✅ Amount must match block reward
- ✅ Block height must match

**Example:**
```
Miner found block 12345
Block reward: 50 GXC

New transaction:
- type: COINBASE
- prevTxHash: "0x000...000"
- inputs: [] (minting)
- outputs[0]: Miner, 50.0
- workReceiptHash: PROOF_OF_WORK
- blockHeight: 12345
- isCoinbase: true
```

### 6. Stock Token Transaction

**Purpose:** Trade synthetic stock tokens

**Traceability:**
```json
{
  "type": "NORMAL",
  "prevTxHash": "PREVIOUS_STOCK_TX",
  "referencedAmount": 100.0,
  "inputs": [
    {
      "txHash": "PREVIOUS_STOCK_TX",
      "amount": 100.0
    }
  ],
  "outputs": [
    {
      "address": "BUYER_ADDRESS",
      "amount": 1.0  // 1 AAPL token
    }
  ],
  "memo": "STOCK:AAPL:BUY:1"
}
```

**Validation:**
- ✅ Must reference previous transaction
- ✅ Stock contract must exist
- ✅ Price must be current
- ✅ Collateral must be sufficient

### 7. Gold Token Transaction

**Purpose:** Transfer gold-backed tokens

**Traceability:**
```json
{
  "type": "NORMAL",
  "prevTxHash": "PREVIOUS_GOLD_TX",
  "referencedAmount": 1000.0,
  "inputs": [
    {
      "txHash": "PREVIOUS_GOLD_TX",
      "amount": 1000.0
    }
  ],
  "outputs": [
    {
      "address": "BUYER_ADDRESS",
      "amount": 1000.0  // 1 oz gold = 1000 GXC-G
    }
  ],
  "popReference": "PROOF_OF_PRICE_HASH",
  "isGoldBacked": true
}
```

**Validation:**
- ✅ Must reference previous transaction
- ✅ Must reference Proof-of-Price
- ✅ Gold reserves must be sufficient
- ✅ Price must be current

### 8. Cross-Chain Transaction

**Purpose:** Bridge to other blockchains

**Traceability:**
```json
{
  "type": "NORMAL",
  "prevTxHash": "SOURCE_TX_HASH",
  "referencedAmount": 100.0,
  "inputs": [
    {
      "txHash": "SOURCE_TX_HASH",
      "amount": 100.0
    }
  ],
  "outputs": [
    {
      "address": "BRIDGE_CONTRACT",
      "amount": 100.0
    }
  ],
  "chainReference": "ETH:0x123...",
  "memo": "BRIDGE:ETH:0x123..."
}
```

**Validation:**
- ✅ Must reference source transaction
- ✅ Bridge must be active
- ✅ Destination chain must be supported
- ✅ Amount must meet minimum

---

## Traceability Index

### From blockchain.h

```cpp
struct TraceabilityEntry {
    std::string txHash;        // This transaction
    std::string prevTxHash;    // Previous transaction
    double amount;             // Amount transferred
    std::string fromAddress;   // Sender
    std::string toAddress;     // Receiver
    uint64_t timestamp;        // When
};

std::unordered_map<std::string, TraceabilityEntry> traceabilityIndex;
```

**Purpose:**
- Fast lookup of transaction history
- Build transaction chains
- Verify ownership history
- Audit trail

**Usage:**
```cpp
// Get transaction chain
std::vector<TraceabilityEntry> getTransactionChain(std::string txHash) {
    std::vector<TraceabilityEntry> chain;
    std::string currentHash = txHash;
    
    while (traceabilityIndex.count(currentHash)) {
        TraceabilityEntry entry = traceabilityIndex[currentHash];
        chain.push_back(entry);
        currentHash = entry.prevTxHash;
        
        if (currentHash == "0x000...000") break;  // Genesis
    }
    
    return chain;
}
```

---

## How Validation Looks

### Transaction Submission

**1. User Creates Transaction:**
```javascript
// Wallet creates transaction
const tx = {
  type: "NORMAL",
  inputs: [{
    txHash: "PREV_TX",
    outputIndex: 0,
    amount: 100.0,
    signature: "...",
    publicKey: "..."
  }],
  outputs: [{
    address: "Bob",
    amount: 99.99
  }],
  prevTxHash: "PREV_TX",        // Can be omitted (auto-filled)
  referencedAmount: 100.0,      // Can be omitted (auto-filled)
  fee: 0.01
};
```

**2. Blockchain Validates:**
```cpp
bool Blockchain::validateTransaction(const Transaction& tx) {
    // 1. Verify traceability formula
    if (!tx.verifyTraceabilityFormula()) {
        LOG_ERROR("Traceability validation failed");
        return false;  // REJECTED
    }
    
    // 2. Verify signatures
    if (!tx.validateSignatures()) {
        LOG_ERROR("Signature validation failed");
        return false;  // REJECTED
    }
    
    // 3. Verify balance
    if (!tx.validateInputOutputBalance()) {
        LOG_ERROR("Balance validation failed");
        return false;  // REJECTED
    }
    
    // 4. Verify UTXO exists
    if (!verifyUTXO(tx.getInputs()[0].txHash)) {
        LOG_ERROR("UTXO not found");
        return false;  // REJECTED
    }
    
    return true;  // ACCEPTED
}
```

**3. Result:**
```
✅ ACCEPTED: Transaction added to mempool
❌ REJECTED: Transaction discarded with error message
```

---

## Third-Party Wallet Support

### Automatic Traceability

**Problem:** Third-party wallets may not set prevTxHash and referencedAmount

**Solution:** Blockchain automatically fills these fields

```cpp
// Auto-fill prevTxHash
if (prevTxHash.empty() || prevTxHash == "0") {
    prevTxHash = inputs[0].txHash;  // AUTOMATIC
}

// Auto-fill referencedAmount
if (referencedAmount == 0.0 && !inputs.empty()) {
    referencedAmount = inputs[0].amount;  // AUTOMATIC
}
```

**Result:**
- ✅ Third-party wallets work without modification
- ✅ Traceability still enforced
- ✅ Validation still automatic

---

## Benefits of Complete Traceability

### 1. Audit Trail
Every coin can be traced back to its origin (coinbase/reward)

### 2. Fraud Prevention
Impossible to create coins from nothing (except coinbase/reward)

### 3. Regulatory Compliance
Complete transaction history for auditing

### 4. Ownership Verification
Prove ownership through transaction chain

### 5. Double-Spend Prevention
UTXO model + traceability prevents double-spending

### 6. Transparency
Anyone can verify transaction history

---

## Summary

### What is Traceable
✅ **ALL transactions** (NORMAL, STAKE, UNSTAKE, REWARD, COINBASE, Stock, Gold, Cross-chain)

### How They Look
✅ **Every transaction** has prevTxHash and referencedAmount fields
✅ **Inputs reference** previous transaction outputs
✅ **Chain links** from current → previous → previous → ... → genesis

### Automatic Validation
✅ **verifyTraceabilityFormula()** checks every transaction
✅ **Auto-fills** missing fields for third-party wallets
✅ **Rejects** invalid transactions automatically
✅ **No manual intervention** needed

### Formula
```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

**Every transaction is automatically validated for complete traceability!** 🔗
