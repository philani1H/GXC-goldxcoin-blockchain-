# Third-Party Mining Software Guide

## Overview

GXC blockchain supports standard Bitcoin-compatible RPC methods for mining, allowing you to use third-party mining software like cgminer, bfgminer, or custom miners.

---

## Supported RPC Methods

### 1. `getblocktemplate`

Get a block template for mining.

**Request:**
```bash
curl -X POST http://localhost:8332/api/getblocktemplate \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "getblocktemplate",
    "params": [{
      "minerAddress": "YOUR_GXC_ADDRESS"
    }]
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "version": 1,
    "previousblockhash": "a3b05adaef965806f910ac0910a1e1352f06e2de...",
    "height": 13,
    "difficulty": 1000.0,
    "target": "0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
    "miner": "GXC9fab7317231b966af85ac453e168c0932",
    "timestamp": 1704556800,
    "transactions": [
      {
        "hash": "coinbase_tx_hash",
        "type": "COINBASE",
        "amount": 50.0
      }
    ],
    "coinbasevalue": 50.0,
    "bits": "1d00ffff"
  }
}
```

### 2. `submitblock`

Submit a mined block.

**Request:**
```bash
curl -X POST http://localhost:8332/api/submitblock \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "submitblock",
    "params": [{
      "height": 13,
      "hash": "a3b05adaef965806f910ac0910a1e1352f06e2de...",
      "previousblockhash": "0e7da51e132244ed604f6d14aa98fe9ea1d01bad...",
      "nonce": 123456,
      "miner": "GXC9fab7317231b966af85ac453e168c0932",
      "timestamp": 1704556800,
      "difficulty": 1000.0
    }]
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "success": true,
    "message": "Block accepted",
    "hash": "a3b05adaef965806f910ac0910a1e1352f06e2de...",
    "height": 13
  }
}
```

---

## Using Third-Party Mining Software

### Option 1: cgminer (CPU/GPU)

**Note:** cgminer is primarily for Bitcoin SHA256 mining. For GXC, you'll need to configure it for the RPC interface.

**Configuration:**
```bash
cgminer \
  --url http://localhost:8332 \
  --user YOUR_GXC_ADDRESS \
  --pass x \
  --algo sha256d
```

**Limitations:**
- cgminer expects Bitcoin-style getwork protocol
- May need adapter/proxy for full compatibility

### Option 2: Custom Python Miner

Create a simple Python miner that uses the RPC API:

**File:** `gxc_miner.py`
```python
#!/usr/bin/env python3
import requests
import hashlib
import time
import json

# Configuration
NODE_URL = "http://localhost:8332"
MINER_ADDRESS = "GXC9fab7317231b966af85ac453e168c0932"
THREADS = 4

def get_block_template():
    """Get block template from node"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "getblocktemplate",
        "params": [{
            "minerAddress": MINER_ADDRESS
        }]
    }
    
    response = requests.post(f"{NODE_URL}/api/getblocktemplate", json=payload)
    return response.json()["result"]

def calculate_hash(block_data, nonce):
    """Calculate block hash"""
    data = f"{block_data['previousblockhash']}{block_data['height']}{block_data['timestamp']}{nonce}"
    return hashlib.sha256(hashlib.sha256(data.encode()).digest()).hexdigest()

def check_difficulty(hash_value, difficulty):
    """Check if hash meets difficulty target"""
    leading_zeros = len(hash_value) - len(hash_value.lstrip('0'))
    required_zeros = int(difficulty / 1000) + 4
    return leading_zeros >= required_zeros

def submit_block(block_data, nonce, block_hash):
    """Submit mined block"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "submitblock",
        "params": [{
            "height": block_data["height"],
            "hash": block_hash,
            "previousblockhash": block_data["previousblockhash"],
            "nonce": nonce,
            "miner": MINER_ADDRESS,
            "timestamp": block_data["timestamp"],
            "difficulty": block_data["difficulty"]
        }]
    }
    
    response = requests.post(f"{NODE_URL}/api/submitblock", json=payload)
    return response.json()

def mine():
    """Main mining loop"""
    print(f"Starting GXC miner...")
    print(f"Miner address: {MINER_ADDRESS}")
    print(f"Node URL: {NODE_URL}")
    print()
    
    nonce = 0
    hash_count = 0
    start_time = time.time()
    
    while True:
        try:
            # Get block template
            template = get_block_template()
            print(f"Mining block {template['height']} (difficulty: {template['difficulty']})")
            
            # Mine block
            while True:
                block_hash = calculate_hash(template, nonce)
                hash_count += 1
                
                # Print hash rate every 10000 hashes
                if hash_count % 10000 == 0:
                    elapsed = time.time() - start_time
                    hash_rate = hash_count / elapsed
                    print(f"Hash rate: {hash_rate:.2f} H/s | Nonce: {nonce}")
                
                # Check if hash meets difficulty
                if check_difficulty(block_hash, template['difficulty']):
                    print(f"✅ Found valid block! Hash: {block_hash[:16]}...")
                    
                    # Submit block
                    result = submit_block(template, nonce, block_hash)
                    
                    if result.get("result", {}).get("success"):
                        print(f"✅ Block accepted! Height: {template['height']}")
                        print(f"Reward: {template['coinbasevalue']} GXC")
                    else:
                        print(f"❌ Block rejected: {result}")
                    
                    # Get new template
                    break
                
                nonce += 1
                
                # Get new template every 60 seconds
                if nonce % 100000 == 0:
                    break
        
        except KeyboardInterrupt:
            print("\nStopping miner...")
            break
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(5)

if __name__ == "__main__":
    mine()
```

**Usage:**
```bash
# Install dependencies
pip3 install requests

# Run miner
python3 gxc_miner.py
```

### Option 3: Node.js Miner

**File:** `gxc-miner.js`
```javascript
const axios = require('axios');
const crypto = require('crypto');

// Configuration
const NODE_URL = 'http://localhost:8332';
const MINER_ADDRESS = 'GXC9fab7317231b966af85ac453e168c0932';

async function getBlockTemplate() {
    const response = await axios.post(`${NODE_URL}/api/getblocktemplate`, {
        jsonrpc: '2.0',
        id: 1,
        method: 'getblocktemplate',
        params: [{
            minerAddress: MINER_ADDRESS
        }]
    });
    return response.data.result;
}

function calculateHash(blockData, nonce) {
    const data = `${blockData.previousblockhash}${blockData.height}${blockData.timestamp}${nonce}`;
    return crypto.createHash('sha256')
        .update(crypto.createHash('sha256').update(data).digest())
        .digest('hex');
}

function checkDifficulty(hash, difficulty) {
    const leadingZeros = hash.match(/^0*/)[0].length;
    const requiredZeros = Math.floor(difficulty / 1000) + 4;
    return leadingZeros >= requiredZeros;
}

async function submitBlock(blockData, nonce, blockHash) {
    const response = await axios.post(`${NODE_URL}/api/submitblock`, {
        jsonrpc: '2.0',
        id: 1,
        method: 'submitblock',
        params: [{
            height: blockData.height,
            hash: blockHash,
            previousblockhash: blockData.previousblockhash,
            nonce: nonce,
            miner: MINER_ADDRESS,
            timestamp: blockData.timestamp,
            difficulty: blockData.difficulty
        }]
    });
    return response.data;
}

async function mine() {
    console.log('Starting GXC miner...');
    console.log(`Miner address: ${MINER_ADDRESS}`);
    console.log(`Node URL: ${NODE_URL}\n`);
    
    let nonce = 0;
    let hashCount = 0;
    const startTime = Date.now();
    
    while (true) {
        try {
            // Get block template
            const template = await getBlockTemplate();
            console.log(`Mining block ${template.height} (difficulty: ${template.difficulty})`);
            
            // Mine block
            while (true) {
                const blockHash = calculateHash(template, nonce);
                hashCount++;
                
                // Print hash rate every 10000 hashes
                if (hashCount % 10000 === 0) {
                    const elapsed = (Date.now() - startTime) / 1000;
                    const hashRate = hashCount / elapsed;
                    console.log(`Hash rate: ${hashRate.toFixed(2)} H/s | Nonce: ${nonce}`);
                }
                
                // Check if hash meets difficulty
                if (checkDifficulty(blockHash, template.difficulty)) {
                    console.log(`✅ Found valid block! Hash: ${blockHash.substring(0, 16)}...`);
                    
                    // Submit block
                    const result = await submitBlock(template, nonce, blockHash);
                    
                    if (result.result && result.result.success) {
                        console.log(`✅ Block accepted! Height: ${template.height}`);
                        console.log(`Reward: ${template.coinbasevalue} GXC`);
                    } else {
                        console.log(`❌ Block rejected:`, result);
                    }
                    
                    // Get new template
                    break;
                }
                
                nonce++;
                
                // Get new template every 100000 hashes
                if (nonce % 100000 === 0) {
                    break;
                }
            }
        } catch (error) {
            console.error('Error:', error.message);
            await new Promise(resolve => setTimeout(resolve, 5000));
        }
    }
}

mine().catch(console.error);
```

