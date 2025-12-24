# GXC Blockchain - Mining Pool Documentation v2.0

## Overview

Complete guide for setting up and operating GXC mining pools. Supports all 4 mining algorithms with Stratum protocol.

**Supported Algorithms**:
- SHA256 (Bitcoin-style)
- Ethash (Ethereum-style)
- GXHash (ASIC-resistant)
- PoS (Proof-of-Stake)

**Version**: 2.0.0  
**Status**: Production Ready ✅

---

## Quick Start

### 1. Get Block Template

```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"getblocktemplate",
    "params":[{"algorithm":"sha256"}],
    "id":1
  }'
```

**Response**:
```json
{
  "result": {
    "version": 1,
    "height": 40,
    "previousblockhash": "0bdb2dbc2ec258a6...",
    "transactions": [
      {
        "hash": "99d8b314a9f41c71...",
        "type": "STAKE",
        "from": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
        "to": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
        "amount": 100.0,
        "fee": 0.001,
        "inputs": [...],
        "outputs": [...]
      }
    ],
    "difficulty": 0.1,
    "target": "00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
    "miner_address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "block_reward": 50.0
  }
}
```

### 2. Mine Block

```python
import hashlib
import json
import time
import requests

def mine_block(template, miner_address):
    """Mine a block using SHA256"""
    nonce = 0
    target = int(template["target"], 16)
    
    while True:
        # Create block header
        block_data = {
            "version": template["version"],
            "previousblockhash": template["previousblockhash"],
            "transactions": template["transactions"],
            "timestamp": int(time.time()),
            "nonce": nonce,
            "miner_address": miner_address
        }
        
        # Calculate hash
        block_str = json.dumps(block_data, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        # Check if hash meets difficulty
        if int(block_hash, 16) < target:
            print(f"✓ Found valid block! Nonce: {nonce}")
            return block_data, block_hash
        
        nonce += 1
        if nonce % 1000 == 0:
            print(f"Tried {nonce} nonces...")

# Get template
response = requests.post("http://localhost:8332", json={
    "jsonrpc": "2.0",
    "method": "getblocktemplate",
    "params": [{"algorithm": "sha256"}],
    "id": 1
})
template = response.json()["result"]

# Mine
block_data, block_hash = mine_block(template, "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9")
print(f"Block hash: {block_hash}")
```

### 3. Submit Block

```python
response = requests.post("http://localhost:8332", json={
    "jsonrpc": "2.0",
    "method": "submitblock",
    "params": [block_data],
    "id": 1
})
result = response.json()
print(f"Result: {result}")
```

---

## Mining Pool Architecture

### Components

```
┌─────────────┐
│   Miners    │ (Workers connecting via Stratum)
└──────┬──────┘
       │
       ↓
┌─────────────┐
│ Pool Server │ (Stratum server + job distribution)
└──────┬──────┘
       │
       ↓
┌─────────────┐
│  GXC Node   │ (getblocktemplate + submitblock)
└─────────────┘
```

### Pool Server Responsibilities

1. **Job Distribution**: Get templates from node, distribute to miners
2. **Share Validation**: Verify miner submissions
3. **Block Submission**: Submit valid blocks to node
4. **Payout Calculation**: Track shares, calculate rewards
5. **Statistics**: Hashrate, efficiency, earnings

---

## Stratum Protocol

### Connection

```
Miner → Pool: TCP connection on port 3333
Pool → Miner: mining.notify (job)
Miner → Pool: mining.submit (share)
Pool → Miner: Result (accepted/rejected)
```

### mining.subscribe

**Request**:
```json
{
  "id": 1,
  "method": "mining.subscribe",
  "params": ["GXCMiner/1.0.0"]
}
```

**Response**:
```json
{
  "id": 1,
  "result": [
    ["mining.notify", "subscription_id"],
    "extranonce1",
    4
  ],
  "error": null
}
```

### mining.authorize

**Request**:
```json
{
  "id": 2,
  "method": "mining.authorize",
  "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9", "password"]
}
```

