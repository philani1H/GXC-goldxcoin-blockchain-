# 💰 GXC Blockchain - Official Tokenomics

## 📊 Supply Overview

**Maximum Supply:** 31,000,000 GXC (Hard Cap)

**Initial Block Reward:** 50 GXC

**Halving Schedule:** Every 1,051,200 blocks (~4 years)

**Genesis Block:** 50 GXC (NOT 50 million - that was a bug!)

---

## 🔢 Supply Distribution

### **Block Reward Schedule**

| Era | Block Range | Reward per Block | Blocks | Total Mined |
|-----|-------------|------------------|--------|-------------|
| 1 | 0 - 1,051,199 | 50 GXC | 1,051,200 | 52,560,000 GXC |
| 2 | 1,051,200 - 2,102,399 | 25 GXC | 1,051,200 | 26,280,000 GXC |
| 3 | 2,102,400 - 3,153,599 | 12.5 GXC | 1,051,200 | 13,140,000 GXC |
| 4 | 3,153,600 - 4,204,799 | 6.25 GXC | 1,051,200 | 6,570,000 GXC |
| 5 | 4,204,800 - 5,255,999 | 3.125 GXC | 1,051,200 | 3,285,000 GXC |
| 6+ | 5,256,000+ | < 3.125 GXC | Continues | Until 31M cap |

**Note:** The actual total from this schedule would exceed 31M, so the code will need to enforce the 31M cap by stopping rewards once reached.

---

## ⏱️ Timeline

**Assuming 2-minute average block time:**

| Era | Duration | Cumulative Time | Approximate Date |
|-----|----------|-----------------|------------------|
| 1 | ~4 years | 4 years | 2029 |
| 2 | ~4 years | 8 years | 2033 |
| 3 | ~4 years | 12 years | 2037 |
| 4 | ~4 years | 16 years | 2041 |
| 5 | ~4 years | 20 years | 2045 |

**31M Cap Reached:** Approximately 20-25 years from genesis

---

## 🎯 Distribution Breakdown

### **Mining Rewards**

**Total from Mining:** ~31,000,000 GXC (capped)

**Distribution:**
- Era 1 (50 GXC): ~52.56M GXC potential (but capped at 31M)
- Actual distribution will stop when 31M is reached

**Fair Launch:**
- ✅ No premine (genesis = 50 GXC only)
- ✅ No ICO
- ✅ No team allocation
- ✅ 100% mined through PoW/PoS

### **Genesis Block**

**Genesis Reward:** 50 GXC

**Genesis Address:**
- Testnet: `tGXC9fab7317231b966af85ac453e168c0932`
- Mainnet: `GXC9fab7317231b966af85ac453e168c0932`

**Purpose:** Bootstrap the blockchain with minimal initial supply

---

## 💸 Inflation Schedule

### **Annual Inflation Rate**

Assuming 2-minute block time = 262,800 blocks/year:

| Year | Blocks/Year | Reward | New Supply | Circulating | Inflation Rate |
|------|-------------|--------|------------|-------------|----------------|
| 1 | 262,800 | 50 GXC | 13,140,000 | 13,140,000 | N/A (first year) |
| 2 | 262,800 | 50 GXC | 13,140,000 | 26,280,000 | 100% |
| 3 | 262,800 | 50 GXC | 13,140,000 | 31,000,000* | 17.9% (capped) |
| 4 | 262,800 | 50 GXC | 0* | 31,000,000 | 0% (cap reached) |
| 5+ | 262,800 | 25 GXC | 0* | 31,000,000 | 0% (cap reached) |

*Cap enforcement will stop new supply once 31M is reached

### **Deflationary Mechanisms**

1. **Transaction Fee Burning**
   - Portion of transaction fees are burned
   - Reduces circulating supply over time
   - Configurable burn rate

2. **Hard Cap**
   - 31M maximum supply enforced
   - No new coins after cap reached
   - Deflationary after cap

---

## 🔐 Supply Security

### **Cap Enforcement**

The 31M cap is defined in `blockchain.h`:

```cpp
static const uint32_t MAX_SUPPLY = 31000000; // 31 million GXC
static const double MAX_MONEY = 31000000.0;  // Utils.h
```

**Implementation Required:**
```cpp
double Blockchain::calculateBlockReward(uint32_t height) const {
    // Check if max supply reached
    if (totalSupply >= MAX_SUPPLY) {
        return 0.00000001; // Minimum reward for security
    }
    
    // Calculate reward with halving
    const uint32_t HALVING_INTERVAL = 1051200;
    const double INITIAL_REWARD = 50.0;
    
    uint32_t halvings = height / HALVING_INTERVAL;
    double reward = INITIAL_REWARD;
    
    for (uint32_t i = 0; i < halvings; i++) {
        reward /= 2.0;
    }
    
    // Ensure we don't exceed max supply
    if (totalSupply + reward > MAX_SUPPLY) {
        reward = MAX_SUPPLY - totalSupply;
    }
    
    return std::max(reward, 0.00000001);
}
```

