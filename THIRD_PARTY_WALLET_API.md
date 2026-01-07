# GXC Third-Party Wallet API - Complete Integration Guide

**Based on actual blockchain code analysis**

This guide is created by reading the actual GXC blockchain implementation to ensure all transactions and fraud reports are submitted correctly according to the protocol.

---

## Table of Contents

1. [Transaction Structure](#transaction-structure)
2. [Creating Transactions](#creating-transactions)
3. [Fraud Report System](#fraud-report-system)
4. [Balance & UTXO Management](#balance--utxo-management)
5. [API Endpoints](#api-endpoints)
6. [Code Examples](#code-examples)

---

## Transaction Structure

### From `include/transaction.h`

```cpp
struct TransactionInput {
    std::string txHash;      // Reference to previous transaction
    uint32_t outputIndex;    // Index of the output in referenced tx
    std::string signature;   // Signature to verify ownership
    double amount;           // Amount from referenced output
    std::string publicKey;   // Public key for verification
};

struct TransactionOutput {
    std::string address;     // Recipient address
    double amount;           // Amount of GXC
    std::string script;      // Output script (optional)
};

enum class TransactionType {
    NORMAL = 0,      // Regular transfer
    STAKE = 1,       // Staking coins
    UNSTAKE = 2,     // Unstaking coins
    REWARD = 3,      // Staking rewards
    COINBASE = 4,    // Mining rewards
    REVERSAL = 5     // Fraud reversal
};
```

### Critical Fields for Traceability

```cpp
// Enhanced Traceability System
std::string prevTxHash;       // Hash of sender's previous transaction (Ti-1)
double referencedAmount;      // Amount transferred from Ti-1
std::string senderAddress;    // Address of the sender
std::string receiverAddress;  // Primary receiver address
uint32_t nonce;              // For transaction uniqueness

// Transaction metadata
double fee;                  // Transaction fee
std::string memo;            // Optional memo field
uint32_t lockTime;           // Time lock for the transaction
```

### Traceability Formula (MUST BE SATISFIED)

```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

**This formula MUST be satisfied for all non-coinbase transactions!**

---

## Creating Transactions

### Step 1: Get UTXOs

```bash
curl -X POST NODE_URL -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "listunspent",
  "params": ["YOUR_ADDRESS"],
  "id": 1
}'
```

**Response:**
```json
{
  "result": [
    {
      "txid": "abc123...",
      "vout": 0,
      "address": "tGXC...",
      "amount": 100.0,
      "confirmations": 10,
      "spendable": true
    }
  ]
}
```

### Step 2: Create Transaction Inputs

```javascript
const inputs = [{
  txHash: "abc123...",        // Previous transaction hash
  outputIndex: 0,             // Output index in previous tx
  amount: 100.0,              // Amount from that output
  signature: "",              // Will be filled after signing
  publicKey: "YOUR_PUBLIC_KEY"
}];
```

### Step 3: Create Transaction Outputs

```javascript
const outputs = [
  {
    address: "RECIPIENT_ADDRESS",
    amount: 95.0,             // Amount to send
    script: ""                // Optional
  },
  {
    address: "YOUR_ADDRESS",  // Change address
    amount: 4.99,             // Change (100 - 95 - 0.01 fee)
    script: ""
  }
];
```

### Step 4: Set Traceability Fields

```javascript
const transaction = {
  type: 0,                    // NORMAL transaction
  inputs: inputs,
  outputs: outputs,
  prevTxHash: "abc123...",    // MUST match inputs[0].txHash
  referencedAmount: 100.0,    // MUST match inputs[0].amount
  senderAddress: "YOUR_ADDRESS",
  receiverAddress: "RECIPIENT_ADDRESS",
  nonce: Date.now(),          // Unique nonce
  fee: 0.01,                  // Transaction fee
  memo: "Payment for goods",  // Optional
  lockTime: 0,                // No time lock
  timestamp: Math.floor(Date.now() / 1000)
};
```

### Step 5: Sign Transaction

```javascript
// Calculate transaction hash
const txData = JSON.stringify({
  type: transaction.type,
  inputs: transaction.inputs.map(i => ({
    txHash: i.txHash,
    outputIndex: i.outputIndex,
    amount: i.amount
  })),
  outputs: transaction.outputs,
  prevTxHash: transaction.prevTxHash,
  timestamp: transaction.timestamp,
  nonce: transaction.nonce
});

const txHash = sha256(txData);

// Sign with private key
const signature = signWithPrivateKey(txHash, YOUR_PRIVATE_KEY);

// Add signature to inputs
transaction.inputs[0].signature = signature;
```

### Step 6: Submit Transaction

```bash
curl -X POST NODE_URL -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "sendrawtransaction",
  "params": [TRANSACTION_HEX],
  "id": 1
}'
```

---

## Fraud Report System

### From `include/MarketMakerAdmin.h` and `src/FraudDetection.cpp`

```cpp
struct FraudReport {
    std::string reportId;
    std::string txHash;           // Transaction to report
    std::string reporterAddress;  // Your address
    double amount;                // Amount stolen
    std::string email;            // Contact email
    std::string description;      // What happened
    std::string evidence;         // Proof (police report, etc.)
    uint64_t timestamp;
    
    // Admin review status
    std::string factsStatus;      // "PENDING", "FACTS_APPROVED", "FACTS_REJECTED"
    std::string reviewedBy;
    uint64_t reviewedAt;
    std::string reviewNotes;
    
    // Protocol execution status
    std::string executionStatus;  // "NOT_STARTED", "VALIDATING", "EXECUTED", "INFEASIBLE"
    std::string proofHash;
    uint64_t validatedAt;
    uint64_t executedAt;
    uint64_t recoveredAmount;
    std::string executionNotes;
};
```

### Submit Fraud Report

**Method 1: GET Endpoint (Railway-compatible)**

```bash
curl "NODE_URL/api/fraud/report?txHash=STOLEN_TX_HASH&reporterAddress=YOUR_ADDRESS&amount=100&description=Wallet%20hacked"
```

**Method 2: POST Endpoint (if available)**

```bash
curl -X POST NODE_URL/api/fraud/report-stolen -H "Content-Type: application/json" -d '{
  "txHash": "STOLEN_TX_HASH",
  "reporterAddress": "YOUR_ADDRESS",
  "amount": 100.0,
  "description": "My wallet was hacked. I did not authorize this transaction.",
  "evidence": "Police report #12345",
  "email": "victim@example.com"
}'
```

**Response:**
```json
{
  "success": true,
  "reportId": "FR_1234567890",
  "message": "Fraud report submitted successfully",
  "status": "pending_review",
  "txHash": "STOLEN_TX_HASH",
  "reporterAddress": "YOUR_ADDRESS",
  "amount": 100.0
}
```

### Check Fraud Report Status

```bash
curl "NODE_URL/api/fraud/check-transaction/STOLEN_TX_HASH"
```

**Response:**
```json
{
  "txHash": "STOLEN_TX_HASH",
  "isTainted": true,
  "taintScore": 0.85,
  "flaggedReports": ["FR_1234567890"],
  "status": "under_review"
}
```

### Taint Propagation Algorithm

From `src/FraudDetection.cpp`:

```
For each descendant transaction T_j:
  τ(T_j) = Σ w_i · τ(T_i)
  where w_i = value_from_T_i / total_input_value

Stop when τ < δ (TAINT_THRESHOLD = 0.01)
```

**What this means:**
- Stolen funds are marked with taint score = 1.0
- As funds move through transactions, taint propagates
- Taint decreases as funds mix with clean funds
- Transactions with taint > 0.01 are flagged

---

## Balance & UTXO Management

### Get Balance

```bash
curl -X POST NODE_URL -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "getbalance",
  "params": ["YOUR_ADDRESS"],
  "id": 1
}'
```

### Get Staking Info

```bash
curl -X POST NODE_URL -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "getstakinginfo",
  "params": ["YOUR_ADDRESS"],
  "id": 1
}'
```

**Response:**
```json
{
  "result": {
    "address": "YOUR_ADDRESS",
    "spendable_balance": 100.0,
    "staked_balance": 50.0,
    "total_balance": 150.0,
    "total_earned_mining": 10.0,
    "total_earned_staking": 5.0,
    "is_validator": false
  }
}
```

### List Unspent Outputs (UTXOs)

```bash
curl -X POST NODE_URL -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "listunspent",
  "params": ["YOUR_ADDRESS"],
  "id": 1
}'
```

### Coinbase Maturity

From `src/RPCAPI.cpp`:

```cpp
const uint32_t COINBASE_MATURITY = isTestnet ? 6 : 100;