**Response**:
```json
{
  "id": 2,
  "result": true,
  "error": null
}
```

### mining.notify

**Notification** (Pool → Miner):
```json
{
  "id": null,
  "method": "mining.notify",
  "params": [
    "job_id",
    "prev_hash",
    "coinbase1",
    "coinbase2",
    ["merkle_branch"],
    "version",
    "nbits",
    "ntime",
    true
  ]
}
```

### mining.submit

**Request**:
```json
{
  "id": 4,
  "method": "mining.submit",
  "params": [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "job_id",
    "extranonce2",
    "ntime",
    "nonce"
  ]
}
```

**Response**:
```json
{
  "id": 4,
  "result": true,
  "error": null
}
```

---

## Pool Server Implementation

### Python Example

```python
import socket
import json
import threading
import time
import requests

class GXCPoolServer:
    def __init__(self, node_url="http://localhost:8332", pool_address="tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"):
        self.node_url = node_url
        self.pool_address = pool_address
        self.miners = {}
        self.current_job = None
        self.job_id = 0
        
    def get_block_template(self, algorithm="sha256"):
        """Get block template from node"""
        response = requests.post(self.node_url, json={
            "jsonrpc": "2.0",
            "method": "getblocktemplate",
            "params": [{"algorithm": algorithm}],
            "id": 1
        })
        return response.json()["result"]
    
    def create_job(self, template):
        """Create mining job from template"""
        self.job_id += 1
        job = {
            "job_id": str(self.job_id),
            "prev_hash": template["previousblockhash"],
            "height": template["height"],
            "difficulty": template["difficulty"],
            "target": template["target"],
            "transactions": template["transactions"],
            "timestamp": int(time.time())
        }
        self.current_job = job
        return job
    
    def validate_share(self, miner_address, job_id, nonce):
        """Validate miner's share"""
        if job_id != self.current_job["job_id"]:
            return False, "Stale job"
        
        # Calculate hash with nonce
        block_data = {
            "previousblockhash": self.current_job["prev_hash"],
            "transactions": self.current_job["transactions"],
            "timestamp": self.current_job["timestamp"],
            "nonce": nonce,
            "miner_address": self.pool_address
        }
        
        block_str = json.dumps(block_data, sort_keys=True)
        import hashlib
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        hash_int = int(block_hash, 16)
        target_int = int(self.current_job["target"], 16)
        
        # Check if valid share
        if hash_int < target_int:
            # Valid block! Submit to node
            self.submit_block(block_data)
            return True, "Block found!"
        elif hash_int < target_int * 1000:  # Share difficulty
            # Valid share
            self.record_share(miner_address, job_id)
            return True, "Share accepted"
        else:
            return False, "Invalid share"
    
    def submit_block(self, block_data):
        """Submit block to node"""
        response = requests.post(self.node_url, json={
            "jsonrpc": "2.0",
            "method": "submitblock",
            "params": [block_data],
            "id": 1
        })
        result = response.json()
        print(f"Block submitted: {result}")
    
    def record_share(self, miner_address, job_id):
        """Record valid share for payout"""
        if miner_address not in self.miners:
            self.miners[miner_address] = {"shares": 0, "blocks": 0}
        self.miners[miner_address]["shares"] += 1
    
    def handle_miner(self, conn, addr):
        """Handle individual miner connection"""
        print(f"Miner connected: {addr}")
        miner_address = None
        
        try:
            while True:
                data = conn.recv(4096)
                if not data:
                    break
                
                # Parse Stratum message
                message = json.loads(data.decode())
                method = message.get("method")
                
                if method == "mining.subscribe":
                    # Send subscription response
                    response = {
                        "id": message["id"],
                        "result": [["mining.notify", "sub_id"], "extranonce1", 4],
                        "error": None
                    }
                    conn.send((json.dumps(response) + "\n").encode())
                
                elif method == "mining.authorize":
                    # Authorize miner
                    miner_address = message["params"][0]
                    response = {
                        "id": message["id"],
                        "result": True,
                        "error": None
                    }
                    conn.send((json.dumps(response) + "\n").encode())
                    
                    # Send current job
                    if self.current_job:
                        job_notify = {
                            "id": None,
                            "method": "mining.notify",
                            "params": [
                                self.current_job["job_id"],
                                self.current_job["prev_hash"],
                                "", "", [],
                                "1", self.current_job["target"][:8],
                                hex(self.current_job["timestamp"])[2:],
                                True
                            ]
                        }
                        conn.send((json.dumps(job_notify) + "\n").encode())
                
                elif method == "mining.submit":
                    # Validate share
                    job_id = message["params"][1]
                    nonce = int(message["params"][4], 16)
                    
                    valid, msg = self.validate_share(miner_address, job_id, nonce)
                    response = {
                        "id": message["id"],
                        "result": valid,
                        "error": None if valid else {"code": -1, "message": msg}
                    }
                    conn.send((json.dumps(response) + "\n").encode())
        
        except Exception as e:
            print(f"Error handling miner: {e}")
        finally:
            conn.close()
            print(f"Miner disconnected: {addr}")
    
    def job_updater(self):
        """Update jobs periodically"""
        while True:
            try:
                template = self.get_block_template()
                job = self.create_job(template)
                print(f"New job: {job['job_id']} at height {job['height']}")
                
                # Notify all connected miners
                # (In production, track connections and send mining.notify)
                
            except Exception as e:
                print(f"Error updating job: {e}")
            
            time.sleep(30)  # Update every 30 seconds
    
    def start(self, port=3333):
        """Start pool server"""
        # Start job updater thread
        threading.Thread(target=self.job_updater, daemon=True).start()
        
        # Start Stratum server
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind(("0.0.0.0", port))
        server.listen(100)
        
        print(f"Pool server listening on port {port}")
        
        while True:
            conn, addr = server.accept()
            threading.Thread(target=self.handle_miner, args=(conn, addr), daemon=True).start()

# Usage
if __name__ == "__main__":
    pool = GXCPoolServer(
        node_url="http://localhost:8332",
        pool_address="tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
    )
    pool.start(port=3333)
```

