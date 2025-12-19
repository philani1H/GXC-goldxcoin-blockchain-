# GXC Wallet Integration Examples

Complete working examples for integrating with the GXC blockchain.

## Quick Start

### 1. Start the GXC Node

```bash
cd build
./gxc-node
```

### 2. Run Examples

#### JavaScript/Node.js
```bash
cd examples
npm install node-fetch
node wallet_integration.js
```

#### Python
```bash
cd examples
python3 wallet_integration.py
```

---

## Available Examples

### Example 1: Send Transaction
Send GXC from one address to another.

```javascript
const txid = await rpc('sendtoaddress', [
  'tGXC...',  // recipient
  10.0        // amount
]);
```

### Example 2: Check Balance
Get spendable and staked balances.

```javascript
const balance = await rpc('getbalance', [address]);
const stakingInfo = await rpc('getStakingInfo', [address]);
```

### Example 3: Register as Validator
Become a validator by staking GXC.

```javascript
const result = await rpc('registerValidator', [
  address,
  100.0,  // stake amount
  30      // staking days
]);
```

### Example 4: Add More Stake
Increase your validator stake.

```javascript
const result = await rpc('addStake', [
  address,
  50.0  // additional amount
]);
```

### Example 5: Unstake
Withdraw staked funds.

```javascript
const result = await rpc('unstake', [
  address,
  50.0  // amount to unstake
]);
```

### Example 6: List All Validators
Get information about all validators.

```javascript
const validators = await rpc('getValidators', []);
```

### Example 7: Verify Traceability
Check if a transaction follows the traceability formula.

```javascript
const tx = await rpc('gettransaction', [txid]);
const valid = (
  tx.inputs[0].txHash === tx.prevTxHash &&
  tx.inputs[0].amount === tx.referencedAmount
);
```

### Example 8: Trace Transaction Chain
Follow the complete history of a transaction.

```javascript
let currentTx = txid;
while (currentTx && currentTx !== '0') {
  const tx = await rpc('gettransaction', [currentTx]);
  console.log(tx);
  currentTx = tx.prevTxHash;
}
```

### Example 9: Estimate Fee
Get recommended transaction fee.

```javascript
const feeInfo = await rpc('estimateFee', []);
console.log(`Recommended: ${feeInfo.recommended_fee} GXC`);
```

### Example 10: Monitor Validator
Track validator performance and rewards.

```javascript
const info = await rpc('getValidatorInfo', [address]);
console.log(`Blocks: ${info.blocks_produced}`);
console.log(`Rewards: ${info.total_rewards} GXC`);
console.log(`APY: ${info.apy}%`);
```

---

## API Methods

### Transaction Methods
- `sendtoaddress(address, amount)` - Send GXC
- `sendrawtransaction(hex)` - Broadcast signed transaction
- `gettransaction(txid)` - Get transaction details
- `estimatefee()` - Get recommended fee

### Staking Methods
- `registerValidator(address, amount, days)` - Register as validator
- `addStake(address, amount)` - Add more stake
- `unstake(address, amount)` - Unstake funds
- `getValidatorInfo(address)` - Get validator info
- `getValidators()` - List all validators

### Balance Methods
- `getbalance(address)` - Get spendable balance
- `listunspent(address)` - List UTXOs
- `getStakingInfo(address)` - Get staking details

### Blockchain Methods
- `getblockchaininfo()` - Get chain info
- `getblock(hash)` - Get block details
- `getblockcount()` - Get current height

---

## Configuration

### Network Endpoints

| Network | URL | Port | Prefix |
|---------|-----|------|--------|
| Mainnet | `http://node.gxc.io` | 8332 | `GXC` |
| Testnet | `http://testnet.gxc.io` | 18332 | `tGXC` |
| Local | `http://localhost` | 8545 | `tGXC` |

### Update Configuration

Edit the configuration at the top of the example files:

```javascript
const NODE_URL = 'http://localhost:8545';
const NETWORK = 'testnet';
```

---

## Traceability Formula

Every transaction must satisfy:
```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

This ensures every coin can be traced back to its origin.

---

## Error Handling

```javascript
try {
  const result = await rpc('sendtoaddress', [address, amount]);
  console.log('Success:', result);
} catch (error) {
  console.error('Error:', error.message);
  
  // Common errors:
  // - "Insufficient funds"
  // - "Invalid address"
  // - "Connection refused"
  // - "Transaction validation failed"
}
```

---

## Best Practices

### 1. Always Use Client-Side Signing
```javascript
// ✅ GOOD - You control your keys
const tx = wallet.createAndSignTransaction(...);
await rpc('sendrawtransaction', [tx]);

// ❌ BAD - Server has your keys
await rpc('sendtoaddress', [...]);
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
  console.log(`Amount: ${info.stake_amount} GXC`);
} else if (info.is_active) {
  console.log('Validator active!');
}
```

### 4. Check Balance Before Sending
```javascript
const balance = await rpc('getbalance', [address]);
const fee = await rpc('estimateFee', []);

if (balance < amount + fee.recommended_fee) {
  throw new Error('Insufficient funds');
}
```

---

## Testing

### Run All Examples
```bash
# JavaScript
node wallet_integration.js

# Python
python3 wallet_integration.py
```

### Run Specific Example
```javascript
// Uncomment the example you want to run
await example1_sendTransaction();
```

---

## Support

- **Documentation**: `/docs`
- **API Reference**: `WALLET_DEV_GUIDE_2025.md`
- **Traceability**: `TRACEABILITY_EXPLAINED.md`
- **Issues**: GitHub Issues

---

## License

MIT License - See LICENSE file for details

---

**Happy Building!** 🚀
