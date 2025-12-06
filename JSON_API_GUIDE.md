# GXC Blockchain JSON API Guide

## Overview

The GXC blockchain RPC API accepts **flexible JSON formats** to support all types of applications:
- ✅ React Native apps
- ✅ Web applications (JavaScript/TypeScript)
- ✅ Mobile apps (iOS/Android)
- ✅ Desktop wallets
- ✅ Any JSON-based client

## Core Principle: Format Flexibility

The API accepts parameters in **multiple formats** for maximum compatibility:

```javascript
// Amount can be:
"amount": 10.5          // ✅ Number
"amount": "10.5"        // ✅ String
"amount": "10.50"       // ✅ String with decimals

// All are valid and handled correctly!
```

## Transaction Submission Methods

### Method 1: JSON Transaction Object (Recommended for Web/Mobile)

Send transactions as **pure JSON** - perfect for React Native, web apps, and mobile apps.

```javascript
// POST to http://node:8332
{
  "jsonrpc": "2.0",
  "method": "sendrawtransaction",
  "params": [{
    "inputs": [
      {
        "txHash": "abc123...",
        "outputIndex": 0,
        "amount": 50.0,        // Can be number or string
        "signature": "sig...",
        "publicKey": "pub..."
      }
    ],
    "outputs": [
      {
        "address": "GXC1abc...",
        "amount": 10.0,        // Can be number or string
        "script": ""
      },
      {
        "address": "GXC1xyz...",  // Change output
        "amount": 39.999,      // Can be number or string
        "script": ""
      }
    ],
    "fee": 0.001,            // Can be number or string
    "senderAddress": "GXC1abc...",
    "receiverAddress": "GXC1def...",
    "prevTxHash": "xyz789...",
    "referencedAmount": 50.0,  // Can be number or string
    "type": "NORMAL"         // NORMAL, STAKE, or UNSTAKE
  }],
  "id": 1
}
```

### Method 2: Hex-Encoded Transaction (Traditional)

Send transactions as hex string - compatible with Bitcoin-style clients.

```javascript
{
  "jsonrpc": "2.0",
  "method": "sendrawtransaction",
  "params": ["0x48656c6c6f..."],  // Hex-encoded serialized transaction
  "id": 1
}
```

## React Native Example

```javascript
// React Native wallet sending GXC
import axios from 'axios';

async function sendGXC(fromAddress, toAddress, amount, privateKey) {
  // 1. Get UTXOs
  const utxos = await rpcCall('listunspent', [fromAddress]);
  
  // 2. Select UTXOs
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
  
  // 3. Create transaction as JSON
  const tx = {
    inputs: selectedUtxos.map(u => ({
      txHash: u.txHash,
      outputIndex: u.outputIndex,
      amount: u.amount,  // React Native can send as number or string
      signature: '',     // Will be filled after signing
      publicKey: ''      // Will be filled after signing
    })),
    outputs: [
      {
        address: toAddress,
        amount: amount,    // React Native can send as number or string
        script: ''
      }
    ],
    fee: fee,            // React Native can send as number or string
    senderAddress: fromAddress,
    receiverAddress: toAddress,
    prevTxHash: selectedUtxos[0].txHash,
    referencedAmount: selectedUtxos[0].amount,
    type: 'NORMAL'
  };
  
  // Add change if needed
  const change = totalInput - amount - fee;
  if (change > 0.00000001) {
    tx.outputs.push({
      address: fromAddress,
      amount: change,
      script: ''
    });
  }
  
  // 4. Sign transaction (client-side!)
  const signedTx = await signTransaction(tx, privateKey);
  
  // 5. Send as JSON directly - no hex encoding needed!
  const response = await axios.post('http://node:8332', {
    jsonrpc: '2.0',
    method: 'sendrawtransaction',
    params: [signedTx],  // Send JSON object directly
    id: 1
  });
  
  return response.data.result;  // Returns transaction hash
}

// Helper function to sign transaction
async function signTransaction(tx, privateKey) {
  // Sign each input
  for (let i = 0; i < tx.inputs.length; i++) {
    const input = tx.inputs[i];
    const message = input.txHash + input.outputIndex + input.amount;
    
    // Use your crypto library (e.g., elliptic, noble-secp256k1)
    const signature = await signMessage(message, privateKey);
    const publicKey = await getPublicKey(privateKey);
    
    input.signature = signature;
    input.publicKey = publicKey;
  }
  
  return tx;
}

// RPC helper
async function rpcCall(method, params) {
  const response = await axios.post('http://node:8332', {
    jsonrpc: '2.0',
    method: method,
    params: params,
    id: 1
  });
  
  if (response.data.error) {
    throw new Error(response.data.error.message);
  }
  
  return response.data.result;
}
```