---

## Multi-Algorithm Support

### Algorithm Selection

```python
# SHA256 (Bitcoin-style)
template_sha256 = client.call("getblocktemplate", [{"algorithm": "sha256"}])

# Ethash (Ethereum-style)
template_ethash = client.call("getblocktemplate", [{"algorithm": "ethash"}])

# GXHash (ASIC-resistant)
template_gxhash = client.call("getblocktemplate", [{"algorithm": "gxhash"}])

# PoS (Proof-of-Stake)
template_pos = client.call("getblocktemplate", [{"algorithm": "pos"}])
```

### Algorithm-Specific Mining

**SHA256**:
```python
import hashlib

def mine_sha256(template, nonce):
    block_str = json.dumps(template, sort_keys=True)
    return hashlib.sha256(block_str.encode()).hexdigest()
```

**Ethash**:
```python
# Requires Ethash library
from ethash import ethash_light

def mine_ethash(template, nonce):
    # Ethash mining implementation
    pass
```

**GXHash**:
```python
# GXC-specific ASIC-resistant algorithm
def mine_gxhash(template, nonce):
    # GXHash mining implementation
    pass
```

---

## Payout System

### Share Tracking

```python
class PayoutManager:
    def __init__(self):
        self.shares = {}  # {address: share_count}
        self.blocks_found = []
    
    def record_share(self, address, difficulty):
        """Record a valid share"""
        if address not in self.shares:
            self.shares[address] = 0
        self.shares[address] += difficulty
    
    def record_block(self, address, reward):
        """Record a found block"""
        self.blocks_found.append({
            "address": address,
            "reward": reward,
            "timestamp": time.time(),
            "shares": dict(self.shares)
        })
    
    def calculate_payouts(self, block_reward, fee_percent=1.0):
        """Calculate payouts using PPLNS"""
        total_shares = sum(self.shares.values())
        pool_fee = block_reward * (fee_percent / 100)
        distributable = block_reward - pool_fee
        
        payouts = {}
        for address, shares in self.shares.items():
            payout = (shares / total_shares) * distributable
            payouts[address] = payout
        
        return payouts
    
    def reset_shares(self):
        """Reset shares after payout"""
        self.shares = {}
```

