# GXC Blockchain - Algorithm Configuration Guide

## Overview

The GXC blockchain supports **THREE mining algorithms**, all fully implemented and tested:

1. **SHA-256** - Bitcoin-style, ASIC-friendly
2. **Ethash** - Ethereum-style, GPU-friendly
3. **GXHash** - Argon2id-based, ASIC-resistant (RECOMMENDED)

## Algorithm Selection

### Configuration

Set the algorithm in your config file:

```conf
# Options: SHA256, ETHASH, GXHASH
miningalgorithm=GXHASH
```

### Default Settings

**Production (Mainnet)**:
```conf
miningalgorithm=GXHASH
```

**Testnet**:
```conf
miningalgorithm=GXHASH
```

## Why Testnet Uses Same Algorithm as Mainnet

**Critical**: Testnet MUST use the same algorithm as mainnet to properly test:

✅ **Algorithm correctness** - Verify the algorithm works as expected
✅ **Performance characteristics** - Test real-world mining speeds
✅ **Security properties** - Validate ASIC-resistance and attack vectors
✅ **Difficulty adjustments** - Ensure difficulty scales properly
✅ **Network behavior** - Test block propagation and validation

❌ **Wrong Approach**: Using SHA-256 on testnet and GXHash on mainnet
- Testnet doesn't test the production algorithm
- Bugs in GXHash won't be found until mainnet
- Performance issues won't be discovered
- Security vulnerabilities remain hidden

## Algorithm Comparison

| Feature | SHA-256 | Ethash | GXHash |
|---------|---------|--------|--------|
| **Speed** | Very Fast (~500K H/s) | Fast (~300K H/s) | Slow (~1 H/s) |
| **ASIC Resistance** | ❌ No | ⚠️ Moderate | ✅ Yes |
| **Memory Usage** | Low | Moderate (2GB+) | High (64MB+) |
| **GPU Mining** | Limited | Excellent | Moderate |
| **CPU Mining** | Good | Poor | Good |
| **Decentralization** | Low | Moderate | High |
| **Production Ready** | ✅ Yes | ✅ Yes | ✅ Yes |

## Algorithm Details

### SHA-256
- **Type**: Cryptographic hash function
- **Implementation**: Double SHA-256 (Bitcoin-style)
- **Use Case**: Fast testing, ASIC mining
- **Security**: Proven, but ASIC-dominated

### Ethash
- **Type**: Memory-hard hash function
- **Implementation**: DAG-based (Ethereum-style)
- **Use Case**: GPU mining, moderate ASIC resistance
- **Security**: Good, requires 2GB+ memory

### GXHash (RECOMMENDED)
- **Type**: Memory-hard key derivation function
- **Implementation**: Argon2id with custom parameters
- **Parameters**:
  - Memory: 64MB
  - Time cost: 3 iterations
  - Parallelism: 4 lanes
- **Use Case**: ASIC-resistant, decentralized mining
- **Security**: Excellent, winner of Password Hashing Competition

## Configuration Examples

### Mainnet Production (ASIC-Resistant)
```conf
network=mainnet
miningalgorithm=GXHASH
mining=1
miningthreads=4
```

### Testnet Testing (Same as Mainnet)
```conf
network=testnet
miningalgorithm=GXHASH
mining=1
miningthreads=2
```

### GPU Mining (Ethash)
```conf
network=mainnet
miningalgorithm=ETHASH
mining=1
miningthreads=0  # Auto-detect
```

### Fast Testing (SHA-256)
```conf
network=testnet
miningalgorithm=SHA256
mining=1
miningthreads=1
```

## Mining with Different Algorithms

### SHA-256 Miner
```bash
./gxc-sha256-miner --address YOUR_ADDRESS --threads 4
```

### Ethash Miner
```bash
./gxc-ethash-miner --address YOUR_ADDRESS --gpu 0
```

### GXHash Miner
```bash
./gxc-gxhash-miner --address YOUR_ADDRESS --threads 4
```

## Testing All Algorithms

Run the comprehensive test suite:

```bash
cd build
./test_all_algorithms
```

This tests:
- ✅ Keccak-256 (used by Ethash)
- ✅ Blake2b (used by GXHash)
- ✅ Argon2id (core of GXHash)
- ✅ Ethash (full implementation)
- ✅ GXHash (full implementation)
- ✅ SHA-256 (full implementation)

## Switching Algorithms

### On Running Node

1. Stop the node:
```bash
./gxc-cli stop
```

2. Edit config file:
```conf
miningalgorithm=GXHASH  # Change to desired algorithm
```

3. Restart node:
```bash
./gxc-node
```

### Important Notes

- ⚠️ **Blockchain validates all algorithms** - blocks mined with any algorithm are valid
- ⚠️ **Network consensus** - majority of miners should use same algorithm
- ⚠️ **Difficulty adjustment** - each algorithm has independent difficulty
- ⚠️ **Testing required** - always test on testnet before mainnet

## Recommendations

### For Production (Mainnet)
**Use GXHASH** for:
- Maximum decentralization
- ASIC resistance
- Fair distribution
- Long-term security

### For Testing (Testnet)
**Use GXHASH** (same as mainnet) for:
- Proper algorithm testing
- Real performance metrics
- Security validation
- Production readiness

### For Development
**Use SHA-256** for:
- Fast block generation
- Quick testing cycles
- Development debugging
- Local testing only

## Security Considerations

### ASIC Resistance
- **SHA-256**: ❌ ASICs dominate, centralization risk
- **Ethash**: ⚠️ ASICs exist but expensive
- **GXHash**: ✅ Memory-hard, ASIC-resistant

### Decentralization
- **SHA-256**: Mining pools dominate
- **Ethash**: GPU farms have advantage
- **GXHash**: CPU/GPU balanced, fair distribution

### Attack Resistance
- **51% Attack**: All algorithms vulnerable if attacker has majority hashrate
- **ASIC Attack**: Only GXHash provides strong resistance
- **GPU Attack**: GXHash and Ethash both resistant

## Conclusion

**The blockchain supports all 3 algorithms and they are all production-ready.**

**Default configuration uses GXHASH** for both mainnet and testnet to ensure:
- Proper testing of production algorithm
- ASIC resistance
- Decentralized mining
- Fair coin distribution

**You can configure any algorithm** based on your needs, but testnet should match mainnet for proper testing.
