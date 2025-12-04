# GXC Wallet Developer Integration Guide

## Complete API Reference for Building Wallets, Exchanges, and dApps

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                              │
│   ██████╗ ██╗  ██╗ ██████╗    ██╗    ██╗ █████╗ ██╗     ██╗     ███████╗   │
│  ██╔════╝ ╚██╗██╔╝██╔════╝    ██║    ██║██╔══██╗██║     ██║     ██╔════╝   │
│  ██║  ███╗ ╚███╔╝ ██║         ██║ █╗ ██║███████║██║     ██║     █████╗     │
│  ██║   ██║ ██╔██╗ ██║         ██║███╗██║██╔══██║██║     ██║     ██╔══╝     │
│  ╚██████╔╝██╔╝ ██╗╚██████╗    ╚███╔███╔╝██║  ██║███████╗███████╗███████╗   │
│   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝     ╚══╝╚══╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝   │
│                                                                              │
│                 DEVELOPER INTEGRATION GUIDE v2.0                            │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Node Connection](#node-connection)
3. [Balance & UTXOs](#balance--utxos)
4. [Sending Transactions](#sending-transactions)
5. [Transaction History](#transaction-history)
6. [Fee Estimation](#fee-estimation)
7. [Staking & Validators](#staking--validators)
8. [Mining Integration](#mining-integration)
9. [Block Explorer API](#block-explorer-api)
10. [Traceability Features](#traceability-features)
11. [WebSocket Subscriptions](#websocket-subscriptions)
12. [Security Best Practices](#security-best-practices)
13. [Code Examples](#code-examples)

---

## Quick Start

### Network Endpoints

| Network | RPC URL | Port | Address Prefix |
|---------|---------|------|----------------|
| **Mainnet** | `http://node.gxc.io` | 8332 | `GXC` |
| **Testnet** | `http://testnet.gxc.io` | 18332 | `tGXC` |
| **Local** | `http://localhost` | 8332 | `tGXC` (testnet) |

### JSON-RPC Format

All requests use JSON-RPC 2.0 format:

```json
{
  "jsonrpc": "2.0",
  "method": "METHOD_NAME",
  "params": [PARAMETERS],
  "id": 1
}
```

### Quick Test

```bash
# Health check
curl http://localhost:8332/

# Get blockchain info
curl -X POST http://localhost:8332/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

---

## Node Connection

### JavaScript/TypeScript

```typescript
// GXC RPC Client
class GXCClient {
  private url: string;
  private id: number = 0;

  constructor(url: string = 'http://localhost:8332') {
    this.url = url;
  }

  async call(method: string, params: any[] = []): Promise<any> {
    const response = await fetch(this.url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        jsonrpc: '2.0',
        method,
        params,
        id: ++this.id
      })
    });

    const data = await response.json();
    if (data.error) {
      throw new Error(`RPC Error ${data.error.code}: ${data.error.message}`);
    }
    return data.result;
  }
}

// Usage
const gxc = new GXCClient('http://localhost:8332');
```

### Python

```python
import requests
import json

class GXCClient:
    def __init__(self, url='http://localhost:8332'):
        self.url = url
        self.id = 0
    
    def call(self, method, params=None):
        if params is None:
            params = []
        
        self.id += 1
        payload = {
            'jsonrpc': '2.0',
            'method': method,
            'params': params,
            'id': self.id
        }
        
        response = requests.post(
            self.url,
            json=payload,
            headers={'Content-Type': 'application/json'}
        )
        
        data = response.json()
        if 'error' in data:
            raise Exception(f"RPC Error {data['error']['code']}: {data['error']['message']}")
        return data['result']

# Usage
gxc = GXCClient('http://localhost:8332')
```

### React Native / Mobile

```javascript
// React Native GXC Client with error handling
const GXC_NODE_URL = 'http://your-node-ip:8332';

const callRPC = async (method, params = []) => {
  try {
    const response = await fetch(GXC_NODE_URL, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
      },
      body: JSON.stringify({
        jsonrpc: '2.0',
        method,
        params,
        id: Date.now(),
      }),
    });

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const data = await response.json();
    
    if (data.error) {
      throw new Error(data.error.message);
    }
    
    return data.result;
  } catch (error) {
    console.error(`RPC Error (${method}):`, error);
    throw error;
  }
};
```

---

## Balance & UTXOs

### Get Balance

```javascript
// Get balance for an address
const balance = await gxc.call('getbalance', ['GXC1abc123...']);
console.log(`Balance: ${balance} GXC`);
```

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getbalance",
  "params": ["GXC1abc123def456..."],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": 150.50000000,
  "id": 1
}
```

### Alternative Method Names (All Work)

| Method | Description |
|--------|-------------|
| `getbalance` | Standard balance query |
| `gxc_getBalance` | Ethereum-style naming |
| `getaddressbalance` | Explicit address balance |
| `getaccountbalance` | Account-style query |

### Get UTXOs (Unspent Outputs)

For building raw transactions, you need the UTXO set:

```javascript
// Get all UTXOs for an address
const utxos = await gxc.call('listunspent', ['GXC1abc123...']);
```

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "listunspent",
  "params": ["GXC1abc123def456..."],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "txid": "abc123def456...",
      "tx_hash": "abc123def456...",
      "vout": 0,
      "output_index": 0,
      "address": "GXC1abc123...",
      "amount": 50.0,
      "value": 50.0,
      "scriptPubKey": "76a914...",
      "script": "76a914...",
      "confirmations": 100,
      "block_height": 1000,
      "spendable": true,
      "solvable": true
    }
  ],
  "id": 1
}
```

---

## Sending Transactions

### Method 1: Simple Send (Recommended)

The node handles UTXO selection and change automatically:

```javascript
// Send 10 GXC to an address
const txHash = await gxc.call('sendtoaddress', [
  'GXC1receiver...',  // Recipient address
  10.0                // Amount in GXC
]);

console.log(`Transaction sent: ${txHash}`);
```

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "sendtoaddress",
  "params": ["GXC1receiver123...", 10.0],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": "txhash123abc456def...",
  "id": 1
}
```

### Method 2: Raw Transaction (Advanced)

For full control over transaction construction:

```javascript
// 1. Get UTXOs
const utxos = await gxc.call('listunspent', [myAddress]);

// 2. Select UTXOs for the transaction
const selectedUtxos = selectUtxos(utxos, amount + fee);

// 3. Create raw transaction
const rawTx = await gxc.call('createrawtransaction', [
  // Inputs
  selectedUtxos.map(utxo => ({
    txid: utxo.txid,
    vout: utxo.vout
  })),
  // Outputs
  {
    [recipientAddress]: amount,
    [myAddress]: change  // Change back to sender
  }
]);

// 4. Sign transaction (client-side)
const signedTx = signTransaction(rawTx, privateKey);

// 5. Broadcast
const txHash = await gxc.call('sendrawtransaction', [signedTx]);
```

### Transaction Traceability

GXC enforces the traceability formula for all transactions:

```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

When building transactions manually, ensure you include:
- `prevTxHash`: Hash of the first input transaction
- `referencedAmount`: Amount of the first input

---

## Transaction History

### List Transactions for Address

```javascript
// Get last 10 transactions
const txs = await gxc.call('listtransactions', [
  'GXC1abc123...',  // Address
  10,               // Count (default: 10)
  0                 // Skip (default: 0)
]);
```

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "listtransactions",
  "params": ["GXC1abc123...", 10, 0],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "hash": "tx123...",
      "txid": "tx123...",
      "block_number": 1000,
      "confirmations": 50,
      "from": "GXC1sender...",
      "from_address": "GXC1sender...",
      "to": "GXC1abc123...",
      "to_address": "GXC1abc123...",
      "amount": 10.0,
      "value": 10.0,
      "fee": 0.001,
      "timestamp": 1699999999,
      "category": "receive",
      "is_coinbase": false,
      "prev_tx_hash": "prevtx123...",
      "referenced_amount": 50.0,
      "traceability_valid": true,
      "inputs": [...],
      "outputs": [...]
    }
  ],
  "id": 1
}
```

### Transaction Categories

| Category | Description |
|----------|-------------|
| `receive` | Incoming payment |
| `send` | Outgoing payment |
| `generate` | Mining reward (PoW) |
| `stake` | Staking transaction |
| `unstake` | Unstaking transaction |

### Get Single Transaction

```javascript
const tx = await gxc.call('gettransaction', ['txhash123...']);
```

**Alternative methods:**
- `gettransaction`
- `gxc_getTransaction`
- `gettx`
- `getrawtransaction`

---

## Fee Estimation

### Get Recommended Fee

```javascript
const feeInfo = await gxc.call('estimatefee', []);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "fee": 0.001,
    "recommended_fee": 0.001,
    "base_fee": 0.001,
    "max_fee": 0.01,
    "pending_transactions": 25,
    "dynamic_fees_enabled": true,
    "fee_tiers": {
      "low": 0.001,
      "medium": 0.0015,
      "high": 0.002,
      "very_high": 0.003
    },
    "current_tier": "low"
  },
  "id": 1
}
```

### Fee Tier Selection

```javascript
async function getOptimalFee(priority = 'medium') {
  const feeInfo = await gxc.call('estimatefee', []);
  return feeInfo.fee_tiers[priority];
}

// Usage
const fee = await getOptimalFee('high'); // For faster confirmation
```

---

## Staking & Validators

### Become a Validator (Stake)

```javascript
// Stake 1000 GXC for 90 days
const result = await gxc.call('registervalidator', [
  'GXC1myaddress...',  // Your address
  1000.0,              // Stake amount (min: 100 GXC)
  90                   // Staking days (7-365)
]);
```

**Alternative methods:**
- `registervalidator`
- `gxc_registerValidator`
- `stake`
- `gxc_stake`

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "GXC1myaddress...",
    "stake_amount": 1000.0,
    "staking_days": 90,
    "weighted_stake": 1315.07,
    "stake_tx": "stakeTxHash123...",
    "message": "Validator registered successfully"
  },
  "id": 1
}
```

### Unstake

```javascript
// Unstake all or partial amount
const result = await gxc.call('unstake', [
  'GXC1myaddress...',  // Your address
  500.0                // Amount to unstake (optional, 0 = all)
]);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "GXC1myaddress...",
    "amount": 500.0,
    "remaining_stake": 500.0,
    "txid": "unstakeTxHash123...",
    "message": "Unstake transaction broadcasted"
  },
  "id": 1
}
```

### Get Staking Info

```javascript
const stakingInfo = await gxc.call('getstakinginfo', ['GXC1myaddress...']);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "GXC1myaddress...",
    "spendable_balance": 500.0,
    "staked_balance": 1000.0,
    "total_balance": 1500.0,
    "total_earned_mining": 100.0,
    "total_earned_staking": 25.0,
    "total_earned": 125.0,
    "is_validator": true,
    "validator_total_rewards": 25.0,
    "validator_pending_rewards": 0.5,
    "blocks_produced": 10,
    "apy": 12.5,
    "traceability_formula": "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount"
  },
  "id": 1
}
```

### List All Validators

```javascript
const validators = await gxc.call('getvalidators', []);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "address": "GXC1validator1...",
      "stake_amount": 10000.0,
      "staking_days": 365,
      "weighted_stake": 15000.0,
      "is_active": true,
      "blocks_produced": 500,
      "missed_blocks": 2,
      "uptime": 99.6,
      "total_rewards": 625.0,
      "apy": 15.5,
      "is_slashed": false
    }
  ],
  "id": 1
}
```

### Staking Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| `MIN_STAKE` | 100 GXC | Minimum stake to become validator |
| `MIN_STAKING_DAYS` | 7 | Minimum staking period |
| `MAX_STAKING_DAYS` | 365 | Maximum staking period |
| Time Weight Formula | `1 + (days/365) × 0.5` | Longer staking = higher rewards |

---

## Mining Integration

### Get Block Template

```javascript
const template = await gxc.call('getblocktemplate', []);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "version": 1,
    "previousblockhash": "0000abc123...",
    "transactions": [],
    "coinbasevalue": 5000000000,
    "coinbase_value": 50.0,
    "block_reward": 50.0,
    "target": "00000000ffff0000...",
    "bits": "1d00ffff",
    "height": 12345,
    "difficulty": 1.0,
    "curtime": 1699999999,
    "consensus_type": "pow",
    "block_type": "pow"
  },
  "id": 1
}
```

### Submit Mined Block

```javascript
const result = await gxc.call('submitblock', [{
  height: 12345,
  hash: 'minedBlockHash123...',
  previousblockhash: 'prevHash...',
  nonce: 123456789,
  miner: 'GXC1miner...',
  timestamp: Date.now() / 1000,
  difficulty: 1.0,
  transactions: []
}]);
```

### PoS Block Creation (Validators Only)

```javascript
// Create PoS block template
const posBlock = await gxc.call('createposblock', ['GXC1validator...']);

