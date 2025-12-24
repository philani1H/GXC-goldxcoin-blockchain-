# P2P Networking and Built-in Mining - Implementation Guide

## Files Created

### 1. P2P Networking

**Header:** `include/p2p_network.h`
- P2PNetwork class definition
- Peer struct for connection management
- Methods for peer discovery, connection, and message handling

**Implementation:** `src/P2PNetwork.cpp`
- Real TCP/IP socket connections
- Peer handshake protocol
- Block propagation
- Message handling (VERSION, HEIGHT, GETBLOCKS, BLOCK, PING/PONG)

### 2. Built-in CPU Mining

**Header:** `include/cpu_miner.h`
- CPUMiner class definition
- Multi-threaded mining support
- Hashrate tracking

**Implementation:** `src/CPUMiner.cpp`
- Mining thread management
- Block template creation
- Nonce iteration
- Real hashrate calculation

---

## Integration Steps

### Step 1: Update CMakeLists.txt

Add new source files to the build:

```cmake
# Add to src/CMakeLists.txt
set(SOURCES
    # ... existing sources ...
    P2PNetwork.cpp
    CPUMiner.cpp
)
```

### Step 2: Update Main Node

Modify `src/main.cpp` to include P2P and miner:

```cpp
#include "p2p_network.h"
#include "cpu_miner.h"

// In main()
Blockchain blockchain;
P2PNetwork p2pNetwork(&blockchain, 8333);  // Port 8333 for P2P
CPUMiner cpuMiner(&blockchain);

// Start P2P
p2pNetwork.start();
p2pNetwork.addSeedNode("seed.gxc.network", 8333);
p2pNetwork.connectToSeeds();

// Start mining if enabled
if (config.getBool("gen")) {
    std::string minerAddress = config.getString("mineraddress");
    int threads = config.getInt("genproclimit", 1);
    cpuMiner.startMining(minerAddress, threads);
}

// ... rest of node code ...

// On shutdown
cpuMiner.stopMining();
p2pNetwork.stop();
```

### Step 3: Update RPCAPI

Replace fake data with real data:

```cpp
// src/RPCAPI.cpp

JsonValue RPCAPI::getPeerInfo(const JsonValue& params) {
    JsonValue result;
    result.setArray();
    
    // Get real peers from P2P network
    auto peers = p2pNetwork->getPeers();
    
    for (const auto& peer : peers) {
        JsonValue peerInfo;
        peerInfo["addr"] = peer->address + ":" + std::to_string(peer->port);
        peerInfo["inbound"] = peer->inbound;
        peerInfo["conntime"] = static_cast<uint64_t>(peer->connectedAt);
        peerInfo["synced_blocks"] = peer->syncedBlocks;
        result.append(peerInfo);
    }
    
    return result;
}

JsonValue RPCAPI::getNetworkInfo(const JsonValue& params) {
    JsonValue result;
    
    result["version"] = 20000;
    result["subversion"] = "/GXC:2.0.0/";
    result["protocolversion"] = 70015;
    result["connections"] = p2pNetwork->getPeerCount();  // Real count
    result["networkactive"] = true;
    
    return result;
}

JsonValue RPCAPI::getMiningInfo(const JsonValue& params) {
    JsonValue result;
    
    result["blocks"] = static_cast<uint64_t>(blockchain->getHeight());
    result["currentblocksize"] = 1000;
    result["currentblocktx"] = 1;
    result["difficulty"] = blockchain->getDifficulty();
    result["generate"] = cpuMiner->isMiningActive();
    result["genproclimit"] = 1;
    result["hashespersec"] = cpuMiner->getHashrate();  // Real hashrate
    
    // Calculate network hashrate from difficulty and block time
    double difficulty = blockchain->getDifficulty();
    double avgBlockTime = blockchain->getAverageBlockTime(10);
    double networkHashrate = (difficulty * pow(2, 32)) / (avgBlockTime > 0 ? avgBlockTime : 120);
    result["networkhashps"] = networkHashrate;
    
    result["pooledtx"] = 0;
    result["testnet"] = Config::isTestnet();
    
    return result;
}
```

### Step 4: Add RPC Methods for Mining Control

```cpp
// Add to RPCAPI.cpp

JsonValue RPCAPI::setGenerate(const JsonValue& params) {
    if (params.size() < 1) {
        throw std::runtime_error("setgenerate requires at least 1 parameter");
    }
    
    bool generate = params[0].asBool();
    int threads = params.size() > 1 ? params[1].asInt() : 1;
    
    if (generate) {
        std::string minerAddress = Config::getString("mineraddress");
        if (minerAddress.empty()) {
            throw std::runtime_error("mineraddress not configured");
        }
        cpuMiner->startMining(minerAddress, threads);
    } else {
        cpuMiner->stopMining();
    }
    
    JsonValue result;
    result["mining"] = generate;
    result["threads"] = threads;
    return result;
}

JsonValue RPCAPI::getHashrate(const JsonValue& params) {
    JsonValue result;
    result["hashrate"] = cpuMiner->getHashrate();
    result["totalhashes"] = cpuMiner->getTotalHashes();
    result["mining"] = cpuMiner->isMiningActive();
    return result;
}
```

