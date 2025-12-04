# GXC Blockchain: Staking & Traceability Fix Summary

This document summarizes all the changes made to fix the blockchain staking implementation and enforce the traceability formula.

## Problems Identified & Fixed

| Problem | Status | Fix |
|---------|--------|-----|
| Staking didn't deduct coins from balance | ✅ FIXED | STAKE tx consumes UTXOs, locks coins |
| No STAKE transaction was created | ✅ FIXED | `createStakeTransaction()` creates on-chain tx |
| Double spending possible | ✅ FIXED | UTXOs removed when spent |
| Mining rewards showing 0 balance | ✅ FIXED | Coinbase outputs added to UTXO set |
| Send transactions failing | ✅ FIXED | Proper fee handling in `createTransaction()` |
| Traceability formula not enforced | ✅ FIXED | `prevTxHash = inputs[0].txHash` enforced |
| UTXO validation missing | ✅ FIXED | All inputs verified against UTXO set |
| Input amount mismatch | ✅ FIXED | Input amounts verified against UTXO amounts |

## The Correct Staking Model

When a user stakes:

```
balance[address] -= stakeAmount
staked[address] += stakeAmount
```

Coins move from spendable balance to the staking pool. They remain owned but cannot be spent.

### STAKE Transaction Structure

- **Inputs**: UTXOs consumed from user's balance
- **Outputs**: Only change (NO output for staked amount - it's "burned" into stake pool)
- **Staked Amount** = Total Inputs - Total Outputs - Fee

### UNSTAKE Transaction Structure

- **Inputs**: Dummy input for signature verification
- **Outputs**: Amount returned to spendable balance
- **Result**: Creates new UTXOs for the unstaked amount

## Files Modified

### 1. `/workspace/include/Wallet.h`

Added new methods:
- `createStakeTransaction()` - Creates STAKE tx with proper coin locking
- `createUnstakeTransaction()` - Creates UNSTAKE tx to return coins
- Updated `createTransaction()` to properly handle fees (default 0.001 GXC)

### 1b. `/workspace/src/Wallet.cpp` - Traceability Enforcement

**CRITICAL FIX**: Both `createTransaction()` and `createStakeTransaction()` now enforce:
```cpp
// TRACEABILITY FORMULA ENFORCEMENT:
// Ti.Inputs[0].txHash == Ti.PrevTxHash
// Ti.Inputs[0].amount == Ti.ReferencedAmount
prevTxHash = inputs[0].txHash;         // NOT lastTxHash!
referencedAmount = inputs[0].amount;   // From actual input
```

This ensures the traceability formula ALWAYS holds.

### 2. `/workspace/src/Wallet.cpp`

Implemented:
- **`createTransaction()`** - Now properly calculates change including fee
- **`createStakeTransaction()`** - Creates tx that consumes inputs but only outputs change (stakes the rest)
- **`createUnstakeTransaction()`** - Creates tx that outputs unstaked amount back to wallet

Key logic for STAKE:
```cpp
double totalRequired = stakeAmount + fee;
// ... collect inputs ...
double change = availableAmount - stakeAmount - fee;
// Only create change output - staked amount has no output (locked)
if (change > 0) {
    outputs.push_back(changeOutput);
}
```

### 3. `/workspace/include/Validator.h`

Added:
- `removeStake(double amount)` - Properly reduces validator stake

### 4. `/workspace/src/Validator.cpp`

Implemented:
- **`removeStake()`** - Reduces stake, auto-deactivates if below minimum

```cpp
void Validator::removeStake(double amount) {
    stakeAmount -= amount;
    if (stakeAmount < MIN_STAKE) {
        isActive = false;
    }
}
```

### 5. `/workspace/src/Blockchain.cpp`

**validateTransaction() improvements:**
- Added STAKE validation: Checks inputs exist, calculates staked amount, validates minimum
- Added UNSTAKE validation: Verifies staked balance, prevents over-unstaking
- **NEW: UTXO validation for ALL transactions:**
  ```cpp
  // Verify each input references a valid, unspent UTXO
  for (const auto& input : tx.getInputs()) {
      std::string utxoKey = input.txHash + "_" + std::to_string(input.outputIndex);
      auto it = utxoSet.find(utxoKey);
      if (it == utxoSet.end()) {
          return false;  // Double-spend attempt!
      }
      // Also verify input amount matches UTXO amount
      if (std::abs(it->second.amount - input.amount) > 0.00000001) {
          return false;  // Amount mismatch!
      }
  }
  ```

**updateUtxoSet() improvements:**
- STAKE processing: 
  1. Calculate input total BEFORE removing from UTXO
  2. Remove spent UTXOs (coins locked)
  3. Calculate staked amount = inputs - outputs - fee
  4. Update validator's stake in memory and DB
  5. Add only change outputs as UTXOs

- UNSTAKE processing:
  1. Get unstake amount from outputs
  2. Reduce validator's stake using proper `removeStake()`
  3. Add outputs as new spendable UTXOs

- NORMAL processing:
  1. Remove input UTXOs (sender's coins spent)
  2. Add output UTXOs (receiver gets coins, sender gets change)

### 6. `/workspace/src/RPCAPI.cpp`

**registerValidator() rewritten:**
- Now creates proper STAKE transaction using `wallet->createStakeTransaction()`
- Verifies wallet controls the address
- Checks sufficient balance (stake + fee)
- Creates on-chain STAKE transaction
- Returns transaction hash for verification

**unstake() rewritten:**
- Uses `wallet->createUnstakeTransaction()`
- Validates staked amount before processing
- Returns remaining stake after unstake

**Added getStakingInfo():**
- Returns complete staking information:
  - `spendable_balance` - Coins in UTXO set
  - `staked_balance` - Coins locked in staking
  - `total_balance` - Sum of both
  - `total_earned_mining` - PoW rewards
  - `total_earned_staking` - PoS rewards
  - Validator-specific info (rewards, APY, blocks produced)

### 7. `/workspace/include/RPCAPI.h`

Added declaration for `getStakingInfo()` method.

## How Staking Now Works

### Register as Validator (Stake)

1. User calls `registervalidator` with address, amount, days
2. System verifies:
   - Wallet controls the address
   - Balance sufficient for stake + fee
   - Minimum stake (100 GXC) and valid staking period (14-365 days)
3. Creates STAKE transaction:
   - Inputs: User's UTXOs
   - Outputs: Change only (staked amount becomes "locked")
4. Transaction added to mempool
5. When mined, `updateUtxoSet()`:
   - Removes input UTXOs
   - Calculates staked amount
   - Updates validator record
   - Adds change UTXOs

### Validator Selection (PoS)

```cpp
double totalWeightedStake = sum(validators.weightedStake)
// Weighted random selection based on stake
selectedValidator = random_weighted_choice(validators, weights=weightedStake)
```

### Block Rewards

- PoW/PoS block rewards go to **spendable balance** (coinbase UTXO)
- Rewards are NOT added to staked balance
- This matches ETH, Cardano, Cosmos behavior

### Unstake

1. User calls `unstake` with address and optional amount
2. System verifies:
   - Wallet controls address
   - Has sufficient staked balance
3. Creates UNSTAKE transaction:
   - Outputs: Unstaked amount to user's address
4. When mined:
   - Validator's stake reduced
   - New UTXOs created for unstaked coins

## Traceability Formula (PRIORITY)

**The traceability formula is the CORE PRIORITY of this blockchain.**

All non-coinbase transactions must satisfy:
```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

This creates a verifiable chain of coin provenance.

### How Traceability Works

```
COINBASE (Block 1)
├── txHash: "abc123..."
├── outputs: [{address: Alice, amount: 50.0}]
└── UTXO created: "abc123..._0" → Alice: 50 GXC

SEND (Alice → Bob: 30 GXC)
├── prevTxHash: "abc123..."  ← MUST match inputs[0].txHash
├── referencedAmount: 50.0   ← MUST match inputs[0].amount
├── inputs: [{txHash: "abc123...", outputIndex: 0, amount: 50.0}]
├── outputs: 
│   ├── [{address: Bob, amount: 30.0}]    ← Receiver gets coins
│   └── [{address: Alice, amount: 19.999}] ← Change back to sender
├── fee: 0.001
└── Result:
    ├── UTXO deleted: "abc123..._0" (Alice's old coins SPENT)
    ├── UTXO created: "def456..._0" (Bob gets 30 GXC)
    └── UTXO created: "def456..._1" (Alice gets 19.999 GXC change)
```

### Traceability Validation in Code

```cpp
// Transaction.cpp - verifyTraceabilityFormula()
if (inputs[0].txHash != prevTxHash) {
    return false;  // FAIL: Hash mismatch
}
if (std::abs(inputs[0].amount - referencedAmount) > 0.00000001) {
    return false;  // FAIL: Amount mismatch
}
return true;

// Blockchain.cpp - validateTransaction()
// Also verifies:
// 1. Each input references a valid UTXO (exists in utxoSet)
// 2. Input amount matches the UTXO amount
// 3. Total inputs >= Total outputs + fee
```

### Why Traceability Matters

1. **Prevents Double-Spending**: Each UTXO can only be spent once
2. **Creates Audit Trail**: Every coin can be traced back to its coinbase origin
3. **Enables Verification**: Anyone can verify the complete history of any coin
4. **Prevents Coin Creation**: Inputs must equal outputs + fee (no money from nothing)

## API Endpoints

| Endpoint | Description |
|----------|-------------|
| `registervalidator` | Register as validator with stake |
| `stake` | Alias for registervalidator |
| `unstake` | Withdraw staked coins |
| `addstake` | Add more stake to existing validator |
| `getvalidators` | List all active validators |
| `getvalidatorinfo` | Get details for one validator |
| `getstakinginfo` | Get complete staking info for address |

## How Sending Coins Works

When Alice sends 30 GXC to Bob:

### Step 1: Create Transaction (Wallet)
```cpp
// Wallet::createTransaction(recipient, amount, utxoSet, fee)

1. Find UTXOs belonging to Alice that total >= amount + fee
2. Create input referencing each UTXO being spent
3. Create output to Bob for the amount (30 GXC)
4. Create change output back to Alice (inputs - amount - fee)
5. Set prevTxHash = inputs[0].txHash (TRACEABILITY!)
6. Set referencedAmount = inputs[0].amount (TRACEABILITY!)
7. Sign all inputs with Alice's private key
```

### Step 2: Validate Transaction (Blockchain)
```cpp
// Blockchain::validateTransaction(tx)

1. Check traceability formula holds
2. Verify each input references a valid UTXO
3. Verify input amounts match UTXO amounts  
4. Check total inputs >= total outputs + fee
5. Verify signatures are valid
```

### Step 3: Update UTXO Set (When Block is Mined)
```cpp
// Blockchain::updateUtxoSet(block)

For each transaction:
1. REMOVE inputs from UTXO set (spent coins deleted)
2. ADD outputs to UTXO set (new coins created)
   - Bob's output becomes spendable
   - Alice's change becomes spendable
```

### Result
- **Alice's balance reduced** by amount + fee
- **Bob's balance increased** by amount
- **Miner receives fee** via coinbase transaction

## Balance Calculation

```cpp
spendable_balance = sum(UTXOs for address)  // From UTXO set
staked_balance = validator.stakeAmount       // From validator registry
total_balance = spendable_balance + staked_balance
```

## Security Guarantees

1. ✅ **No double spending** - Staked coins removed from UTXO set
2. ✅ **On-chain verification** - STAKE/UNSTAKE are real transactions
3. ✅ **Balance integrity** - inputs = outputs + fee (or inputs = change + staked + fee)
4. ✅ **Traceability** - All transactions traceable to origin
5. ✅ **Rewards to spendable** - Validators can spend rewards without unstaking