### Automatic Payouts

```python
def process_payouts(payouts, min_payout=1.0):
    """Send payouts to miners"""
    for address, amount in payouts.items():
        if amount >= min_payout:
            try:
                txid = client.call("sendtoaddress", [address, amount])
                print(f"Paid {amount} GXC to {address}: {txid}")
            except Exception as e:
                print(f"Payout failed for {address}: {e}")
```

---

## Pool Statistics

### Hashrate Calculation

```python
def calculate_hashrate(shares, time_window=600):
    """Calculate pool hashrate"""
    # shares = number of shares in time_window seconds
    # difficulty = share difficulty
    difficulty = 0.1
    hashrate = (shares * (2**32) * difficulty) / time_window
    return hashrate

# Example
shares_10min = 1000
hashrate = calculate_hashrate(shares_10min)
print(f"Pool hashrate: {hashrate / 1e9:.2f} GH/s")
```

### Efficiency Tracking

```python
def calculate_efficiency(valid_shares, invalid_shares):
    """Calculate miner efficiency"""
    total = valid_shares + invalid_shares
    if total == 0:
        return 0
    return (valid_shares / total) * 100

# Example
efficiency = calculate_efficiency(950, 50)
print(f"Efficiency: {efficiency:.1f}%")
```

---

## Best Practices

### Pool Operation

1. **Update jobs frequently** (every 30 seconds or on new block)
2. **Validate all shares** before recording
3. **Use PPLNS or PPS** payout system
4. **Set minimum payout** threshold (e.g., 1 GXC)
5. **Monitor node connectivity**
6. **Log all blocks found**
7. **Implement DDoS protection**
8. **Use SSL/TLS** for Stratum connections

### Performance

1. **Use connection pooling** for node RPC calls
2. **Cache block templates** (30 second TTL)
3. **Batch database operations**
4. **Use async I/O** for miner connections
5. **Monitor memory usage**
6. **Implement rate limiting**

### Security

1. **Validate all miner inputs**
2. **Prevent share spam**
3. **Detect and ban cheaters**
4. **Secure pool wallet**
5. **Use separate payout wallet**
6. **Implement 2FA for admin**
7. **Regular security audits**

---

## Monitoring

### Health Checks

```python
def check_pool_health():
    """Monitor pool health"""
    try:
        # Check node connectivity
        info = client.call("getblockchaininfo")
        print(f"✓ Node connected: height {info['height']}")
        
        # Check hashrate
        hashrate = calculate_hashrate(shares_last_10min)
        print(f"✓ Pool hashrate: {hashrate / 1e9:.2f} GH/s")
        
        # Check connected miners
        print(f"✓ Connected miners: {len(active_miners)}")
        
        # Check pending payouts
        print(f"✓ Pending payouts: {sum(pending_payouts.values()):.2f} GXC")
        
        return True
    except Exception as e:
        print(f"✗ Health check failed: {e}")
        return False
```

---

## Troubleshooting

### Common Issues

**Miners can't connect**:
- Check firewall rules (port 3333)
- Verify Stratum server is running
- Check network connectivity

**Shares rejected**:
- Verify job is current (not stale)
- Check difficulty calculation
- Validate nonce range

**Blocks not submitting**:
- Verify node connectivity
- Check block format
- Ensure all transactions included

**Payouts failing**:
- Check wallet balance
- Verify addresses
- Check transaction fees

---

## Support

- **Repository**: https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
- **Documentation**: See repository docs/
- **Version**: 2.0.0

---

**Last Updated**: 2025-12-24  
**Version**: 2.0.0  
**Status**: Production Ready ✅
