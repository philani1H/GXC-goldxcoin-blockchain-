# 💰 Validator Stake Amount Explained

## Quick Answer

**YES** - The amount you specify when registering as a validator **IS** the amount that gets staked (locked).

---

## How It Works

### When You Register as Validator

```bash
# Register with 1000 GXC stake
registervalidator "tGXC_your_address" 1000.0 365

# OR (external wallet)
registerexternalvalidator "tGXC_your_address" 1000.0 365
```

**What happens:**

1. **Validator record created** with your address and staking period
2. **STAKE transaction created** automatically
3. **1000 GXC locked** (removed from spendable balance)
4. **Validator activated** when transaction confirms in next block

---

## The Flow in Detail

### Step 1: Check Your Balance

```bash
# You need: stake amount + fee
# Example: 1000 GXC stake + 0.001 GXC fee = 1000.001 GXC total

getbalance "tGXC_your_address"
# Returns: 1500.0 GXC (you have enough!)
```

### Step 2: Register as Validator

```bash
registervalidator "tGXC_your_address" 1000.0 365
```

**Behind the scenes:**
```cpp
// From RPCAPI.cpp - registerValidator()

// 1. Validate you have enough balance
double fee = 0.001;
double totalRequired = stakeAmount + fee;  // 1000.001 GXC
double balance = blockchain->getBalance(address);  // 1500.0 GXC

if (balance < totalRequired) {
    throw "Insufficient funds";
}

// 2. Create STAKE transaction
Transaction stakeTx = wallet->createStakeTransaction(stakeAmount, utxoSet, fee);
// This transaction:
// - Consumes UTXOs totaling >= 1000.001 GXC
// - Locks 1000 GXC as stake
// - Pays 0.001 GXC as fee
// - Returns change (if any) to your address

// 3. Add to mempool
blockchain->addTransaction(stakeTx);

// 4. Create validator record
Validator validator(address, 0.0, stakingDays);  // Initially 0 stake
validator.setPending(true);  // Pending until tx confirms
blockchain->registerValidator(validator);
```

### Step 3: Transaction Confirms in Block

```cpp
// From Blockchain.cpp - updateUtxoSet()

// When block is mined with your STAKE transaction:

// 1. Calculate staked amount
double totalInput = 1000.001;  // Your UTXOs
double totalOutput = 0.0;      // No outputs (all locked)
double fee = 0.001;
double stakedAmount = totalInput - totalOutput - fee;  // 1000 GXC

// 2. Update validator's stake
for (auto& v : validators) {
    if (v.getAddress() == stakerAddress) {
        v.addStake(stakedAmount);  // Add 1000 GXC to validator
        
        // From Validator.cpp - addStake()
        stakeAmount += amount;  // 0 + 1000 = 1000 GXC
        
        if (stakeAmount >= MIN_STAKE) {  // 1000 >= 100
            isActive = true;      // ✅ Validator activated!
            isPending = false;    // No longer pending
        }
    }
}

// 3. Remove spent UTXOs from UTXO set
// Your 1000.001 GXC is now locked as stake
```

### Step 4: Check Your New Balances

```bash
# Spendable balance (reduced)
getbalance "tGXC_your_address"
# Returns: 499.999 GXC (1500 - 1000 - 0.001)

# Staked balance (new!)
getvalidatorinfo "tGXC_your_address"
# Returns:
{
  "address": "tGXC_your_address",
  "stakeAmount": 1000.0,      # ← Your locked stake
  "isActive": true,
  "isPending": false,
  "stakingDays": 365
}
```

---

## Balance Breakdown

### Before Registration

```
Total Balance: 1500 GXC
├─ Spendable: 1500 GXC  ✅ Can send/spend
└─ Staked: 0 GXC
```

### After Registration (1000 GXC stake)

```
Total Balance: 1500 GXC
├─ Spendable: 499.999 GXC  ✅ Can send/spend
└─ Staked: 1000 GXC        🔒 Locked (earning rewards)
```

**Transaction fee:** 0.001 GXC (burned)

---

## Important Points

### 1. Stake Amount = Locked Amount