## Web Application Example (JavaScript)

```javascript
// Web wallet using fetch API
class GXCWallet {
  constructor(nodeUrl) {
    this.nodeUrl = nodeUrl;
  }
  
  async rpc(method, params) {
    const response = await fetch(this.nodeUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        jsonrpc: '2.0',
        method: method,
        params: params,
        id: Date.now()
      })
    });
    
    const data = await response.json();
    if (data.error) throw new Error(data.error.message);
    return data.result;
  }
  
  async getBalance(address) {
    return await this.rpc('getbalance', [address]);
  }
  
  async sendTransaction(txJson) {
    // Send JSON transaction directly
    return await this.rpc('sendrawtransaction', [txJson]);
  }
  
  async createAndSendTransaction(from, to, amount, privateKey) {
    // Get UTXOs
    const utxos = await this.rpc('listunspent', [from]);
    
    // Build transaction
    const tx = this.buildTransaction(from, to, amount, utxos);
    
    // Sign transaction
    const signedTx = await this.signTransaction(tx, privateKey);
    
    // Send as JSON
    const txHash = await this.sendTransaction(signedTx);
    
    return txHash;
  }
  
  buildTransaction(from, to, amount, utxos) {
    // Select UTXOs
    let totalInput = 0;
    const selectedUtxos = [];
    const fee = 0.001;
    
    for (const utxo of utxos) {
      selectedUtxos.push(utxo);
      totalInput += utxo.amount;
      if (totalInput >= amount + fee) break;
    }
    
    if (totalInput < amount + fee) {
      throw new Error('Insufficient funds');
    }
    
    // Build transaction JSON
    const tx = {
      inputs: selectedUtxos.map(u => ({
        txHash: u.txHash,
        outputIndex: u.outputIndex,
        amount: u.amount,
        signature: '',
        publicKey: ''
      })),
      outputs: [
        { address: to, amount: amount, script: '' }
      ],
      fee: fee,
      senderAddress: from,
      receiverAddress: to,
      prevTxHash: selectedUtxos[0].txHash,
      referencedAmount: selectedUtxos[0].amount,
      type: 'NORMAL'
    };
    
    // Add change
    const change = totalInput - amount - fee;
    if (change > 0.00000001) {
      tx.outputs.push({ address: from, amount: change, script: '' });
    }
    
    return tx;
  }
  
  async signTransaction(tx, privateKey) {
    // Import your crypto library
    // Example with noble-secp256k1:
    // import * as secp from '@noble/secp256k1';
    
    for (const input of tx.inputs) {
      const message = input.txHash + input.outputIndex + input.amount;
      const messageHash = await this.sha256(message);
      
      // Sign with private key
      const signature = await secp.sign(messageHash, privateKey);
      const publicKey = secp.getPublicKey(privateKey);
      
      input.signature = this.toHex(signature);
      input.publicKey = this.toHex(publicKey);
    }
    
    return tx;
  }
  
  async sha256(message) {
    const encoder = new TextEncoder();
    const data = encoder.encode(message);
    const hashBuffer = await crypto.subtle.digest('SHA-256', data);
    return new Uint8Array(hashBuffer);
  }
  
  toHex(bytes) {
    return Array.from(bytes)
      .map(b => b.toString(16).padStart(2, '0'))
      .join('');
  }
}

// Usage
const wallet = new GXCWallet('http://localhost:8332');

// Send transaction
const txHash = await wallet.createAndSendTransaction(
  'GXC1abc...',  // from
  'GXC1def...',  // to
  10.5,          // amount
  privateKey     // your private key
);

console.log('Transaction sent:', txHash);
```

