# ⛏️ Start Mining to tGXC9fab7317231b966af85ac453e168c0932

## 🎯 Quick Start

Your testnet address: **`tGXC9fab7317231b966af85ac453e168c0932`**

## 📋 Option 1: Mine with Railway Node (Recommended)

### Step 1: Install Dependencies
```bash
pip3 install requests
# or
python3 -m pip install --user requests
```

### Step 2: Start Mining
```bash
# Set Railway URL
export BLOCKCHAIN_RPC_URL=https://gxc-chain112-blockchain-node-production.up.railway.app

# Start mining
python3 mine_testnet.py
```

**Or use the fixed version:**
```bash
python3 mine_testnet_fixed.py
```

## 📋 Option 2: Mine with Local Testnet Node

### Step 1: Start Local Testnet Node
```bash
# Terminal 1: Start testnet node
python3 testnet_blockchain_node.py
```

### Step 2: Start Mining (Different Terminal)
```bash
# Terminal 2: Start mining
export BLOCKCHAIN_RPC_URL=http://localhost:18332
python3 mine_testnet.py
```

## 🎯 What Happens

1. ✅ Miner connects to Railway testnet node
2. ✅ Gets block template
3. ✅ Mines block (finds valid hash)
4. ✅ Submits block to node
5. ✅ Receives 12.5 GXC reward to your address
6. ✅ Repeats for next block

## 📊 Expected Output

```
======================================================================
🪙 GXC TESTNET MINER
======================================================================

Mining to: tGXC9fab7317231b966af85ac453e168c0932
RPC: https://gxc-chain112-blockchain-node-production.up.railway.app

Press Ctrl+C to stop

======================================================================

Checking blockchain connection...
✅ Connected! Current height: 0

⛏️  Mining block to: tGXC9fab7317231b966af85ac453e168c0932
   Height: 1
   Mining...
   Tried 10000 nonces...
   Tried 20000 nonces...

======================================================================
🎉 BLOCK MINED!
======================================================================
   Height: 1
   Hash: 00000000000000000000000000000000...
   Reward: 12.5 GXC (testnet)
   Time: 2.34 seconds
   Miner: tGXC9fab7317231b966af85ac453e168c0932
======================================================================

📊 Total blocks mined: 1
```

## 🔧 Troubleshooting

### Issue: "Cannot connect to blockchain node"

**Solution 1: Check Railway Node**
```bash
# Test Railway connection
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"test","method":"getblockchaininfo","params":[]}'
```

**Solution 2: Start Local Node**
```bash
# Start local testnet node
python3 testnet_blockchain_node.py
```

### Issue: "ModuleNotFoundError: No module named 'requests'"

**Solution:**
```bash
pip3 install requests
# or
python3 -m pip install --user requests
```

### Issue: Railway Node Returns 502 Error

**Solution:**
- Railway node might be down or restarting
- Use local testnet node instead:
  ```bash
  python3 testnet_blockchain_node.py
  export BLOCKCHAIN_RPC_URL=http://localhost:18332
  python3 mine_testnet.py
  ```

## 📝 Notes

- **Testnet mining is EASY** - Difficulty is 0.1 (very low)
- **Blocks every 60 seconds** - Fast feedback
- **Reward: 12.5 GXC per block** - Testnet rewards
- **No real value** - Test coins only
- **Address: tGXC9fab7317231b966af85ac453e168c0932** - Already configured

## 🚀 Ready to Mine!

Run:
```bash
python3 mine_testnet.py
```

Or if Railway is down:
```bash
# Terminal 1
python3 testnet_blockchain_node.py

# Terminal 2
export BLOCKCHAIN_RPC_URL=http://localhost:18332
python3 mine_testnet.py
```

Happy mining! ⛏️