// Submit PoS block
await gxc.call('submitposblock', [posBlock]);
```

---

## Block Explorer API

### Get Block by Hash or Height

```javascript
// By height
const block = await gxc.call('getblock', [12345, true]);

// By hash
const block = await gxc.call('getblock', ['blockhash123...', true]);

// Latest block
const block = await gxc.call('getlatestblock', []);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "hash": "blockhash123...",
    "height": 12345,
    "confirmations": 100,
    "version": 1,
    "merkleroot": "merkle123...",
    "time": 1699999999,
    "timestamp": 1699999999,
    "nonce": 123456789,
    "difficulty": 1.0,
    "previousblockhash": "prevhash...",
    "miner": "GXC1miner...",
    "miner_address": "GXC1miner...",
    "block_reward": 50.0,
    "size": 1234,
    "consensus_type": "pow",
    "transaction_count": 5,
    "transactions": [...]
  },
  "id": 1
}
```

### Get Blockchain Info

```javascript
const info = await gxc.call('getblockchaininfo', []);
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "main",
    "blocks": 12345,
    "height": 12345,
    "bestblockhash": "latesthash...",
    "difficulty": 1.5,
    "block_reward": 50.0,
    "mediantime": 1699999999,
    "verificationprogress": 1.0,
    "initialblockdownload": false
  },
  "id": 1
}
```

---

## Traceability Features

### Trace Transaction Lineage

```javascript
// Get the full lineage of a transaction
const lineage = await gxc.call('tracetransaction', ['txhash123...']);
// Returns: ['genesis_tx', 'tx1', 'tx2', ..., 'txhash123']
```

### Verify Transaction Lineage

```javascript
// Verify the transaction follows the traceability formula
const isValid = await gxc.call('verifytransactionlineage', ['txhash123...']);
// Returns: true/false
```

### Get Transaction Chain for Address

```javascript
// Get all transactions in the chain for an address
const chain = await gxc.call('gettransactionchain', [
  'GXC1address...',
  100  // Depth (optional, default: 100)
]);
```

---

## WebSocket Subscriptions

### Connect to WebSocket

```javascript
const ws = new WebSocket('ws://localhost:8081');

