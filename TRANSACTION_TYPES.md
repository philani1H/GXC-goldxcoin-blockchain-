# GXC Blockchain Transaction Types

This document shows how each transaction type looks in the blockchain.

## Transaction Type Enum

```cpp
enum class TransactionType {
    NORMAL = 0,   // Regular coin transfer
    STAKE = 1,    // Lock coins for staking
    UNSTAKE = 2   // Unlock coins from staking
};
```

---

## 1. COINBASE Transaction (Block Reward)

**Purpose**: Create new coins as block reward for miners/validators

```json
{
  "txHash": "f528962c9e04406416dd1f96cd0ae5f4b4e6c4c2...",
  "type": "COINBASE",
  "isCoinbase": true,
  "inputs": [],
  "outputs": [
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 12.5,
      "script": "OP_DUP OP_HASH160 tGXC9fab... OP_EQUALVERIFY OP_CHECKSIG"
    }
  ],
  "prevTxHash": "0",
  "referencedAmount": 0.0,
  "fee": 0.0,
  "memo": "",
  "blockHeight": 1,
  "confirmations": 100
}
```

**Characteristics**:
- ✅ No inputs (coins created from nothing)
- ✅ Single output to miner/validator address
- ✅ prevTxHash is "0" (genesis reference)
- ✅ Creates new UTXO for the recipient

---

## 2. NORMAL Transaction (Send Coins)

**Purpose**: Transfer coins from one address to another

```json
{
  "txHash": "a1b2c3d4e5f6789012345678901234567890abcd...",
  "type": "NORMAL",
  "isCoinbase": false,
  "inputs": [
    {
      "txHash": "f528962c9e04406416dd1f96cd0ae5f4b4e6c4c2...",
      "outputIndex": 0,
      "amount": 100.0,
      "signature": "3045022100abcd...1234",
      "publicKey": "04abc123..."
    }
  ],
  "outputs": [
    {
      "address": "GXCrecipient123456789012345678901234",
      "amount": 50.0
    },
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 49.999
    }
  ],
  "prevTxHash": "f528962c9e04406416dd1f96cd0ae5f4b4e6c4c2...",
  "referencedAmount": 100.0,
  "senderAddress": "tGXC9fab7317231b966af85ac453e168c0932",
  "receiverAddress": "GXCrecipient123456789012345678901234",
  "fee": 0.001,
  "memo": "Payment for services"
}
```

**Traceability Formula Verification**:
```
✅ inputs[0].txHash == prevTxHash
   "f528962c..." == "f528962c..."

✅ inputs[0].amount == referencedAmount
   100.0 == 100.0
```

**Balance Changes**:
```
Sender:   -100.0 GXC (input spent)
          +49.999 GXC (change received)
          NET: -50.001 GXC (amount + fee)

Receiver: +50.0 GXC
Miner:    +0.001 GXC (fee)
```

---

## 3. STAKE Transaction (Lock Coins for Staking)

**Purpose**: Lock coins to become a validator

```json
{
  "txHash": "stake123456789abcdef0123456789abcdef01234...",
  "type": "STAKE",
  "isCoinbase": false,
  "inputs": [
    {
      "txHash": "a1b2c3d4e5f6789012345678901234567890abcd...",
      "outputIndex": 1,
      "amount": 49.999,
      "signature": "3045022100efgh...5678",
      "publicKey": "04def456..."
    },
    {
      "txHash": "previous_tx_hash_1234567890...",
      "outputIndex": 0,
      "amount": 100.0,
      "signature": "3045022100ijkl...9012",
      "publicKey": "04def456..."
    }
  ],
  "outputs": [
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 49.998
    }
  ],
  "prevTxHash": "a1b2c3d4e5f6789012345678901234567890abcd...",
  "referencedAmount": 49.999,
  "senderAddress": "tGXC9fab7317231b966af85ac453e168c0932",
  "receiverAddress": "tGXC9fab7317231b966af85ac453e168c0932",
  "fee": 0.001,
  "memo": "Stake: 100.0 GXC"
}
```

**Key Difference**: 
- **Staked amount (100 GXC) is NOT in outputs!**
- Only change is output (49.999 - 0.001 fee = 49.998)
- Staked coins are "locked" by not having a UTXO

**Balance Changes**:
```
Total Inputs:  49.999 + 100.0 = 149.999 GXC
Total Outputs: 49.998 GXC (change only)
Fee:           0.001 GXC
Staked:        149.999 - 49.998 - 0.001 = 100.0 GXC (LOCKED!)

Spendable Balance: -100.001 GXC
Staked Balance:    +100.0 GXC
```

---

## 4. UNSTAKE Transaction (Unlock Coins)

**Purpose**: Withdraw coins from staking back to spendable balance

