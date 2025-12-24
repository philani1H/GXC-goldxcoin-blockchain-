# GXC Blockchain - Mining Pool Owner Guide

## Complete Guide for Pool Operators

**Version:** 2.0.0  
**Network:** Mainnet & Testnet  
**Protocol:** Stratum + JSON-RPC

---

## Table of Contents

1. [Pool Architecture](#pool-architecture)
2. [Node Setup](#node-setup)
3. [RPC Methods for Pools](#rpc-methods-for-pools)
4. [Block Template](#block-template)
5. [Share Validation](#share-validation)
6. [Payout System](#payout-system)
7. [Stratum Protocol](#stratum-protocol)
8. [Pool Statistics](#pool-statistics)
9. [Security Considerations](#security-considerations)
10. [Complete Pool Implementation](#complete-pool-implementation)

---

## Pool Architecture

### Recommended Setup

```
┌─────────────────────────────────────────────────────────────┐
│                     Mining Pool Architecture                 │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  Miners (Stratum)                                            │
│      ↓                                                        │
│  Stratum Server (Port 3333)                                  │
│      ↓                                                        │
│  Pool Backend                                                │
│      ↓                                                        │
│  GXC Node (RPC Port 8332)                                    │
│      ↓                                                        │
│  Blockchain                                                   │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Components

1. **GXC Node** - Full blockchain node with RPC enabled
2. **Stratum Server** - Handles miner connections
3. **Pool Backend** - Share validation, payout calculation
4. **Database** - Store shares, payouts, statistics
5. **Web Interface** - Pool statistics and miner dashboard

---

## Node Setup

### 1. Install and Configure Node

```bash
# Download and build
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install
sudo make install
```

### 2. Create Pool Configuration

Create `pool_node.conf`:

```ini
# Network
testnet=0
port=8333

# RPC
rpc_port=8332
rpc_user=pooluser
rpc_password=STRONG_PASSWORD_HERE
rpc_allow_ip=127.0.0.1

# Mining
gen=0  # Pool doesn't mine directly
txindex=1  # Enable transaction indexing

# Performance
dbcache=4096
maxconnections=200

# Logging
debug=0
```

### 3. Start Node

```bash
gxc-node --conf=pool_node.conf --datadir=/var/lib/gxc_pool
```

### 4. Verify Node is Running

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:STRONG_PASSWORD_HERE \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

---

## RPC Methods for Pools

### Essential Methods

#### 1. Get Block Template

**Method:** `getblocktemplate`

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:password \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblocktemplate",
    "params": [],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": {
    "height": 12345,
    "previousblockhash": "00000000000abc123...",
    "transactions": [
      {
        "txid": "tx1hash...",
        "data": "01000000..."
      }
    ],
    "coinbasevalue": 5000000000,
    "difficulty": 1000.5,
    "target": "00000000ffff0000...",
    "mintime": 1703462400,
    "curtime": 1703462450
  }
}
```

**What to do:**
1. Get template every 30 seconds
2. Include pending transactions
3. Create coinbase transaction with pool address
4. Distribute work to miners

#### 2. Submit Block

**Method:** `submitblock`

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:password \
  -d '{
    "jsonrpc": "2.0",
    "method": "submitblock",
    "params": ["block_hex_data"],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": null  // null = success
}
```

**On error:**
```json
{
  "error": {
    "code": -1,
    "message": "duplicate"
  }
}
```

#### 3. Get Mining Info

**Method:** `getmininginfo`

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:password \
  -d '{
    "jsonrpc": "2.0",
    "method": "getmininginfo",
    "params": [],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": {
    "blocks": 12345,
    "difficulty": 1000.5,
    "networkhashps": 1500000000,
    "pooledtx": 25,
    "testnet": false
  }
}
```

#### 4. Get Network Hashrate

**Method:** `getnetworkhashps`

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:password \
  -d '{
    "jsonrpc": "2.0",
    "method": "getnetworkhashps",
    "params": [120],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": 1500000000  // H/s
}
```

#### 5. Get Block by Height

**Method:** `getblock`

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:password \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblock",
    "params": ["12345"],
    "id": 1
  }'
```

#### 6. Validate Address

**Method:** `validateaddress`

```bash
curl -X POST http://localhost:8332 \
  -u pooluser:password \
  -d '{
    "jsonrpc": "2.0",
    "method": "validateaddress",
    "params": ["GXCaddress123..."],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": {
    "isvalid": true,
    "address": "GXCaddress123...",
    "ismine": false
  }
}
```

---

## Block Template

### Creating Coinbase Transaction

```python
import struct
import hashlib

def create_coinbase(height, pool_address, reward, extra_nonce):
    """
    Create coinbase transaction for pool
    """
    # Coinbase input
    coinbase_script = (
        struct.pack("<I", height) +  # Block height
        extra_nonce.encode() +        # Extra nonce for uniqueness
        b"/GXC Pool/"                 # Pool signature
    )
    
    # Coinbase output (reward to pool address)
    output_script = create_output_script(pool_address)
    
    tx = {
        "version": 1,
        "inputs": [{
            "txid": "0" * 64,  # Coinbase has no input
            "vout": 0xffffffff,
            "script": coinbase_script.hex()
        }],
        "outputs": [{
            "value": reward,  # 50 GXC = 5000000000 satoshis
            "script": output_script
        }],
        "locktime": 0
    }
    
    return tx

def create_output_script(address):
    """
    Create P2PKH output script
    """
    # Decode address to get pubkey hash
    pubkey_hash = decode_address(address)
    
    # OP_DUP OP_HASH160 <pubkey_hash> OP_EQUALVERIFY OP_CHECKSIG
    script = bytes([0x76, 0xa9, 0x14]) + pubkey_hash + bytes([0x88, 0xac])
    
    return script.hex()
```

### Building Block Header

```python
def build_block_header(template, coinbase_tx, nonce):
    """
    Build block header for mining
    """
    # Calculate merkle root with coinbase
    merkle_root = calculate_merkle_root([coinbase_tx] + template['transactions'])
    
    header = struct.pack(
        "<I32s32sIII",
        1,  # Version
        bytes.fromhex(template['previousblockhash'])[::-1],
        merkle_root[::-1],
        template['curtime'],
        int(template['target'], 16),
        nonce
    )
    
    return header

def calculate_merkle_root(transactions):
    """
    Calculate merkle root of transactions
    """
    if len(transactions) == 0:
        return bytes(32)
    
    hashes = [hashlib.sha256(hashlib.sha256(tx).digest()).digest() 
              for tx in transactions]
    
    while len(hashes) > 1:
        if len(hashes) % 2 == 1:
            hashes.append(hashes[-1])
        
        hashes = [
            hashlib.sha256(hashlib.sha256(hashes[i] + hashes[i+1]).digest()).digest()
            for i in range(0, len(hashes), 2)
        ]
    
    return hashes[0]
```

---

## Share Validation

### Validate Miner Share

```python
def validate_share(block_header, target_difficulty):
    """
    Validate if share meets difficulty target
    """
    # Calculate block hash
    hash1 = hashlib.sha256(block_header).digest()
    hash2 = hashlib.sha256(hash1).digest()
    block_hash = hash2[::-1]  # Reverse for little-endian
    
    # Convert to integer
    hash_int = int.from_bytes(block_hash, 'big')
    
    # Calculate target from difficulty
    target = calculate_target(target_difficulty)
    
    # Check if hash meets target
    if hash_int <= target:
        return True, block_hash.hex()
    
    return False, None

def calculate_target(difficulty):
    """
    Calculate target from difficulty
    """
    # GXC uses: target = max_target / difficulty
    max_target = 0x00000000FFFF0000000000000000000000000000000000000000000000000000
    target = max_target // int(difficulty)
    return target
```

### Share Difficulty

```python
def calculate_share_difficulty(pool_difficulty, network_difficulty):
    """
    Calculate appropriate share difficulty for miners
    
    Pool difficulty should be lower than network difficulty
    to allow miners to submit shares more frequently
    """
    # Typical pool difficulty is 1/1000 of network difficulty
    share_diff = max(1.0, network_difficulty / 1000)
    
    return share_diff
```

---

## Payout System

### Calculate Miner Rewards

```python
def calculate_payout(shares, block_reward, pool_fee=0.01):
    """
    Calculate payout for each miner
    
    Args:
        shares: dict of {miner_address: share_count}
        block_reward: 50 GXC
        pool_fee: 1% default
    """
    total_shares = sum(shares.values())
    pool_amount = block_reward * pool_fee
    miner_amount = block_reward - pool_amount
    
    payouts = {}
    for address, share_count in shares.items():
        payout = (share_count / total_shares) * miner_amount
        payouts[address] = payout
    
    return payouts, pool_amount
```

### Payout Methods

**1. PPLNS (Pay Per Last N Shares)**
```python
def pplns_payout(recent_shares, block_reward, N=1000000):
    """
    Pay based on last N shares
    More fair for loyal miners
    """
    # Get last N shares
    last_n_shares = recent_shares[-N:]
    
    # Count shares per miner
    miner_shares = {}
    for share in last_n_shares:
        miner = share['address']
        miner_shares[miner] = miner_shares.get(miner, 0) + 1
    
    return calculate_payout(miner_shares, block_reward)
```

**2. PPS (Pay Per Share)**
```python
def pps_payout(share_difficulty, network_difficulty, block_reward):
    """
    Pay fixed amount per share
    Pool takes all risk
    """
    expected_shares = network_difficulty / share_difficulty
    payout_per_share = block_reward / expected_shares
    
    return payout_per_share
```

---

## Stratum Protocol

### Stratum Server Implementation

```python
import socket
import json
import threading

class StratumServer:
    def __init__(self, host='0.0.0.0', port=3333):
        self.host = host
        self.port = port
        self.miners = {}
        
    def start(self):
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind((self.host, self.port))
        server.listen(100)
        
        print(f"Stratum server listening on {self.host}:{self.port}")
        
        while True:
            client, address = server.accept()
            thread = threading.Thread(target=self.handle_miner, args=(client, address))
            thread.start()
    
    def handle_miner(self, client, address):
        print(f"Miner connected: {address}")
        
        while True:
            try:
                data = client.recv(4096).decode()
                if not data:
                    break
                
                # Parse JSON-RPC request
                request = json.loads(data)
                method = request.get('method')
                
                if method == 'mining.subscribe':
                    response = self.handle_subscribe(request)
                elif method == 'mining.authorize':
                    response = self.handle_authorize(request, address)
                elif method == 'mining.submit':
                    response = self.handle_submit(request, address)
                else:
                    response = {'error': 'Unknown method'}
                
                # Send response
                client.send((json.dumps(response) + '\n').encode())
                
            except Exception as e:
                print(f"Error handling miner {address}: {e}")
                break
        
        client.close()
        print(f"Miner disconnected: {address}")
    
    def handle_subscribe(self, request):
        """Handle mining.subscribe"""
        return {
            'id': request['id'],
            'result': [
                [['mining.notify', 'subscription_id']],
                'extranonce1',
                4  # extranonce2 size
            ],
            'error': None
        }
    
    def handle_authorize(self, request, address):
        """Handle mining.authorize"""
        params = request['params']
        username = params[0]  # Usually miner address
        password = params[1]
        
        # Validate address
        if self.validate_address(username):
            self.miners[address] = {
                'address': username,
                'authorized': True
            }
            return {
                'id': request['id'],
                'result': True,
                'error': None
            }
        
        return {
            'id': request['id'],
            'result': False,
            'error': 'Invalid address'
        }
    
    def handle_submit(self, request, address):
        """Handle mining.submit (share submission)"""
        params = request['params']
        worker = params[0]
        job_id = params[1]
        extranonce2 = params[2]
        ntime = params[3]
        nonce = params[4]
        
        # Validate share
        valid = self.validate_share(job_id, nonce, ntime, extranonce2)
        
        if valid:
            # Record share
            self.record_share(address, worker)
            
            return {
                'id': request['id'],
                'result': True,
                'error': None
            }
        
        return {
            'id': request['id'],
            'result': False,
            'error': 'Invalid share'
        }
```

### Stratum Messages

**1. mining.notify (Server → Miner)**
```json
{
  "method": "mining.notify",
  "params": [
    "job_id",
    "prevhash",
    "coinb1",
    "coinb2",
    "merkle_branches",
    "version",
    "nbits",
    "ntime",
    true
  ]
}
```

**2. mining.submit (Miner → Server)**
```json
{
  "method": "mining.submit",
  "params": [
    "worker_name",
    "job_id",
    "extranonce2",
    "ntime",
    "nonce"
  ],
  "id": 1
}
```

---

## Pool Statistics

### Track Pool Performance

```python
class PoolStats:
    def __init__(self):
        self.total_shares = 0
        self.valid_shares = 0
        self.invalid_shares = 0
        self.blocks_found = 0
        self.total_paid = 0
        
    def record_share(self, valid=True):
        self.total_shares += 1
        if valid:
            self.valid_shares += 1
        else:
            self.invalid_shares += 1
    
    def record_block(self, reward):
        self.blocks_found += 1
        self.total_paid += reward
    
    def get_stats(self):
        return {
            'total_shares': self.total_shares,
            'valid_shares': self.valid_shares,
            'invalid_shares': self.invalid_shares,
            'blocks_found': self.blocks_found,
            'total_paid': self.total_paid,
            'reject_rate': self.invalid_shares / max(1, self.total_shares)
        }
```

### Calculate Pool Hashrate

```python
def calculate_pool_hashrate(shares_per_minute, share_difficulty):
    """
    Calculate pool hashrate from submitted shares
    """
    # shares_per_minute * share_difficulty * 2^32 / 60
    hashrate = (shares_per_minute * share_difficulty * 4294967296) / 60
    return hashrate
```

---

## Security Considerations

### 1. DDoS Protection

```python
# Rate limiting
from collections import defaultdict
import time

class RateLimiter:
    def __init__(self, max_requests=100, window=60):
        self.max_requests = max_requests
        self.window = window
        self.requests = defaultdict(list)
    
    def allow_request(self, ip):
        now = time.time()
        # Remove old requests
        self.requests[ip] = [t for t in self.requests[ip] if now - t < self.window]
        
        if len(self.requests[ip]) < self.max_requests:
            self.requests[ip].append(now)
            return True
        
        return False
```

### 2. Share Validation

- Always validate share difficulty
- Check for duplicate shares
- Verify block header format
- Validate coinbase transaction

### 3. Payout Security

- Use cold wallet for pool funds
- Implement minimum payout threshold
- Add payout confirmation delay
- Monitor for unusual activity

---

## Complete Pool Implementation

See `mining_pool/pool_server.py` for full implementation:

```bash
cd mining_pool
python3 pool_server.py --port 3333 --rpc-host localhost --rpc-port 8332
```

**Features:**
- ✅ Stratum protocol support
- ✅ Share validation
- ✅ PPLNS payout system
- ✅ Real-time statistics
- ✅ Web dashboard
- ✅ Database integration
- ✅ Multi-worker support

---

## Testing Your Pool

```bash
# Test with cpuminer
cpuminer -a sha256d -o stratum+tcp://localhost:3333 -u GXCyouraddress -p x

# Test with cgminer
cgminer -o stratum+tcp://localhost:3333 -u GXCyouraddress -p x --algo sha256d
```

---

## Pool Configuration Example

```python
POOL_CONFIG = {
    'pool_address': 'GXCpooladdress123...',
    'pool_fee': 0.01,  # 1%
    'min_payout': 1.0,  # 1 GXC
    'payout_interval': 3600,  # 1 hour
    'share_difficulty': 1.0,
    'vardiff': {
        'enabled': True,
        'min_diff': 1.0,
        'max_diff': 1000.0,
        'target_time': 15  # seconds per share
    },
    'stratum': {
        'host': '0.0.0.0',
        'port': 3333
    },
    'rpc': {
        'host': 'localhost',
        'port': 8332,
        'user': 'pooluser',
        'password': 'password'
    }
}
```

---

## Summary for Pool Owners

✅ **Node Setup** - Full node with RPC enabled  
✅ **Block Templates** - Get work from blockchain  
✅ **Share Validation** - Verify miner submissions  
✅ **Payout System** - PPLNS or PPS  
✅ **Stratum Protocol** - Standard mining protocol  
✅ **Statistics** - Track pool performance  
✅ **Security** - DDoS protection, rate limiting  

**Everything you need to run a successful GXC mining pool!** 🎯