ws.onopen = () => {
  console.log('Connected to GXC WebSocket');
  
  // Subscribe to new blocks
  ws.send(JSON.stringify({
    type: 'subscribe',
    channel: 'blocks'
  }));
  
  // Subscribe to transactions for an address
  ws.send(JSON.stringify({
    type: 'subscribe',
    channel: 'address',
    address: 'GXC1myaddress...'
  }));
};

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  
  switch (data.type) {
    case 'new_block':
      console.log('New block:', data.hash, 'Height:', data.height);
      break;
    case 'new_transaction':
      console.log('New TX:', data.hash, 'Amount:', data.amount);
      break;
    case 'price_update':
      console.log('GXC Price:', data.price);
      break;
  }
};
```

---

## Security Best Practices

### 1. Never Expose Private Keys

```javascript
// BAD - Never do this
const response = await gxc.call('sendtoaddress', [to, amount, privateKey]);

// GOOD - Sign transactions client-side
const signedTx = signTransactionLocally(rawTx, privateKey);
const txHash = await gxc.call('sendrawtransaction', [signedTx]);
```

### 2. Validate Addresses

```javascript
const validation = await gxc.call('validateaddress', [address]);
if (!validation.isvalid) {
  throw new Error('Invalid address');
}
```

### 3. Use HTTPS in Production

```javascript
// Production
const gxc = new GXCClient('https://node.gxc.io:8332');

