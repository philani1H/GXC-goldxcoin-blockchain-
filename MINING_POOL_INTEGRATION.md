# GXC Blockchain - Mining Pool Integration Guide

## Overview

This guide provides everything external mining pools need to integrate GXC blockchain mining into their infrastructure.

**Network**: GXC (GoldXCoin)
**Consensus**: Hybrid PoW/PoS
**Algorithms**: SHA-256, Ethash, GXHash (Argon2id-based)
**Block Time**: 120 seconds (2 minutes)
**Block Reward**: 50 GXC (halves every 210,000 blocks)
**Difficulty Adjustment**: Every 2,016 blocks

---

## Quick Start

### 1. Network Endpoints

#### Mainnet
```
Network Port: 9333
RPC Port: 8332
REST API Port: 8080
WebSocket Port: 8081
```

#### Testnet
```
Network Port: 19333
RPC Port: 18332
REST API Port: 18080
WebSocket Port: 18081
```

### 2. Connection URLs

**Mainnet RPC**:
```
http://your-node-ip:8332
```

**Testnet RPC**:
```
http://your-node-ip:18332
```

---

## Mining Algorithms

GXC supports three mining algorithms. Pools can support one or all three:

### Algorithm 1: SHA-256 (Bitcoin-style)

**Hashrate**: ~500K H/s (CPU), ~1 TH/s (ASIC)
**Hardware**: ASIC miners, CPU
**Difficulty**: Dynamic
**Best For**: ASIC mining farms

**Algorithm Details**:
- Double SHA-256 hash
- Same as Bitcoin mining
- ASIC-friendly
- Fastest block generation

### Algorithm 2: Ethash (Ethereum-style)

**Hashrate**: ~30 MH/s (GPU)
**Hardware**: GPU miners (4GB+ VRAM)
**Difficulty**: Dynamic
**Best For**: GPU mining farms

**Algorithm Details**:
- DAG-based memory-hard algorithm
- Requires 2GB+ memory
- GPU-optimized
- Moderate ASIC resistance

### Algorithm 3: GXHash (Argon2id-based) ⭐ RECOMMENDED

**Hashrate**: ~1 H/s (CPU), ~10 H/s (optimized)
**Hardware**: CPU, GPU
**Difficulty**: Dynamic
**Best For**: Decentralized mining, CPU farms

**Algorithm Details**:
- Argon2id memory-hard function
- 64MB memory per hash
- ASIC-resistant
- Fair distribution
- Energy efficient

---

## RPC API Reference

### Authentication

GXC uses JSON-RPC 2.0 with optional authentication.

**No Authentication** (default):
```bash
curl -X POST http://node-ip:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

**With Authentication** (if configured):
```bash
curl -X POST http://node-ip:8332 \
  -u username:password \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

---

## Essential RPC Methods for Mining Pools

### 1. Get Blockchain Info

**Method**: `getblockchaininfo`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "getblockchaininfo",
  "params": [],
  "id": 1
}
```

**Response**:
```json
{
  "result": {
    "chain": "main",
    "blocks": 150000,
    "headers": 150000,
    "bestblockhash": "0000000000000a3290f20e75860d505ce0e948a1d1d846bec7e39015d242884b",
    "difficulty": 1000.0,
    "mediantime": 1702234567,
    "chainwork": "00000000000000000000000000000000000000000000001234567890abcdef",
    "pruned": false
  }
}
```

### 2. Get Block Template (Mining Work)

**Method**: `getblocktemplate`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "getblocktemplate",
  "params": [{
    "algorithm": "gxhash",
    "capabilities": ["coinbasetxn", "workid", "coinbase/append"]
  }],
  "id": 1
}
```

**Parameters**:
- `algorithm`: "sha256", "ethash", or "gxhash"
- `capabilities`: Array of supported features

**Response**:
```json
{
  "result": {
    "version": 1,
    "previousblockhash": "0000000000000a3290f20e75860d505ce0e948a1d1d846bec7e39015d242884b",
    "transactions": [
      {
        "data": "01000000...",
        "hash": "abc123...",
        "fee": 0.001
      }
    ],
    "coinbaseaux": {
      "flags": "062f503253482f"
    },
    "coinbasevalue": 5000000000,
    "target": "00000000ffff0000000000000000000000000000000000000000000000000000",
    "mintime": 1702234567,
    "mutable": ["time", "transactions", "prevblock"],
    "noncerange": "00000000ffffffff",
    "sigoplimit": 20000,
    "sizelimit": 1000000,
    "curtime": 1702234600,
    "bits": "1d00ffff",
    "height": 150001,
    "algorithm": "gxhash"
  }
}
```

### 3. Submit Block (Mining Solution)