---

## 📈 Economic Model

### **Hybrid Consensus**

**Proof of Work (PoW):**
- SHA-256, Ethash, GXHash algorithms
- Block rewards distributed to miners
- Secures the network

**Proof of Stake (PoS):**
- Minimum stake: 100 GXC
- Validators earn transaction fees
- Additional security layer

### **Transaction Fees**

**Fee Structure:**
- Base fee: Dynamic based on network congestion
- Priority fee: Optional for faster confirmation
- Burn rate: Configurable % of fees burned

**Fee Distribution:**
- Miners: 50%
- Validators: 30%
- Burned: 20%

---

## 🎁 Reward Calculation

### **Block Reward Formula**

```python
def calculate_block_reward(height):
    HALVING_INTERVAL = 1_051_200
    INITIAL_REWARD = 50.0
    MAX_SUPPLY = 31_000_000
    
    # Calculate halvings
    halvings = height // HALVING_INTERVAL
    reward = INITIAL_REWARD / (2 ** halvings)
    
    # Minimum reward
    reward = max(reward, 0.00000001)
    
    # Check supply cap
    if current_supply >= MAX_SUPPLY:
        reward = 0.00000001
    elif current_supply + reward > MAX_SUPPLY:
        reward = MAX_SUPPLY - current_supply
    
    return reward
```

### **Example Rewards**

```
Block 0: 50 GXC
Block 100,000: 50 GXC
Block 1,051,200: 25 GXC (first halving)
Block 2,102,400: 12.5 GXC (second halving)
Block 3,153,600: 6.25 GXC (third halving)
...
Block when 31M reached: 0.00000001 GXC (minimum)
```

---

## 🔍 Supply Verification

### **Check Current Supply**

```bash
# Via RPC
curl -X POST http://localhost:8332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getsupplyinfo","params":[],"id":1}'

# Response:
{
  "current_supply": 13140000.0,
  "max_supply": 31000000.0,
  "circulating_supply": 13140000.0,
  "burned_supply": 0.0,
  "inflation_rate": 100.0,
  "blocks_until_halving": 788400
}
```

### **Check Block Reward**

```bash
# Via RPC
curl -X POST http://localhost:8332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockreward","params":[1000000],"id":1}'

# Response:
{
  "height": 1000000,
  "reward": 50.0,
  "era": 1,
  "halvings": 0,
  "blocks_until_halving": 51200
}
```

---

## 📊 Comparison with Other Cryptocurrencies

| Coin | Max Supply | Initial Reward | Halving Interval | Time to Cap |
|------|------------|----------------|------------------|-------------|
| **GXC** | **31M** | **50 GXC** | **~4 years** | **~20-25 years** |
| Bitcoin | 21M | 50 BTC | ~4 years | ~140 years |
| Litecoin | 84M | 50 LTC | ~4 years | ~140 years |
| Ethereum | Unlimited | Variable | N/A | N/A |

**GXC Advantages:**
- ✅ Faster cap achievement (20-25 years vs 140 years)
- ✅ Deflationary sooner
- ✅ Scarcity premium
- ✅ Fair distribution (no premine)

---

## ⚠️ Important Notes

### **Genesis Block Correction**

**Previous (WRONG):**
```cpp
output.amount = 50000000.0; // ❌ 50M GXC - MORE THAN MAX SUPPLY!
```

**Current (CORRECT):**
```cpp
output.amount = 50.0; // ✅ 50 GXC - Same as initial block reward
```

**Why This Matters:**
- The genesis block was giving 50 million GXC
- This is MORE than the 31 million max supply!
- This would break the entire economic model
- Now fixed to 50 GXC (fair launch)

### **Supply Cap Enforcement**

**Current Status:** ⚠️ Cap defined but not enforced in code

**Required Implementation:**
- Add supply tracking in Blockchain class
- Check totalSupply before issuing rewards
- Stop rewards when 31M reached
- Maintain minimum reward for security (0.00000001 GXC)

**Priority:** HIGH (before mainnet launch)

---

## 🎯 Summary

**GXC Tokenomics:**
- ✅ 31 million max supply (hard cap)
- ✅ 50 GXC initial block reward
- ✅ Halving every ~4 years
- ✅ Fair launch (no premine)
- ✅ Deflationary after cap
- ✅ Fee burning mechanism
- ⚠️ Cap enforcement needs implementation

**Genesis Block:**
- ✅ 50 GXC (CORRECTED from 50M)
- ✅ Goes to valid wallet address
- ✅ Fair distribution starts from block 0

**Economic Model:**
- ✅ Hybrid PoW/PoS
- ✅ Multiple mining algorithms
- ✅ Transaction fee burning
- ✅ Scarcity-driven value

---

*Last Updated: December 1, 2025*
*Version: 2.0.0*
*Status: Genesis Corrected, Cap Enforcement Pending*
