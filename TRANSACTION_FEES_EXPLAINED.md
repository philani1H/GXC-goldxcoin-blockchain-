# Transaction Fees Explained - GXC Blockchain

Complete guide to understanding transaction fees in the GXC blockchain.

---

## 📋 Quick Answer

**No, not all transactions have fees.**

- ✅ **Regular transactions** (sending coins): **Have fees** (default: 0.001 GXC)
- ❌ **Coinbase transactions** (mining rewards): **No fees** (fee = 0.0)

---

## 💰 Transaction Fee Overview

### What Are Transaction Fees?

Transaction fees are small amounts of GXC paid by the sender to incentivize miners to include their transaction in a block. Fees are collected by miners as part of their block reward.

### Default Transaction Fee

**Default Fee**: `0.001 GXC` per transaction

This can be configured in `gxc.conf`:
```ini
transaction_fee=0.001
```

---

## 🔍 Transaction Types and Fees

### 1. Regular Transactions (Transfer)

**Fee**: ✅ **Yes, required**

**Example:**
- Sending 10 GXC to someone
- Transaction fee: 0.001 GXC
- Total deducted from sender: 10.001 GXC
- Recipient receives: 10 GXC
- Miner receives: 0.001 GXC (as part of block reward)

**Code Reference:**
```cpp
// From Transaction.cpp - Regular transaction constructor
Transaction::Transaction(...)
    : fee(0.0), ...  // Fee set separately
```

**Fee Validation:**
```cpp
// From Blockchain.cpp - Transaction validation
double inputTotal = tx.getTotalInputAmount();
double outputTotal = tx.getTotalOutputAmount();
double fee = tx.getFee();

if (inputTotal < outputTotal + fee) {
    // Transaction rejected - insufficient funds for fee
    return false;
}
```

### 2. Coinbase Transactions (Mining Rewards)

**Fee**: ❌ **No fee** (fee = 0.0)

**Example:**
- Miner finds a block
- Block reward: 50 GXC
- Transaction fee: 0.0 GXC
- Miner receives: 50 GXC (full reward, no fee deducted)

**Code Reference:**
```cpp
// From Transaction.cpp - Coinbase constructor
Transaction::Transaction(const std::string& minerAddress, double blockReward)
    : isCoinbase(true), fee(0.0), ...  // Coinbase has no fee
```

**Why No Fee?**
- Coinbase transactions create new coins (mining rewards)
- They don't spend existing coins, so no fee is needed
- They're automatically included in every block

---

## 📊 Fee Structure

### Fee Calculation

For regular transactions:
```
Fee = Transaction Fee (default: 0.001 GXC)
```

**Note**: Currently, fees are fixed per transaction. Future versions may support:
- Dynamic fees based on network congestion
- Fee per byte (for larger transactions)
- Priority fees for faster confirmation

### Fee Validation

A transaction is valid only if:
```
Input Amount >= Output Amount + Fee
```

**Example:**
- Input: 10.0 GXC
- Output: 9.5 GXC
- Fee: 0.001 GXC
- **Valid**: 10.0 >= 9.5 + 0.001 ✅

**Invalid Example:**
- Input: 10.0 GXC
- Output: 10.0 GXC
- Fee: 0.001 GXC
- **Invalid**: 10.0 < 10.0 + 0.001 ❌

---

## 🎯 How Fees Work

### Transaction Flow

1. **User Creates Transaction**
   ```
   Send: 10 GXC
   Fee: 0.001 GXC
   Total Needed: 10.001 GXC
   ```

2. **Transaction Validated**
   - Check sender has sufficient balance (including fee)
   - Verify: `inputTotal >= outputTotal + fee`

3. **Transaction Included in Block**
   - Miner includes transaction in block
   - Fee is collected

4. **Fee Goes to Miner**
   - All transaction fees in block are added to miner's reward
   - Miner receives: `Block Reward + Sum of All Transaction Fees`

### Fee Collection

**Code Reference:**
```cpp
// From Blockchain.cpp - Calculate block reward
double transactionFees = 0.0;
for (const auto& tx : pendingTransactions) {
    transactionFees += tx.getFee();  // Sum all fees
}

// Add fees to miner reward (not subject to supply cap)
finalReward += transactionFees;
```

**Example Block:**
- Base block reward: 50 GXC
- Transaction 1 fee: 0.001 GXC
- Transaction 2 fee: 0.001 GXC
- Transaction 3 fee: 0.001 GXC
- **Total miner reward**: 50.003 GXC

---

## 📈 Fee Examples

### Example 1: Simple Transfer

**Transaction:**
- Sender: Alice
- Recipient: Bob
- Amount: 100 GXC
- Fee: 0.001 GXC

**Result:**
- Alice's balance decreases by: 100.001 GXC
- Bob's balance increases by: 100 GXC
- Miner receives: 0.001 GXC (when block is mined)

### Example 2: Multiple Transactions in Block

**Block contains:**
- Transaction 1: 50 GXC (fee: 0.001)
- Transaction 2: 25 GXC (fee: 0.001)
- Transaction 3: 10 GXC (fee: 0.001)
- Coinbase: 50 GXC (fee: 0.0)

**Miner Reward:**
- Base reward: 50 GXC
- Transaction fees: 0.003 GXC
- **Total**: 50.003 GXC

### Example 3: Coinbase Transaction

**Block:**
- Coinbase transaction: 50 GXC
- Fee: 0.0 GXC

**Result:**
- Miner receives: 50 GXC (full amount, no fee)

---

