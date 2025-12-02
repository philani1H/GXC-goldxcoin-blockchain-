# GXC Wallet Web Developer Quick Reference

Quick reference guide for building GXC wallet web applications.

---

## 🔗 API Endpoints

### RPC API (Primary)

**Base URL:** `http://localhost:18332` (testnet) or `http://localhost:8332` (mainnet)

**Request Format:**
```json
{
  "jsonrpc": "2.0",
  "method": "method_name",
  "params": [...],
  "id": 1
}
```

### Quick RPC Calls

#### Generate Address
```javascript
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'getnewaddress',
    params: [],
    id: 1
  })
})
```

#### Get Balance
```javascript
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'getbalance',
    params: ['tGXC...address'],
    id: 1
  })
})
```

#### Get Transactions
```javascript
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'listtransactions',
    params: ['tGXC...address', 100],
    id: 1
  })
})
```

#### Estimate Fee (Get Recommended Fee)
```javascript
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'estimateFee',
    params: [],
    id: 1
  })
})
// Returns: { fee: 0.001, recommended_fee: 0.001, pending_transactions: 5, ... }
```

#### Send Coins (with Fee)
```javascript
// Step 1: Get recommended fee from blockchain
const feeResponse = await fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'estimateFee',
    params: [],
    id: 1
  })
});
const feeData = await feeResponse.json();
const fee = feeData.result.recommended_fee; // e.g., 0.001

// Step 2: Check balance includes amount + fee
const balance = await getBalance('from_address');
if (balance < (100.0 + fee)) {
  throw new Error('Insufficient balance (need amount + fee)');
}

// Step 3: Send transaction (fee is automatically included)
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'sendtoaddress',
    params: ['from_address', 'to_address', 100.0],
    id: 1
  })
})
```

#### Register Validator (Stake)
```javascript
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'registervalidator',
    params: ['tGXC...address', 1000.0, 90],
    id: 1
  })
})
```

#### Get Validators
```javascript
fetch('http://localhost:18332', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'getvalidators',
    params: [],
    id: 1
  })
})
```

---

## 💻 JavaScript/TypeScript Helper

### Simple RPC Client

```javascript
class GXCRPC {
  constructor(url = 'http://localhost:18332') {
    this.url = url;
    this.id = 1;
  }

  async call(method, params = []) {
    const response = await fetch(this.url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        jsonrpc: '2.0',
        method,
        params,
        id: this.id++
      })
    });

    const data = await response.json();
    if (data.error) throw new Error(data.error.message);
    return data.result;
  }

  // Wallet methods
  async getNewAddress() {
    return this.call('getnewaddress');
  }

  async getBalance(address) {
    return this.call('getbalance', [address]);
  }

  async getTransactions(address, limit = 100) {
    return this.call('listtransactions', [address, limit]);
  }

  async estimateFee() {
    return this.call('estimateFee', []);
  }

  async sendCoins(from, to, amount) {
    // Get recommended fee from blockchain
    const feeInfo = await this.estimateFee();
    const fee = feeInfo.recommended_fee || feeInfo.fee || 0.001;
    
    // Check balance includes amount + fee
    const balance = await this.getBalance(from);
    if (balance < (amount + fee)) {
      throw new Error(`Insufficient balance. Need ${amount + fee} GXC (amount: ${amount} + fee: ${fee})`);
    }
    
    // Send transaction (fee handled automatically by blockchain)
    return this.call('sendtoaddress', [from, to, amount]);
  }

  // Staking methods
  async stake(address, amount, days) {
    return this.call('registervalidator', [address, amount, days]);
  }

  async getValidators() {
    return this.call('getvalidators');
  }
}

// Usage
const rpc = new GXCRPC('http://localhost:18332');
const address = await rpc.getNewAddress();
const balance = await rpc.getBalance(address);
```

---

## ⚡ React Hook Example

