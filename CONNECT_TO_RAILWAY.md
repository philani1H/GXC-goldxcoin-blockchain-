# 🔗 Connect Your Services to Railway Node

## 📋 Quick Reference

Once you deploy your blockchain node to Railway, you'll get a URL like:
```
https://gxc-blockchain-node-production.up.railway.app
```

Use this URL in all your services instead of `http://localhost:18332`

---

## 🔧 Update Each Service

### 1. Miner (mine_testnet_fixed.py)

**Before (Local):**
```python
RPC_URL = "http://localhost:18332"
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
```

**After (Railway):**
```python
RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
```

Then run:
```bash
python mine_testnet_fixed.py
```

---

### 2. Wallet Service (api/wallet_service.py)

**Option A: Environment Variable (Recommended)**
```bash
export BLOCKCHAIN_RPC_URL=https://gxc-blockchain-node-production.up.railway.app
python api/wallet_service.py
```

**Option B: Edit Code**

Find this line in `api/wallet_service.py`:
```python
BLOCKCHAIN_RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', 'http://localhost:18332')
```

Change to:
```python
BLOCKCHAIN_RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', 'https://gxc-blockchain-node-production.up.railway.app')
```

---

### 3. Explorer Service (api/explorer_service.py)

**Option A: Environment Variable (Recommended)**
```bash
export BLOCKCHAIN_NODE_URL=https://gxc-blockchain-node-production.up.railway.app
python api/explorer_service.py
```

**Option B: Edit Code**

Find the blockchain node URL configuration and update it:
```python
BLOCKCHAIN_NODE_URL = "https://gxc-blockchain-node-production.up.railway.app"
```

---

### 4. Balance Checker (check_balance.py)

**Before:**
```python
RPC_URL = "http://localhost:18332"
```

**After:**
```python
RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"
```

---

### 5. Transaction Tracer (trace_transaction.py)

**Before:**
```python
RPC_URL = "http://localhost:18332"
```

**After:**
```python
RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"
```

---

## 🧪 Test Connection

Use the test script:

```bash
python test_railway_node.py https://gxc-blockchain-node-production.up.railway.app
```

Expected output:
```
Testing node at: https://gxc-blockchain-node-production.up.railway.app
============================================================

1. Testing getblocktemplate...
   ✅ getblocktemplate works!
   Block height: 15
   Difficulty: 0.1

2. Testing getblockcount...
   ✅ getblockcount works!
   Current height: 15

3. Testing getdifficulty...
   ✅ getdifficulty works!
   Difficulty: 0.1

============================================================
✅ Node is accessible and responding!
```

---

## 🌐 Environment Variables (Best Practice)

Create a `.env` file:

```bash
# .env
BLOCKCHAIN_RPC_URL=https://gxc-blockchain-node-production.up.railway.app
BLOCKCHAIN_NODE_URL=https://gxc-blockchain-node-production.up.railway.app
MINER_ADDRESS=tGXC9fab7317231b966af85ac453e168c0932
```

Then load it:

```bash
# Load environment variables
export $(cat .env | xargs)

# Run your services
python mine_testnet_fixed.py
python api/wallet_service.py
python api/explorer_service.py
```

---

## 📊 Verify Everything Works

### 1. Check Node Status

```bash
curl -X POST https://gxc-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"1","method":"getblockcount","params":[]}'
```

### 2. Mine a Block

```bash
python mine_testnet_fixed.py
```

Should output:
```
Mining block 16...
✅ Block 16 mined successfully!
Block hash: 00000abc...
Reward: 12.5 GXC
```

### 3. Check Balance

```bash
python check_balance.py tGXC9fab7317231b966af85ac453e168c0932
```

Should show your mining rewards.

---

## 🔄 Switch Between Local and Railway

### Use Environment Variables

```bash
# Use Railway node
export BLOCKCHAIN_RPC_URL=https://gxc-blockchain-node-production.up.railway.app
python mine_testnet_fixed.py

# Use local node
export BLOCKCHAIN_RPC_URL=http://localhost:18332
python mine_testnet_fixed.py
```

### Or Create Separate Scripts

**mine_testnet_local.py:**
```python
RPC_URL = "http://localhost:18332"
# ... rest of code
```

**mine_testnet_railway.py:**
```python
RPC_URL = "https://gxc-blockchain-node-production.up.railway.app"
# ... rest of code
```

---

## 📋 Connection Checklist

After deploying to Railway:

- [ ] Get Railway URL from `railway domain`
- [ ] Test connection with `test_railway_node.py`
- [ ] Update miner script
- [ ] Update wallet service
- [ ] Update explorer service
- [ ] Update balance checker
- [ ] Update transaction tracer
- [ ] Test mining a block
- [ ] Verify balance updates
- [ ] Document URL for team

---

## ⚠️ Important Notes

### 1. HTTPS vs HTTP

Railway uses HTTPS, not HTTP:
```
✅ https://your-app.up.railway.app
❌ http://your-app.up.railway.app
```

### 2. No Trailing Slash

Don't add trailing slash:
```
✅ https://your-app.up.railway.app
❌ https://your-app.up.railway.app/
```

### 3. Port Not Needed

Railway handles ports automatically:
```
✅ https://your-app.up.railway.app
❌ https://your-app.up.railway.app:18332
```

### 4. Testnet vs Mainnet

Make sure you're using the right network:
```
Testnet: tGXC addresses, 60s blocks
Mainnet: GXC addresses, 600s blocks
```

---

## 🎯 Summary

```
╔════════════════════════════════════════════════════════╗
║                                                        ║
║   🔗 CONNECTION PATTERN                                ║
║                                                        ║
║   Old (Local):                                         ║
║   http://localhost:18332                               ║
║                                                        ║
║   New (Railway):                                       ║
║   https://gxc-blockchain-node-production.up.railway.app║
║                                                        ║
║   Update in:                                           ║
║   ✅ Miner                                             ║
║   ✅ Wallet                                            ║
║   ✅ Explorer                                          ║
║   ✅ Balance checker                                   ║
║   ✅ Transaction tracer                                ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

---

## 🚀 Quick Start

```bash
# 1. Get your Railway URL
railway domain

# 2. Test it
python test_railway_node.py https://your-railway-url.up.railway.app

# 3. Update miner
# Edit mine_testnet_fixed.py line 8:
RPC_URL = "https://your-railway-url.up.railway.app"

# 4. Mine!
python mine_testnet_fixed.py

# Done! 🎉
```

That's it! Your services now connect to the Railway node instead of localhost!
