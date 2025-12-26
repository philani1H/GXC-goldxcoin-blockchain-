# Mempool (Memory Pool) - Complete Explanation

## What is the Mempool?

The **mempool** (memory pool) is a temporary storage area for **unconfirmed transactions** waiting to be included in a block.

Think of it like a waiting room:
- Transactions arrive → Wait in mempool → Get included in a block → Confirmed

---

## What's Stored in the Mempool

### Data Structure

```cpp
// From include/blockchain.h line 21
std::vector<Transaction> pendingTransactions;
```

### Each Transaction Contains

```cpp
class Transaction {
    std::string txHash;              // Unique transaction ID
    TransactionType type;            // NORMAL, STAKE, UNSTAKE
    std::vector<TransactionInput> inputs;   // Where coins come from
    std::vector<TransactionOutput> outputs; // Where coins go to
    uint64_t timestamp;              // When transaction was created
    std::string signature;           // Cryptographic proof
    std::string publicKey;           // Sender's public key
    
    // Traceability fields
    std::string prevTxHash;          // Previous transaction reference
    double referencedAmount;         // Amount from previous UTXO
}
```

### Transaction Input

```cpp
struct TransactionInput {
    std::string txHash;      // Hash of previous transaction
    uint32_t outputIndex;    // Which output from that transaction
    double amount;           // Amount being spent
    std::string signature;   // Proof of ownership
}
```

### Transaction Output

```cpp
struct TransactionOutput {
    std::string address;     // Recipient address
    double amount;           // Amount of GXC
    std::string script;      // Output script (for smart contracts)
}
```

---

## Mempool Lifecycle

### 1. Transaction Arrives

```
User creates transaction
    ↓
Wallet signs transaction
    ↓
sendrawtransaction RPC call
    ↓
Node receives transaction
```

### 2. Validation

```cpp
bool Blockchain::addTransaction(const Transaction& tx) {
    // Step 1: Validate transaction
    if (!validateTransaction(tx)) {
        return false;  // Rejected
    }
    
    // Step 2: Check for duplicates
    if (already_in_mempool) {
        return false;  // Already exists
    }
    
    // Step 3: Add to mempool
    pendingTransactions.push_back(tx);
    
    return true;  // Success
}
```

### 3. Validation Checks

✅ **Format validation**
- Transaction has valid structure
- Hash is correct
- Signature is valid

✅ **UTXO validation**
- Input UTXOs exist
- Input UTXOs are unspent
- Input UTXOs are not immature coinbase

✅ **Amount validation**
- Sum(inputs) ≥ Sum(outputs)
- Fee is reasonable

✅ **Signature validation**
- Signature matches public key
- Public key matches input addresses

✅ **Traceability validation**
- prevTxHash references valid transaction
- referencedAmount matches UTXO amount

### 4. Waiting in Mempool

```
Transaction sits in mempool
    ↓
Miners request block template
    ↓
Node includes pending transactions
    ↓
Miner mines block
    ↓
Block submitted with transactions
```

### 5. Confirmation

```
Block accepted by network
    ↓
Transactions removed from mempool
    ↓
UTXOs updated
    ↓
Balances updated
    ↓
Transaction confirmed!
```

---

## Mempool Operations

### Add Transaction

```cpp
// src/Blockchain.cpp line 1658
bool addTransaction(const Transaction& tx) {
    // Validate and add to pendingTransactions
}
```

### Get Pending Transactions

```cpp
// src/Blockchain.cpp line 1700
std::vector<Transaction> getPendingTransactions(size_t maxCount) {
    // Returns up to maxCount transactions from mempool
    // Used by miners to build blocks
}
```

### Remove Expired Transactions

```cpp
// src/Blockchain.cpp line 1686
void processTransactions() {
    // Remove transactions older than 1 hour
    auto currentTime = Utils::getCurrentTimestamp();
    pendingTransactions.erase(
        std::remove_if(pendingTransactions.begin(), pendingTransactions.end(),
                      [currentTime](const Transaction& tx) {
                          return currentTime - tx.getTimestamp() > 3600; // 1 hour
                      }),
        pendingTransactions.end()
    );
}
```

### Clear Confirmed Transactions

```cpp
// When block is added, remove included transactions
for (const auto& tx : block.getTransactions()) {
    // Remove from pendingTransactions
}
```

---

## Mempool Info