// Coinbase outputs must have COINBASE_MATURITY confirmations
// before they can be spent
```

**Important:** Mining rewards cannot be spent immediately!
- **Testnet:** 6 confirmations required
- **Mainnet:** 100 confirmations required

---

## API Endpoints

### Core Wallet Operations

| Method | Parameters | Description |
|--------|-----------|-------------|
| `getnewaddress` | [label] | Generate new address |
| `getbalance` | [address] | Get balance |
| `listunspent` | [address] | List UTXOs |
| `sendtoaddress` | to, amount, [from] | Send coins |
| `sendrawtransaction` | hex | Submit signed transaction |
| `getstakinginfo` | [address] | Get staking details |

### Transaction Creation

| Method | Parameters | Description |
|--------|-----------|-------------|
| `createrawtransaction` | inputs, outputs | Create unsigned transaction |
| `signrawtransaction` | hex, privkey | Sign transaction |
| `decoderawtransaction` | hex | Decode transaction |

### Fraud Detection

| Method | Parameters | Description |
|--------|-----------|-------------|
| `GET /api/fraud/report` | txHash, reporterAddress, amount, description | Submit fraud report |
| `GET /api/fraud/check-transaction` | txHash | Check if transaction is tainted |
| `GET /api/fraud/check-address` | address | Check if address is flagged |

### Staking Operations

| Method | Parameters | Description |
|--------|-----------|-------------|
| `stake` | address, amount, days | Stake coins |
| `unstake` | address, amount | Unstake coins |
| `registervalidator` | address | Register as validator |
| `listvalidators` | - | List all validators |

---

## Code Examples

### Complete Transaction Example (JavaScript)

```javascript
const GXCWallet = {
  nodeUrl: "https://node.gxc.network",
  
  async getUTXOs(address) {
    const response = await fetch(this.nodeUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        jsonrpc: "2.0",
        method: "listunspent",
        params: [address],
        id: 1
      })
    });
    return (await response.json()).result;
  },
  
  async sendTransaction(fromAddress, toAddress, amount, privateKey) {
    // Step 1: Get UTXOs
    const utxos = await this.getUTXOs(fromAddress);
    if (utxos.length === 0) {
      throw new Error("No UTXOs available");
    }
    
    // Step 2: Select UTXO (simple: use first one)
    const utxo = utxos[0];
    if (utxo.amount < amount) {
      throw new Error("Insufficient funds in UTXO");
    }
    
    // Step 3: Calculate fee and change
    const fee = 0.01;
    const change = utxo.amount - amount - fee;
    
    // Step 4: Create transaction
    const tx = {
      type: 0, // NORMAL
      inputs: [{
        txHash: utxo.txid,
        outputIndex: utxo.vout,
        amount: utxo.amount,
        signature: "",
        publicKey: getPublicKey(privateKey)
      }],
      outputs: [
        { address: toAddress, amount: amount, script: "" },
        { address: fromAddress, amount: change, script: "" }
      ],
      prevTxHash: utxo.txid,        // MUST match inputs[0].txHash
      referencedAmount: utxo.amount, // MUST match inputs[0].amount
      senderAddress: fromAddress,
      receiverAddress: toAddress,
      nonce: Date.now(),
      fee: fee,
      memo: "",
      lockTime: 0,
      timestamp: Math.floor(Date.now() / 1000)
    };
    
    // Step 5: Sign transaction
    const txHash = this.calculateTxHash(tx);
    tx.inputs[0].signature = this.sign(txHash, privateKey);
    
    // Step 6: Submit transaction
    const response = await fetch(this.nodeUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        jsonrpc: "2.0",
        method: "sendrawtransaction",
        params: [this.serializeTransaction(tx)],
        id: 1
      })
    });
    
    return await response.json();
  },
  
  calculateTxHash(tx) {
    // Implement SHA-256 double hash
    const data = JSON.stringify({
      type: tx.type,
      inputs: tx.inputs.map(i => ({
        txHash: i.txHash,
        outputIndex: i.outputIndex,
        amount: i.amount
      })),
      outputs: tx.outputs,
      prevTxHash: tx.prevTxHash,
      timestamp: tx.timestamp,
      nonce: tx.nonce
    });
    return sha256d(data);
  },
  
  sign(hash, privateKey) {
    // Implement ECDSA signing
    return signECDSA(hash, privateKey);
  },
  
  serializeTransaction(tx) {
    // Serialize to hex format
    return Buffer.from(JSON.stringify(tx)).toString('hex');
  }
};

