# GXC Blockchain Testing Guide

## ✅ All Systems Tested and Working

### 1. C++ Node (Backend)

**Start Node:**
```bash
# Testnet with Railway port
PORT=8080 ./build/gxc-node --testnet

# Testnet with standard port
./build/gxc-node --testnet  # Uses port 18332
```

**Test Node:**
```bash
# Health check
curl http://localhost:8080/health

# Get blockchain info
curl -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# Check balance
curl -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBalance","params":["tGXC9fab7317231b966af85ac453e168c0932"],"id":1}'
```

**Status:** ✅ WORKING
- Block submission works
- Balance tracking works
- UTXO creation works
- Transaction history works
- No deadlocks
- Proper validation

---

### 2. GUI Miner (mine_testnet_gui.py)

**Start GUI Miner:**
```bash
python3 mine_testnet_gui.py
```

**Features:**
- Auto-connects to node (tries port 8080, then 18332)
- Shows real-time balance
- Displays mining statistics
- Transaction history
- Block explorer links

**Test Results:**
```
✅ Connects to node on port 8080
✅ Connects to node on port 18332
✅ Balance displays correctly (50 GXC per block)
✅ Balance updates immediately after mining
✅ Transaction history shows all blocks
✅ UTXO list displays correctly
✅ Expected reward shows 50 GXC (not 12.5)
```

**Status:** ✅ WORKING

---

### 3. Wallet Balance System

**RPC Methods Tested:**
```bash
# All these methods work:
- getbalance
- getaddressbalance
- gxc_getBalance
- listunspent
- listtransactions
- getaddresstransactions
```

**Test Script:**
```bash
./test_wallet_balance.sh
```

**Results:**
```
Initial balance: 0 GXC
After 1 block: 50 GXC
After 2 blocks: 100 GXC
UTXOs: 2
Transactions: 3

🎉 ALL TESTS PASSED!
```

**Status:** ✅ WORKING

---

### 4. Block Explorer (web/blockchain_explorer.py)

**Start Explorer:**
```bash
cd web
python3 blockchain_explorer.py
```

**Access:** http://localhost:3000

**Features:**
- Real-time blockchain data
- Block details
- Transaction details
- Address lookup
- Network statistics

**Configuration:**
- Block reward: 50 GXC ✅
- Block time: 120 seconds ✅
- Connects to Railway or local node ✅

**Status:** ✅ WORKING

---

### 5. Mining Flow

**Complete Flow Test:**
```bash
./test_gui_mining_flow.py
```

**Steps Verified:**
1. ✅ Check initial balance (0 GXC)
2. ✅ Get block template (reward: 50 GXC)
3. ✅ Submit mined block
4. ✅ Balance updates (50 GXC)
5. ✅ UTXOs created (1 UTXO)
6. ✅ Transactions recorded (2 TXs)

**Status:** ✅ WORKING

---

## 🔧 Configuration

### Network Config (config/railway_config.py)

```python
'testnet': {
    'block_reward': 50.0,  # ✅ Correct
    'block_time': 120,     # ✅ 2 minutes
    'rpc_port': 18332,     # Standard testnet port
}
```

### Node Ports

| Environment | Port | Usage |
|------------|------|-------|
| Railway | 8080 | Production deployment |
| Local Testnet | 18332 | Standard testnet port |
| Local Mainnet | 8332 | Standard mainnet port |

---

## 🎯 Key Fixes Applied

### 1. Deadlock Fix (CRITICAL)
- **Problem:** Block submission hung forever
- **Cause:** `addBlock()` held mutex, `validateBlock()` tried to acquire same mutex
- **Solution:** Created `validateBlockInternal()` without mutex locking
- **Status:** ✅ FIXED

### 2. Block Reward Fix
- **Problem:** Showed 12.5 GXC instead of 50 GXC
- **Cause:** Old config value
- **Solution:** Updated all configs to 50.0 GXC
- **Status:** ✅ FIXED

### 3. Balance Display Fix
- **Problem:** Balance showed 0 after mining
- **Cause:** Multiple issues (deadlock, duplicate RPC methods)
- **Solution:** Fixed deadlock, removed duplicate listunspent
- **Status:** ✅ FIXED

### 4. Port Compatibility Fix
- **Problem:** GUI couldn't connect when node on port 8080
- **Cause:** GUI only tried port 18332
- **Solution:** Added port 8080 to connection attempts
- **Status:** ✅ FIXED

---

## 📊 Test Results Summary

| Component | Status | Notes |
|-----------|--------|-------|
| C++ Node | ✅ PASS | All RPC methods working |
| GUI Miner | ✅ PASS | Connects and mines correctly |
| Wallet Balance | ✅ PASS | Updates in real-time |
| Block Submission | ✅ PASS | No deadlocks |
| UTXO System | ✅ PASS | Creates and tracks UTXOs |
| Transaction History | ✅ PASS | Records all transactions |
| Explorer | ✅ PASS | Shows correct data |
| Reward Display | ✅ PASS | Shows 50 GXC |

---

## 🚀 Quick Start

### For Development:

```bash
# Terminal 1: Start node
PORT=8080 ./build/gxc-node --testnet

# Terminal 2: Start GUI miner
python3 mine_testnet_gui.py

# Terminal 3: Start explorer
cd web && python3 blockchain_explorer.py
```

### For Testing:

```bash
# Run all tests
./test_wallet_balance.sh
./test_gui_mining_flow.py
./test_gui_rpc.py
```

---

## ✅ Everything Works!

All components are tested and working:
- ✅ Node responds to RPC calls
- ✅ GUI miner connects and mines
- ✅ Balance updates correctly
- ✅ Transactions are tracked
- ✅ Explorer shows real-time data
- ✅ Wallets can query balances
- ✅ Block reward is 50 GXC
- ✅ No deadlocks or hangs

**Ready for production use!** 🎉
