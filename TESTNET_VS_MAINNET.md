# Testnet vs Mainnet - Current Configuration

## YES, They Are Different!

### Quick Comparison

| Feature | Testnet | Mainnet |
|---------|---------|---------|
| **Address Prefix** | `tGXC` | `GXC` |
| **Initial Difficulty** | **1.0** | **1000.0** |
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
**SAME for both:**
- Mandatory coinbase at index 0
- Minimum difficulty 1.0 (enforced by consensus)
- Maximum difficulty 1000000.0
- Coinbase maturity 100 blocks
- PoW validation required
- SecurityEngine active

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

3. **SecurityEngine Works on Both**
   - Testnet: Adjusts from 1.0 baseline
   - Mainnet: Adjusts from 1000.0 baseline
   - Both: Bounded by MIN 1.0, MAX 1000000.0

4. **Always Test First**
   - Test on testnet
   - Verify everything works
   - Then deploy to mainnet

---

## Summary

**Testnet:** Easy mining (1.0 difficulty), fast testing, no real value
**Mainnet:** Hard mining (1000.0 difficulty), production use, real value

Both networks use the same consensus rules and SecurityEngine, just different starting points.
