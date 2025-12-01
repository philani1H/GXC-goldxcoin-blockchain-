# 💸 GXC Transaction Example - Sending 50 GXC

## 📊 Your Scenario

**Your Situation:**
- You mined 8 blocks
- Each block reward: 50 GXC
- **Total Balance: 400 GXC**
- You want to send: **50 GXC to Venda**

---

## 🔄 What Happens Step-by-Step

### **Step 1: Check Your Balance**

**Your UTXOs (Unspent Transaction Outputs):**
```
UTXO 1: 50 GXC (from block 1 mining reward)
UTXO 2: 50 GXC (from block 2 mining reward)
UTXO 3: 50 GXC (from block 3 mining reward)
UTXO 4: 50 GXC (from block 4 mining reward)
UTXO 5: 50 GXC (from block 5 mining reward)
UTXO 6: 50 GXC (from block 6 mining reward)
UTXO 7: 50 GXC (from block 7 mining reward)
UTXO 8: 50 GXC (from block 8 mining reward)

Total: 400 GXC
```

---

### **Step 2: Create Transaction**

**Transaction Details:**
```
From: Your Address (tGXC9fab7317231b966af85ac453e168c0932)
To: Venda's Address (tGXCvenda123456789abcdef)
Amount: 50 GXC
Fee: 0.001 GXC (default)
```

**What Your Wallet Does:**

1. **Select UTXOs:**
   - Wallet picks UTXO 1 (50 GXC)
   - This is enough to cover 50 GXC + 0.001 GXC fee

2. **Create Inputs:**
   ```json
   {
     "inputs": [
       {
         "txHash": "block1_coinbase_tx_hash",
         "outputIndex": 0,
         "amount": 50.0,
         "address": "tGXC9fab7317231b966af85ac453e168c0932"
       }
     ]
   }
   ```

3. **Create Outputs:**
   ```json
   {
     "outputs": [
       {
         "address": "tGXCvenda123456789abcdef",
         "amount": 50.0
       }
     ]
   }
   ```

4. **No Change Needed:**
   - Input: 50 GXC
   - Output: 50 GXC
   - Fee: 0.001 GXC (taken from input)
   - Change: 0 GXC (no change output needed)

---

### **Step 3: Sign Transaction**

**Signing Process:**
```cpp
// 1. Create message to sign
message = txHash + outputIndex + amount

// 2. Sign with your private key using ECDSA
signature = ECDSA_sign(message, privateKey)

// 3. Add signature to input
input.signature = signature
input.publicKey = yourPublicKey
```

**Result:**
```json
{
  "inputs": [
    {
      "txHash": "block1_coinbase_tx_hash",
      "outputIndex": 0,
      "amount": 50.0,
      "signature": "3045022100abc123...def789",
      "publicKey": "02abc123...def789"
    }
  ]
}
```

---

### **Step 4: Broadcast Transaction**

**Transaction Broadcast:**
```
Your Wallet → Network Nodes → Mempool
```

**Transaction Structure:**
```json
{
  "txHash": "tx_abc123def456...",
  "timestamp": 1733029042,
  "inputs": [
    {
      "txHash": "block1_coinbase_tx_hash",
      "outputIndex": 0,
      "amount": 50.0,
      "signature": "3045022100abc123...def789",
      "publicKey": "02abc123...def789",
      "address": "tGXC9fab7317231b966af85ac453e168c0932"
    }
  ],
  "outputs": [
    {
      "address": "tGXCvenda123456789abcdef",
      "amount": 50.0
    }
  ],
  "fee": 0.001,
  "confirmations": 0
}
```

---

### **Step 5: Miner Includes in Block**

**Miner's Actions:**
1. Sees transaction in mempool
2. Validates transaction:
   - ✅ Signature is valid
   - ✅ UTXO exists and unspent
   - ✅ Amount is sufficient
   - ✅ No double-spend
3. Includes in block template
4. Mines block
5. Adds block to blockchain

**Block Structure:**
```json
{
  "height": 9,
  "hash": "block9_hash...",
  "previousHash": "block8_hash...",
  "timestamp": 1733029042,
  "difficulty": 0.1,
  "nonce": 12345,
  "miner": "tGXCminer123...",
  "transactions": [
    {
      "txHash": "coinbase_tx",
      "type": "coinbase",
      "amount": 50.0
    },
    {
      "txHash": "tx_abc123def456...",
      "type": "transfer",
      "from": "tGXC9fab7317231b966af85ac453e168c0932",
      "to": "tGXCvenda123456789abcdef",
      "amount": 50.0,
      "fee": 0.001
    }
  ]
}
```

