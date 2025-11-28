# 🔍 How Blockchain Differentiates Testnet vs Mainnet

## 🎯 Quick Answer

The blockchain uses **5 key differences** to keep testnet and mainnet completely separate:

```
1. Different Data Directories
2. Different Network Ports
3. Different Genesis Blocks
4. Different Address Prefixes
5. Different Network IDs
```

---

## 📊 Detailed Breakdown

### 1. Different Data Directories 📁

**Mainnet:**
```
Data Directory: ./gxc_data/
Database: ./gxc_data/blockchain.db

Stores:
- Mainnet blocks
- Mainnet transactions
- Mainnet UTXO set
- Mainnet state
```

**Testnet:**
```
Data Directory: ./gxc_testnet_data/
Database: ./gxc_testnet_data/blockchain.db

Stores:
- Testnet blocks
- Testnet transactions
- Testnet UTXO set
- Testnet state
```

**Result:** ✅ Completely separate databases, no mixing possible!

---

### 2. Different Network Ports 🌐

**Mainnet:**
```
RPC Port:  8332   (for mining pools)
REST Port: 8080   (for wallets/apps)
P2P Port:  9333   (for node communication)
```

**Testnet:**
```
RPC Port:  18332  (for mining pools)
REST Port: 18080  (for wallets/apps)
P2P Port:  19333  (for node communication)
```

**Result:** ✅ Can run both simultaneously without conflicts!

**Example:**
```bash
# Terminal 1: Mainnet
./gxc-node --rpc-port=8332 --datadir=./gxc_data

# Terminal 2: Testnet (same machine!)
./gxc-node --testnet --rpc-port=18332 --datadir=./gxc_testnet_data

# Both run at the same time! ✅
```

---

### 3. Different Genesis Blocks 🎬

**Mainnet Genesis:**
```cpp
// In C++ code
if (!testnet) {
    genesis_message = "GXC Mainnet Genesis Block";
    genesis_hash = sha256("GXC Mainnet Genesis Block");
    genesis_timestamp = 1700000000;  // Fixed timestamp
}
```

**Testnet Genesis:**
```cpp
// In C++ code
if (testnet) {
    genesis_message = "GXC Testnet Genesis Block";
    genesis_hash = sha256("GXC Testnet Genesis Block");
    genesis_timestamp = 1700000000;  // Different or same
}
```

**Result:** ✅ Different genesis blocks = Different blockchains!

**Why this matters:**
```
Every block references the previous block
Block 1 → Block 0 (Genesis)

Mainnet Block 1 → Mainnet Genesis (hash: abc123...)
Testnet Block 1 → Testnet Genesis (hash: def456...)

Different genesis = Different chain = Can't mix!
```

---

### 4. Different Address Prefixes 🏷️

**Mainnet Addresses:**
```
Prefix: GXC
Example: GXC21d83ca607604df4f86bd60761e3c4ee30
Length: 37 characters (GXC + 34 hex)
```

**Testnet Addresses:**
```
Prefix: tGXC
Example: tGXC991ebf33fdfaf2db54287b4d09a9d60d1
Length: 37 characters (tGXC + 33 hex)
```

**Validation in Code:**
```cpp
// C++ blockchain validates addresses
bool validateAddress(string address, bool testnet) {
    if (testnet) {
        return address.startsWith("tGXC") && address.length() == 37;
    } else {
        return address.startsWith("GXC") && 
               !address.startsWith("tGXC") && 
               address.length() == 37;
    }
}

// Reject wrong network addresses
if (testnet && address.startsWith("GXC")) {
    return error("Mainnet address not valid on testnet");
}

if (!testnet && address.startsWith("tGXC")) {
    return error("Testnet address not valid on mainnet");
}
```

**Result:** ✅ Can't send testnet coins to mainnet address!

---

### 5. Different Network IDs 🆔

**In Database:**
```sql
-- Every record has network field
CREATE TABLE blocks (
    height INTEGER,
    hash TEXT,
    network TEXT  -- 'mainnet' or 'testnet'
);

CREATE TABLE transactions (
    tx_hash TEXT,
    amount REAL,
    network TEXT  -- 'mainnet' or 'testnet'
);
```

**Queries Always Filter by Network:**
```sql
-- Get mainnet blocks only
SELECT * FROM blocks WHERE network = 'mainnet';

-- Get testnet blocks only
SELECT * FROM blocks WHERE network = 'testnet';

-- Can't mix them!
```

**Result:** ✅ Even if same database, data is separated!

---

## 🔄 Complete Separation Flow

### When You Start Mainnet Node:

```
1. Read --testnet flag → FALSE
   ↓
2. Set data directory → ./gxc_data/
   ↓
3. Set RPC port → 8332
   ↓
4. Load/create genesis → "GXC Mainnet Genesis Block"
   ↓
5. Set address prefix → "GXC"
   ↓
6. Set network ID → "mainnet"
   ↓
7. Start node → Mainnet blockchain running ✅
```

### When You Start Testnet Node:

