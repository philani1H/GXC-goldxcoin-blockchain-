# GXC Mining Pool Development Guide

## Complete Guide to Building Your Own Mining Pool

This comprehensive guide will walk you through building a complete mining pool for the GXC blockchain, including Stratum protocol implementation, web dashboard, database management, and deployment.

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Prerequisites](#prerequisites)
4. [Core Components](#core-components)
5. [Stratum Protocol Implementation](#stratum-protocol-implementation)
6. [Database Schema](#database-schema)
7. [Web Dashboard](#web-dashboard)
8. [Blockchain Integration](#blockchain-integration)
9. [Deployment](#deployment)
10. [API Reference](#api-reference)
11. [Security Considerations](#security-considerations)
12. [Testing](#testing)

---

## Overview

A mining pool is a service that combines the computational power of multiple miners to increase the probability of finding blocks. When a block is found, rewards are distributed among pool participants based on their contribution (shares submitted).

### Key Features

- **Stratum Protocol Server**: TCP server for miner connections
- **Share Validation**: Validates and records miner shares
- **Block Submission**: Submits found blocks to the blockchain
- **Reward Distribution**: Calculates and distributes rewards
- **Web Dashboard**: Real-time statistics and monitoring
- **Database Persistence**: Stores miners, shares, blocks, and payouts

---

## Architecture

```
┌─────────────┐
│   Miners    │
│  (Stratum)  │
└──────┬──────┘
       │
       ▼
┌─────────────────┐
│  Mining Pool    │
│  - Stratum TCP  │
│  - Share Valid. │
│  - Job Distrib. │
└──────┬──────────┘
       │
       ▼
┌─────────────────┐
│   Blockchain    │
│   RPC/REST API  │
└─────────────────┘
       │
       ▼
┌─────────────────┐
│   Database      │
│   (SQLite)      │
└─────────────────┘
```

---

## Prerequisites

### Required Knowledge

- Python 3.8+
- Flask web framework
- SQLite database
- TCP socket programming
- JSON-RPC protocol
- Blockchain basics

### Required Libraries

```bash
pip install flask flask-socketio sqlite3 requests bcrypt
```

---

## Core Components

### 1. Pool Base Class (`pool_base.py`)

The base class handles all pool functionality:

```python
class MiningPool:
    def __init__(self, pool_name, algorithm, port, rpc_url):
        self.pool_name = pool_name
        self.algorithm = algorithm
        self.port = port
        self.rpc_url = rpc_url
        self.miners = {}
        self.current_job = None
        self.difficulty = 1000.0
```

**Key Methods:**

- `init_database()` - Initialize SQLite database
- `start_stratum_server()` - Start TCP Stratum server
- `generate_mining_job()` - Create new mining job
- `validate_share()` - Validate miner share
- `handle_block_found()` - Process found block
- `get_pool_stats()` - Get pool statistics

---

## Stratum Protocol Implementation

### Protocol Overview

Stratum is a JSON-RPC over TCP protocol for mining. Key methods:

1. **mining.subscribe** - Miner subscribes to jobs
2. **mining.authorize** - Miner authenticates
3. **mining.notify** - Pool sends new job
4. **mining.submit** - Miner submits share

### Implementation Example

```python
def handle_stratum_request(self, data, client_socket):
    """Handle Stratum protocol request"""
    try:
        request = json.loads(data)
        method = request.get('method')
        params = request.get('params', [])
        request_id = request.get('id')
        
        if method == 'mining.subscribe':
            # Generate subscription ID
            sub_id = str(uuid.uuid4())
            response = {
                'id': request_id,
                'result': [
                    [['mining.notify', sub_id], ['mining.set_difficulty', sub_id]],
                    '08000000'  # Extra nonce
                ],
                'error': None
            }
            client_socket.send(json.dumps(response).encode() + b'\n')
            
        elif method == 'mining.authorize':
            username = params[0]
            password = params[1]
            # Validate miner
            miner_id = self.authenticate_miner(username, password)
            response = {
                'id': request_id,
                'result': miner_id is not None,
                'error': None
            }
            client_socket.send(json.dumps(response).encode() + b'\n')
            
        elif method == 'mining.submit':
            # Validate and process share
            is_valid = self.validate_share(params)
            response = {
                'id': request_id,
                'result': is_valid,
                'error': None
            }
            client_socket.send(json.dumps(response).encode() + b'\n')
            
    except Exception as e:
        logger.error(f"Error handling Stratum request: {e}")
```

### Job Distribution

```python
def send_mining_job(self, client_socket, job):
    """Send mining job to miner"""
    notification = {
        'method': 'mining.notify',
        'params': [
            job['job_id'],
            job['prev_hash'],
            job['coinb1'],
            job['coinb2'],
            job['merkle_branches'],
            job['version'],
            job['nbits'],
            job['ntime'],
            True  # clean_jobs
        ]
    }
    client_socket.send(json.dumps(notification).encode() + b'\n')
```

---

## Database Schema

### Miners Table

```sql
CREATE TABLE miners (
    miner_id TEXT PRIMARY KEY,
    username TEXT NOT NULL,
    address TEXT,
    algorithm TEXT NOT NULL,
    connected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total_shares INTEGER DEFAULT 0,
    accepted_shares INTEGER DEFAULT 0,
    rejected_shares INTEGER DEFAULT 0,
    hash_rate REAL DEFAULT 0.0,
    difficulty REAL DEFAULT 0.0,
    is_active BOOLEAN DEFAULT 1
);
```

### Shares Table

```sql
CREATE TABLE shares (
    share_id INTEGER PRIMARY KEY AUTOINCREMENT,
    miner_id TEXT NOT NULL,
    job_id TEXT NOT NULL,
    nonce TEXT NOT NULL,
    extra_nonce2 TEXT,
    difficulty REAL NOT NULL,
    is_valid BOOLEAN NOT NULL,
    is_block BOOLEAN DEFAULT 0,
    block_hash TEXT,
    submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (miner_id) REFERENCES miners (miner_id)
);
```

### Blocks Table

```sql
CREATE TABLE blocks (
    block_id INTEGER PRIMARY KEY AUTOINCREMENT,
    block_hash TEXT UNIQUE NOT NULL,
    block_number INTEGER NOT NULL,
    miner_id TEXT,
    job_id TEXT,
    found_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    confirmed BOOLEAN DEFAULT 0,
    reward REAL DEFAULT 0.0,
    FOREIGN KEY (miner_id) REFERENCES miners (miner_id)
);
```

### Payouts Table

```sql
CREATE TABLE payouts (
    payout_id INTEGER PRIMARY KEY AUTOINCREMENT,
    miner_id TEXT NOT NULL,
    amount REAL NOT NULL,
    address TEXT NOT NULL,
    status TEXT DEFAULT 'pending',
    tx_hash TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    paid_at TIMESTAMP,
    FOREIGN KEY (miner_id) REFERENCES miners (miner_id)
);
```

---

## Web Dashboard

### Flask Routes

```python
@app.route('/')
def dashboard():
    stats = pool.get_pool_stats()
    miners = pool.get_miners_list()
    return render_template('pool_dashboard.html',
                         pool_name=pool.pool_name,
                         stats=stats,
                         miners=miners)

@app.route('/api/stats')
def api_stats():
    return jsonify(pool.get_pool_stats())

@app.route('/miner/<miner_id>')
def miner_dashboard(miner_id):
    miner_stats = pool.get_miner_stats(miner_id)
    return render_template('miner_dashboard.html',
                         miner=miner_stats)
```

### Available Pages

1. **Dashboard** (`/`) - Pool overview and statistics
2. **Statistics** (`/statistics`) - Detailed analytics
3. **Leaderboard** (`/leaderboard`) - Top miners
4. **Blocks** (`/blocks`) - Recent blocks found
5. **Payouts** (`/payouts`) - Payout history
6. **Miner Dashboard** (`/miner/<id>`) - Individual miner stats
7. **Calculator** (`/calculator`) - Profitability calculator
8. **API Docs** (`/api-docs`) - API documentation
9. **Help** (`/help`) - Getting started guide
10. **FAQ** (`/faq`) - Frequently asked questions
11. **Settings** (`/settings`) - Pool configuration
12. **Network Status** (`/network-status`) - Service status

---

## Blockchain Integration

### Getting Block Template

```python
def get_block_template(self):
    """Get block template from blockchain RPC"""
    payload = {
        "jsonrpc": "2.0",
        "method": "gxc_getBlockTemplate",
        "params": [{"algorithm": self.algorithm}],
        "id": 1
    }
    response = requests.post(self.rpc_url, json=payload, timeout=5)
    if response.status_code == 200:
        result = response.json().get('result')
        return result
    return None
```

### Submitting Block

```python
def submit_block(self, block_data):
    """Submit found block to blockchain"""
    payload = {
        "jsonrpc": "2.0",
        "method": "gxc_submitBlock",
        "params": [block_data],
        "id": 1
    }
    response = requests.post(self.rpc_url, json=payload, timeout=10)
    if response.status_code == 200:
        result = response.json().get('result')
        return result is None or result is True
    return False
```

---

## Deployment

### Local Development

```bash
# Install dependencies
pip install -r requirements.txt

# Run pool
python general_pool.py
```

### Vercel Deployment

1. **Create API wrapper** (`api.py`):

```python
import sys
import os
pool_path = os.path.dirname(__file__)
sys.path.insert(0, pool_path)
os.chdir(pool_path)
from general_pool import app
```

2. **Configure `vercel.json`**:

```json
{
  "version": 2,
  "builds": [{
    "src": "api.py",
    "use": "@vercel/python"
  }],
  "rewrites": [{
    "source": "/(.*)",
    "destination": "/api.py"
  }],
  "env": {
    "BLOCKCHAIN_NODE_URL": "http://your-blockchain-node:8545",
    "DATABASE_PATH": "/tmp/pool.db"
  }
}
```

3. **Deploy**:

```bash
vercel deploy
```

---

## API Reference

### REST API Endpoints

#### GET /api/stats
Get pool statistics.

**Response:**
```json
{
  "pool_name": "general-pool",
  "algorithm": "sha256",
  "active_miners": 10,
  "total_shares": 1000,
  "blocks_found": 5,
  "difficulty": 1000.0
}
```

#### GET /api/miners
Get list of active miners.

**Response:**
```json
[
  {
    "miner_id": "miner1",
    "username": "miner1",
    "address": "GXC...",
    "accepted_shares": 100,
    "hash_rate": 1000.0
  }
]
```

#### GET /api/blocks
Get recent blocks found.

**Response:**
```json
[
  {
    "block_hash": "0x...",
    "block_number": 12345,
    "miner_id": "miner1",
    "reward": 50.0,
    "found_at": "2024-01-01T12:00:00"
  }
]
```

### Stratum Protocol

#### mining.subscribe
Miner subscribes to receive mining jobs.

**Request:**
```json
{
  "id": 1,
  "method": "mining.subscribe",
  "params": []
}
```

**Response:**
```json
{
  "id": 1,
  "result": [
    [["mining.notify", "subscription_id"], ["mining.set_difficulty", "subscription_id"]],
    "08000000"
  ],
  "error": null
}
```

#### mining.authorize
Miner authenticates with pool.

**Request:**
```json
{
  "id": 2,
  "method": "mining.authorize",
  "params": ["username", "password"]
}
```

**Response:**
```json
{
  "id": 2,
  "result": true,
  "error": null
}
```

#### mining.submit
Miner submits a share.

**Request:**
```json
{
  "id": 3,
  "method": "mining.submit",
  "params": ["username", "job_id", "extra_nonce2", "ntime", "nonce"]
}
```

**Response:**
```json
{
  "id": 3,
  "result": true,
  "error": null
}
```

---

## Security Considerations

### 1. Input Validation

Always validate miner input:

```python
def validate_share(self, params):
    """Validate share submission"""
    if len(params) < 5:
        return False
    
    username = params[0]
    job_id = params[1]
    nonce = params[4]
    
    # Validate miner exists
    if username not in self.miners:
        return False
    
    # Validate job exists
    if job_id != self.current_job['job_id']:
        return False
    
    # Validate nonce format
    if not re.match(r'^[0-9a-fA-F]+$', nonce):
        return False
    
    return True
```

### 2. Rate Limiting

Implement rate limiting to prevent abuse:

```python
from collections import defaultdict
from time import time

class RateLimiter:
    def __init__(self, max_requests=100, window=60):
        self.max_requests = max_requests
        self.window = window
        self.requests = defaultdict(list)
    
    def is_allowed(self, client_id):
        now = time()
        client_requests = self.requests[client_id]
        # Remove old requests
        client_requests[:] = [t for t in client_requests if now - t < self.window]
        
        if len(client_requests) >= self.max_requests:
            return False
        
        client_requests.append(now)
        return True
```

### 3. SQL Injection Prevention

Always use parameterized queries:

```python
# ✅ Good
cursor.execute('SELECT * FROM miners WHERE miner_id = ?', (miner_id,))

# ❌ Bad
cursor.execute(f'SELECT * FROM miners WHERE miner_id = "{miner_id}"')
```

### 4. DDoS Protection

- Implement connection limits
- Use firewall rules
- Monitor for suspicious activity
- Implement CAPTCHA for web forms

---

## Testing

### Unit Tests

```python
import unittest
from pool_base import MiningPool

class TestMiningPool(unittest.TestCase):
    def setUp(self):
        self.pool = MiningPool('test-pool', 'sha256', 3333, 'http://localhost:8545')
    
    def test_share_validation(self):
        # Test valid share
        params = ['miner1', 'job1', 'extra', 'ntime', 'nonce123']
        result = self.pool.validate_share(params)
        self.assertTrue(result)
    
    def test_block_submission(self):
        block_data = {'hash': '0x123...', 'number': 1}
        result = self.pool.submit_block(block_data)
        self.assertTrue(result)

if __name__ == '__main__':
    unittest.main()
```

### Integration Tests

Test full mining workflow:

1. Connect miner via Stratum
2. Receive mining job
3. Submit valid share
4. Verify share recorded in database
5. Submit block
6. Verify block submitted to blockchain

---

## Reward Distribution

### Pay-Per-Share (PPS)

Miners are paid for each valid share, regardless of whether a block is found.

```python
def calculate_pps_reward(self, miner_id, shares):
    """Calculate PPS reward"""
    block_reward = 50.0  # GXC per block
    pool_fee = 0.01  # 1%
    share_value = block_reward / self.network_difficulty
    total_reward = shares * share_value
    return total_reward * (1 - pool_fee)
```

### Proportional

Miners receive a share of block rewards proportional to their contribution.

```python
def calculate_proportional_reward(self, miner_id, block_reward):
    """Calculate proportional reward"""
    miner_shares = self.get_miner_shares(miner_id)
    total_shares = self.get_total_shares()
    pool_fee = 0.01
    
    if total_shares == 0:
        return 0.0
    
    miner_portion = miner_shares / total_shares
    reward = block_reward * miner_portion
    return reward * (1 - pool_fee)
```

---

## Best Practices

1. **Monitor Performance**: Track pool uptime, share acceptance rate, block finding rate
2. **Backup Database**: Regularly backup SQLite database
3. **Logging**: Implement comprehensive logging for debugging
4. **Error Handling**: Gracefully handle network errors, invalid shares, etc.
5. **Scalability**: Design for horizontal scaling if needed
6. **Documentation**: Keep API and protocol documentation up to date
7. **Security Updates**: Regularly update dependencies and security patches

---

## Troubleshooting

### Common Issues

1. **Miners can't connect**
   - Check firewall rules
   - Verify Stratum port is open
   - Check pool server is running

2. **Shares rejected**
   - Verify difficulty calculation
   - Check job ID matches
   - Validate nonce format

3. **Blocks not submitting**
   - Verify RPC connection
   - Check block format
   - Review blockchain node logs

4. **Database errors**
   - Check file permissions
   - Verify SQLite version
   - Review database schema

---

## Additional Resources

- [Stratum Protocol Specification](https://en.bitcoin.it/wiki/Stratum_mining_protocol)
- [GXC Blockchain Documentation](../README.md)
- [Flask Documentation](https://flask.palletsprojects.com/)
- [SQLite Documentation](https://www.sqlite.org/docs.html)

---

## Conclusion

This guide provides a complete foundation for building a mining pool. The implementation includes:

- ✅ Stratum protocol server
- ✅ Share validation and recording
- ✅ Block submission to blockchain
- ✅ Web dashboard with multiple pages
- ✅ Database persistence
- ✅ API endpoints
- ✅ Security best practices

For questions or contributions, please refer to the main project documentation.

---

**Version:** 1.0.0  
**Last Updated:** 2024  
**License:** MIT