## 🔧 Configuration

### Setting Transaction Fee

**In `gxc.conf`:**
```ini
# Transaction fee in GXC
transaction_fee=0.001

# Minimum transaction fee
min_tx_fee=0.0001
```

**Via RPC:**
```json
{
  "method": "setconfig",
  "params": ["transaction_fee", "0.001"]
}
```

### Checking Current Fee

**Via RPC:**
```json
{
  "method": "getconfig",
  "params": ["transaction_fee"]
}
```

**Response:**
```json
{
  "result": "0.001"
}
```

---

## 💡 Fee Best Practices

### For Users

1. **Always Account for Fees**
   - When sending coins, ensure balance covers amount + fee
   - Example: To send 10 GXC, need at least 10.001 GXC

2. **Fee is Small**
   - Default fee (0.001 GXC) is very small
   - Don't worry about fees for normal transactions

3. **Fees Go to Miners**
   - Fees incentivize miners to include your transaction
   - Higher fees = faster confirmation (in future versions)

### For Developers

1. **Always Include Fee in Calculations**
   ```python
   amount_to_send = 10.0
   fee = 0.001
   total_needed = amount_to_send + fee
   ```

2. **Check Balance Before Sending**
   ```python
   balance = get_balance(address)
   if balance < (amount + fee):
       raise InsufficientFundsError()
   ```

3. **Handle Fee in Transaction Creation**
   ```python
   transaction = {
       "from": sender_address,
       "to": recipient_address,
       "amount": amount,
       "fee": fee  # Always include fee
   }
   ```

---

## 🚫 Transactions Without Fees

### Coinbase Transactions

**Why No Fee?**
- Creates new coins (doesn't spend existing coins)
- Automatically included in every block
- No need to incentivize inclusion

**Code:**
```cpp
// Coinbase transaction always has fee = 0.0
Transaction::Transaction(const std::string& minerAddress, double blockReward)
    : isCoinbase(true), fee(0.0), ...
```

### Genesis Transactions

**Why No Fee?**
- Initial blockchain setup
- Creates initial supply
- No previous transactions to reference

---

## 📊 Fee Statistics

### Average Fees Per Block

**Typical Block:**
- Transactions per block: 10-50
- Average fee per transaction: 0.001 GXC
- Total fees per block: 0.01-0.05 GXC

**High Activity Block:**
- Transactions per block: 100+
- Total fees per block: 0.1+ GXC

### Fee Revenue for Miners

**Daily Fee Revenue (estimated):**
- Blocks per day: ~720 (2 minute blocks)
- Average fees per block: 0.02 GXC
- **Daily fee revenue**: ~14.4 GXC/day

**Note**: Actual fees depend on network activity.

---

## 🔮 Future Fee Features

### Planned Enhancements

1. **Dynamic Fees**
   - Fees adjust based on network congestion
   - Higher fees = faster confirmation

2. **Fee Per Byte**
   - Larger transactions pay more
   - Encourages efficient transaction size

3. **Priority Fees**
   - Optional higher fees for urgent transactions
   - Guaranteed faster inclusion

4. **Fee Estimation**
   - RPC method to estimate optimal fee
   - Based on current network conditions

---

## ❓ Frequently Asked Questions

### Q: Do I pay fees when receiving coins?

**A:** No, only the sender pays fees. Recipients receive the full amount.

### Q: Can I set a custom fee?

**A:** Currently, fees are fixed at 0.001 GXC. Future versions will support custom fees.

### Q: What happens if I don't include a fee?

**A:** Regular transactions require fees. Transactions without fees will be rejected.

### Q: Do mining rewards have fees?

**A:** No, coinbase transactions (mining rewards) have no fees.

### Q: Who receives the fees?

**A:** Miners receive all transaction fees as part of their block reward.

### Q: Are fees subject to the supply cap?

**A:** No, transaction fees are added on top of the base reward and are not subject to the supply cap.

### Q: Can fees be zero?

**A:** Only for coinbase transactions. Regular transactions must have a fee (minimum 0.001 GXC).

---

## 📚 Code References

### Key Files

- **`src/Transaction.cpp`**: Transaction creation and fee handling
- **`src/Blockchain.cpp`**: Fee validation and collection
- **`src/Config.cpp`**: Fee configuration (default: 0.001)
- **`src/RPCAPI.cpp`**: RPC methods for transaction creation

### Key Functions

- `Transaction::getFee()`: Get transaction fee
- `Transaction::setFee()`: Set transaction fee
- `Blockchain::validateTransaction()`: Validates fee is sufficient
- `Blockchain::calculateBlockReward()`: Adds fees to miner reward

---

## ✅ Summary

| Transaction Type | Has Fee? | Fee Amount | Who Pays? |
|-----------------|----------|------------|-----------|
| **Regular Transfer** | ✅ Yes | 0.001 GXC | Sender |
| **Coinbase (Mining)** | ❌ No | 0.0 GXC | N/A |
| **Genesis** | ❌ No | 0.0 GXC | N/A |

**Key Points:**
- ✅ Regular transactions require fees (default: 0.001 GXC)
- ❌ Coinbase transactions have no fees
- 💰 Fees go to miners as part of block reward
- 🔒 Fees are validated: `input >= output + fee`
- 📈 Fees incentivize miners to include transactions

---

## 🔗 Related Documentation

- **Transaction Guide**: See transaction documentation
- **Mining Guide**: See `MINERS_README.md`
- **RPC API**: See RPC API documentation
- **Configuration**: See `gxc.conf.example`