**Method**: `submitblock`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "submitblock",
  "params": [
    "block_hex_data_here"
  ],
  "id": 1
}
```

**Response (Success)**:
```json
{
  "result": null,
  "error": null,
  "id": 1
}
```

**Response (Failure)**:
```json
{
  "result": null,
  "error": {
    "code": -1,
    "message": "rejected: duplicate"
  },
  "id": 1
}
```

### 4. Get Mining Info

**Method**: `getmininginfo`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "getmininginfo",
  "params": [],
  "id": 1
}
```

**Response**:
```json
{
  "result": {
    "blocks": 150000,
    "currentblocksize": 1000,
    "currentblocktx": 10,
    "difficulty": 1000.0,
    "networkhashps": 1000000000,
    "pooledtx": 25,
    "chain": "main",
    "algorithm": "gxhash"
  }
}
```

### 5. Get Network Hash Rate

**Method**: `getnetworkhashps`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "getnetworkhashps",
  "params": [120, -1],
  "id": 1
}
```

**Parameters**:
- `blocks`: Number of blocks to average (default: 120)
- `height`: Block height (-1 = current)

**Response**:
```json
{
  "result": 1000000000
}
```

### 6. Validate Address

**Method**: `validateaddress`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "validateaddress",
  "params": ["GXC_ADDRESS_HERE"],
  "id": 1
}
```

**Response**:
```json
{
  "result": {
    "isvalid": true,
    "address": "GXC_ADDRESS_HERE",
    "scriptPubKey": "76a914...",
    "ismine": false,
    "iswatchonly": false
  }
}
```

### 7. Get Block by Hash

**Method**: `getblock`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "getblock",
  "params": ["block_hash_here", 2],
  "id": 1
}
```

**Parameters**:
- `blockhash`: Block hash
- `verbosity`: 0 (hex), 1 (json), 2 (json with tx details)

**Response**:
```json
{
  "result": {
    "hash": "0000000000000a3290f20e75860d505ce0e948a1d1d846bec7e39015d242884b",
    "confirmations": 100,
    "height": 150000,
    "version": 1,
    "merkleroot": "abc123...",
    "time": 1702234567,
    "nonce": 123456789,
    "bits": "1d00ffff",
    "difficulty": 1000.0,
    "chainwork": "00000000000000000000000000000000000000000000001234567890abcdef",
    "previousblockhash": "0000000000000a3290...",
    "nextblockhash": "0000000000000a3290...",
    "tx": [
      {
        "txid": "abc123...",
        "hash": "abc123...",
        "version": 1,
        "size": 250,
        "vsize": 250,
        "locktime": 0,
        "vin": [...],
        "vout": [...]
      }
    ]
  }
}
```

### 8. Get Transaction

**Method**: `gettransaction`

**Request**:
```json
{
  "jsonrpc": "2.0",
  "method": "gettransaction",
  "params": ["txid_here"],
  "id": 1
}
```

**Response**:
```json
{
  "result": {
    "txid": "abc123...",
    "hash": "abc123...",
    "version": 1,
    "size": 250,
    "confirmations": 10,
    "time": 1702234567,
    "blocktime": 1702234567,
    "blockhash": "0000000000000a3290...",
    "amount": 50.0,
    "fee": 0.001
  }
}
```

---

## Stratum Protocol Support

GXC supports Stratum mining protocol for efficient pool mining.

### Stratum Connection

**Endpoint**: `stratum+tcp://node-ip:3333`

**Supported Methods**:
- `mining.subscribe`
- `mining.authorize`
- `mining.set_difficulty`
- `mining.notify`
- `mining.submit`

### Example Stratum Session

**1. Subscribe**:
```json
{"id": 1, "method": "mining.subscribe", "params": ["MinerName/1.0.0"]}
```

**Response**:
```json
{
  "id": 1,
  "result": [
    [["mining.set_difficulty", "subscription_id"], ["mining.notify", "subscription_id"]],
    "extranonce1",
    4
  ],
  "error": null
}
```

**2. Authorize**:
```json
{"id": 2, "method": "mining.authorize", "params": ["worker_name", "password"]}
```

**Response**:
```json
{"id": 2, "result": true, "error": null}
```

**3. Set Difficulty**:
```json
{"id": null, "method": "mining.set_difficulty", "params": [1000]}
```

**4. Notify (New Work)**:
```json
{
  "id": null,
  "method": "mining.notify",
  "params": [
    "job_id",
    "prevhash",
    "coinb1",
    "coinb2",
    ["merkle_branch"],
    "version",
    "nbits",
    "ntime",
    true
  ]
}
```

**5. Submit Share**:
```json
{
  "id": 4,
  "method": "mining.submit",
  "params": ["worker_name", "job_id", "extranonce2", "ntime", "nonce"]
}
```