```
1. Read --testnet flag → TRUE
   ↓
2. Set data directory → ./gxc_testnet_data/
   ↓
3. Set RPC port → 18332
   ↓
4. Load/create genesis → "GXC Testnet Genesis Block"
   ↓
5. Set address prefix → "tGXC"
   ↓
6. Set network ID → "testnet"
   ↓
7. Start node → Testnet blockchain running ✅
```

---

## 🛡️ Safety Mechanisms

### 1. Address Validation
```cpp
// Transaction validation
bool Transaction::validate(bool testnet) {
    // Check sender address
    if (testnet && !from_address.startsWith("tGXC")) {
        return false;  // Reject mainnet address on testnet
    }
    
    if (!testnet && from_address.startsWith("tGXC")) {
        return false;  // Reject testnet address on mainnet
    }
    
    // Check recipient address
    if (testnet && !to_address.startsWith("tGXC")) {
        return false;  // Can't send to mainnet address
    }
    
    if (!testnet && to_address.startsWith("tGXC")) {
        return false;  // Can't send to testnet address
    }
    
    return true;
}
```

### 2. Block Validation
```cpp
// Block validation
bool Block::validate(bool testnet) {
    // Check miner address
    if (testnet && !miner_address.startsWith("tGXC")) {
        return false;  // Mainnet address can't mine testnet
    }
    
    if (!testnet && miner_address.startsWith("tGXC")) {
        return false;  // Testnet address can't mine mainnet
    }
    
    // Check all transactions
    for (auto& tx : transactions) {
        if (!tx.validate(testnet)) {
            return false;
        }
    }
    
    return true;
}
```

### 3. Network Isolation
```cpp
// P2P network
void Network::connect(Peer peer) {
    // Check network ID
    if (peer.network_id != this->network_id) {
        disconnect(peer);  // Different network, reject!
        return;
    }
    
    // Testnet nodes only connect to testnet nodes
    // Mainnet nodes only connect to mainnet nodes
}
```

---

## 📋 Configuration Comparison

| Feature | Mainnet | Testnet |
|---------|---------|---------|
| **Data Dir** | `./gxc_data/` | `./gxc_testnet_data/` |
| **RPC Port** | 8332 | 18332 |
| **REST Port** | 8080 | 18080 |
| **P2P Port** | 9333 | 19333 |
| **Address Prefix** | `GXC` | `tGXC` |
| **Network ID** | `mainnet` | `testnet` |
| **Genesis** | Mainnet genesis | Testnet genesis |
| **Block Time** | 150 seconds | 60 seconds |
| **Difficulty** | Higher | Lower (0.1) |
| **Block Reward** | 50 GXC | 12.5 GXC |
| **Coins Value** | Real value | No value |

---

## 🎯 Practical Example

### Scenario: You Try to Mix Networks

**Attempt 1: Send Mainnet Coins to Testnet Address**
```bash
# On mainnet blockchain
./gxc-node --datadir=./gxc_data

# Try to send
send_transaction(
    from: "GXC21d83ca607604df4f86bd60761e3c4ee30",  # Mainnet
    to: "tGXC991ebf33fdfaf2db54287b4d09a9d60d1",    # Testnet
    amount: 10
)

# Result:
❌ ERROR: Invalid recipient address
   Testnet address not valid on mainnet
   Transaction rejected!
```

**Attempt 2: Mine Testnet with Mainnet Address**
```bash
# On testnet blockchain
./gxc-node --testnet --datadir=./gxc_testnet_data

# Try to mine
mine_block(miner_address: "GXC21d83ca607604df4f86bd60761e3c4ee30")

# Result:
❌ ERROR: Invalid miner address
   Mainnet address not valid on testnet
   Block rejected!
```

**Attempt 3: Connect Testnet Node to Mainnet Network**
```bash
# Testnet node tries to connect to mainnet peer
testnet_node.connect("mainnet_peer:9333")

# Handshake:
Testnet Node: "Hello, I'm on network: testnet"
Mainnet Peer: "I'm on network: mainnet"
Mainnet Peer: "Network mismatch! Disconnecting..."

# Result:
❌ Connection rejected!
   Different network IDs
```

---

## ✅ Summary

**How blockchain differentiates:**

1. **Data Directory** - Separate files
2. **Network Ports** - Different ports
3. **Genesis Block** - Different starting point
4. **Address Prefix** - tGXC vs GXC
5. **Network ID** - Metadata in every record

**Result:**
```
╔════════════════════════════════════════════════════════╗
║                                                        ║
║   Testnet and Mainnet are COMPLETELY SEPARATE         ║
║                                                        ║
║   ✅ Different blockchains                            ║
║   ✅ Different databases                              ║
║   ✅ Different addresses                              ║
║   ✅ Different ports                                  ║
║   ✅ Can't mix them                                   ║
║   ✅ Can run both simultaneously                      ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

**Your address `GXC21d83ca607604df4f86bd60761e3c4ee30` only works on MAINNET!** ✅
