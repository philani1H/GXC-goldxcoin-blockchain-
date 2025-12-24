# GXC Node Setup Guide for Home Users

## Quick Start

### Prerequisites

- Linux, macOS, or Windows (WSL)
- 2GB RAM minimum
- 10GB disk space
- Internet connection

### Installation

```bash
# 1. Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# 2. Build node (if not already built)
cd build
cmake ..
make

# 3. Go back to root
cd ..
```

---

## Running Your Node

### Step 1: Start the Node

```bash
# Start testnet node
./gxc-node --testnet
```

**You should see:**
```
[INFO] GXC Node starting...
[INFO] Network: TESTNET
[INFO] RPC Server listening on port 8332
[INFO] Blockchain initialized
```

### Step 2: Verify Node is Running

```bash
# Check blockchain info
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

**Expected Output:**
```json
{
  "result": {
    "blocks": 53,
    "chain": "test",
    "difficulty": 0.1
  }
}
```

---

## Mining

### Option 1: Command Line Miner

```bash
# In a new terminal
python3 mine_testnet.py
```

**Output:**
```
[MINER] Starting GXC Testnet Miner
[MINER] Mining to address: tGXC3tz6MsTbP2NCMvc3
[MINER] Block 54 mined! Hash: 0a1b2c3d...
```

### Option 2: GUI Miner (Recommended)

```bash
# Start GUI miner
python3 mine_testnet_gui.py
```

**Features:**
- Visual interface
- Real-time statistics
- Balance tracking
- Transaction history

---

## Configuration

### Basic Configuration

Create `gxc-testnet.conf`:

```ini
# Network
testnet=1

# RPC
rpcport=8332
rpcallowip=127.0.0.1
rpcuser=gxcuser
rpcpassword=your_secure_password

# Data
datadir=./gxc_testnet_data

# Logging
debug=1
```

### Advanced Configuration

```ini
# Network
testnet=1
port=8333

# RPC
rpcport=8332
rpcallowip=0.0.0.0
rpcuser=gxcuser
rpcpassword=your_secure_password

# Mining (when built-in mining is available)
# gen=1
# genproclimit=2
# mineraddress=tGXC_your_address

# Data
datadir=./gxc_testnet_data
maxconnections=125

# Logging
debug=1
logfile=./gxc_testnet_data/logs/debug.log
```

---

## Connecting to Seed Node

### Current Limitation

⚠️ **P2P connectivity is not yet implemented**

The node currently runs standalone. To connect to the network:

1. Use the same seed node for RPC
2. All miners connect to the same node
3. Node acts as central server

### Future (When P2P is Implemented)

```bash
# Connect to seed node
./gxc-node --testnet --connect=seed.gxc.network:8333
```

**Or in config:**
```ini
connect=seed.gxc.network:8333
addnode=seed2.gxc.network:8333
```

---

## Checking Your Node Status

### 1. Blockchain Info

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
  | jq
```

### 2. Network Info

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getnetworkinfo","params":[],"id":1}' \
  | jq
```

### 3. Mining Info

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getmininginfo","params":[],"id":1}' \
  | jq
```

### 4. Your Balance

```bash
# Replace with your address
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["tGXC_your_address"],"id":1}' \
  | jq
```

---

## Troubleshooting

### Node Won't Start

**Error:** `Address already in use`

**Solution:**
```bash
# Check if node is already running
ps aux | grep gxc-node

# Kill existing process
pkill gxc-node

# Start again
./gxc-node --testnet
```

### Can't Connect to Node

**Error:** `Connection refused`

**Check:**
```bash
# Is node running?
ps aux | grep gxc-node

# Is port open?
netstat -an | grep 8332

# Try localhost
curl http://localhost:8332
```

### Mining Not Working

**Check:**
```bash
# 1. Is node running?
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# 2. Is miner running?
ps aux | grep mine_testnet

# 3. Check miner logs
tail -f /tmp/miner.log
```