```bash
# If you register with 1000 GXC
registervalidator "address" 1000.0 365

# Then 1000 GXC is LOCKED
# You CANNOT spend it until you unstake
```

### 2. Minimum Stake Required

```cpp
// From Validator.h
static constexpr double MIN_STAKE = 100.0;  // 100 GXC minimum
```

```bash
# ❌ This will fail
registervalidator "address" 50.0 365
# Error: Stake amount must be at least 100 GXC

# ✅ This works
registervalidator "address" 100.0 365
```

### 3. You Need Extra for Fee

```bash
# To stake 1000 GXC, you need:
# - 1000 GXC for stake
# - 0.001 GXC for transaction fee
# = 1000.001 GXC total

# ❌ This will fail
# Balance: 1000.0 GXC
registervalidator "address" 1000.0 365
# Error: Insufficient funds. Required: 1000.001 GXC, Available: 1000.0 GXC

# ✅ This works
# Balance: 1000.001 GXC or more
registervalidator "address" 1000.0 365
```

### 4. Stake Earns Rewards

```bash
# Your staked amount earns block rewards
# When you create PoS blocks, you receive:
# - Block reward: 50 GXC
# - Transaction fees: varies

# Rewards go to your SPENDABLE balance, not locked stake
# So your spendable balance grows while stake stays locked
```

### 5. Unstaking Returns Your Coins

```bash
# To get your 1000 GXC back
unstake "tGXC_your_address" 1000.0

# After unstake transaction confirms:
# - Staked: 0 GXC
# - Spendable: 1499.999 GXC (original 499.999 + 1000 returned)
```

---

## Examples

### Example 1: Register with Minimum Stake

```bash
# Starting balance
getbalance "tGXC_address"
# Returns: 200.0 GXC

# Register with minimum stake
registervalidator "tGXC_address" 100.0 365

# After confirmation
getbalance "tGXC_address"
# Returns: 99.999 GXC (200 - 100 - 0.001)

getvalidatorinfo "tGXC_address"
# Returns: { "stakeAmount": 100.0, "isActive": true }
```

### Example 2: Register with Large Stake

```bash
# Starting balance
getbalance "tGXC_address"
# Returns: 10000.0 GXC

# Register with large stake
registervalidator "tGXC_address" 5000.0 365

# After confirmation
getbalance "tGXC_address"
# Returns: 4999.999 GXC (10000 - 5000 - 0.001)

getvalidatorinfo "tGXC_address"
# Returns: { "stakeAmount": 5000.0, "isActive": true }
```

### Example 3: Add More Stake Later

```bash
# Current state
getvalidatorinfo "tGXC_address"
# Returns: { "stakeAmount": 1000.0, "isActive": true }

getbalance "tGXC_address"
# Returns: 2000.0 GXC

# Add 500 more GXC to stake
addstake "tGXC_address" 500.0 365

# After confirmation
getvalidatorinfo "tGXC_address"
# Returns: { "stakeAmount": 1500.0, "isActive": true }

getbalance "tGXC_address"
# Returns: 1499.999 GXC (2000 - 500 - 0.001)
```

---

## UTXO Mechanics

### How Staking Works with UTXOs

**Before staking:**
```
Your UTXOs:
├─ UTXO 1: 500 GXC   (from mining)
├─ UTXO 2: 300 GXC   (from mining)
└─ UTXO 3: 700 GXC   (from mining)
Total: 1500 GXC
```

**When you stake 1000 GXC:**
```
STAKE Transaction:
Inputs:
├─ UTXO 1: 500 GXC   ✅ Consumed
├─ UTXO 2: 300 GXC   ✅ Consumed
└─ UTXO 3: 700 GXC   ✅ Consumed (partial)
Total Input: 1500 GXC

Outputs:
└─ Change: 499.999 GXC → Back to your address

Locked as Stake: 1000 GXC
Fee: 0.001 GXC
```

**After staking:**
```
Your UTXOs:
└─ UTXO 4: 499.999 GXC   (change from stake tx)
Total Spendable: 499.999 GXC

Your Stake:
└─ Locked: 1000 GXC   🔒 (not a UTXO, tracked separately)
```

---

## Validator Selection Weight

