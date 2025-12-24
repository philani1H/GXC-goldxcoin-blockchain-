# GXC Blockchain - Mining Pool Documentation

## Overview

This document provides comprehensive documentation for setting up and operating a GXC mining pool. Pool operators can use this guide to build mining pools that support multiple miners and distribute rewards fairly.

---

## Table of Contents

1. [Mining Overview](#mining-overview)
2. [Pool Architecture](#pool-architecture)
3. [Setup Guide](#setup-guide)
4. [Mining API Reference](#mining-api-reference)
5. [Reward Distribution](#reward-distribution)
6. [Pool Management](#pool-management)
7. [Miner Integration](#miner-integration)
8. [Code Examples](#code-examples)
9. [Best Practices](#best-practices)

---

## Mining Overview

### Supported Algorithms

GXC blockchain supports multiple mining algorithms:

1. **SHA-256** - Bitcoin-compatible
2. **Ethash** - Ethereum-compatible
3. **GXHash** - Custom algorithm

### Block Rewards

- **Initial Reward**: 50 GXC per block
- **Halving**: Every 210,000 blocks
- **Minimum Reward**: 0.00000001 GXC

### Difficulty

- **Testnet**: 0.1 (easy mining for testing)
- **Mainnet**: 1000.0 (requires ~4 leading zero bytes)
- **Adjustment**: Dynamic based on network hashrate

---

## Pool Architecture

### Components

```
┌─────────────┐
│   Miners    │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Pool Server │ ◄─── Stratum Protocol
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  GXC Node   │ ◄─── RPC API
└─────────────┘
```

### Pool Server Responsibilities

1. **Work Distribution**: Provide mining work to miners
2. **Share Validation**: Verify submitted shares
3. **Reward Calculation**: Calculate miner rewards
4. **Payout Processing**: Distribute rewards to miners
5. **Statistics**: Track hashrate, shares, earnings

---

## Setup Guide

### Prerequisites

- GXC node running and synced
- Pool server software
- Database (PostgreSQL/MySQL recommended)
- Redis (for caching)

### 1. Install GXC Node

```bash
# Clone repository
git clone https://github.com/your-repo/GXC-goldxcoin-blockchain.git
cd GXC-goldxcoin-blockchain

# Build
make -j$(nproc)

# Start node
./gxc-node --datadir=/path/to/data
```

### 2. Configure Node for Pool

Edit `gxc.conf`:

```ini
# RPC Configuration
rpcuser=pooluser
rpcpassword=secure_password
rpcport=8332
rpcallowip=127.0.0.1

# Mining Configuration
gen=0  # Disable solo mining
server=1

# Network
port=9333
maxconnections=100
```

### 3. Install Pool Software

```bash
# Install dependencies
npm install

# Configure pool
cp config.example.json config.json
nano config.json
```

### 4. Configure Pool

`config.json`:

```json
{
  "coin": "GXC",
  "algorithm": "sha256",
  
  "node": {
    "host": "127.0.0.1",
    "port": 8332,
    "user": "pooluser",
    "password": "secure_password"
  },
  
  "pool": {
    "address": "GXC_POOL_ADDRESS",
    "fee": 1.0,
    "minPayout": 1.0,
    "payoutInterval": 3600
  },
  
  "stratum": {
    "port": 3333,
    "difficulty": 0.1,
    "varDiff": {
      "enabled": true,
      "minDiff": 0.01,
      "maxDiff": 100,
      "targetTime": 15
    }
  },
  
  "database": {
    "host": "localhost",
    "port": 5432,
    "database": "gxc_pool",
    "user": "pool",
    "password": "password"
  }
}
```

### 5. Start Pool

```bash
# Start pool server
npm start

# Or with PM2
pm2 start pool.js --name gxc-pool
```

---

## Mining API Reference

### `getblocktemplate`

Get mining work template.

**Parameters**:
- `algorithm` (string): Mining algorithm ("sha256", "ethash", "gxhash")

**Returns**: `object` - Block template

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblocktemplate",
    "params": [{"algorithm": "sha256"}],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "version": 1,
    "previousblockhash": "0cf1aa42a976e21e948f1de701decb61f437797e12ca1dc0213e7f8e0987c716",
    "transactions": [],
    "coinbaseaux": {},
    "coinbasevalue": 50.0,
    "target": "0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
    "mintime": 1766579232,
    "mutable": ["time", "transactions", "prevblock"],
    "noncerange": "00000000ffffffff",
    "sigoplimit": 20000,
    "sizelimit": 1000000,
    "curtime": 1766579240,
    "bits": "1d00ffff",
    "height": 30,
    "difficulty": 0.1
  },
  "id": 1
}
```

---

### `submitblock`

Submit mined block.

**Parameters**:
- `blockdata` (object): Block data including hash, nonce, transactions

**Returns**: `null` on success, error on failure

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "submitblock",
    "params": [{
      "hash": "0999cc3d857af900...",
      "previousblockhash": "0775ae01732f40d8...",
      "height": 30,
      "nonce": 12345,
      "miner": "GXC_POOL_ADDRESS",
      "timestamp": 1766579240,
      "difficulty": 0.1,
      "transactions": []
    }],
    "id": 1
  }'
```

---

### `getmininginfo`

Get mining statistics.

**Parameters**: None

**Returns**: `object` - Mining info

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getmininginfo",
    "params": [],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "blocks": 30,
    "currentblocksize": 1000,
    "currentblocktx": 0,
    "difficulty": 0.1,
    "networkhashps": 1000000.0,
    "pooledtx": 2,
    "chain": "test"
  },
  "id": 1
}
```

---

## Reward Distribution

### Payment Methods

#### 1. PPS (Pay Per Share)
- **Description**: Fixed payment per share
- **Risk**: Pool operator bears variance risk
- **Formula**: `reward = (share_difficulty / network_difficulty) * block_reward`

#### 2. PPLNS (Pay Per Last N Shares)
- **Description**: Payment based on last N shares
- **Risk**: Shared between miners and pool
- **Formula**: `reward = (miner_shares / total_shares) * block_reward * (1 - pool_fee)`

#### 3. PROP (Proportional)
- **Description**: Proportional to shares in round
- **Risk**: Vulnerable to pool hopping
- **Formula**: `reward = (miner_shares / round_shares) * block_reward`

### Recommended: PPLNS

```javascript
function calculatePPLNS(minerShares, totalShares, blockReward, poolFee) {
    const netReward = blockReward * (1 - poolFee / 100);
    const minerReward = (minerShares / totalShares) * netReward;
    return minerReward;
}

// Example
const reward = calculatePPLNS(
    1000,      // Miner submitted 1000 shares
    10000,     // Total 10000 shares in window
    50.0,      // Block reward is 50 GXC
    1.0        // Pool fee is 1%
);
// Result: 4.95 GXC
```

---

## Pool Management

### Database Schema

```sql
-- Miners table
CREATE TABLE miners (
    id SERIAL PRIMARY KEY,
    address VARCHAR(64) UNIQUE NOT NULL,
    hashrate BIGINT DEFAULT 0,
    shares_valid BIGINT DEFAULT 0,
    shares_invalid BIGINT DEFAULT 0,
    balance DECIMAL(20, 8) DEFAULT 0,
    paid DECIMAL(20, 8) DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Shares table
CREATE TABLE shares (
    id SERIAL PRIMARY KEY,
    miner_id INTEGER REFERENCES miners(id),
    difficulty DECIMAL(20, 8),
    valid BOOLEAN,
    block_hash VARCHAR(64),
    created_at TIMESTAMP DEFAULT NOW()
);

-- Blocks table
CREATE TABLE blocks (
    id SERIAL PRIMARY KEY,
    height INTEGER,
    hash VARCHAR(64) UNIQUE,
    reward DECIMAL(20, 8),
    difficulty DECIMAL(20, 8),
    status VARCHAR(20), -- 'pending', 'confirmed', 'orphaned'
    miner_id INTEGER REFERENCES miners(id),
    created_at TIMESTAMP DEFAULT NOW()
);

-- Payments table
CREATE TABLE payments (
    id SERIAL PRIMARY KEY,
    miner_id INTEGER REFERENCES miners(id),
    amount DECIMAL(20, 8),
    tx_hash VARCHAR(64),
    status VARCHAR(20), -- 'pending', 'sent', 'confirmed'
    created_at TIMESTAMP DEFAULT NOW()
);
```

### Pool Statistics

Track and display:

1. **Pool Hashrate**: Sum of all miner hashrates
2. **Active Miners**: Miners with shares in last hour
3. **Blocks Found**: Total blocks mined by pool
4. **Pool Luck**: Actual shares vs expected shares
5. **Network Stats**: Network hashrate, difficulty

---

## Miner Integration

### Stratum Protocol

#### Connection

```
stratum+tcp://pool.example.com:3333
```

#### Authentication

```json
{
  "id": 1,
  "method": "mining.authorize",
  "params": ["GXC_MINER_ADDRESS", "password"]
}
```

#### Subscribe

```json
{
  "id": 2,
  "method": "mining.subscribe",
  "params": ["miner-software/1.0"]
}
```

#### Receive Job

```json
{
  "id": null,
  "method": "mining.notify",
  "params": [
    "job_id",
    "prev_hash",
    "coinbase1",
    "coinbase2",
    "merkle_branches",
    "version",
    "nbits",
    "ntime",
    true
  ]
}
```

#### Submit Share

```json
{
  "id": 3,
  "method": "mining.submit",
  "params": [
    "GXC_MINER_ADDRESS",
    "job_id",
    "extranonce2",
    "ntime",
    "nonce"
  ]
}
```

---

## Code Examples

### Pool Server (Node.js)

```javascript
const net = require('net');
const crypto = require('crypto');

class MiningPool {
    constructor(config) {
        this.config = config;
        this.miners = new Map();
        this.currentJob = null;
    }
    
    start() {
        const server = net.createServer((socket) => {
            this.handleMiner(socket);
        });
        
        server.listen(this.config.port, () => {
            console.log(`Pool listening on port ${this.config.port}`);
        });
        
        // Update mining job every 30 seconds
        setInterval(() => this.updateJob(), 30000);
    }
    
    handleMiner(socket) {
        const miner = {
            socket: socket,
            address: null,
            difficulty: this.config.initialDifficulty
        };
        
        socket.on('data', (data) => {
            const message = JSON.parse(data.toString());
            this.handleMessage(miner, message);
        });
        
        socket.on('close', () => {
            if (miner.address) {
                this.miners.delete(miner.address);
            }
        });
    }
    
    handleMessage(miner, message) {
        switch (message.method) {
            case 'mining.authorize':
                this.authorize(miner, message);
                break;
            case 'mining.subscribe':
                this.subscribe(miner, message);
                break;
            case 'mining.submit':
                this.submitShare(miner, message);
                break;
        }
    }
    
    authorize(miner, message) {
        miner.address = message.params[0];
        this.miners.set(miner.address, miner);
        
        miner.socket.write(JSON.stringify({
            id: message.id,
            result: true,
            error: null
        }) + '\n');
    }
    
    subscribe(miner, message) {
        miner.socket.write(JSON.stringify({
            id: message.id,
            result: [
                [["mining.set_difficulty", "subscription_id"], ["mining.notify", "subscription_id"]],
                "extranonce1",
                4
            ],
            error: null
        }) + '\n');
        
        // Send current job
        if (this.currentJob) {
            this.sendJob(miner);
        }
    }
    
    async submitShare(miner, message) {
        const [address, jobId, extranonce2, ntime, nonce] = message.params;
        
        // Validate share
        const valid = await this.validateShare(jobId, nonce, ntime);
        
        if (valid) {
            // Record share in database
            await this.recordShare(miner, valid.difficulty, valid.isBlock);
            
            if (valid.isBlock) {
                // Submit block to node
                await this.submitBlock(valid.blockData);
            }
            
            miner.socket.write(JSON.stringify({
                id: message.id,
                result: true,
                error: null
            }) + '\n');
        } else {
            miner.socket.write(JSON.stringify({
                id: message.id,
                result: false,
                error: [21, "Invalid share", null]
            }) + '\n');
        }
    }
    
    async updateJob() {
        // Get block template from node
        const template = await this.getBlockTemplate();
        
        this.currentJob = {
            id: crypto.randomBytes(4).toString('hex'),
            prevHash: template.previousblockhash,
            height: template.height,
            target: template.target
        };
        
        // Send to all miners
        for (const miner of this.miners.values()) {
            this.sendJob(miner);
        }
    }
    
    sendJob(miner) {
        miner.socket.write(JSON.stringify({
            id: null,
            method: 'mining.notify',
            params: [
                this.currentJob.id,
                this.currentJob.prevHash,
                "coinbase1",
                "coinbase2",
                [],
                "00000001",
                "1d00ffff",
                Math.floor(Date.now() / 1000).toString(16),
                true
            ]
        }) + '\n');
    }
}

// Start pool
const config = require('./config.json');
const pool = new MiningPool(config);
pool.start();
```

---

### Miner (Python)

```python
import socket
import json
import hashlib
import time

class GXCMiner:
    def __init__(self, pool_host, pool_port, address):
        self.pool_host = pool_host
        self.pool_port = pool_port
        self.address = address
        self.socket = None
        self.job = None
    
    def connect(self):
        """Connect to pool"""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.pool_host, self.pool_port))
        
        # Authorize
        self.send_message({
            "id": 1,
            "method": "mining.authorize",
            "params": [self.address, ""]
        })
        
        # Subscribe
        self.send_message({
            "id": 2,
            "method": "mining.subscribe",
            "params": ["gxc-miner/1.0"]
        })
    
    def send_message(self, message):
        """Send JSON message to pool"""
        data = json.dumps(message) + '\n'
        self.socket.send(data.encode())
    
    def receive_message(self):
        """Receive JSON message from pool"""
        data = self.socket.recv(4096).decode()
        return json.loads(data)
    
    def mine(self):
        """Main mining loop"""
        while True:
            # Receive messages from pool
            try:
                message = self.receive_message()
                
                if message.get('method') == 'mining.notify':
                    self.job = message['params']
                    self.mine_job()
            except Exception as e:
                print(f"Error: {e}")
                time.sleep(1)
    
    def mine_job(self):
        """Mine current job"""
        job_id = self.job[0]
        prev_hash = self.job[1]
        
        # Try different nonces
        for nonce in range(1000000):
            # Calculate hash
            header = f"{prev_hash}{nonce}"
            hash_result = hashlib.sha256(header.encode()).hexdigest()
            
            # Check if valid share
            if hash_result.startswith('0'):
                # Submit share
                self.send_message({
                    "id": 3,
                    "method": "mining.submit",
                    "params": [
                        self.address,
                        job_id,
                        "00000000",
                        hex(int(time.time()))[2:],
                        hex(nonce)[2:]
                    ]
                })
                print(f"Share submitted: {hash_result[:16]}...")

# Usage
miner = GXCMiner("pool.example.com", 3333, "GXC_YOUR_ADDRESS")
miner.connect()
miner.mine()
```

---

## Best Practices

### 1. Pool Security

- Use SSL/TLS for Stratum connections
- Implement DDoS protection
- Rate limit connections
- Validate all inputs
- Monitor for attacks

### 2. Performance

- Use Redis for caching
- Optimize database queries
- Implement connection pooling
- Use load balancing for high traffic
- Monitor server resources

### 3. Reliability

- Implement failover nodes
- Regular backups
- Monitor node sync status
- Automatic restart on failure
- Alert system for issues

### 4. Fairness

- Transparent fee structure
- Regular payouts
- Accurate statistics
- Clear terms of service
- Responsive support

### 5. Monitoring

Track and alert on:
- Pool hashrate drops
- Node disconnections
- Database errors
- Payment failures
- Orphaned blocks

---

## Troubleshooting

### Common Issues

#### Miners Can't Connect
- Check firewall rules
- Verify port is open
- Check pool server logs
- Test with telnet

#### Shares Rejected
- Verify difficulty settings
- Check share validation logic
- Review miner software version
- Check for stale shares

#### Payments Not Sent
- Verify node balance
- Check payment threshold
- Review transaction logs
- Verify address format

---

## Support

- **Documentation**: https://github.com/your-repo/docs
- **Pool Operators**: Discord channel
- **Issues**: https://github.com/your-repo/issues

---

## Changelog

### Version 2.0.0
- Initial mining pool documentation
- Stratum protocol support
- PPLNS reward system
- Complete code examples

---

**Last Updated**: December 24, 2024  
**Version**: 2.0.0  
**Status**: Production Ready
