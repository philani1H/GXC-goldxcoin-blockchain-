# GXC Wallet Developer Guide 2025

## Complete Integration Guide for Wallet Developers

**Version**: 2.0  
**Updated**: December 2025  
**Status**: Production Ready ✅

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Sending Transactions](#sending-transactions)
3. [Staking & Validators](#staking--validators)
4. [Balance & UTXOs](#balance--utxos)
5. [Traceability](#traceability)
6. [Code Examples](#code-examples)

---

## Quick Start

### Network Endpoints

| Network | RPC URL | Port | Prefix |
|---------|---------|------|--------|
| Mainnet | `http://node.gxc.io` | 8332 | `GXC` |
| Testnet | `http://testnet.gxc.io` | 18332 | `tGXC` |

### Basic RPC Call

```javascript
const response = await fetch('http://localhost:8545', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'getblockchaininfo',
    params: [],
    id: 1
  })
});
const data = await response.json();
```

---

## Sending Transactions

### Method 1: Client-Side Signing (Recommended)

```javascript
// 1. Create wallet
const wallet = new GXCWallet();
const address = wallet.getAddress();

// 2. Get UTXOs
const utxos = await rpc('listunspent', [address]);

// 3. Create transaction
const tx = wallet.createTransaction({
  to: 'tGXC...',
  amount: 10.0,
  utxos: utxos,
  fee: 0.001
});

// 4. Sign transaction
const signedTx = wallet.signTransaction(tx);

// 5. Broadcast
const txid = await rpc('sendrawtransaction', [signedTx]);
```

### Method 2: Server-Side (Simple)

```javascript
const txid = await rpc('sendtoaddress', [
  'tGXC...',  // recipient
  10.0        // amount
]);
```

---

## Staking & Validators

### Register as Validator

```javascript
const result = await rpc('registerValidator', [
  address,      // your address
  100.0,        // stake amount (min 100 GXC)
  30            // staking days (14-365)
]);

console.log(result);
// {
//   success: true,
//   stake_amount: 100.0,
//   status: "pending",
//   stake_tx: "0x..."
// }
```

### Check Validator Status

```javascript
const info = await rpc('getValidatorInfo', [address]);

console.log(info);
// {
//   address: "tGXC...",
//   stake_amount: 100.0,
//   status: "active",
//   is_pending: false,
//   is_active: true,
//   blocks_produced: 42,
//   total_rewards: 5.2
// }
```

### Add More Stake

```javascript
const result = await rpc('addStake', [
  address,
  50.0  // additional amount
]);
```

### Unstake

```javascript
const result = await rpc('unstake', [
  address,
  50.0  // amount to unstake
]);
```

---

## Balance & UTXOs

### Get Balance

```javascript
const balance = await rpc('getbalance', [address]);
console.log(`Balance: ${balance} GXC`);
```

### Get Staking Info

```javascript
const info = await rpc('getStakingInfo', [address]);

console.log(info);
// {
//   spendable_balance: 50.0,
//   staked_amount: 100.0,
//   total_balance: 150.0,
//   pending_rewards: 0.5
// }
```

### List UTXOs

```javascript
const utxos = await rpc('listunspent', [address]);

console.log(utxos);
// [
//   {
//     txid: "0x...",
//     vout: 0,
//     amount: 50.0,
//     confirmations: 10
//   }
// ]
```

---

## Traceability

### Verify Transaction Chain

```javascript
const tx = await rpc('gettransaction', [txid]);

// Check traceability formula
const valid = (
  tx.inputs[0].txHash === tx.prevTxHash &&
  tx.inputs[0].amount === tx.referencedAmount
);

console.log(`Traceability: ${valid ? 'VALID' : 'INVALID'}`);
```

### Trace Transaction History

```javascript
async function traceTransaction(txid) {
  const chain = [];
  let currentTx = txid;
  
  while (currentTx && currentTx !== '0') {
    const tx = await rpc('gettransaction', [currentTx]);
    chain.push(tx);
    currentTx = tx.prevTxHash;
  }
  
  return chain;
}

const history = await traceTransaction(txid);
console.log(`Chain length: ${history.length}`);
```

---

## Code Examples

### Complete Wallet Class

```javascript
class GXCWallet {
  constructor() {
    this.privateKey = this.generatePrivateKey();
    this.publicKey = this.derivePublicKey(this.privateKey);
    this.address = this.deriveAddress(this.publicKey);
  }
  
  async sendTransaction(to, amount) {
    // Get UTXOs
    const utxos = await this.getUTXOs();
    
    // Create transaction
    const tx = this.createTransaction({
      to: to,
      amount: amount,
      utxos: utxos,
      fee: 0.001
    });
    
    // Sign
    const signed = this.signTransaction(tx);
    
    // Broadcast
    return await this.broadcast(signed);
  }
  
  async stake(amount, days) {
    return await rpc('registerValidator', [
      this.address,
      amount,
      days
    ]);
  }
  
  async getBalance() {
    return await rpc('getbalance', [this.address]);
  }
}
```

### Usage Example

```javascript
// Create wallet
const wallet = new GXCWallet();
console.log(`Address: ${wallet.address}`);

// Send transaction
const txid = await wallet.sendTransaction('tGXC...', 10.0);
console.log(`Sent! TX: ${txid}`);

// Stake
const result = await wallet.stake(100.0, 30);
console.log(`Staked: ${result.stake_amount} GXC`);

// Check balance
const balance = await wallet.getBalance();
console.log(`Balance: ${balance} GXC`);
```

---

## API Reference

### Transaction Methods

- `sendtoaddress(address, amount)` - Send GXC
- `sendrawtransaction(hex)` - Broadcast signed TX
- `gettransaction(txid)` - Get TX details
- `estimatefee()` - Get recommended fee

### Staking Methods

- `registerValidator(address, amount, days)` - Register
- `addStake(address, amount)` - Add stake
- `unstake(address, amount)` - Unstake
- `getValidatorInfo(address)` - Get info
- `getValidators()` - List all validators

### Balance Methods

- `getbalance(address)` - Get balance
- `listunspent(address)` - List UTXOs
- `getStakingInfo(address)` - Staking details

### Blockchain Methods

- `getblockchaininfo()` - Chain info
- `getblock(hash)` - Get block
- `getblockcount()` - Get height

---

## Best Practices

### 1. Always Use Client-Side Signing
```javascript
// ✅ GOOD
const tx = wallet.createAndSignTransaction(...);
await rpc('sendrawtransaction', [tx]);

// ❌ BAD
await rpc('sendtoaddress', [...]);  // Server has your keys!
```

### 2. Verify Traceability
```javascript
const tx = await rpc('gettransaction', [txid]);
if (!verifyTraceability(tx)) {
  throw new Error('Invalid transaction chain!');
}
```

### 3. Handle Pending Stakes
```javascript
const info = await rpc('getValidatorInfo', [address]);
if (info.is_pending) {
  console.log('Stake pending confirmation...');
} else if (info.is_active) {
  console.log('Validator active!');
}
```

---

## Support

- Documentation: `/docs`
- Examples: `/examples`
- Issues: GitHub Issues

**Happy Building!** 🚀