---

### **Step 6: Confirmations**

**Confirmation Timeline:**
```
Block 9 mined: 1 confirmation (2 minutes)
Block 10 mined: 2 confirmations (4 minutes)
Block 11 mined: 3 confirmations (6 minutes)
Block 12 mined: 4 confirmations (8 minutes)
Block 13 mined: 5 confirmations (10 minutes)
Block 14 mined: 6 confirmations (12 minutes) ✅ FINAL
```

---

## 📊 Complete Transaction Flow

### **Visual Representation**

```
BEFORE TRANSACTION:
┌─────────────────────────────────────┐
│ Your Wallet                         │
│ Address: tGXC9fab...                │
│                                     │
│ UTXOs:                              │
│ ├─ 50 GXC (block 1) ← WILL BE SPENT│
│ ├─ 50 GXC (block 2)                 │
│ ├─ 50 GXC (block 3)                 │
│ ├─ 50 GXC (block 4)                 │
│ ├─ 50 GXC (block 5)                 │
│ ├─ 50 GXC (block 6)                 │
│ ├─ 50 GXC (block 7)                 │
│ └─ 50 GXC (block 8)                 │
│                                     │
│ Total: 400 GXC                      │
└─────────────────────────────────────┘

TRANSACTION:
┌─────────────────────────────────────┐
│ Transaction: tx_abc123...           │
│                                     │
│ INPUT:                              │
│ └─ 50 GXC from block 1 coinbase     │
│                                     │
│ OUTPUT:                             │
│ └─ 50 GXC to Venda (tGXCvenda...)   │
│                                     │
│ Fee: 0.001 GXC (to miner)           │
└─────────────────────────────────────┘

AFTER TRANSACTION (6 confirmations):
┌─────────────────────────────────────┐
│ Your Wallet                         │
│ Address: tGXC9fab...                │
│                                     │
│ UTXOs:                              │
│ ├─ [SPENT] (block 1) ← USED         │
│ ├─ 50 GXC (block 2)                 │
│ ├─ 50 GXC (block 3)                 │
│ ├─ 50 GXC (block 4)                 │
│ ├─ 50 GXC (block 5)                 │
│ ├─ 50 GXC (block 6)                 │
│ ├─ 50 GXC (block 7)                 │
│ └─ 50 GXC (block 8)                 │
│                                     │
│ Total: 350 GXC                      │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│ Venda's Wallet                      │
│ Address: tGXCvenda...               │
│                                     │
│ UTXOs:                              │
│ └─ 50 GXC (from you) ← NEW!         │
│                                     │
│ Total: 50 GXC                       │
└─────────────────────────────────────┘
```

---

## 💰 Example with Change

**Scenario: You want to send 30 GXC (not 50)**

### **Transaction with Change:**

**Input:**
```json
{
  "inputs": [
    {
      "txHash": "block1_coinbase_tx_hash",
      "outputIndex": 0,
      "amount": 50.0
    }
  ]
}
```

**Outputs:**
```json
{
  "outputs": [
    {
      "address": "tGXCvenda123456789abcdef",
      "amount": 30.0
    },
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 19.999
    }
  ]
}
```

**Breakdown:**
- Input: 50 GXC
- Output 1 (to Venda): 30 GXC
- Output 2 (change to you): 19.999 GXC
- Fee: 0.001 GXC

**After Transaction:**
```
Your Balance:
- Lost: 50 GXC (UTXO 1 spent)
- Gained: 19.999 GXC (change)
- Net: -30.001 GXC

New Balance: 369.999 GXC
```

---

## 🔍 Transaction Details

### **Full Transaction Object**

```json
{
  "txHash": "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0",
  "version": 1,
  "timestamp": 1733029042,
  "lockTime": 0,
  "isCoinbase": false,
  "isGoldBacked": false,
  
  "inputs": [
    {
      "txHash": "block1_coinbase_tx_hash",
      "outputIndex": 0,
      "amount": 50.0,
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "signature": "3045022100abc123def456789...0123456789abcdef",
      "publicKey": "02abc123def456789...0123456789abcdef"
    }
  ],
  
  "outputs": [
    {
      "address": "tGXCvenda123456789abcdef",
      "amount": 50.0,
      "scriptPubKey": "OP_DUP OP_HASH160 <pubKeyHash> OP_EQUALVERIFY OP_CHECKSIG"
    }
  ],
  
  "fee": 0.001,
  "size": 250,
  "weight": 1000,
  "confirmations": 6,
  "blockHash": "block9_hash...",
  "blockHeight": 9,
  "blockTime": 1733029042
}
```