**Response**:
```json
{"id": 4, "result": true, "error": null}
```

---

## Mining Pool Implementation Guide

### Step 1: Connect to Node

```python
import requests
import json

class GXCNode:
    def __init__(self, host, port, username=None, password=None):
        self.url = f"http://{host}:{port}"
        self.auth = (username, password) if username else None
        self.id = 0
    
    def rpc_call(self, method, params=[]):
        self.id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": self.id
        }
        response = requests.post(self.url, json=payload, auth=self.auth)
        return response.json()

# Connect to node
node = GXCNode("localhost", 8332)

# Get blockchain info
info = node.rpc_call("getblockchaininfo")
print(f"Current height: {info['result']['blocks']}")
```

### Step 2: Get Mining Work

```python
def get_work(algorithm="gxhash"):
    template = node.rpc_call("getblocktemplate", [{
        "algorithm": algorithm,
        "capabilities": ["coinbasetxn", "workid"]
    }])
    return template['result']

# Get work for GXHash mining
work = get_work("gxhash")
print(f"Mining height: {work['height']}")
print(f"Target: {work['target']}")
```

### Step 3: Create Coinbase Transaction

```python
def create_coinbase(height, reward, pool_address, extra_nonce):
    # Coinbase transaction structure
    coinbase = {
        "version": 1,
        "inputs": [{
            "txid": "0" * 64,
            "vout": 0xffffffff,
            "scriptSig": f"{height:08x}{extra_nonce}",
            "sequence": 0xffffffff
        }],
        "outputs": [{
            "value": reward,
            "scriptPubKey": f"OP_DUP OP_HASH160 {pool_address} OP_EQUALVERIFY OP_CHECKSIG"
        }],
        "locktime": 0
    }
    return coinbase
```

### Step 4: Build Block Header

```python
import hashlib
import struct

def build_block_header(template, nonce, extra_nonce):
    # Block header structure (80 bytes for SHA-256/Ethash)
    header = b""
    header += struct.pack("<I", template['version'])
    header += bytes.fromhex(template['previousblockhash'])[::-1]
    header += calculate_merkle_root(template, extra_nonce)
    header += struct.pack("<I", template['curtime'])
    header += struct.pack("<I", int(template['bits'], 16))
    header += struct.pack("<I", nonce)
    return header
```

### Step 5: Mine Block

```python
def mine_block_sha256(template, pool_address):
    target = int(template['target'], 16)
    nonce = 0
    extra_nonce = 0
    
    while True:
        header = build_block_header(template, nonce, extra_nonce)
        hash_result = hashlib.sha256(hashlib.sha256(header).digest()).digest()
        hash_int = int.from_bytes(hash_result, 'little')
        
        if hash_int < target:
            # Found valid block!
            return submit_block(header, template)
        
        nonce += 1
        if nonce > 0xffffffff:
            nonce = 0
            extra_nonce += 1
```

### Step 6: Submit Block

```python
def submit_block(block_hex):
    result = node.rpc_call("submitblock", [block_hex])
    if result.get('error'):
        print(f"Block rejected: {result['error']['message']}")
        return False
    print("Block accepted!")
    return True
```

---

## Pool Payout System

### Recommended Payout Methods

#### 1. PPLNS (Pay Per Last N Shares)
- Fair for miners
- Discourages pool hopping
- Variance in payouts

#### 2. PPS (Pay Per Share)
- Predictable payouts
- Pool takes variance risk
- Higher pool fees

#### 3. PROP (Proportional)
- Simple to implement
- Vulnerable to pool hopping
- Lower fees

### Payout Calculation

```python
def calculate_payouts(shares, block_reward, pool_fee=0.01):
    total_shares = sum(shares.values())
    pool_cut = block_reward * pool_fee
    miner_reward = block_reward - pool_cut
    
    payouts = {}
    for miner, miner_shares in shares.items():
        payout = (miner_shares / total_shares) * miner_reward
        payouts[miner] = payout
    
    return payouts
```

### Send Payouts

```python
def send_payout(address, amount):
    result = node.rpc_call("sendtoaddress", [address, amount])
    return result['result']  # Returns transaction ID
```

---

## Difficulty Management

### Understanding Difficulty

GXC uses dynamic difficulty adjustment every 2,016 blocks (approximately 2 weeks).

**Difficulty Formula**:
```
new_difficulty = old_difficulty * (expected_time / actual_time)
```

**Limits**:
- Maximum increase: 4x per adjustment
- Maximum decrease: 1/4x per adjustment

### Pool Difficulty (Share Difficulty)

Pools should set share difficulty based on miner hashrate:

```python
def calculate_share_difficulty(hashrate):
    # Target: 1 share every 10 seconds
    target_time = 10  # seconds
    shares_per_second = hashrate / (2**32)  # For SHA-256
    difficulty = shares_per_second * target_time
    return max(1, difficulty)
```