### Step 5: Update Configuration

Add to `config/gxc-testnet.conf`:

```ini
# P2P Network
port=8333
maxconnections=125
connect=seed.gxc.network:8333

# Built-in Mining
gen=0
genproclimit=2
mineraddress=tGXC_your_address_here
```

---

## Building

```bash
cd build
cmake ..
make -j$(nproc)
```

---

## Testing

### Test 1: Start Node with P2P

```bash
./gxc-node --testnet
```

**Expected Output:**
```
[INFO] GXC Node starting...
[INFO] P2P Network listening on port 8333
[INFO] Connected to seed node
[INFO] RPC Server listening on port 8332
```

### Test 2: Check Real Peers

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getpeerinfo","params":[],"id":1}' \
  | jq
```

**Expected:** Real peer addresses (not 192.168.1.x)

### Test 3: Start Mining

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"setgenerate","params":[true, 2],"id":1}'
```

### Test 4: Check Real Hashrate

```bash
curl -X POST http://localhost:8332 \
  -d '{"jsonrpc":"2.0","method":"getmininginfo","params":[],"id":1}' \
  | jq '.result.hashespersec'
```

**Expected:** Real hashrate (e.g., 50000 H/s), not 0 or fake value

---

## Quick Fix (Without Full Implementation)

If you want to quickly fix the fake data without full P2P implementation:

### Fix 1: Show 0 Peers Instead of Fake

```cpp
// src/RPCAPI.cpp - getPeerInfo()
JsonValue RPCAPI::getPeerInfo(const JsonValue& params) {
    JsonValue result;
    result.setArray();
    // Return empty array instead of fake peers
    return result;
}
```

### Fix 2: Show 0 Hashrate When Not Mining

```cpp
// src/RPCAPI.cpp - getMiningInfo()
JsonValue RPCAPI::getMiningInfo(const JsonValue& params) {
    JsonValue result;
    
    result["blocks"] = static_cast<uint64_t>(blockchain->getHeight());
    result["difficulty"] = blockchain->getDifficulty();
    result["generate"] = false;
    result["hashespersec"] = 0;  // Show 0 instead of fake
    result["networkhashps"] = 0.0;  // Show 0 instead of 1M
    result["pooledtx"] = 0;
    
    return result;
}
```

### Fix 3: Calculate Network Hashrate from Blocks

```cpp
// src/RPCAPI.cpp - getNetworkHashPS()
JsonValue RPCAPI::getNetworkHashPS(const JsonValue& params) {
    // Calculate from recent blocks
    int numBlocks = params.size() > 0 ? params[0].asInt() : 120;
    
    double avgBlockTime = blockchain->getAverageBlockTime(numBlocks);
    double difficulty = blockchain->getDifficulty();
    
    // Hashrate = (difficulty * 2^32) / block_time
    double networkHashrate = 0.0;
    if (avgBlockTime > 0) {
        networkHashrate = (difficulty * pow(2, 32)) / avgBlockTime;
    }
    
    return networkHashrate;
}
```

---

## Deployment

### For Seed Node (Railway/VPS)

```bash
# Build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Configure
cat > gxc-testnet.conf << EOF
testnet=1
rpcport=8332
rpcallowip=0.0.0.0
port=8333
maxconnections=125
gen=0
EOF

# Run
./gxc-node --testnet --daemon
```

### For Home User

```bash
# Configure
cat > gxc-testnet.conf << EOF
testnet=1
rpcport=8332
rpcallowip=127.0.0.1
port=8333
connect=seed.gxc.network:8333
gen=1
genproclimit=2
mineraddress=tGXC_your_address
EOF

# Run
./gxc-node --testnet
```

---

## Summary

**Files Created:**
- ✅ `include/p2p_network.h` - P2P header
- ✅ `src/P2PNetwork.cpp` - P2P implementation
- ✅ `include/cpu_miner.h` - Miner header
- ✅ `src/CPUMiner.cpp` - Miner implementation

**Integration Needed:**
- Update CMakeLists.txt
- Modify main.cpp
- Update RPCAPI.cpp
- Add RPC methods (setgenerate, gethashrate)
- Build and test

**Estimated Time:**
- Integration: 2-3 hours
- Testing: 1-2 hours
- Debugging: 2-4 hours
- **Total: 5-9 hours**

**Quick Fix Alternative:**
- Remove fake peer data (5 minutes)
- Show 0 hashrate when not mining (5 minutes)
- Calculate network hashrate from blocks (15 minutes)
- **Total: 25 minutes**

Choose quick fix for immediate deployment, or full implementation for production-ready P2P network.
