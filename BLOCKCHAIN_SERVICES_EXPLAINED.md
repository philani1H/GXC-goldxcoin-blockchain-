# What Services Run When the Blockchain Starts?

## ✅ Yes! All Services Start Automatically

When you start the GXC blockchain node, **all three services start automatically**:

1. **Network Server** (P2P)
2. **RPC Server** (RPC API)
3. **REST Server** (REST API)

---

## 📋 Services Overview

### 1. **Network Server (P2P)**
- **Purpose**: Peer-to-peer networking for blockchain synchronization
- **Default Port**: `9333` (Mainnet) or `19333` (Testnet)
- **What it does**:
  - Connects to other blockchain nodes
  - Syncs blocks and transactions
  - Broadcasts new blocks and transactions
  - Maintains the peer network

### 2. **RPC Server (RPC API)**
- **Purpose**: JSON-RPC API for programmatic access
- **Default Port**: `8332` (Mainnet) or `18332` (Testnet)
- **What it does**:
  - Provides JSON-RPC 2.0 interface
  - Used by wallets, miners, and applications
  - Methods like `getbalance`, `sendtoaddress`, `getblocktemplate`
- **URL Format**: `http://localhost:8332` (or your configured port)

### 3. **REST Server (REST API)**
- **Purpose**: HTTP REST API for web applications
- **Default Port**: `8080` (Mainnet) or `18080` (Testnet)
- **What it does**:
  - Provides RESTful HTTP endpoints
  - Used by web explorers, wallets, and web apps
  - Endpoints like `/api/blockchain/info`, `/api/transactions`, `/api/addresses`
- **URL Format**: `http://localhost:8080/api/...` (or your configured port)

---

## 🔍 How to Verify Services Are Running

When you start the blockchain node, you'll see output like this:

```
=== Node Information ===
Network: Mainnet
Data Directory: ./gxc_data
Blockchain Height: 0
Network Port: 9333
RPC Port: 8332
REST Port: 8080
Traceability: VALID
========================

[INFO] Network listening on port: 9333
[INFO] RPC server listening on port: 8332
[INFO] REST server listening on port: 8080
```

---

## 🌐 Default Ports

### Mainnet (Production)
- **Network**: `9333`
- **RPC**: `8332`
- **REST**: `8080`

### Testnet (Testing)
- **Network**: `19333`
- **RPC**: `18332`
- **REST**: `18080`

---

## 🔧 Custom Ports

You can change ports using command-line arguments:

```bash
./gxc_node --rpc-port=9000 --rest-port=9001 --port=9002
```

Or in testnet mode:
```bash
./gxc_node --testnet --rpc-port=19000 --rest-port=19001
```

---

## 📡 How Services Are Used

### RPC API (Port 8332)
Used by:
- **Wallets** (Python wallet service)
- **Miners** (GXHash miner)
- **Mining Pools** (Stratum pools)
- **Applications** (any app needing blockchain data)

**Example RPC Call:**
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["address"],"id":1}'
```

### REST API (Port 8080)
Used by:
- **Web Explorers** (blockchain explorer web UI)
- **Web Wallets** (browser-based wallets)
- **Web Applications** (any web app needing blockchain data)

**Example REST Call:**
```bash
curl http://localhost:8080/api/blockchain/info
curl http://localhost:8080/api/transactions/recent
curl http://localhost:8080/api/addresses/ADDRESS_HERE/balance
```

---

## ⚙️ What Happens in the Main Loop

The blockchain node continuously processes:

1. **Blockchain Operations**: Validates and processes transactions
2. **Network Updates**: Syncs with peers, broadcasts blocks
3. **RPC Requests**: Handles incoming RPC API calls
4. **REST Requests**: Handles incoming REST API calls

All of this happens automatically in the background!

---

## 🚀 Starting the Blockchain

### Windows
```powershell
.\build\gxc_node.exe
```

### Linux/macOS
```bash
./build/gxc_node
```

### With Custom Ports
```bash
./build/gxc_node --rpc-port=8332 --rest-port=8080
```

### Testnet Mode
```bash
./build/gxc_node --testnet
```

---

## ✅ Summary

**YES!** When the blockchain node runs:
- ✅ Network server starts automatically
- ✅ RPC server starts automatically  
- ✅ REST server starts automatically
- ✅ All services are accessible immediately
- ✅ No additional setup needed

**You don't need to start anything separately** - just run the blockchain node and all services are available!

---

## 🔗 Related Documentation

- `REST_API_EXPLAINED.md` - Detailed REST API guide
- `DEPLOY_BLOCKCHAIN.md` - Deployment instructions
- `BUILD_INSTRUCTIONS.md` - Build instructions

