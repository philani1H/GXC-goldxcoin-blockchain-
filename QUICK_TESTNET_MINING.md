# Quick Testnet Mining Guide

## Testnet Block Reward

**Answer: 50.0 GXC per block** (for blocks < 1,051,200)

The testnet uses the same reward structure as mainnet:
- **Initial Reward**: 50.0 GXC
- **Halving Interval**: Every 1,051,200 blocks (~4 years)
- **Current Reward**: 50.0 GXC (since testnet is early)

## Quick Start: Deploy Testnet Node & Mine

### Step 1: Start Testnet Node

```bash
# Option A: If C++ node is built
./build/gxc-node --testnet --rpc-port=18332 --rest-port=18080

# Option B: Use Python node (if available)
python3 start_testnet_node.py
```

### Step 2: Start Mining with Your Address

```bash
# Start GUI miner
python3 mine_testnet_gui.py

# In the GUI:
# 1. Set wallet address: tGXC5066644d2aa70a7bf8947ee25af5e7c4e
# 2. Click "Start Mining"
# 3. Watch your balance increase by 50.0 GXC per block!
```

### Step 3: Verify Mining

```bash
# Check balance
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXC5066644d2aa70a7bf8947ee25af5e7c4e"],
    "id": 1
  }'

# Check latest block
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblock",
    "params": ["latest", true],
    "id": 1
  }'
```

## Expected Results

- **Block Reward**: 50.0 GXC per block
- **Your Address**: tGXC5066644d2aa70a7bf8947ee25af5e7c4e
- **Balance**: Increases by 50.0 GXC each time you mine a block
- **Coinbase Transaction**: Automatically created by the node

## Troubleshooting

If node won't start:
```bash
# Check if ports are free
netstat -tulpn | grep -E '18332|19333|18080'

# Try different ports
./build/gxc-node --testnet --rpc-port=18333
```

If mining doesn't work:
```bash
# Verify node is running
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

## Summary

✅ **Testnet Reward**: 50.0 GXC per block  
✅ **Your Address**: tGXC5066644d2aa70a7bf8947ee25af5e7c4e  
✅ **Testnet Ports**: RPC 18332, Network 19333, REST 18080  
✅ **Mining**: Use `mine_testnet_gui.py`  

Happy mining! 🚀⛏️