// Never use HTTP for mainnet with real funds
```

### 4. Implement Rate Limiting

```javascript
class RateLimitedClient extends GXCClient {
  private lastCall = 0;
  private minInterval = 100; // 100ms between calls

  async call(method, params = []) {
    const now = Date.now();
    const wait = this.lastCall + this.minInterval - now;
    
    if (wait > 0) {
      await new Promise(r => setTimeout(r, wait));
    }
    
    this.lastCall = Date.now();
    return super.call(method, params);
  }
}
```

### 5. Handle Errors Gracefully

```javascript
async function sendTransaction(to, amount) {
  try {
    const txHash = await gxc.call('sendtoaddress', [to, amount]);
    return { success: true, txHash };
  } catch (error) {
    if (error.message.includes('Insufficient funds')) {
      return { success: false, error: 'Not enough GXC' };
    }
    if (error.message.includes('Invalid address')) {
      return { success: false, error: 'Invalid recipient address' };
    }
    throw error;
  }
}
```

---

## Code Examples

### Complete Wallet Class (JavaScript)

```javascript
class GXCWallet {
  constructor(nodeUrl = 'http://localhost:8332') {
    this.client = new GXCClient(nodeUrl);
    this.address = null;
  }

  // Initialize wallet with address
  setAddress(address) {
    this.address = address;
  }

