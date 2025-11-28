# GXC Full Testnet Setup Guide

Complete guide to setting up and running the GXC testnet with blockchain node, mining pools, and all services.

## 🚀 Quick Start

### 1. Start Full Testnet

```powershell
.\scripts\start-testnet-full.ps1
```

This will start:
- ✅ Blockchain Node (RPC on port 18332)
- ✅ General Mining Pool (port 5000, Stratum 3333)
- ✅ ASIC Mining Pool (port 5001, Stratum 3333)
- ✅ GPU Mining Pool (port 5002, Stratum 4444)
- ✅ Wallet API (port 15000)
- ✅ Blockchain Explorer (port 13000)

### 2. Stop Testnet

```powershell
.\scripts\stop-testnet-full.ps1
```

## 📊 Services & Endpoints

| Service | URL | Purpose |
|---------|-----|---------|
| **Blockchain Node RPC** | http://localhost:18332 | JSON-RPC interface |
| **General Mining Pool** | http://localhost:5000 | Dashboard & Stratum:3333 |
| **ASIC Mining Pool** | http://localhost:5001 | Dashboard & Stratum:3333 |
| **GPU Mining Pool** | http://localhost:5002 | Dashboard & Stratum:4444 |
| **Wallet API** | http://localhost:15000 | Wallet management |
| **Blockchain Explorer** | http://localhost:13000 | Block & transaction explorer |

## ⛏️ Mining Configuration

### Connect Your Miner

**General Pool (All Algorithms):**
```
URL: stratum+tcp://localhost:3333
Username: your_worker
Password: x
```

**ASIC Pool (SHA-256):**
```
URL: stratum+tcp://localhost:3333
Username: your_worker
Password: x
Algorithm: SHA-256
```

**GPU Pool (Ethash):**
```
URL: stratum+tcp://localhost:4444
Username: your_worker
Password: x
Algorithm: Ethash
```

### Example Miner Commands

**BFGMiner/CGMiner (ASIC):**
```bash
bfgminer -o stratum+tcp://localhost:3333 -u your_worker -p x
```

**Ethminer (GPU):**
```bash
ethminer -P stratum1+tcp://your_worker:x@localhost:4444
```

**GXC Miner (CPU/General):**
```bash
./gxc-miner --pool stratum+tcp://localhost:3333 --user your_worker --pass x
```

## 🔧 Configuration

### Blockchain Node

The testnet node uses:
- **RPC Port:** 18332
- **Network Port:** 19333
- **Data Directory:** `./gxc_testnet_data`
- **Config File:** `config/gxc-testnet.conf`

### Mining Pools

Pools automatically connect to the blockchain node at `http://localhost:18332`.

To change the RPC URL, set environment variable:
```powershell
$env:BLOCKCHAIN_NODE_URL = "http://your-node:18332"
```

## 📝 RPC Methods

The pools use these RPC methods to communicate with the blockchain:

- `getblockchaininfo` - Get blockchain status
- `getblocktemplate` - Get mining template
- `submitblock` - Submit mined block

The pools will try both standard (`getblocktemplate`) and GXC-specific (`gxc_getBlockTemplate`) method names for compatibility.

## 🐛 Troubleshooting

### Pools Can't Connect to Node

1. Check if blockchain node is running:
   ```powershell
   Get-Process | Where-Object {$_.ProcessName -like "*gxc*"}
   ```

2. Check node logs:
   ```powershell
   Get-Content logs\blockchain_node_testnet.log -Tail 50
   ```

3. Verify RPC is accessible:
   ```powershell
   curl http://localhost:18332 -Method POST -Body '{"method":"getblockchaininfo","params":[],"id":1}' -ContentType "application/json"
   ```

### Miner Can't Connect to Pool

1. Check pool is running:
   ```powershell
   Get-Content logs\pool_general_testnet.log -Tail 50
   ```

2. Verify Stratum port is open:
   ```powershell
   netstat -ano | findstr "3333 4444"
   ```

3. Check firewall settings

### No Blocks Being Found

1. Check difficulty in testnet config (should be low for testing)
2. Verify miners are submitting shares:
   - Visit pool dashboard: http://localhost:5000
   - Check "Accepted Shares" counter
3. Check block submission logs:
   ```powershell
   Get-Content logs\pool_general_testnet.log | Select-String "block"
   ```

## 📁 Log Files

All logs are in the `logs/` directory:

- `blockchain_node_testnet.log` - Blockchain node
- `pool_general_testnet.log` - General pool
- `pool_asic_testnet.log` - ASIC pool
- `pool_gpu_testnet.log` - GPU pool
- `wallet_api_testnet.log` - Wallet API
- `explorer_testnet.log` - Blockchain explorer

## 🎯 Testing Checklist

- [ ] Blockchain node starts and RPC responds
- [ ] All three mining pools start successfully
- [ ] Pool dashboards are accessible
- [ ] Miner can connect to Stratum endpoint
- [ ] Shares are being accepted by pool
- [ ] Blocks can be found and submitted
- [ ] Blockchain explorer shows new blocks
- [ ] Wallet API can create addresses

## 🔗 Next Steps

1. **Start Mining:** Connect your miner to one of the pools
2. **Monitor:** Watch pool dashboards for stats
3. **Explore:** Use blockchain explorer to view blocks
4. **Test:** Create wallets and send test transactions

## 📚 Additional Resources

- **Mining Guide:** https://gxc-docs.vercel.app/mining
- **API Documentation:** https://gxc-docs.vercel.app/api
- **Testnet Guide:** See `TESTNET_GUIDE.md`

---

**Ready to mine!** 🚀 Start the testnet and connect your miners!

