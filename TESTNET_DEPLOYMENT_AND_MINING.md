# Testnet Deployment and Mining Guide

## Testnet Block Reward

**Initial Reward: 50.0 GXC per block**
- Same as mainnet
- Halving occurs every 1,051,200 blocks (~4 years)
- For early testnet blocks (height < 1,051,200), reward is **50.0 GXC**
- After first halving, reward becomes **25.0 GXC**
- Minimum reward: 0.00000001 GXC

**Current Reward Calculation:**
```cpp
// For block height < 1,051,200: 50.0 GXC
// For block height >= 1,051,200: 25.0 GXC
// And so on...
```

## Step 1: Deploy Testnet Node

### Option A: Local Testnet Node

```bash
# 1. Build the node (if not already built)
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node

# 2. Create testnet configuration
mkdir -p ~/.gxc-testnet
cat > ~/.gxc-testnet/gxc-testnet.conf << EOF
network_port=19333
rpc_port=18332
rest_port=18080
data_dir=./testnet_data
testnet=true
log_level=INFO
log_file=testnet.log
EOF

# 3. Start testnet node
cd /workspace
./build/gxc-node --testnet --config=~/.gxc-testnet/gxc-testnet.conf

# Or with explicit ports:
./build/gxc-node \
  --testnet \
  --network-port=19333 \
  --rpc-port=18332 \
  --rest-port=18080 \
  --data-dir=./testnet_data
```

### Option B: Using Python Testnet Node (If C++ not built)

```bash
# Start Python testnet node (if available)
python3 start_testnet_node.py
```

### Verify Node is Running

```bash
# Test RPC endpoint
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'

# Expected response:
# {
#   "result": {
#     "chain": "testnet",
#     "blocks": 0,
#     "height": 0,
#     "block_reward": 50.0,
#     ...
#   }
# }
```

## Step 2: Test Mining with Your Address

### Your Mining Address
```
tGXC5066644d2aa70a7bf8947ee25af5e7c4e
```

### Option A: Use GUI Testnet Miner

```bash
# Start GUI miner
python3 mine_testnet_gui.py

# In the GUI:
# 1. Set wallet address: tGXC5066644d2aa70a7bf8947ee25af5e7c4e
# 2. Set RPC URL: http://localhost:18332/rpc
# 3. Click "Start Mining"
```

### Option B: Use CLI Testnet Miner

```bash
# Start CLI miner
python3 mine_testnet.py \
  --address tGXC5066644d2aa70a7bf8947ee25af5e7c4e \
  --rpc-url http://localhost:18332/rpc
```

### Option C: Manual Mining Test (Python Script)

Create `test_mining.py`:

```python
#!/usr/bin/env python3
import requests
import hashlib
import time
import json

RPC_URL = "http://localhost:18332/rpc"
MINER_ADDRESS = "tGXC5066644d2aa70a7bf8947ee25af5e7c4e"

def rpc_call(method, params=None):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    response = requests.post(RPC_URL, json=payload, timeout=10)
    return response.json().get("result")

def mine_block():
    # Get block template
    template = rpc_call("getblocktemplate", [{"algorithm": "gxhash"}])
    if not template:
        print("❌ Failed to get block template")
        return False
    
    print(f"📦 Block Template:")
    print(f"   Height: {template.get('height', 'N/A')}")
    print(f"   Previous Hash: {template.get('previousblockhash', 'N/A')[:16]}...")
    print(f"   Difficulty: {template.get('difficulty', 'N/A')}")
    print(f"   Block Reward: {template.get('block_reward', template.get('reward', 'N/A'))} GXC")
    
    # Mine block (simplified - just find a hash)
    target = template.get("target", "0" * 64)
    nonce = 0
    max_nonce = 1000000
    
    print(f"⛏️  Mining block...")
    start_time = time.time()
    
    while nonce < max_nonce:
        # Create block data
        block_data = {
            "height": template.get("height", 0),
            "previousHash": template.get("previousblockhash", ""),
            "timestamp": int(time.time()),
            "nonce": nonce,
            "minerAddress": MINER_ADDRESS,
            "difficulty": template.get("difficulty", 1000),
            "transactions": []
        }
        
        # Calculate hash
        block_str = json.dumps(block_data, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        # Check if hash meets difficulty
        if block_hash < target or block_hash.startswith("0" * 4):
            print(f"✅ Found valid hash: {block_hash[:16]}...")
            
            # Submit block
            result = rpc_call("submitblock", [block_data])
            if result is None:
                print(f"🎉 Block submitted successfully!")
                
                # Check balance
                time.sleep(2)  # Wait for block to be processed
                balance = rpc_call("getbalance", [MINER_ADDRESS])
                print(f"💰 Balance: {balance} GXC")
                return True
            else:
                print(f"❌ Block submission failed: {result}")
                return False
        
        nonce += 1
        
        if nonce % 10000 == 0:
            elapsed = time.time() - start_time
            hashrate = nonce / elapsed if elapsed > 0 else 0
            print(f"   Hashes: {nonce}, Rate: {hashrate:.0f} H/s", end="\r")
    
    print(f"\n❌ Failed to find valid hash after {max_nonce} attempts")
    return False

if __name__ == "__main__":
    print(f"🚀 GXC Testnet Miner")
    print(f"📍 Address: {MINER_ADDRESS}")
    print(f"📍 RPC URL: {RPC_URL}")
    print()
    
    # Test connection
    info = rpc_call("getblockchaininfo")
    if info:
        print(f"✅ Connected to testnet node")
        print(f"   Chain: {info.get('chain', 'N/A')}")
        print(f"   Height: {info.get('height', 0)}")
        print(f"   Block Reward: {info.get('block_reward', 50.0)} GXC")
        print()
    else:
        print("❌ Failed to connect to testnet node")
        print("   Make sure the node is running on port 18332")
        exit(1)
    
    # Mine blocks
    while True:
        mine_block()
        time.sleep(1)
```