### Higher Stake = Higher Selection Probability

```cpp
// From Validator.cpp
double Validator::getWeightedStake() const {
    double baseWeight = stakeAmount;
    
    // Age bonus (up to 20% for 1 year)
    double ageBonus = std::min(0.2, (stakingDays / 365.0) * 0.2);
    
    return baseWeight * (1.0 + ageBonus);
}
```

**Example:**
```
Validator A: 1000 GXC stake, 365 days
Weighted Stake: 1000 * (1 + 0.2) = 1200

Validator B: 500 GXC stake, 365 days
Weighted Stake: 500 * (1 + 0.2) = 600

Validator A has 2x higher chance of being selected to create blocks!
```

---

## Common Questions

### Q: Can I stake more than I have?

**A:** No. You need `stakeAmount + 0.001 GXC` in your balance.

```bash
# Balance: 500 GXC
registervalidator "address" 1000.0 365
# ❌ Error: Insufficient funds
```

### Q: Can I spend my staked coins?

**A:** No. Staked coins are locked until you unstake.

```bash
# Staked: 1000 GXC
# Spendable: 100 GXC

sendtoaddress "recipient" 500.0
# ❌ Error: Insufficient funds (only 100 GXC spendable)
```

### Q: Do rewards add to my stake?

**A:** No. Block rewards go to your **spendable balance**, not your stake.

```bash
# Before creating block
Staked: 1000 GXC
Spendable: 100 GXC

# After creating block (50 GXC reward)
Staked: 1000 GXC        (unchanged)
Spendable: 150 GXC      (100 + 50)
```

### Q: Can I partially unstake?

**A:** Yes! You can unstake any amount up to your total stake.

```bash
# Current stake: 1000 GXC
unstake "address" 300.0

# After confirmation
# Staked: 700 GXC
# Spendable: +300 GXC
```

### Q: What happens if I unstake below minimum?

**A:** Your validator becomes inactive.

```bash
# Current stake: 150 GXC (active)
unstake "address" 100.0

# After confirmation
# Staked: 50 GXC
# Status: INACTIVE (below 100 GXC minimum)
# You won't be selected to create blocks until you stake more
```

---

## Summary

### Key Points

1. **Stake Amount = Locked Amount**
   - If you register with 1000 GXC, exactly 1000 GXC is locked

2. **You Need Extra for Fee**
   - Total required = stake + 0.001 GXC

3. **Minimum Stake: 100 GXC**
   - Can't register with less

4. **Stake is Locked**
   - Can't spend until you unstake

5. **Rewards are Spendable**
   - Block rewards go to spendable balance, not stake

6. **Higher Stake = More Blocks**
   - More stake = higher selection probability

7. **Can Add/Remove Stake**
   - Use `addstake` to increase
   - Use `unstake` to decrease/withdraw

---

## Visual Summary

```
┌─────────────────────────────────────────────────────────┐
│  REGISTER VALIDATOR WITH 1000 GXC                       │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│  BEFORE:                                                │
│  ├─ Total Balance: 1500 GXC                            │
│  ├─ Spendable: 1500 GXC                                │
│  └─ Staked: 0 GXC                                      │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│  STAKE TRANSACTION CREATED:                             │
│  ├─ Inputs: 1500 GXC (your UTXOs)                      │
│  ├─ Locked as Stake: 1000 GXC                          │
│  ├─ Fee: 0.001 GXC                                     │
│  └─ Change Output: 499.999 GXC                         │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│  AFTER (Transaction Confirmed):                         │
│  ├─ Total Balance: 1500 GXC                            │
│  ├─ Spendable: 499.999 GXC  ✅ Can spend              │
│  └─ Staked: 1000 GXC        🔒 Locked (earning)       │
│                                                         │
│  Validator Status:                                      │
│  ├─ Active: YES ✅                                     │
│  ├─ Stake: 1000 GXC                                    │
│  └─ Can create PoS blocks and earn rewards!            │
└─────────────────────────────────────────────────────────┘
```

---

**The amount you specify IS the amount that gets staked!** 💰🔒

---

*Last Updated: December 9, 2024*  
*Version: 1.0.0*  
*Status: Complete ✅*
