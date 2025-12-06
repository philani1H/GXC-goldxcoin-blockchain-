# Third-Party Wallet Integration Guide

## Core Principle: True Decentralization

**GXC is a truly decentralized blockchain.** You do NOT need to import your private key into the node to use your wallet. The blockchain validates transactions based on cryptographic signatures, not on whether the node "controls" an address.

## How It Works

### 1. Wallet Ownership = Private Key Ownership

```
Private Key → Public Key → Address
```

- **You own the wallet** if you have the private key
- **The blockchain verifies ownership** through signatures
- **The node does NOT need your private key** to validate transactions

### 2. Transaction Flow for Third-Party Wallets

```
Third-Party Wallet (Client Side)
    ↓
1. Create transaction with inputs/outputs
    ↓
2. Sign transaction with private key (CLIENT SIDE - NEVER SEND KEY!)
    ↓
3. Serialize transaction to hex
    ↓
4. Send via RPC: sendrawtransaction <hex>
    ↓
Node validates:
  - Signature matches public key
  - Public key derives to address
  - Address owns the UTXOs being spent
  - Sum(inputs) >= Sum(outputs) + fee
    ↓
Transaction accepted to mempool
    ↓
Mined into block
    ↓
Balance updated on-chain
```

## RPC Methods for Third-Party Wallets

### ✅ Recommended: Use `sendrawtransaction`

This is the **proper decentralized way** to submit transactions.

```json
{
  "jsonrpc": "2.0",
  "method": "sendrawtransaction",
  "params": ["<hex_encoded_signed_transaction>"],
  "id": 1
}
```

**Advantages:**
- ✅ Private key stays on client side
- ✅ Truly decentralized
- ✅ Works with any wallet software
- ✅ No trust required in node operator
- ✅ Follows Bitcoin/Ethereum model

### ❌ Not Recommended: `sendtoaddress` with `importprivkey`

```json
// DON'T DO THIS - Requires importing private key
{
  "jsonrpc": "2.0",
  "method": "sendtoaddress",
  "params": ["<to_address>", <amount>, "<from_address>"],
  "id": 1
}
```

**Problems:**
- ❌ Requires `importprivkey` first
- ❌ Exposes private key to node
- ❌ Not decentralized
- ❌ Security risk if node is compromised

## Step-by-Step: Creating and Sending Transactions

### Step 1: Query Balance and UTXOs

```bash
# Get balance
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "getbalance",
  "params": ["<your_address>"],
  "id": 1
}'

# Get UTXOs (unspent outputs)
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "listunspent",
  "params": ["<your_address>"],
  "id": 1
}'
```

### Step 2: Create Transaction (Client Side)

```cpp
// Example in C++ (your wallet code)
Transaction tx;

// Add inputs from UTXOs
for (const auto& utxo : selectedUtxos) {
    TransactionInput input;
    input.txHash = utxo.txHash;
    input.outputIndex = utxo.outputIndex;
    input.amount = utxo.amount;
    tx.addInput(input);
}

// Add output to recipient
TransactionOutput output;
output.address = recipientAddress;
output.amount = sendAmount;
tx.addOutput(output);

// Add change output if needed
if (change > 0) {
    TransactionOutput changeOutput;
    changeOutput.address = myAddress;
    changeOutput.amount = change;
    tx.addOutput(changeOutput);
}

// Set fee and other fields
tx.setFee(0.001);
tx.setSenderAddress(myAddress);
tx.setReceiverAddress(recipientAddress);

// CRITICAL: Sign with YOUR private key (client side)
tx.signInputs(myPrivateKey);

// Serialize to hex
std::string txHex = Utils::toHex(tx.serialize());
```

### Step 3: Send via RPC

```bash
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "sendrawtransaction",
  "params": ["<tx_hex>"],
  "id": 1
}'
```

### Step 4: Verify Transaction

