# GXC Blockchain API Architecture & Transaction Types

## Table of Contents
1. [API Architecture Overview](#api-architecture-overview)
2. [How Python APIs Connect to the Node](#how-python-apis-connect-to-the-node)
3. [Transaction Types](#transaction-types)
4. [What Makes Our Blockchain Special](#what-makes-our-blockchain-special)

---

## API Architecture Overview

### System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     User Applications                        │
│  (Web Browser, Mobile App, CLI Tools, Third-party Apps)    │
└────────────────────┬────────────────────────────────────────┘
                     │ HTTP/HTTPS
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                   Python API Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Wallet API   │  │ Explorer API │  │  Forum API   │     │
│  │ (Port 5000)  │  │ (Port 3000)  │  │ (Port 3001)  │     │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘     │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            │                                 │
└────────────────────────────┼─────────────────────────────────┘
                             │ RPC/REST
                             ▼
┌─────────────────────────────────────────────────────────────┐
│              GXC Blockchain Node (C++)                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ RPC Server   │  │ REST Server  │  │ WebSocket    │     │
│  │ (Port 18332) │  │ (Port 8080)  │  │ (Port 8081)  │     │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘     │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            │                                 │
│  ┌─────────────────────────┴──────────────────────────┐    │
│  │           Blockchain Core Engine                    │    │
│  │  • Block Validation  • Transaction Pool            │    │
│  │  • Consensus (PoW/PoS) • UTXO Management           │    │
│  │  • P2P Network       • Traceability Validation     │    │
│  └─────────────────────────┬──────────────────────────┘    │
└────────────────────────────┼─────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                    Data Storage Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Blockchain   │  │ UTXO Set     │  │ State DB     │     │
│  │ Database     │  │ (LevelDB)    │  │ (SQLite)     │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

---

## How Python APIs Connect to the Node

### 1. Connection Methods

The Python APIs use **three primary methods** to communicate with the blockchain node:

#### A. JSON-RPC (Primary Method)
```python
class BlockchainClient:
    def __init__(self, rpc_url='http://localhost:18332'):
        self.rpc_url = rpc_url
        self.timeout = 10
    
    def call_rpc(self, method, params=[]):
        """Make JSON-RPC call to blockchain node"""
        payload = {
            'jsonrpc': '2.0',
            'id': str(uuid.uuid4()),
            'method': method,
            'params': params
        }
        
        response = requests.post(
            self.rpc_url,
            json=payload,
            timeout=self.timeout,
            headers={'Content-Type': 'application/json'}
        )
        
        if response.status_code == 200:
            result = response.json()
            return result.get('result')
        else:
            raise Exception(f"RPC call failed: {response.status_code}")
```

**Available RPC Methods:**
- `getblockcount` - Get current blockchain height
- `getblock <hash>` - Get block by hash
- `getblockhash <height>` - Get block hash by height
- `getrawtransaction <txid>` - Get transaction details
- `sendrawtransaction <hex>` - Broadcast transaction
- `getbalance <address>` - Get address balance
- `getmempoolinfo` - Get mempool statistics
- `getpeerinfo` - Get connected peers
- `getmininginfo` - Get mining statistics
- `submitblock <hex>` - Submit mined block

#### B. REST API (Alternative Method)
```python
def get_address_balance(self, address):
    """Get balance via REST API"""
    url = f"{self.rest_url}/api/v1/address/{address}/balance"
    response = requests.get(url, timeout=self.timeout)
    
    if response.status_code == 200:
        data = response.json()
        return float(data.get('balance', 0.0))
    return 0.0

def get_address_transactions(self, address, limit=50):
    """Get transactions via REST API"""
    url = f"{self.rest_url}/api/v1/address/{address}/transactions"
    response = requests.get(url, params={'limit': limit})
    
    if response.status_code == 200:
        return response.json()
    return []
```

**REST Endpoints:**
- `GET /api/v1/block/<height>` - Get block by height
- `GET /api/v1/transaction/<hash>` - Get transaction
- `GET /api/v1/address/<address>/balance` - Get balance
- `GET /api/v1/address/<address>/transactions` - Get tx history
- `POST /api/v1/transaction/send` - Send transaction
- `GET /api/v1/stats` - Get network statistics

#### C. WebSocket (Real-time Updates)
```python
import socketio

sio = socketio.Client()

@sio.on('new_block')
def on_new_block(data):
    """Handle new block notification"""
    print(f"New block: {data['height']}")

@sio.on('new_transaction')
def on_new_transaction(data):
    """Handle new transaction notification"""
    print(f"New tx: {data['hash']}")

# Connect to node
sio.connect('http://localhost:8081')
```

**WebSocket Events:**
- `new_block` - New block mined
- `new_transaction` - New transaction in mempool
- `block_confirmed` - Block reached N confirmations
- `address_activity` - Activity on subscribed address

### 2. Connection Flow Example

```python
# Example: Send a transaction from Python API to Node

# Step 1: User creates transaction via Wallet API
POST /api/v1/wallets/{id}/send
{
    "to_address": "GXC1abc...",
    "amount": 10.0,
    "password": "user_password"
}

# Step 2: Wallet API processes request
wallet_api.py:
    1. Authenticate user
    2. Decrypt private key
    3. Build transaction
    4. Sign transaction
    5. Serialize to hex

# Step 3: Wallet API sends to Node via RPC
blockchain_client.call_rpc('sendrawtransaction', [tx_hex])

# Step 4: Node processes transaction
blockchain_node (C++):
    1. Deserialize transaction
    2. Validate signature
    3. Check UTXO availability
    4. Verify traceability formula
    5. Add to mempool
    6. Broadcast to peers

# Step 5: Node returns transaction hash
return {'txid': 'abc123...'}

# Step 6: Wallet API returns to user
return jsonify({
    'success': True,
    'transaction_hash': 'abc123...',
    'status': 'pending'
})
```

### 3. Data Flow Patterns

#### Pattern 1: Query Data (Read)
```
User → Wallet API → RPC Call → Node → Database → Response
```

#### Pattern 2: Submit Transaction (Write)
```
User → Wallet API → Sign TX → RPC Call → Node → Validate → Mempool → Broadcast
```

#### Pattern 3: Real-time Updates (Subscribe)
```
User → Explorer API → WebSocket → Node → Event Stream → Live Updates
```

---

## Transaction Types

Our blockchain supports **multiple transaction types**, each with unique characteristics:

### 1. Standard Transaction (UTXO-based)

**Structure:**
```cpp
struct Transaction {
    string txHash;              // Transaction hash
    time_t timestamp;           // Creation time
    string prevTxHash;          // Previous transaction hash (traceability)
    double referencedAmount;    // Amount from previous tx
    string senderAddress;       // Sender
    string receiverAddress;     // Receiver
    uint32_t nonce;            // Random nonce
    double fee;                // Transaction fee
    string memo;               // Optional message
    uint32_t lockTime;         // Time lock (0 = immediate)
    bool isCoinbase;           // Is this a coinbase tx?
    bool isGoldBacked;         // Is this gold-backed?
    string popReference;       // Proof of Price reference
    
    vector<TransactionInput> inputs;   // Input UTXOs
    vector<TransactionOutput> outputs; // Output UTXOs
};
```

**Example:**
```json
{
    "txHash": "a1b2c3d4...",
    "timestamp": 1704067200,
    "prevTxHash": "x9y8z7w6...",
    "referencedAmount": 10.0,
    "senderAddress": "GXC1sender...",
    "receiverAddress": "GXC1receiver...",
    "fee": 0.001,
    "inputs": [
        {
            "txHash": "x9y8z7w6...",
            "outputIndex": 0,
            "amount": 10.0,
            "signature": "sig123...",
            "publicKey": "pub456..."
        }
    ],
    "outputs": [
        {
            "address": "GXC1receiver...",
            "amount": 9.999,
            "script": "OP_DUP OP_HASH160 ..."
        }
    ]
}
```

### 2. Coinbase Transaction (Mining Reward)

**Special Properties:**
- No inputs (creates new coins)
- Single output to miner
- First transaction in every block
- Includes block reward + fees

**Structure:**
```cpp
Transaction createCoinbaseTransaction(
    const string& minerAddress,
    double blockReward
) {
    Transaction tx;
    tx.isCoinbase = true;
    tx.prevTxHash = "0";
    tx.inputs.clear(); // No inputs!
    
    TransactionOutput output;
    output.address = minerAddress;
    output.amount = blockReward;
    tx.outputs.push_back(output);
    
    return tx;
}
```

**Example:**
```json
{
    "txHash": "coinbase123...",
    "isCoinbase": true,
    "prevTxHash": "0000000000...",
    "inputs": [],
    "outputs": [
        {
            "address": "GXC1miner...",
            "amount": 12.5,
            "script": "OP_DUP OP_HASH160 ..."
        }
    ]
}
```

### 3. Gold-Backed Transaction (GXC-G)

**Special Properties:**
- Backed by physical gold
- Includes Proof of Price (PoP) reference
- Redeemable for gold
- Audited reserves

**Structure:**
```cpp
Transaction createGoldBackedTransaction(
    const vector<TransactionInput>& inputs,
    const vector<TransactionOutput>& outputs,
    const string& prevTxHash,
    const string& popReference  // Proof of Price
) {
    Transaction tx(inputs, outputs, prevTxHash);
    tx.isGoldBacked = true;
    tx.popReference = popReference;
    return tx;
}
```

**Example:**
```json
{
    "txHash": "gold789...",
    "isGoldBacked": true,
    "popReference": "POP-2024-01-15-GOLD-1850.50",
    "inputs": [...],
    "outputs": [
        {
            "address": "GXC1receiver...",
            "amount": 1.0,
            "goldGrams": 31.1035,
            "vaultId": "VAULT-001"
        }
    ]
}
```

### 4. Multi-Signature Transaction

**Special Properties:**
- Requires multiple signatures
- M-of-N signing scheme
- Enhanced security

**Example:**
```json
{
    "txHash": "multisig456...",
    "inputs": [
        {
            "txHash": "prev...",
            "outputIndex": 0,
            "amount": 100.0,
            "signatures": [
                "sig1_from_key1...",
                "sig2_from_key2...",
                "sig3_from_key3..."
            ],
            "publicKeys": ["pub1...", "pub2...", "pub3..."],
            "requiredSignatures": 2  // 2-of-3 multisig
        }
    ],
    "outputs": [...]
}
```

### 5. Smart Contract Transaction

**Special Properties:**
- Executes contract code
- State changes
- Gas fees

**Example:**
```json
{
    "txHash": "contract789...",
    "type": "contract_call",
    "contractAddress": "GXC1contract...",
    "function": "transfer",
    "params": ["GXC1to...", 100],
    "gasLimit": 100000,
    "gasPrice": 0.00001,
    "inputs": [...],
    "outputs": [...]
}
```

### 6. Staking Transaction

**Special Properties:**
- Locks coins for staking
- Earns rewards
- Time-locked

**Example:**
```json
{
    "txHash": "stake123...",
    "type": "stake",
    "stakeAmount": 1000.0,
    "stakingPeriod": 30,  // days
    "validatorAddress": "GXC1validator...",
    "lockTime": 1706659200,  // Unix timestamp
    "inputs": [...],
    "outputs": [
        {
            "address": "GXC1staker...",
            "amount": 1000.0,
            "locked": true,
            "unlockTime": 1706659200
        }
    ]
}
```

---

## What Makes Our Blockchain Special

### 1. 🔗 Built-in Traceability Formula

**The Core Innovation:**
```cpp
bool Transaction::verifyTraceabilityFormula() const {
    // Every transaction MUST reference its previous transaction
    if (inputs[0].txHash != prevTxHash) {
        return false;  // Broken chain!
    }
    
    // Amount must match
    if (inputs[0].amount != referencedAmount) {
        return false;  // Amount mismatch!
    }
    
    return true;  // Traceable!
}
```

**Why This Matters:**
- ✅ **Complete Transaction History**: Every coin can be traced back to its origin
- ✅ **Fraud Prevention**: Impossible to create fake transaction history
- ✅ **Regulatory Compliance**: Full audit trail for compliance
- ✅ **Transparency**: Anyone can verify the entire chain
- ✅ **Double-Spend Prevention**: Enhanced protection

**Example Trace:**
```
Genesis Block (Coinbase)
    ↓
TX1: Miner receives 12.5 GXC
    ↓ (prevTxHash = TX1)
TX2: Miner sends 10 GXC to Alice
    ↓ (prevTxHash = TX2)
TX3: Alice sends 5 GXC to Bob
    ↓ (prevTxHash = TX3)
TX4: Bob sends 2 GXC to Charlie
    ↓ (prevTxHash = TX4)
TX5: Charlie sends 1 GXC to Dave

Every transaction references the previous one!
```

### 2. 🏆 Gold-Backed Tokens (GXC-G)

**Unique Features:**
- Each token backed by physical gold
- Proof of Price (PoP) system
- Redeemable for actual gold
- Audited reserves
- Price stability

**How It Works:**
```
1. Gold deposited in vault → Audit → Certificate
2. Certificate hash stored on blockchain
3. GXC-G tokens minted (1 token = X grams gold)
4. Tokens tradeable on blockchain
5. Redeemable for physical gold
```

### 3. 🔄 Hybrid Consensus (PoW + PoS)

**Best of Both Worlds:**
- **PoW**: Security through mining
- **PoS**: Energy efficiency through staking
- **Hybrid**: Combines both for optimal balance

**Block Production:**
```
Block N   (PoW) → Mined by GPU/ASIC
Block N+1 (PoS) → Validated by staker
Block N+2 (PoW) → Mined by GPU/ASIC
Block N+3 (PoS) → Validated by staker
...
```

### 4. 🌉 Cross-Chain Bridge

**Connect to Other Blockchains:**
- Bridge to Ethereum
- Bridge to Bitcoin
- Bridge to Binance Smart Chain
- Atomic swaps
- Multi-signature validators

### 5. 🏛️ On-Chain Governance

**Community-Driven:**
- Proposal system
- Voting mechanism
- Parameter changes
- Upgrade decisions
- Treasury management

### 6. 📊 Multiple Mining Algorithms

**Inclusive Mining:**
- **SHA256**: ASIC miners (Bitcoin-compatible)
- **Ethash**: GPU miners (Ethereum-compatible)
- **GXHash**: Custom algorithm (CPU-friendly)

### 7. 🔐 Advanced Security Features

**Multi-Layer Security:**
- UTXO model (like Bitcoin)
- Traceability validation
- Multi-signature support
- Time-locked transactions
- Replay protection
- Checkpointing

### 8. ⚡ Fast Block Times

**Quick Confirmations:**
- Block time: 2 seconds (testnet: 60s)
- Fast finality
- High throughput
- Low latency

### 9. 💰 Low Transaction Fees

**Affordable Transactions:**
- Dynamic fee calculation
- Fee burning mechanism
- Deflationary pressure
- Sustainable economics

### 10. 🛠️ Developer-Friendly

**Easy Integration:**
- RESTful API
- JSON-RPC interface
- WebSocket support
- Python SDK
- JavaScript SDK
- Comprehensive documentation

---

## Comparison with Other Blockchains

| Feature | GXC | Bitcoin | Ethereum | Binance Smart Chain |
|---------|-----|---------|----------|---------------------|
| **Traceability Formula** | ✅ Built-in | ❌ No | ❌ No | ❌ No |
| **Gold-Backed Tokens** | ✅ Yes | ❌ No | ⚠️ Via contracts | ⚠️ Via contracts |
| **Consensus** | ✅ Hybrid PoW/PoS | PoW only | PoS only | PoS only |
| **Block Time** | 2 seconds | 10 minutes | 12 seconds | 3 seconds |
| **Transaction Fee** | Very Low | High | Variable | Low |
| **Smart Contracts** | ✅ Yes | ❌ Limited | ✅ Yes | ✅ Yes |
| **Cross-Chain Bridge** | ✅ Built-in | ❌ No | ⚠️ Via bridges | ⚠️ Via bridges |
| **Governance** | ✅ On-chain | ❌ Off-chain | ⚠️ Partial | ⚠️ Partial |
| **Mining Algorithms** | 3 (SHA256, Ethash, GXHash) | 1 (SHA256) | N/A | N/A |

---

## API Usage Examples

### Example 1: Get Balance
```python
from api.wallet_service import BlockchainClient

client = BlockchainClient()
balance = client.get_address_balance("GXC1abc...")
print(f"Balance: {balance} GXC")
```

### Example 2: Send Transaction
```python
# Via RPC
tx_hex = "0100000001..."  # Signed transaction
result = client.call_rpc('sendrawtransaction', [tx_hex])
print(f"Transaction ID: {result}")
```

### Example 3: Get Block
```python
# Get latest block
block = client.call_rpc('getblock', ['latest'])
print(f"Block height: {block['height']}")
print(f"Transactions: {len(block['transactions'])}")
```

### Example 4: Subscribe to Events
```python
import socketio

sio = socketio.Client()

@sio.on('new_block')
def on_new_block(data):
    print(f"New block mined: {data['height']}")

sio.connect('http://localhost:8081')
sio.wait()
```

---

## Conclusion

The GXC blockchain combines:
- ✅ **Traceability**: Every transaction is fully traceable
- ✅ **Stability**: Gold-backed tokens for price stability
- ✅ **Security**: Hybrid consensus + advanced features
- ✅ **Speed**: Fast block times and confirmations
- ✅ **Flexibility**: Multiple transaction types
- ✅ **Interoperability**: Cross-chain bridges
- ✅ **Developer-Friendly**: Comprehensive APIs

This makes GXC ideal for:
- Financial applications
- Supply chain tracking
- Asset tokenization
- DeFi platforms
- Enterprise solutions
- Payment systems

---

**For more information:**
- API Documentation: http://localhost:5000/api/docs
- Explorer: http://localhost:3000
- Forum: http://localhost:3001