```jsx
import { useState, useEffect } from 'react';

function useGXCWallet(rpcUrl = 'http://localhost:18332') {
  const [address, setAddress] = useState(null);
  const [balance, setBalance] = useState(0);
  const [loading, setLoading] = useState(false);

  const rpc = new GXCRPC(rpcUrl);

  useEffect(() => {
    loadWallet();
  }, []);

  const loadWallet = async () => {
    setLoading(true);
    try {
      const addr = await rpc.getNewAddress();
      setAddress(addr);
      const bal = await rpc.getBalance(addr);
      setBalance(bal);
    } catch (error) {
      console.error('Error loading wallet:', error);
    } finally {
      setLoading(false);
    }
  };

  const refreshBalance = async () => {
    if (address) {
      const bal = await rpc.getBalance(address);
      setBalance(bal);
    }
  };

  const sendCoins = async (to, amount) => {
    // Get fee first
    const feeInfo = await rpc.estimateFee();
    const fee = feeInfo.recommended_fee || feeInfo.fee || 0.001;
    
    // Check balance
    const bal = await rpc.getBalance(address);
    if (bal < (amount + fee)) {
      throw new Error(`Insufficient balance. Need ${amount + fee} GXC`);
    }
    
    return rpc.sendCoins(address, to, amount);
  };

  return {
    address,
    balance,
    loading,
    refreshBalance,
    sendCoins,
    estimateFee: () => rpc.estimateFee(),
    stake: (amount, days) => rpc.stake(address, amount, days),
  };
}

// Usage in component
function Wallet() {
  const { address, balance, loading, refreshBalance } = useGXCWallet();

  if (loading) return <div>Loading...</div>;

  return (
    <div>
      <h1>GXC Wallet</h1>
      <p>Address: {address}</p>
      <p>Balance: {balance} GXC</p>
      <button onClick={refreshBalance}>Refresh</button>
    </div>
  );
}
```

---

## 🎨 Vue 3 Composition API Example

```vue
<template>
  <div>
    <h1>GXC Wallet</h1>
    <p>Address: {{ address }}</p>
    <p>Balance: {{ balance }} GXC</p>
    <button @click="refreshBalance">Refresh</button>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue';
import { GXCRPC } from './rpc';

const rpc = new GXCRPC('http://localhost:18332');
const address = ref(null);
const balance = ref(0);
const loading = ref(false);

onMounted(async () => {
  loading.value = true;
  try {
    address.value = await rpc.getNewAddress();
    balance.value = await rpc.getBalance(address.value);
  } catch (error) {
    console.error('Error:', error);
  } finally {
    loading.value = false;
  }
});

const refreshBalance = async () => {
  if (address.value) {
    balance.value = await rpc.getBalance(address.value);
  }
};
</script>
```

---

## 📋 All RPC Methods

### Wallet
- `getnewaddress` - Generate address
- `getbalance` - Get balance
- `listtransactions` - Get transactions
- `estimateFee` - Get recommended transaction fee (IMPORTANT: Use before sending)
- `sendtoaddress` - Send coins (fee handled automatically)
- `validateaddress` - Validate address
- `gettransaction` - Get transaction details

### Staking
- `registervalidator` - Register validator
- `stake` - Stake coins (alias)
- `addstake` - Add more stake
- `unstake` - Withdraw stake
- `getvalidators` - List validators
- `getvalidatorinfo` - Get validator info

### Blockchain
- `getblockchaininfo` - Chain info
- `getblockcount` - Block count
- `getblock` - Get block
- `getlatestblock` - Latest block

---

## 🔐 Security Checklist

- [ ] Use HTTPS in production
- [ ] Validate all inputs
- [ ] Never store private keys in code
- [ ] Use environment variables for URLs
- [ ] Implement rate limiting
- [ ] Handle errors gracefully
- [ ] Validate addresses before sending
- [ ] Check balance before transactions
- [ ] Get fee from blockchain using `estimateFee` before sending
- [ ] Verify balance >= amount + fee before transaction

---

## 🐛 Common Errors

| Error | Solution |
|-------|----------|
| Connection refused | Check node is running |
| Invalid address | Validate address format |
| Insufficient balance | Check balance before sending |
| RPC error -32601 | Method not found, check method name |
| Timeout | Increase timeout or check network |

---

## 📚 Full Documentation

See `WEB_WALLET_DEVELOPER_GUIDE.md` for complete documentation.

---

## 🚀 Quick Start

1. **Install dependencies:**
   ```bash
   npm install axios
   ```

2. **Create RPC client:**
   ```javascript
   const rpc = new GXCRPC('http://localhost:18332');
   ```

3. **Use in your app:**
   ```javascript
   const address = await rpc.getNewAddress();
   const balance = await rpc.getBalance(address);
   ```

That's it! You're ready to build your GXC wallet app! 🎉