```bash
# Get transaction status
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "gettransaction",
  "params": ["<tx_hash>"],
  "id": 1
}'

# Check new balance
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "getbalance",
  "params": ["<your_address>"],
  "id": 1
}'
```

## Staking with Third-Party Wallets

### Step 1: Register as Validator

```bash
# First, register as validator (creates validator record)
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "registervalidator",
  "params": ["<your_address>", 100.0, 30],
  "id": 1
}'
```

**Note:** This will fail if you haven't imported the private key. This is a limitation we need to fix.

### Step 2: Create Stake Transaction (Client Side)

```cpp
// Create STAKE transaction
Transaction stakeTx;
stakeTx.setType(TransactionType::STAKE);

// Add inputs (UTXOs to stake)
// ... (same as normal transaction)

// Add change output only (staked amount is locked, not sent)
if (change > 0) {
    TransactionOutput changeOutput;
    changeOutput.address = myAddress;
    changeOutput.amount = change;
    stakeTx.addOutput(changeOutput);
}

stakeTx.setFee(0.001);
stakeTx.signInputs(myPrivateKey);

std::string stakeTxHex = Utils::toHex(stakeTx.serialize());
```

### Step 3: Send Stake Transaction

```bash
curl -X POST http://node:8332 -H "Content-Type: application/json" -d '{
  "jsonrpc": "2.0",
  "method": "sendrawtransaction",
  "params": ["<stake_tx_hex>"],
  "id": 1
}'
```

## Security Best Practices

### ✅ DO:
1. **Keep private keys on client side** - Never send to node
2. **Sign transactions locally** - Use your wallet software
3. **Verify addresses** - Check network prefix (GXC for mainnet, tGXC for testnet)
4. **Use HTTPS** - Encrypt communication with node
5. **Verify transaction before signing** - Check amounts and addresses
6. **Back up private keys** - Use mnemonic phrases

### ❌ DON'T:
1. **Never send private keys over network** - Not even to your own node
2. **Don't trust node with keys** - Node could be compromised
3. **Don't reuse addresses** - Use HD wallets for privacy
4. **Don't skip signature verification** - Always verify before broadcasting

## Traceability Formula

GXC implements a unique traceability system:

```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

**What this means:**
- Every transaction references its previous transaction
- The first input must match the previous transaction hash
- The first input amount must match the referenced amount
- This creates an unbreakable chain of custody

**For wallet developers:**
When creating a transaction, ensure:
```cpp
tx.setPrevTxHash(inputs[0].txHash);
tx.setReferencedAmount(inputs[0].amount);
```

## Example: Complete Transaction Flow

```javascript
// JavaScript example for web wallet
async function sendGXC(fromAddress, toAddress, amount, privateKey) {
    // 1. Get UTXOs
    const utxos = await rpc('listunspent', [fromAddress]);
    
    // 2. Select UTXOs (simple: use first sufficient UTXO)
    let totalInput = 0;
    const selectedUtxos = [];
    const fee = 0.001;
    const required = amount + fee;
    
    for (const utxo of utxos) {
        selectedUtxos.push(utxo);
        totalInput += utxo.amount;
        if (totalInput >= required) break;
    }
    
    if (totalInput < required) {
        throw new Error('Insufficient funds');
    }
    
    // 3. Create transaction
    const tx = {
        inputs: selectedUtxos.map(u => ({
            txHash: u.txHash,
            outputIndex: u.outputIndex,
            amount: u.amount
        })),
        outputs: [
            { address: toAddress, amount: amount }
        ],
        fee: fee,
        senderAddress: fromAddress,
        receiverAddress: toAddress,
        prevTxHash: selectedUtxos[0].txHash,
        referencedAmount: selectedUtxos[0].amount
    };
    
    // Add change if needed
    const change = totalInput - amount - fee;
    if (change > 0.00000001) {
        tx.outputs.push({ address: fromAddress, amount: change });
    }
    
    // 4. Sign transaction (CLIENT SIDE!)
    const signedTx = signTransaction(tx, privateKey);
    
    // 5. Serialize to hex
    const txHex = serializeTransaction(signedTx);
    
    // 6. Send to node
    const txHash = await rpc('sendrawtransaction', [txHex]);
    
    return txHash;
}
```

## Node Validation Process

When you submit a transaction via `sendrawtransaction`, the node validates:

1. **Signature Verification**
   ```cpp
   // Node checks: Does signature match public key?
   bool valid = Crypto::verifySignature(message, signature, publicKey);
   ```

2. **Address Derivation**
   ```cpp
   // Node checks: Does public key derive to claimed address?
   std::string derivedAddress = deriveAddress(publicKey);
   if (derivedAddress != claimedAddress) return false;
   ```

3. **UTXO Ownership**
   ```cpp
   // Node checks: Does address own the UTXOs being spent?
   for (const auto& input : tx.getInputs()) {
       const UTXO& utxo = utxoSet[input.txHash + "_" + input.outputIndex];
       if (utxo.address != derivedAddress) return false;
   }
   ```

4. **Balance Check**
   ```cpp
   // Node checks: Are inputs sufficient?
   double inputTotal = sumInputs(tx);
   double outputTotal = sumOutputs(tx);
   if (inputTotal < outputTotal + tx.fee) return false;
   ```

5. **Double-Spend Check**
   ```cpp
   // Node checks: Are UTXOs unspent?
   for (const auto& input : tx.getInputs()) {
       if (utxoSet.find(utxoKey) == utxoSet.end()) return false;
   }
   ```

**All of this happens WITHOUT the node having your private key!**

## Common Issues and Solutions

### Issue 1: "Wallet does not control address"

**Problem:** You're using `sendtoaddress` instead of `sendrawtransaction`

**Solution:** Use `sendrawtransaction` with a client-signed transaction

### Issue 2: "Import the private key first"

**Problem:** Node is asking for your private key (bad!)

**Solution:** Don't import! Use `sendrawtransaction` instead

### Issue 3: "Transaction verification failed"

**Possible causes:**
- Signature is invalid (check your signing code)
- UTXOs don't exist or are already spent
- Insufficient balance (inputs < outputs + fee)
- Address doesn't match public key

**Solution:** Verify transaction locally before sending

### Issue 4: "Validator not registered"

**Problem:** Trying to stake without registering as validator first

**Solution:** 
1. Register as validator first (this currently requires key import - we need to fix this)
2. Then send stake transaction via `sendrawtransaction`

## Future Improvements Needed

### 1. External Validator Registration

Currently, `registervalidator` requires importing the private key. We should add:

```json
{
  "jsonrpc": "2.0",
  "method": "registerexternalvalidator",
  "params": [
    "<address>",
    <stake_amount>,
    <staking_days>,
    "<signature>",
    "<message>",
    "<public_key>"
  ],
  "id": 1
}
```

Where:
- `signature` = sign("register_validator:" + address + ":" + stake_amount + ":" + staking_days)
- `message` = the message that was signed
- `public_key` = your public key (for verification)

### 2. Better Error Messages

RPC methods should guide users to `sendrawtransaction` instead of asking for private key import.

### 3. Transaction Builder Helper

Add RPC method to help build transactions:

```json
{
  "jsonrpc": "2.0",
  "method": "createrawtransaction",
  "params": [
    [{"txid": "...", "vout": 0}],  // inputs
    {"address1": 10.0, "address2": 5.0}  // outputs
  ],
  "id": 1
}
```

Returns unsigned transaction that client can sign.

## Conclusion

**GXC is a truly decentralized blockchain.** You can use it with any wallet software without trusting the node operator with your private keys. Always use `sendrawtransaction` for maximum security and decentralization.

**Remember:**
- 🔑 Your keys = Your coins
- 🚫 Never share private keys
- ✅ Sign transactions client-side
- 🌐 Use `sendrawtransaction` for true decentralization