### RPC Method: getmempoolinfo

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getmempoolinfo","params":[],"id":1}'
```

**Response:**
```json
{
  "size": 5,              // Number of transactions
  "bytes": 1250,          // Total size in bytes
  "usage": 1250,          // Memory usage
  "maxmempool": 300000000, // Max size (300 MB)
  "mempoolminfee": 0.00001, // Minimum fee
  "minrelaytxfee": 0.00001  // Minimum relay fee
}
```

---

## Why Mempool Matters

### 1. Transaction Ordering

Miners select transactions from mempool based on:
- **Fee**: Higher fee = higher priority
- **Age**: Older transactions may get priority
- **Size**: Smaller transactions are cheaper to include

### 2. Fee Estimation

```cpp
// src/Blockchain.cpp line 1738
double calculateRecommendedFee() const {
    size_t pendingTxCount = pendingTransactions.size();
    
    // More transactions in mempool = higher recommended fee
    // SecurityEngine calculates dynamic fee based on congestion
    
    return fee;
}
```

### 3. Double-Spend Prevention

```cpp
// Check if UTXO is already spent in mempool
std::unordered_set<std::string> spentInMempool;
for (const auto& tx : pendingTransactions) {
    for (const auto& input : tx.getInputs()) {
        spentInMempool.insert(input.txHash + "_" + std::to_string(input.outputIndex));
    }
}

// Reject transaction if trying to spend already-spent UTXO
if (spentInMempool.find(utxoKey) != spentInMempool.end()) {
    return false;  // Double-spend attempt
}
```

### 4. Pending Balance

```cpp
// src/Blockchain.cpp - getPendingBalance
double getPendingBalance(const std::string& address) const {
    double pendingBalance = 0.0;
    
    // Sum all outputs to this address in mempool
    for (const auto& tx : pendingTransactions) {
        for (const auto& output : tx.getOutputs()) {
            if (output.address == address) {
                pendingBalance += output.amount;
            }
        }
    }
    
    return pendingBalance;
}
```

---

## Mempool Limits

### Size Limit

```cpp
maxmempool: 300000000  // 300 MB
```

When mempool is full:
- Lowest fee transactions are evicted
- New transactions must pay higher fee

### Time Limit

```cpp
// Transactions expire after 1 hour
if (currentTime - tx.getTimestamp() > 3600) {
    // Remove from mempool
}
```

### Transaction Limit

```cpp
// No hard limit on number of transactions
// Limited by memory size (300 MB)
```

---

## Example: Transaction Flow

### Step 1: User Sends Transaction

```bash
curl -X POST http://localhost:8332 \
  -d '{
    "jsonrpc":"2.0",
    "method":"sendtoaddress",
    "params":["recipient_address", 30.0, "sender_address"],
    "id":1
  }'
```

### Step 2: Transaction Enters Mempool

```
[INFO] Transaction added to pool: a1b2c3d4...
[INFO] Mempool size: 1 transactions
```

### Step 3: Miner Requests Template

```bash
curl -X POST http://localhost:8332 \
  -d '{
    "jsonrpc":"2.0",
    "method":"getblocktemplate",
    "params":[{"minerAddress":"miner_address"}],
    "id":1
  }'
```

**Response includes:**
```json
{
  "transactions": [
    {
      "hash": "a1b2c3d4...",
      "from": "sender_address",
      "to": "recipient_address",
      "amount": 30.0
    }
  ]
}
```

### Step 4: Miner Mines Block

```
Miner finds valid nonce
    ↓
Submits block with transaction
    ↓
Block validated and accepted
```

### Step 5: Transaction Confirmed

```
[INFO] Block added successfully. Height: 106
[INFO] Transaction confirmed: a1b2c3d4...
[INFO] Mempool size: 0 transactions
```

---

## Mempool vs UTXO Set

### Mempool (Temporary)

- **Unconfirmed** transactions
- **Volatile** (can be removed)
- **Not spendable** yet
- **In memory** only

### UTXO Set (Permanent)

- **Confirmed** transactions
- **Persistent** (in database)
- **Spendable** (after maturity)
- **On disk** and in memory

---

## Security Considerations

### 1. Double-Spend Protection

```cpp
// Check mempool before accepting transaction
if (utxo_spent_in_mempool) {
    reject_transaction();
}
```

### 2. Fee Requirements

```cpp
// Minimum fee prevents spam
if (fee < mempoolminfee) {
    reject_transaction();
}
```

### 3. Expiration

```cpp
// Old transactions removed automatically
if (age > 1_hour) {
    remove_from_mempool();
}
```

### 4. Size Limits

```cpp
// Prevents memory exhaustion
if (mempool_size > maxmempool) {
    evict_lowest_fee_transactions();
}
```

---

## Summary

### What's in the Mempool?

✅ **Unconfirmed transactions** waiting for confirmation
✅ **Transaction inputs** (which UTXOs are being spent)
✅ **Transaction outputs** (where coins are going)
✅ **Signatures** (proof of ownership)
✅ **Timestamps** (when transaction was created)
✅ **Fees** (payment to miners)

### What's NOT in the Mempool?

❌ **Confirmed transactions** (those are in blocks)
❌ **Coinbase transactions** (created by miners)
❌ **Invalid transactions** (rejected during validation)
❌ **Expired transactions** (older than 1 hour)

### Key Points

1. **Temporary storage** for unconfirmed transactions
2. **Validated** before entering mempool
3. **Ordered** by fee and age
4. **Removed** when included in block
5. **Expires** after 1 hour
6. **Limited** to 300 MB

**The mempool is the bridge between transaction creation and blockchain confirmation!**
