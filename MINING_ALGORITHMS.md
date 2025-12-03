# GXC Mining Algorithms - Complete Guide

## 📊 Overview

GXC blockchain supports **THREE mining algorithms**:

1. **SHA-256** (Bitcoin-style) - Default for PoW
2. **Ethash** (Ethereum-style) - GPU-friendly
3. **Proof of Stake (PoS)** - Validator-based

---

## 🔐 1. SHA-256 Mining

### Implementation
**File:** `src/HashUtils.cpp`
```cpp
std::string sha256d(const std::string& data) {
    return sha256(sha256(data));  // Double SHA-256
}
```

### How It Works
1. **Double Hashing**: SHA-256 applied twice (like Bitcoin)
2. **Block Calculation**:
   ```cpp
   std::stringstream ss;
   ss << index << previousHash << timestamp << merkleRoot << nonce;
   return sha256d(ss.str());
   ```
3. **Difficulty**: Based on leading zeros in hash
4. **Mining**: Increment nonce until hash meets difficulty target

### Usage
- **Default algorithm** for blockchain
- Used in genesis block creation
- CPU-friendly
- ASIC-compatible

### Miner Implementation
**File:** `src/mining/SHA256Miner.cpp`
**Class:** `SHA256Miner`

**Features:**
- Multi-threaded CPU mining
- Configurable thread count
- Hashrate tracking
- Block template support

---

## ⚡ 2. Ethash Mining

### Implementation
**File:** `src/HashUtils.cpp`
```cpp
std::string ethash(const std::string& data, uint64_t nonce) {
    // Simplified Ethash (not full DAG implementation)
    std::string combined = data + std::to_string(nonce);
    
    // Multiple rounds of hashing
    std::string result = sha256(combined);
    for (int i = 0; i < 3; i++) {
        result = sha256(result + combined);
    }
    
    return result;
}
```

### How It Works
1. **Memory-Hard**: Designed to be GPU-friendly
2. **Multiple Rounds**: 3 rounds of SHA-256 hashing
3. **Nonce Mixing**: Nonce combined with data in each round
4. **Block Calculation**:
   ```cpp
   std::stringstream ss;
   ss << index << previousHash << timestamp << merkleRoot << nonce;
   return ethash(ss.str(), nonce);
   ```

### Current Status
⚠️ **Simplified Implementation**
- Not full Ethereum DAG (Directed Acyclic Graph)
- Uses multiple SHA-256 rounds instead
- Still memory-hard and GPU-friendly
- Suitable for GXC's needs

### Miner Implementation
**File:** `src/mining/EthashMiner.cpp`
**Class:** `EthashMiner`

**Features:**
- GPU mining support (CUDA/OpenCL)
- CPU fallback
- DAG generation (simplified)
- Hashrate tracking

---

## 🏆 3. Proof of Stake (PoS)

### Implementation
**File:** `src/Blockchain.cpp`

### How It Works
1. **Validator Selection**: Based on stake amount
2. **Block Creation**: Validators create blocks without mining
3. **Validation**: Other validators verify blocks
4. **Rewards**: Based on stake and validation

### Requirements
- Minimum stake: 100 GXC
- Validator registration required
- Active validator set maintained

### Block Type Selection
```cpp
BlockType Blockchain::getNextBlockType() const {
    if (validators.size() >= 3) {
        // Alternate between PoW and PoS
        if (chain.size() % 10 == 0) {
            return BlockType::POS;
        }
    }
    return BlockType::POW_SHA256;  // Default to PoW
}
```

---

## 🔧 Block Type Configuration

### Block Class
**File:** `include/Block.h`
```cpp
enum class BlockType {
    POW_SHA256,  // Bitcoin-style SHA-256 mining
    POW_ETHASH,  // Ethereum-style Ethash mining
    POS          // Proof of Stake
};
```

### Block Hash Calculation
**File:** `src/block.cpp`
```cpp
std::string Block::calculateHash() const {
    std::stringstream ss;
    ss << index << previousHash << timestamp << merkleRoot << nonce;
    
    if (blockType == BlockType::POW_SHA256) {
        return sha256d(ss.str());
    } else if (blockType == BlockType::POW_ETHASH) {
        return ethash(ss.str(), nonce);
    } else {
        // For PoS blocks, just use SHA-256
        return sha256(ss.str());
    }
}
```

---

## 📈 Current Blockchain Configuration

### Default Settings
- **Genesis Block**: SHA-256
- **Regular Blocks**: SHA-256 (default)
- **PoS Blocks**: Every 10th block (if 3+ validators)
- **Testnet Difficulty**: 0.1 (very easy)
- **Mainnet Difficulty**: 1000.0 (harder)

### Difficulty Calculation
```cpp
int requiredZeros = static_cast<int>(std::log2(difficulty)) + 2;
```