**Usage:**
```bash
# Install dependencies
npm install axios

# Run miner
node gxc-miner.js
```

---

## Testing Third-Party Miner

### Step 1: Start GXC Node

```bash
./gxc-node --testnet
```

### Step 2: Test RPC Endpoints

**Test getblocktemplate:**
```bash
curl -X POST http://localhost:8332/api/getblocktemplate \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "getblocktemplate",
    "params": [{
      "minerAddress": "GXC9fab7317231b966af85ac453e168c0932"
    }]
  }' | jq '.'
```

**Expected Output:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "version": 1,
    "previousblockhash": "...",
    "height": 13,
    "difficulty": 1000.0,
    ...
  }
}
```

### Step 3: Run Third-Party Miner

**Python:**
```bash
python3 gxc_miner.py
```

**Node.js:**
```bash
node gxc-miner.js
```

### Step 4: Verify Mining

**Check blockchain height:**
```bash
curl -s http://localhost:8332/api/getinfo | jq '.result.height'
```

**Check balance:**
```bash
curl -s "http://localhost:8332/api/getbalance?address=YOUR_ADDRESS" | jq '.'
```

---

## Troubleshooting

### Problem: Connection Refused

**Solution:**
```bash
# Check if node is running
curl http://localhost:8332/health

# Start node if not running
./gxc-node --testnet
```

### Problem: Invalid Address

**Solution:**
```bash
# Generate valid address
./gxc-keygen

# Use address starting with GXC (mainnet) or tGXC (testnet)
```

### Problem: Block Rejected

**Possible causes:**
- Invalid hash (doesn't meet difficulty)
- Stale block (another miner found it first)
- Invalid nonce

**Solution:**
- Check difficulty calculation
- Get fresh block template
- Verify hash calculation

---

## Performance Comparison

| Miner Type | Language | Hash Rate | Difficulty |
|------------|----------|-----------|------------|
| Built-in GXC miner | C++ | ~5-10 MH/s | Best |
| Python miner | Python | ~0.1-1 MH/s | Good for testing |
| Node.js miner | JavaScript | ~0.5-2 MH/s | Good for testing |
| cgminer (with adapter) | C | ~10-50 MH/s | Best (requires adapter) |

---

## Summary

✅ **GXC supports standard mining RPC methods**
- `getblocktemplate` - Get block template
- `submitblock` - Submit mined block

✅ **Compatible with third-party miners**
- Python miner (included)
- Node.js miner (included)
- cgminer (with adapter)
- Custom miners via RPC

✅ **Easy to integrate**
- Standard JSON-RPC interface
- Bitcoin-compatible methods
- Well-documented API

**Start mining with third-party software today!**