```json
{
  "txHash": "unstake98765432109876543210abcdef012345...",
  "type": "UNSTAKE",
  "isCoinbase": false,
  "inputs": [
    {
      "txHash": "stake123456789abcdef0123456789abcdef01234...",
      "outputIndex": 0,
      "amount": 0.0,
      "signature": "3045022100mnop...3456",
      "publicKey": "04def456..."
    }
  ],
  "outputs": [
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 100.0
    }
  ],
  "prevTxHash": "stake123456789abcdef0123456789abcdef01234...",
  "referencedAmount": 0.0,
  "senderAddress": "tGXC9fab7317231b966af85ac453e168c0932",
  "receiverAddress": "tGXC9fab7317231b966af85ac453e168c0932",
  "fee": 0.0,
  "memo": "Unstake: 100.0 GXC"
}
```

**Key Difference**:
- Input has 0 amount (coins come from stake pool, not UTXO)
- Output creates new UTXO with unstaked amount
- No fee required (no inputs to pay from)

**Balance Changes**:
```
Staked Balance:    -100.0 GXC
Spendable Balance: +100.0 GXC (new UTXO created)
```

---

## Transaction Validation Rules

### All Transactions
```cpp
// Basic validation
if (tx.getHash().empty()) return false;
if (tx.getOutputs().empty()) return false;

// Coinbase exempt from below
if (tx.isCoinbaseTransaction()) return true;
```

### NORMAL Transactions
```cpp
// 1. Traceability Formula
if (inputs[0].txHash != prevTxHash) return false;
if (inputs[0].amount != referencedAmount) return false;

// 2. UTXO Validation
for (input : inputs) {
    utxoKey = input.txHash + "_" + input.outputIndex;
    if (!utxoSet.contains(utxoKey)) return false;  // Double-spend!
    if (utxoSet[utxoKey].amount != input.amount) return false;
}

// 3. Balance Check
totalInputs = sum(inputs.amount);
totalOutputs = sum(outputs.amount);
if (totalInputs < totalOutputs + fee) return false;

// 4. Minimum Fee
if (fee < 0.001) return false;
```

### STAKE Transactions
```cpp
// Calculate staked amount
stakedAmount = totalInputs - totalOutputs - fee;

// Minimum stake
if (stakedAmount < 100.0) return false;

// Minimum fee
if (fee < 0.001) return false;

// All inputs must be valid UTXOs
for (input : inputs) {
    if (!utxoSet.contains(input.txHash + "_" + input.outputIndex)) {
        return false;
    }
}
```

### UNSTAKE Transactions
```cpp
// Get staker address
stakerAddress = outputs[0].address;

// Verify they have staked balance
validator = validatorMap[stakerAddress];
if (!validator) return false;

unstakeAmount = sum(outputs.amount);
if (validator.stakeAmount < unstakeAmount) return false;
```

---

## UTXO Set Changes by Transaction Type

### COINBASE
```
Before: utxoSet = { ... }
After:  utxoSet = { ..., "txHash_0": {address: miner, amount: 12.5} }
```

### NORMAL (Send)
```
Before: utxoSet = { "inputTx_0": {address: sender, amount: 100} }
After:  utxoSet = { 
           "newTx_0": {address: recipient, amount: 50},
           "newTx_1": {address: sender, amount: 49.999}  // change
        }
Input UTXO removed, output UTXOs added
```

### STAKE
```
Before: utxoSet = { "inputTx_0": {address: staker, amount: 200} }
After:  utxoSet = { "stakeTx_0": {address: staker, amount: 99.999} }  // change only!

Staked 100 GXC has NO UTXO - it's locked in validator registry!
```

### UNSTAKE
```
Before: utxoSet = { ... }  // No change
        validatorRegistry = { staker: 100 GXC staked }
        
After:  utxoSet = { ..., "unstakeTx_0": {address: staker, amount: 100} }
        validatorRegistry = { staker: 0 GXC staked }

Creates new UTXO from stake pool, not from existing UTXOs
```

---

## Transaction Category in Wallet History

| Transaction Type | Category | UI Display |
|-----------------|----------|------------|
| COINBASE to you | `generate` | ⛏️ +12.5 GXC (Mining Reward) |
| NORMAL from you | `send` | 📤 -50.0 GXC (Sent) |
| NORMAL to you | `receive` | 📥 +50.0 GXC (Received) |
| STAKE | `stake` | 🔒 -100.0 GXC (Staked) |
| UNSTAKE | `unstake` | 🔓 +100.0 GXC (Unstaked) |

---

## RPC Examples

### Send Coins (NORMAL)
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "sendtoaddress",
    "params": ["GXCrecipient...", 50.0],
    "id": 1
  }'
```

### Stake Coins (STAKE)
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "registervalidator",
    "params": ["tGXCyouraddress...", 100.0, 30],
    "id": 1
  }'
```

### Unstake Coins (UNSTAKE)
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "unstake",
    "params": ["tGXCyouraddress...", 50.0],
    "id": 1
  }'
```

### Get Staking Info
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getstakinginfo",
    "params": ["tGXCyouraddress..."],
    "id": 1
  }'
```

Response:
```json
{
  "result": {
    "address": "tGXCyouraddress...",
    "spendable_balance": 49.998,
    "staked_balance": 100.0,
    "total_balance": 149.998,
    "is_validator": true,
    "validator_total_rewards": 25.0,
    "validator_blocks_produced": 2,
    "apy": 2.5
  }
}
```