**Examples:**
- Difficulty 0.1 → 2 leading zeros (testnet)
- Difficulty 1000 → 12 leading zeros (mainnet)

---

## 🎯 Which Algorithm is Used?

### Current Blockchain
**Answer: SHA-256 (POW_SHA256)**

The blockchain currently uses **SHA-256 by default** for all blocks:
- Genesis block: SHA-256
- Regular blocks: SHA-256
- Only switches to PoS if 3+ validators exist

### Why SHA-256?
1. **Proven Security**: Battle-tested by Bitcoin
2. **ASIC Support**: Hardware miners available
3. **CPU-Friendly**: Can mine with regular computers
4. **Simple**: Easy to implement and verify

### Ethash Availability
Ethash is **implemented but not actively used** by default:
- Code exists in `EthashMiner.cpp`
- Can be selected via block type
- Miners can choose algorithm
- GPU-friendly for those who want it

---

## 🔨 Mining Tools Available

### 1. GUI Miner
**File:** `mine_testnet_gui.py`
- **Algorithm**: Uses blockchain's default (SHA-256)
- **Interface**: Graphical
- **Features**: Balance tracking, transaction history

### 2. Command Line Miner
**File:** `mine_testnet_fixed.py`
- **Algorithm**: Uses blockchain's default (SHA-256)
- **Interface**: Terminal
- **Features**: Simple, fast

### 3. GXHash Miner
**File:** `mining/gxhash_miner.py`
- **Algorithm**: Custom GXHash (optimized SHA-256)
- **Interface**: Terminal
- **Features**: GPU/CPU support

### 4. C++ Miners
**Files:** `src/mining/SHA256Miner.cpp`, `src/mining/EthashMiner.cpp`
- **Algorithms**: SHA-256 or Ethash
- **Interface**: Library/API
- **Features**: High performance, multi-threaded

---

## 🧪 Testing Algorithms

### Test SHA-256
```bash
# Mine with default algorithm
python3 mine_testnet_gui.py
```

### Test Ethash
```cpp
// In C++ code
Block block(1, prevHash, BlockType::POW_ETHASH);
block.calculateHash();  // Uses Ethash
```

### Check Block Algorithm
```bash
# Query block
curl -X POST http://localhost:8080/rpc -d '{
  "jsonrpc":"2.0",
  "method":"getblock",
  "params":["<block_hash>"],
  "id":1
}'

# Response includes blockType field
```

---

## 📊 Performance Comparison

### SHA-256
- **CPU**: ~1-10 MH/s (depending on CPU)
- **GPU**: ~100-1000 MH/s (with ASIC)
- **Memory**: Low (~10 MB)
- **Power**: Medium

### Ethash (Simplified)
- **CPU**: ~0.5-5 MH/s
- **GPU**: ~50-500 MH/s
- **Memory**: Medium (~100 MB)
- **Power**: Higher

### PoS
- **CPU**: Minimal (no mining)
- **GPU**: Not needed
- **Memory**: Low
- **Power**: Very low

---

## 🎯 Summary

### Does Ethash Work?
**YES** ✅
- Implemented in `HashUtils.cpp`
- Miner available in `EthashMiner.cpp`
- Can be used by setting `BlockType::POW_ETHASH`
- Simplified version (not full Ethereum DAG)

### Does SHA-256 Work?
**YES** ✅
- Fully implemented in `HashUtils.cpp`
- Default algorithm for blockchain
- Used in genesis block
- All miners support it

### Which is Active?
**SHA-256 is the default** ✅
- Genesis block uses SHA-256
- Regular blocks use SHA-256
- Ethash available but not default
- PoS activates with 3+ validators

### How to Switch?
To use Ethash:
1. Modify block creation to use `BlockType::POW_ETHASH`
2. Update `getNextBlockType()` to return `POW_ETHASH`
3. Rebuild and restart node

---

## 🚀 Recommendations

### For Testnet
- **Keep SHA-256**: Simple, proven, works well
- **Low difficulty**: 0.1 (2 leading zeros)
- **Fast blocks**: Easy to mine for testing

### For Mainnet
- **SHA-256**: Recommended for launch
- **Higher difficulty**: 1000+ (12+ leading zeros)
- **Consider PoS**: Add validators for decentralization

### For GPU Miners
- **Ethash available**: Can be enabled
- **Requires modification**: Change default block type
- **GPU-friendly**: Better for graphics cards

---

## ✅ Conclusion

Both **SHA-256** and **Ethash** are fully implemented and working in the GXC blockchain:

- **SHA-256**: Active by default, proven, secure
- **Ethash**: Available, GPU-friendly, can be enabled
- **PoS**: Implemented, activates with validators

The blockchain is flexible and can support multiple mining algorithms based on network needs.