  // Get current balance
  async getBalance() {
    if (!this.address) throw new Error('Address not set');
    return await this.client.call('getbalance', [this.address]);
  }

  // Get staking info
  async getStakingInfo() {
    if (!this.address) throw new Error('Address not set');
    return await this.client.call('getstakinginfo', [this.address]);
  }

  // Send GXC
  async send(to, amount) {
    if (!this.address) throw new Error('Address not set');
    
    // Validate recipient
    const validation = await this.client.call('validateaddress', [to]);
    if (!validation.isvalid) {
      throw new Error('Invalid recipient address');
    }
    
    // Check balance
    const balance = await this.getBalance();
    const fee = await this.getRecommendedFee();
    
    if (balance < amount + fee) {
      throw new Error(`Insufficient funds. Need ${amount + fee} GXC, have ${balance} GXC`);
    }
    
    return await this.client.call('sendtoaddress', [to, amount]);
  }

  // Get recommended fee
  async getRecommendedFee() {
    const feeInfo = await this.client.call('estimatefee', []);
    return feeInfo.recommended_fee;
  }

  // Get transaction history
  async getTransactions(count = 10, skip = 0) {
    if (!this.address) throw new Error('Address not set');
    return await this.client.call('listtransactions', [this.address, count, skip]);
  }

  // Stake GXC
  async stake(amount, days = 30) {
    if (!this.address) throw new Error('Address not set');
    
    if (amount < 100) {
      throw new Error('Minimum stake is 100 GXC');
    }
    
    if (days < 7 || days > 365) {
      throw new Error('Staking days must be between 7 and 365');
    }
    
    return await this.client.call('registervalidator', [this.address, amount, days]);
  }

  // Unstake GXC
  async unstake(amount = 0) {
    if (!this.address) throw new Error('Address not set');
    return await this.client.call('unstake', [this.address, amount]);
  }

  // Get UTXOs
  async getUTXOs() {
    if (!this.address) throw new Error('Address not set');
    return await this.client.call('listunspent', [this.address]);
  }
}