Run it:
```bash
python3 test_mining.py
```

## Step 3: Verify Mining Results

### Check Balance

```bash
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXC5066644d2aa70a7bf8947ee25af5e7c4e"],
    "id": 1
  }'
```

### Check Latest Blocks

```bash
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblock",
    "params": ["latest", true],
    "id": 1
  }'
```

### Check Transactions

```bash
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listtransactions",
    "params": ["tGXC5066644d2aa70a7bf8947ee25af5e7c4e"],
    "id": 1
  }'
```

## Step 4: Monitor Mining

### Real-time Monitoring Script

Create `monitor_mining.py`:

```python
#!/usr/bin/env python3
import requests
import time
import json

RPC_URL = "http://localhost:18332/rpc"
MINER_ADDRESS = "tGXC5066644d2aa70a7bf8947ee25af5e7c4e"

def rpc_call(method, params=None):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    try:
        response = requests.post(RPC_URL, json=payload, timeout=5)
        return response.json().get("result")
    except:
        return None

while True:
    # Get blockchain info
    info = rpc_call("getblockchaininfo")
    if info:
        height = info.get("height", 0)
        reward = info.get("block_reward", 50.0)
        
        # Get balance
        balance = rpc_call("getbalance", [MINER_ADDRESS]) or 0.0
        
        # Get latest block
        latest_block = rpc_call("getblock", ["latest", True])
        miner = latest_block.get("miner", "N/A") if latest_block else "N/A"
        
        print(f"\r📊 Height: {height} | Reward: {reward} GXC | Balance: {balance} GXC | Latest Miner: {miner[:16]}...", end="")
    
    time.sleep(5)
```

Run it:
```bash
python3 monitor_mining.py
```

## Expected Results

### When You Mine a Block:

1. **Block Reward**: 50.0 GXC (for blocks < 1,051,200)
2. **Coinbase Transaction**: Automatically created by the node
3. **Balance Update**: Your balance should increase by 50.0 GXC
4. **Block Height**: Increases by 1
5. **Transaction Hash**: You'll see a coinbase transaction in your address

### Example Output:

```
🎉 BLOCK MINED! Height: 1
💰 Expected reward: 50.0 GXC (will verify from blockchain)
⏳ Waiting for blockchain confirmation...
💰 Balance from blockchain: 50.00000000 GXC
🔗 Block Explorer: http://localhost:3000/block/1
🔗 Transaction Explorer: http://localhost:3000/tx/abc123...
```

## Troubleshooting

### Node Not Starting

```bash
# Check if ports are in use
netstat -tulpn | grep -E '18332|19333|18080'

# Check logs
tail -f testnet.log

# Try different ports
./build/gxc-node --testnet --rpc-port=18333 --network-port=19334
```

### Mining Not Working

```bash
# Verify node is running
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# Check block template
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblocktemplate","params":[{"algorithm":"gxhash"}],"id":1}'
```

### Balance Not Updating

```bash
# Wait a few seconds after mining
# Check transactions
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listtransactions",
    "params": ["tGXC5066644d2aa70a7bf8947ee25af5e7c4e"],
    "id": 1
  }'
```

## Quick Start Commands

```bash
# 1. Start testnet node
./build/gxc-node --testnet --rpc-port=18332 --rest-port=18080

# 2. In another terminal, start GUI miner
python3 mine_testnet_gui.py

# 3. Set address to: tGXC5066644d2aa70a7bf8947ee25af5e7c4e
# 4. Click "Start Mining"
# 5. Watch your balance increase by 50.0 GXC per block!
```

## Summary

- **Testnet Reward**: 50.0 GXC per block (same as mainnet)
- **Your Address**: tGXC5066644d2aa70a7bf8947ee25af5e7c4e
- **Testnet Ports**: RPC 18332, Network 19333, REST 18080
- **Mining**: Use `mine_testnet_gui.py` or create custom script
- **Verification**: Check balance and transactions via RPC

Happy mining! 🚀⛏️