### No Peers Connected

**This is normal!** P2P is not yet implemented.

**Current behavior:**
- Node shows 3 "peers" (fake/simulated)
- No real peer connections
- Node runs standalone

**Workaround:**
- Run single node
- All miners connect to same node

---

## Port Reference

| Service | Port | Purpose |
|---------|------|---------|
| Node RPC | 8332 | JSON-RPC API |
| Node P2P | 8333 | Peer connections (future) |
| Explorer | 5001 | Web interface |
| Admin API | 5002 | Admin management |
| Forum | 5000 | Community forum |

---

## Security

### For Home Use

```ini
# Only allow local connections
rpcallowip=127.0.0.1
```

### For Public Node

```ini
# Allow all connections (use with firewall)
rpcallowip=0.0.0.0

# Use strong password
rpcuser=gxcuser
rpcpassword=very_strong_password_here
```

**Firewall Rules:**
```bash
# Allow RPC (if needed)
sudo ufw allow 8332/tcp

# Allow P2P (future)
sudo ufw allow 8333/tcp
```

---

## Monitoring

### Check Node Health

```bash
#!/bin/bash
# save as check_node.sh

echo "GXC Node Health Check"
echo "===================="

# Check if running
if pgrep -x "gxc-node" > /dev/null; then
    echo "✅ Node is running"
else
    echo "❌ Node is NOT running"
    exit 1
fi

# Check RPC
BLOCKS=$(curl -s -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
  | jq -r '.result.blocks')

echo "✅ Current block: $BLOCKS"

# Check connections
PEERS=$(curl -s -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getnetworkinfo","params":[],"id":1}' \
  | jq -r '.result.connections')

echo "ℹ️  Peers: $PEERS (simulated)"

echo "===================="
echo "Node is healthy!"
```

---

## Backup

### Important Files

```bash
# Backup wallet
cp -r gxc_testnet_data/wallet wallet_backup_$(date +%Y%m%d)

# Backup blockchain (optional, can resync)
cp -r gxc_testnet_data/blockchain_testnet.db blockchain_backup_$(date +%Y%m%d)
```

### Restore

```bash
# Restore wallet
cp -r wallet_backup_20241224 gxc_testnet_data/wallet
```

---

## Upgrading

```bash
# 1. Stop node
pkill gxc-node

# 2. Backup data
cp -r gxc_testnet_data gxc_testnet_data_backup

# 3. Pull latest code
git pull origin master

# 4. Rebuild
cd build
make

# 5. Start node
cd ..
./gxc-node --testnet
```

---

## FAQ

### Q: Can I mine with my home computer?

**A:** Yes! Use the GUI miner:
```bash
python3 mine_testnet_gui.py
```

### Q: How do I connect to other nodes?

**A:** P2P is not yet implemented. Currently, all miners connect to the same node via RPC.

### Q: What's my mining reward?

**A:** 50 GXC per block (halves every ~4 years)

### Q: How long to mine a block?

**A:** Testnet: ~2 minutes (difficulty 0.1)
Mainnet: ~10 minutes (difficulty varies)

### Q: Can I run multiple nodes?

**A:** Yes, but they won't connect to each other yet. Each runs independently.

### Q: Is this safe for mainnet?

**A:** Not yet. Use testnet only. Mainnet requires:
- P2P implementation
- Security audit
- Stress testing

---

## Getting Help

- **GitHub Issues:** https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues
- **Documentation:** See repository docs
- **Explorer:** http://localhost:5001

---

## Summary

**Current Setup:**
1. ✅ Run node locally
2. ✅ Mine with external miner
3. ✅ Check balance and transactions
4. ❌ No P2P connectivity (yet)
5. ❌ No built-in mining (yet)

**Future Features:**
- Real peer-to-peer networking
- Built-in CPU mining
- Automatic peer discovery
- Block synchronization

For now, enjoy mining on testnet and exploring the blockchain!