// Usage Example
async function main() {
  const wallet = new GXCWallet('http://localhost:8332');
  wallet.setAddress('GXC1abc123...');

  // Check balance
  const balance = await wallet.getBalance();
  console.log(`Balance: ${balance} GXC`);

  // Get staking info
  const stakingInfo = await wallet.getStakingInfo();
  console.log(`Staked: ${stakingInfo.staked_balance} GXC`);
  console.log(`Earned: ${stakingInfo.total_earned} GXC`);

  // Send transaction
  const txHash = await wallet.send('GXC1receiver...', 10.0);
  console.log(`Sent! TX: ${txHash}`);

  // Stake
  const stakeResult = await wallet.stake(1000, 90);
  console.log(`Staked! TX: ${stakeResult.stake_tx}`);
}
```

### Python Wallet Example

```python
class GXCWallet:
    def __init__(self, node_url='http://localhost:8332'):
        self.client = GXCClient(node_url)
        self.address = None
    
    def set_address(self, address):
        self.address = address
    
    def get_balance(self):
        if not self.address:
            raise ValueError('Address not set')
        return self.client.call('getbalance', [self.address])
    
    def get_staking_info(self):
        if not self.address:
            raise ValueError('Address not set')
        return self.client.call('getstakinginfo', [self.address])
    
    def send(self, to, amount):
        if not self.address:
            raise ValueError('Address not set')
        
        # Validate
        validation = self.client.call('validateaddress', [to])
        if not validation['isvalid']:
            raise ValueError('Invalid recipient address')
        
        return self.client.call('sendtoaddress', [to, amount])
    
    def get_transactions(self, count=10, skip=0):
        if not self.address:
            raise ValueError('Address not set')
        return self.client.call('listtransactions', [self.address, count, skip])
    
    def stake(self, amount, days=30):
        if not self.address:
            raise ValueError('Address not set')
        return self.client.call('registervalidator', [self.address, amount, days])
    
    def unstake(self, amount=0):
        if not self.address:
            raise ValueError('Address not set')
        return self.client.call('unstake', [self.address, amount])


# Usage
if __name__ == '__main__':
    wallet = GXCWallet()
    wallet.set_address('tGXC1abc123...')
    
    # Check balance
    balance = wallet.get_balance()
    print(f'Balance: {balance} GXC')
    
    # Get transactions
    txs = wallet.get_transactions(5)
    for tx in txs:
        print(f"{tx['category']}: {tx['amount']} GXC - {tx['hash'][:16]}...")
```

---

## Complete API Reference

### Blockchain Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `getblockchaininfo` | - | Get blockchain state |
| `getbestblockhash` | - | Get latest block hash |
| `getblockcount` | - | Get current height |
| `getdifficulty` | - | Get current difficulty |
| `getblock` | hash/height, verbose | Get block data |
| `getblockhash` | height | Get block hash by height |
| `getlatestblock` | - | Get latest block |

### Transaction Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `gettransaction` | txhash | Get transaction details |
| `listtransactions` | address, count, skip | List address transactions |
| `sendtoaddress` | address, amount | Send GXC |
| `sendrawtransaction` | hex | Broadcast raw tx |
| `getrawtransaction` | txhash, verbose | Get raw transaction |

### Balance Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `getbalance` | address | Get address balance |
| `listunspent` | address | Get UTXOs |
| `estimatefee` | - | Get recommended fee |
| `validateaddress` | address | Validate address format |

### Staking Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `registervalidator` | address, amount, days | Become a validator |
| `stake` | address, amount, days | Alias for registervalidator |
| `unstake` | address, amount | Unstake GXC |
| `addstake` | address, amount | Add more stake |
| `getvalidators` | - | List all validators |
| `getvalidatorinfo` | address | Get validator details |
| `getstakinginfo` | address | Get staking summary |

### Mining Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `getblocktemplate` | - | Get mining template |
| `submitblock` | blockdata | Submit mined block |
| `getmininginfo` | - | Get mining stats |
| `createposblock` | validator | Create PoS block |
| `submitposblock` | blockdata | Submit PoS block |

### Traceability Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `tracetransaction` | txhash | Get tx lineage |
| `verifytransactionlineage` | txhash | Verify traceability |
| `gettransactionchain` | address, depth | Get address tx chain |

### Network Methods

| Method | Parameters | Description |
|--------|------------|-------------|
| `getpeerinfo` | - | Get connected peers |
| `getconnectioncount` | - | Get peer count |
| `addnode` | ip, command | Add/remove node |
| `getinfo` | - | Get node info |

---

## Support & Resources

| Resource | URL |
|----------|-----|
| **Documentation** | https://docs.gxc.io |
| **API Explorer** | https://api.gxc.io |
| **Block Explorer** | https://explorer.gxc.io |
| **GitHub** | https://github.com/gxc-blockchain |
| **Discord** | https://discord.gg/gxc |
| **Testnet Faucet** | https://faucet.gxc.io |

---

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                              │
│   Built with ❤️ for developers                                              │
│                                                                              │
│   GXC Blockchain - The Traceable, Secure, AI-Protected Blockchain          │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```