## TypeScript Example

```typescript
interface TransactionInput {
  txHash: string;
  outputIndex: number;
  amount: number | string;  // Flexible!
  signature: string;
  publicKey: string;
}

interface TransactionOutput {
  address: string;
  amount: number | string;  // Flexible!
  script: string;
}

interface Transaction {
  inputs: TransactionInput[];
  outputs: TransactionOutput[];
  fee: number | string;  // Flexible!
  senderAddress: string;
  receiverAddress: string;
  prevTxHash: string;
  referencedAmount: number | string;  // Flexible!
  type: 'NORMAL' | 'STAKE' | 'UNSTAKE';
}

class GXCClient {
  constructor(private nodeUrl: string) {}
  
  async sendTransaction(tx: Transaction): Promise<string> {
    const response = await fetch(this.nodeUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        jsonrpc: '2.0',
        method: 'sendrawtransaction',
        params: [tx],  // Send JSON directly
        id: 1
      })
    });
    
    const data = await response.json();
    if (data.error) throw new Error(data.error.message);
    return data.result;
  }
}
```

## Staking with JSON

```javascript
// Stake 100 GXC for 30 days
async function stakeGXC(address, stakeAmount, stakingDays, privateKey) {
  // 1. Register as validator (if not already registered)
  try {
    await rpcCall('registerexternalvalidator', [
      address,
      stakeAmount,
      stakingDays,
      signature,  // Sign "register_validator:address:amount:days"
      message,
      publicKey
    ]);
  } catch (e) {
    // Already registered, continue
  }
  
  // 2. Get UTXOs
  const utxos = await rpcCall('listunspent', [address]);
  
  // 3. Create STAKE transaction
  const fee = 0.001;
  const totalRequired = stakeAmount + fee;
  
  let totalInput = 0;
  const selectedUtxos = [];
  for (const utxo of utxos) {
    selectedUtxos.push(utxo);
    totalInput += utxo.amount;
    if (totalInput >= totalRequired) break;
  }
  
  const change = totalInput - stakeAmount - fee;
  
  const stakeTx = {
    inputs: selectedUtxos.map(u => ({
      txHash: u.txHash,
      outputIndex: u.outputIndex,
      amount: u.amount,
      signature: '',
      publicKey: ''
    })),
    outputs: change > 0 ? [{
      address: address,
      amount: change,
      script: ''
    }] : [],
    fee: fee,
    senderAddress: address,
    receiverAddress: address,
    prevTxHash: selectedUtxos[0].txHash,
    referencedAmount: selectedUtxos[0].amount,
    type: 'STAKE'  // Important!
  };
  
  // 4. Sign transaction
  const signedStakeTx = await signTransaction(stakeTx, privateKey);
  
  // 5. Send as JSON
  const txHash = await rpcCall('sendrawtransaction', [signedStakeTx]);
  
  return txHash;
}
```

## All RPC Methods Support Flexible JSON

### Get Balance
```javascript
// All these work:
await rpc('getbalance', ['GXC1abc...']);
await rpc('getbalance', ["GXC1abc..."]);
```

### Send Transaction
```javascript
// Method 1: JSON object
await rpc('sendrawtransaction', [{ /* transaction object */ }]);

// Method 2: Hex string
await rpc('sendrawtransaction', ['0x48656c6c6f...']);
```

### Register Validator
```javascript
await rpc('registerexternalvalidator', [
  'GXC1abc...',     // address
  100.0,            // stake amount (number)
  30,               // staking days
  'signature...',   // signature
  'message...',     // message
  'pubkey...'       // public key
]);

// Or with string amounts:
await rpc('registerexternalvalidator', [
  'GXC1abc...',
  '100.0',          // stake amount (string)
  30,
  'signature...',
  'message...',
  'pubkey...'
]);
```

## Error Handling

```javascript
try {
  const txHash = await rpc('sendrawtransaction', [tx]);
  console.log('Success:', txHash);
} catch (error) {
  // Error messages are descriptive
  console.error('Transaction failed:', error.message);
  
  // Common errors:
  // - "Insufficient funds"
  // - "Transaction verification failed"
  // - "Invalid signature"
  // - "UTXO already spent"
  // - "Validator not registered"
}
```