---

## 🔐 Security & Validation

### **What Gets Validated:**

1. **Signature Verification:**
   ```cpp
   bool valid = Crypto::verifySignature(
       message,
       input.signature,
       input.publicKey
   );
   ```

2. **UTXO Verification:**
   ```cpp
   // Check UTXO exists
   bool exists = utxoSet.find(utxoKey) != utxoSet.end();
   
   // Check UTXO not already spent
   bool unspent = !utxoSet[utxoKey].spent;
   ```

3. **Amount Verification:**
   ```cpp
   // Input amount >= Output amount + Fee
   double totalIn = sum(inputs.amount);
   double totalOut = sum(outputs.amount);
   double fee = totalIn - totalOut;
   
   bool valid = (totalIn >= totalOut) && (fee >= minFee);
   ```

4. **Double-Spend Check:**
   ```cpp
   // Check if same UTXO used in mempool
   bool doubleSpend = mempool.hasConflictingTx(tx);
   ```

---

## 📊 UTXO Model Explained

### **How UTXOs Work:**

**Think of UTXOs like physical bills:**
- You have eight $50 bills (8 UTXOs of 50 GXC each)
- To pay $50, you give one $50 bill
- You can't split a bill (must use whole UTXO)
- If you need change, you get a new bill back

**Example:**

```
You have: [$50] [$50] [$50] [$50] [$50] [$50] [$50] [$50]

Pay $50:
- Give: [$50]
- Receive: Nothing (exact amount)
- Keep: [$50] [$50] [$50] [$50] [$50] [$50] [$50]

Pay $30:
- Give: [$50]
- Receive: [$19.99] (change)
- Keep: [$50] [$50] [$50] [$50] [$50] [$50] [$50] [$19.99]
```

---

## 🎯 Real-World Example

### **Complete Scenario:**

**Initial State:**
```
Your Address: tGXC9fab7317231b966af85ac453e168c0932
Balance: 400 GXC (8 UTXOs of 50 GXC each)

Venda's Address: tGXCvenda123456789abcdef
Balance: 0 GXC
```

**Transaction:**
```bash
# You create transaction
wallet.createTransaction("tGXCvenda123456789abcdef", 50.0)

# Transaction broadcast
network.broadcast(transaction)

# Miner includes in block
miner.includeInBlock(transaction)

# Block mined
blockchain.addBlock(block)
```

**Final State (after 6 confirmations):**
```
Your Address: tGXC9fab7317231b966af85ac453e168c0932
Balance: 350 GXC (7 UTXOs of 50 GXC each)

Venda's Address: tGXCvenda123456789abcdef
Balance: 50 GXC (1 UTXO of 50 GXC)

Miner: tGXCminer123...
Earned: 50.001 GXC (50 GXC block reward + 0.001 GXC fee)
```

---

## 📈 Transaction Traceability

### **GXC Traceability Formula:**

```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

**Your Transaction Chain:**
```
Genesis Block (50 GXC)
    ↓
Block 1 Coinbase (50 GXC to you)
    ↓
Your Transaction (50 GXC to Venda)
    ↓
Venda's Transaction (50 GXC to someone else)
    ↓
...traceable forever
```

**Every coin can be traced back to its origin!**

---

## ✅ Summary

**When You Send 50 GXC to Venda:**

1. **Your wallet selects** one 50 GXC UTXO
2. **Creates transaction** with:
   - Input: 50 GXC from your mining reward
   - Output: 50 GXC to Venda
   - Fee: 0.001 GXC to miner
3. **Signs transaction** with your private key (ECDSA)
4. **Broadcasts** to network
5. **Miner includes** in block
6. **After 6 confirmations** (~12 minutes):
   - ✅ Your balance: 350 GXC
   - ✅ Venda's balance: 50 GXC
   - ✅ Transaction final

**Transaction Structure:**
- Uses UTXO model (like Bitcoin)
- Proper ECDSA signatures
- Fully traceable
- Secure and validated

**Your 400 GXC from mining is real, spendable, and secure!** 💰

---

*Last Updated: December 1, 2025*
*Version: 2.0.0*
*Status: Transaction System Working ✅*