---

## Fee Recommendations

### Network Fees

GXC uses dynamic fees based on network congestion:

**Get Recommended Fee**:
```python
fee_info = node.rpc_call("estimatefee")
recommended_fee = fee_info['result']['recommended_fee']
```

**Fee Tiers**:
- Low: 0.001 GXC (< 10 pending tx)
- Medium: 0.0015 GXC (10-49 pending tx)
- High: 0.002 GXC (50-99 pending tx)
- Very High: 0.003 GXC (100+ pending tx)

### Pool Fees

**Recommended Pool Fees**:
- PPLNS: 1-2%
- PPS: 3-5%
- PROP: 1-3%

---

## Monitoring and Statistics

### Essential Metrics

```python
def get_pool_stats():
    info = node.rpc_call("getmininginfo")['result']
    
    stats = {
        "network_hashrate": info['networkhashps'],
        "difficulty": info['difficulty'],
        "blocks": info['blocks'],
        "pending_tx": info['pooledtx'],
        "algorithm": info['algorithm']
    }
    return stats
```

### Block Notifications

Use WebSocket for real-time block notifications:

```python
import websocket

def on_new_block(ws, message):
    data = json.loads(message)
    if data['type'] == 'new_block':
        print(f"New block: {data['height']}")
        # Update mining work
        update_work()

ws = websocket.WebSocketApp(
    "ws://node-ip:8081",
    on_message=on_new_block
)
ws.run_forever()
```

---

## Security Best Practices

### 1. Node Security

- ✅ Use firewall to restrict RPC access
- ✅ Enable RPC authentication
- ✅ Use SSL/TLS for RPC connections
- ✅ Keep node software updated

### 2. Pool Security

- ✅ Validate all miner submissions
- ✅ Implement rate limiting
- ✅ Monitor for suspicious activity
- ✅ Use cold wallets for large balances

### 3. DDoS Protection

- ✅ Use DDoS protection service
- ✅ Implement connection limits
- ✅ Rate limit API requests
- ✅ Use multiple nodes for redundancy

---

## Troubleshooting

### Common Issues

**Issue**: "Block rejected: duplicate"
**Solution**: Block already submitted by another miner. Get new work.

**Issue**: "Block rejected: invalid"
**Solution**: Check block header construction and nonce calculation.

**Issue**: "Connection refused"
**Solution**: Check node is running and RPC port is accessible.

**Issue**: "Unauthorized"
**Solution**: Check RPC username/password in node configuration.

---

## Support and Resources

### Official Resources

- **Website**: https://gxc-blockchain.com
- **GitHub**: https://github.com/gxc-blockchain
- **Documentation**: https://docs.gxc-blockchain.com
- **Discord**: https://discord.gg/gxc-blockchain

### Pool Operator Support

For pool integration support:
- **Email**: pools@gxc-blockchain.com
- **Telegram**: @GXCPoolSupport
- **Discord**: #pool-operators channel

### Example Pool Implementations

- **Python**: https://github.com/gxc-blockchain/pool-python
- **Node.js**: https://github.com/gxc-blockchain/pool-nodejs
- **Go**: https://github.com/gxc-blockchain/pool-go

---

## Appendix: Algorithm Specifications

### SHA-256 Specification

**Hash Function**: Double SHA-256
**Block Header**: 80 bytes
**Nonce**: 32-bit integer
**Target**: 256-bit number

**Pseudocode**:
```
hash = SHA256(SHA256(block_header))
if hash < target:
    block_valid = true
```

### Ethash Specification

**Hash Function**: Ethash (Keccak-256 based)
**DAG Size**: ~2GB (epoch 0)
**Cache Size**: ~16MB (epoch 0)
**Nonce**: 64-bit integer

**Pseudocode**:
```
seed = Keccak256(header + nonce)
mix = generate_mix(seed, dag)
hash = Keccak256(seed + mix)
if hash < target:
    block_valid = true
```

### GXHash Specification

**Hash Function**: Argon2id
**Memory**: 64MB per hash
**Time Cost**: 3 iterations
**Parallelism**: 4 lanes
**Nonce**: 64-bit integer

**Pseudocode**:
```
hash = Argon2id(
    password=block_header + nonce,
    salt=previous_block_hash,
    memory=64MB,
    iterations=3,
    parallelism=4
)
if hash < target:
    block_valid = true
```

---

## License

This documentation is provided under MIT License.

Copyright (c) 2024 GXC Blockchain

Permission is hereby granted, free of charge, to any person obtaining a copy of this documentation...

---

**Last Updated**: December 10, 2024
**Version**: 2.0.0
**Status**: Production Ready ✅