// Usage
await GXCWallet.sendTransaction(
  "tGXCYourAddress...",
  "tGXCRecipientAddress...",
  100.0,
  "YOUR_PRIVATE_KEY"
);
```

### Fraud Report Example (JavaScript)

```javascript
async function reportStolenTransaction(txHash, yourAddress, amount, description) {
  const nodeUrl = "https://node.gxc.network";
  
  // Method 1: GET endpoint (Railway-compatible)
  const url = `${nodeUrl}/api/fraud/report?` +
    `txHash=${txHash}&` +
    `reporterAddress=${yourAddress}&` +
    `amount=${amount}&` +
    `description=${encodeURIComponent(description)}`;
  
  const response = await fetch(url);
  const result = await response.json();
  
  console.log("Report ID:", result.reportId);
  console.log("Status:", result.status);
  
  return result;
}

// Usage
await reportStolenTransaction(
  "abc123...",
  "tGXCYourAddress...",
  100.0,
  "My wallet was hacked"
);
```

### Check Transaction Taint Example

```javascript
async function checkTransactionTaint(txHash) {
  const nodeUrl = "https://node.gxc.network";
  
  const response = await fetch(
    `${nodeUrl}/api/fraud/check-transaction/${txHash}`
  );
  
  const result = await response.json();
  
  if (result.isTainted) {
    console.warn(`⚠️  Transaction is tainted!`);
    console.warn(`Taint Score: ${result.taintScore}`);
    console.warn(`Flagged Reports: ${result.flaggedReports.length}`);
  } else {
    console.log("✅ Transaction is clean");
  }
  
  return result;
}
```

---

## Critical Requirements

### ✅ MUST DO

1. **Set prevTxHash correctly** - Must match inputs[0].txHash
2. **Set referencedAmount correctly** - Must match inputs[0].amount
3. **Sign all inputs** - Use ECDSA with your private key
4. **Calculate fees** - Use `blockchain->calculateRecommendedFee()`
5. **Check UTXO maturity** - Coinbase outputs need 6-100 confirmations
6. **Validate before submitting** - Check balance, fees, and traceability

### ❌ MUST NOT DO

1. **Don't skip traceability fields** - prevTxHash and referencedAmount are required
2. **Don't reuse nonces** - Each transaction needs a unique nonce
3. **Don't spend immature coinbase** - Wait for required confirmations
4. **Don't submit without signatures** - All inputs must be signed
5. **Don't ignore taint warnings** - Check transactions before accepting

---

## Testing Your Integration

### Test Checklist

- [ ] Can create new addresses
- [ ] Can query balance correctly
- [ ] Can list UTXOs
- [ ] Can create valid transactions
- [ ] Transactions pass traceability validation
- [ ] Can sign transactions correctly
- [ ] Can submit transactions successfully
- [ ] Can submit fraud reports
- [ ] Can check transaction taint
- [ ] Can handle coinbase maturity
- [ ] Can calculate fees correctly

### Test on Testnet First

```javascript
const TESTNET_NODE = "https://testnet.gxc.network";
const MAINNET_NODE = "https://node.gxc.network";

// Always test on testnet first!
const nodeUrl = TESTNET_NODE;
```

---

## Support

For questions or issues:
1. Check this documentation
2. Review the blockchain source code
3. Test on testnet before mainnet
4. Contact GXC support

---

## Summary

This API documentation is based on actual blockchain code analysis:
- Transaction structure from `include/transaction.h`
- Fraud detection from `src/FraudDetection.cpp`
- RPC methods from `src/RPCAPI.cpp`
- Traceability requirements from the protocol

**All examples ensure transactions are submitted correctly according to the GXC protocol!**
