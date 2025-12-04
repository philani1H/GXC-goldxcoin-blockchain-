# Staking Logic Fix Summary

This document summarizes all the changes made to fix the blockchain staking implementation.

## Problems Identified

1. **Staking didn't deduct coins from balance** - Users could stake without actually locking their coins
2. **No STAKE transaction was created** - Staking was just a database entry without on-chain verification
3. **Double spending possible** - Staked coins could still be spent
4. **Mining rewards showing 0 balance** - UTXO set not properly tracking coinbase outputs
5. **Send transactions failing** - Fee handling was incorrect
6. **Traceability formula not enforced** - Missing proper input/output validation

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
- Improved normal tx validation with proper fee and traceability checks

**updateUtxoSet() improvements:**
- STAKE processing: 
  1. Calculate input total BEFORE removing from UTXO
  2. Remove spent UTXOs
  3. Calculate staked amount = inputs - outputs - fee
  4. Update validator's stake in memory and DB
  5. Add only change outputs as UTXOs

- UNSTAKE processing:
  1. Get unstake amount from outputs
  2. Reduce validator's stake using proper `removeStake()`
  3. Add outputs as new spendable UTXOs

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

## Traceability Formula

All non-coinbase transactions must satisfy:
```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

This creates a verifiable chain of coin provenance.

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