## Security Engine Validation

**All transactions are validated** regardless of format (JSON or hex):

1. ✅ **Signature Verification**
   - Signature matches public key
   - Public key derives to address

2. ✅ **UTXO Validation**
   - UTXOs exist and are unspent
   - Address owns the UTXOs
   - Amounts match exactly

3. ✅ **Balance Check**
   - Sum(inputs) >= Sum(outputs) + fee
   - No coin creation allowed

4. ✅ **Double-Spend Protection**
   - UTXOs checked against UTXO set
   - Mempool checked for pending spends

5. ✅ **Traceability Formula**
   - Ti.Inputs[0].txHash == Ti.PrevTxHash
   - Ti.Inputs[0].amount == Ti.ReferencedAmount

6. ✅ **Type-Specific Validation**
   - STAKE: Validator must be registered
   - UNSTAKE: Sufficient staked amount
   - NORMAL: Standard validation

## Best Practices

### 1. Always Sign Client-Side
```javascript
// ✅ GOOD: Sign on client
const signedTx = await signTransaction(tx, privateKey);
await rpc('sendrawtransaction', [signedTx]);

// ❌ BAD: Never send private key to node
await rpc('sendtoaddress', [to, amount, privateKey]);  // DON'T DO THIS!
```

### 2. Handle Both Number and String Amounts
```javascript
// Your app might send amounts as strings (React Native often does)
const tx = {
  outputs: [{
    address: to,
    amount: "10.5"  // String is fine!
  }]
};
```

### 3. Verify Before Sending
```javascript
// Check balance first
const balance = await rpc('getbalance', [fromAddress]);
if (balance < amount + fee) {
  throw new Error('Insufficient funds');
}

// Then send transaction
const txHash = await rpc('sendrawtransaction', [signedTx]);
```

### 4. Use HTTPS in Production
```javascript
// Development
const nodeUrl = 'http://localhost:8332';

// Production
const nodeUrl = 'https://node.gxc.network:8332';
```

## Mobile App Considerations

### React Native
- ✅ Amounts can be numbers or strings
- ✅ JSON transactions work perfectly
- ✅ No hex encoding needed
- ✅ Use `react-native-crypto` for signing

### Flutter
```dart
// Flutter example
Future<String> sendTransaction(Map<String, dynamic> tx) async {
  final response = await http.post(
    Uri.parse('http://node:8332'),
    headers: {'Content-Type': 'application/json'},
    body: jsonEncode({
      'jsonrpc': '2.0',
      'method': 'sendrawtransaction',
      'params': [tx],  // Send JSON map directly
      'id': 1
    })
  );
  
  final data = jsonDecode(response.body);
  return data['result'];
}
```

### Swift (iOS)
```swift
// Swift example
func sendTransaction(tx: [String: Any]) async throws -> String {
    let url = URL(string: "http://node:8332")!
    var request = URLRequest(url: url)
    request.httpMethod = "POST"
    request.setValue("application/json", forHTTPHeaderField: "Content-Type")
    
    let rpcRequest: [String: Any] = [
        "jsonrpc": "2.0",
        "method": "sendrawtransaction",
        "params": [tx],  // Send dictionary directly
        "id": 1
    ]
    
    request.httpBody = try JSONSerialization.data(withJSONObject: rpcRequest)
    
    let (data, _) = try await URLSession.shared.data(for: request)
    let response = try JSONDecoder().decode(RPCResponse.self, from: data)
    
    return response.result
}
```

## Summary

**The GXC blockchain accepts flexible JSON formats** to support all types of applications:

✅ **React Native** - Send amounts as strings or numbers  
✅ **Web Apps** - Use JSON transactions directly  
✅ **Mobile Apps** - No hex encoding needed  
✅ **Any JSON Client** - Maximum compatibility  

**Security is maintained** regardless of format:
- All transactions validated by security engine
- Signatures verified cryptographically
- UTXOs checked for existence and ownership
- Balance requirements enforced
- Traceability formula validated

**Your private keys stay safe:**
- Sign transactions client-side
- Never send private keys to node
- True decentralization maintained

Use `sendrawtransaction` with JSON objects for the easiest integration!
