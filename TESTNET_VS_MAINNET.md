# Testnet vs Mainnet - Current Configuration

## YES, They Are Different!

### Quick Comparison

| Feature | Testnet | Mainnet |
|---------|---------|---------|
| **Address Prefix** | `tGXC` | `GXC` |
| **Initial Difficulty** | **1.0** | **1000.0** |
| **Min Difficulty** | **1.0** | **1000.0** |
| **Max Difficulty** | **10,000** | **10,000,000** |
| **Database** | `blockchain_testnet.db` | `blockchain.db` |
| **Purpose** | Testing | Production |
| **Coin Value** | None (testing only) | Real value |
| **Mining Speed** | Fast (~16 attempts) | Slow (millions of attempts) |
| **Network Port** | 18333 | 8333 |
| **RPC Port** | 18332 | 8332 |

---

## Key Differences

### 1. Difficulty
- **Testnet:** Starts at 1.0 (1 leading zero = ~16 attempts)
- **Mainnet:** Starts at 1000.0 (requires serious mining hardware)

### 2. Addresses
- **Testnet:** `tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n`
- **Mainnet:** `GXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n`
- **Validation:** Transactions with wrong prefix are REJECTED

### 3. Databases
- **Testnet:** `./gxc_data/blockchain_testnet.db` (can be deleted)
- **Mainnet:** `./gxc_data/blockchain.db` (contains real value)

### 4. Consensus Rules
**SAME rules, DIFFERENT bounds:**
- Mandatory coinbase at index 0 ✅ SAME
- Coinbase maturity 100 blocks ✅ SAME
- PoW validation required ✅ SAME
- SecurityEngine active ✅ SAME
- **Minimum difficulty:** Testnet 1.0, Mainnet 1000.0 ❗ DIFFERENT
- **Maximum difficulty:** Testnet 10,000, Mainnet 10,000,000 ❗ DIFFERENT

---

## How to Use

### Start Testnet
```bash
./gxc-node --testnet --rpc-port=8332
```

### Start Mainnet
```bash
./gxc-node --rpc-port=8332
```

### Check Network
```bash
curl -s http://localhost:8332 | python3 -m json.tool
```

**Output:**
```json
{
    "network": "testnet",  // or "mainnet"
    "difficulty": 1.0,     // or 1000.0
    "height": 14
}
```

---

## Why Different Difficulties?

### Testnet (1.0)
- **Purpose:** Fast testing
- **Mining:** ~16 attempts per block
- **Time:** ~0.001 seconds
- **Hardware:** Any CPU
- **Cost:** Free

### Mainnet (1000.0)
- **Purpose:** Real security
- **Mining:** Millions of attempts per block
- **Time:** Hours/days
- **Hardware:** Serious mining rigs
- **Cost:** Electricity + hardware

---

## Important Notes

1. **Coins NOT Interchangeable**
   - Testnet coins = no value
   - Mainnet coins = real value
   - Cannot send between networks

2. **Same Codebase**
   - Both use same blockchain code
   - Both use same consensus rules
   - Only difference: initial difficulty and address prefix

3. **SecurityEngine Works on Both (with different bounds)**
   - Testnet: Adjusts from 1.0 baseline, bounded by MIN 1.0, MAX 10,000
   - Mainnet: Adjusts from 1000.0 baseline, bounded by MIN 1000.0, MAX 10,000,000
   - Mainnet minimum = initial difficulty (prevents difficulty from dropping below launch level)

4. **Always Test First**
   - Test on testnet
   - Verify everything works
   - Then deploy to mainnet

---

## Summary

**Testnet:** Easy mining (1.0 difficulty), fast testing, no real value
**Mainnet:** Hard mining (1000.0 difficulty), production use, real value

Both networks use the same consensus rules and SecurityEngine, just different starting points.
