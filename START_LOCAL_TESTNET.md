# 🚀 Start Local Testnet Node - Quick Guide

## ⚠️ Railway Node Issue

Your Railway node is timing out. This is normal - Railway nodes can be slow or restarting.

## ✅ Solution: Use Local Testnet Node

### Step 1: Start Local Testnet Node

**Windows (PowerShell):**
```powershell
# Navigate to project directory
cd C:\Users\phila\GXC-goldxcoin-blockchain-

# Activate virtual environment (if not already)
.\.venv\Scripts\Activate.ps1

# Start testnet node
python testnet_blockchain_node.py
```

**Keep this terminal open!** The node needs to keep running.

### Step 2: Start Mining (New Terminal)

**Open a NEW PowerShell window:**

```powershell
# Navigate to project directory
cd C:\Users\phila\GXC-goldxcoin-blockchain-

# Activate virtual environment
.\.venv\Scripts\Activate.ps1

# Set local node URL
$env:BLOCKCHAIN_RPC_URL="http://localhost:18332"

# Start mining
python mine_testnet_fixed.py
```

## 🎯 What You'll See

### Terminal 1 (Node):
```
============================================================
GXC Testnet Blockchain Node
============================================================
RPC Port: 18332
Data Directory: ./gxc_testnet_data
Block Time: 60 seconds
Initial Difficulty: 0.1
============================================================

[BLOCKCHAIN] RPC server started on http://127.0.0.1:18332
[BLOCKCHAIN] Current height: 0
Ready to accept RPC requests from mining pools!
```

### Terminal 2 (Miner):
```
======================================================================
🪙 GXC TESTNET MINER - FIXED VERSION
======================================================================

Mining to: tGXC9fab7317231b966af85ac453e168c0932
RPC: http://localhost:18332

Press Ctrl+C to stop

======================================================================

Checking blockchain connection...
✅ Connected! Current height: 0

⛏️  Mining block to: tGXC9fab7317231b966af85ac453e168c0932
   Height: 1
   Mining...
```

## 🔧 Alternative: Update Miner to Auto-Fallback

The miner now automatically tries localhost if Railway times out!

Just run:
```powershell
python mine_testnet_fixed.py
```

It will:
1. Try Railway first
2. If timeout, automatically try localhost:18332
3. Connect to whichever is available

## ✅ Benefits of Local Node

- ⚡ **Faster** - No network latency
- 🔒 **More reliable** - No Railway timeouts
- 🎮 **Better for testing** - Instant feedback
- 💰 **Same rewards** - Still testnet coins

## 📝 Quick Commands

**Start Node:**
```powershell
python testnet_blockchain_node.py
```

**Start Mining (with local fallback):**
```powershell
python mine_testnet_fixed.py
```

**Or explicitly use local:**
```powershell
$env:BLOCKCHAIN_RPC_URL="http://localhost:18332"
python mine_testnet_fixed.py
```

## 🎯 You're Ready!

Your miner will now:
- ✅ Try Railway first
- ✅ Auto-fallback to local node
- ✅ Mine to your address: `tGXC9fab7317231b966af85ac453e168c0932`

Happy mining! ⛏️
