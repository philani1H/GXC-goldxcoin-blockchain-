# Validator Registration and Staking Flow

## Overview
This document explains how validator registration and staking works in the GXC blockchain, including the recent fixes to improve user experience.

## The Problem (Before Fix)
When users registered as validators, they would see:
- Validator created with **0 GXC stake**
- Status: "pending"
- Funds were locked but not visible

This was confusing because:
1. Users couldn't see their staked amount until block confirmation
2. It appeared that registration didn't stake the funds
3. The validator showed 0 stake even though funds were locked

## The Solution (After Fix)
Now when users register as validators:
- Validator shows the **actual stake amount immediately**
- Status: "pending_confirmation"
- Funds are locked in mempool and visible to user
- When block is mined, validator is activated

## Registration Flow

### Step 1: User Calls `registerValidator` RPC
```json
{
  "method": "registerValidator",
  "params": ["tGXC_address", 100.0, 30]
}
```

Parameters:
- `address`: Validator's wallet address
- `stakeAmount`: Amount to stake (minimum 100 GXC)
- `stakingDays`: Staking period (14-365 days)

### Step 2: System Creates STAKE Transaction
The RPC method:
1. Validates the user has sufficient balance (stake + fee)
2. Creates a STAKE transaction that:
   - Consumes UTXOs from the user's wallet
   - Locks the staked amount (not in outputs)
   - Creates change output if needed
3. Adds transaction to mempool
4. **Funds are now locked** (UTXOs marked as spent in mempool)

### Step 3: Validator Record Created
```cpp
Validator validator(address, stakeAmount, stakingDays);
validator.setPending(true);      // Waiting for confirmation
validator.setIsActive(false);    // Not active yet
```

**Key Change**: Validator now shows `stakeAmount` immediately (not 0)

### Step 4: User Sees Pending Validator
When user calls `getValidatorInfo`:
```json
{
  "address": "tGXC_address",
  "stake_amount": 100.0,           // ✅ Shows actual amount!
  "staking_days": 30,
  "is_active": false,
  "is_pending": true,
  "status": "pending_confirmation",
  "message": "Stake of 100.0 GXC is pending confirmation"
}
```

### Step 5: Block is Mined
When the STAKE transaction is included in a block:
1. `updateUtxoSet` is called
2. System finds validator in `validatorMap`
3. Checks if validator is pending
4. **Activates the validator**:
   ```cpp
   validator.setPending(false);
   validator.setIsActive(true);
   ```
5. UTXOs are permanently removed (funds locked in stake)
6. Change outputs added to UTXO set

### Step 6: Validator is Active
Now when user calls `getValidatorInfo`:
```json
{
  "address": "tGXC_address",
  "stake_amount": 100.0,
  "staking_days": 30,
  "is_active": true,              // ✅ Now active!
  "is_pending": false,
  "status": "active",
  "weighted_stake": 100.0,
  "can_produce_blocks": true
}
```

## Fund Locking Mechanism

### When are funds locked?
Funds are locked in **two stages**:

#### Stage 1: Mempool (Soft Lock)
- STAKE transaction added to mempool
- UTXOs marked as spent in mempool
- User cannot double-spend these UTXOs
- **Funds are effectively locked** but not confirmed

#### Stage 2: Block Confirmation (Hard Lock)
- STAKE transaction included in block
- UTXOs permanently removed from UTXO set
- Validator activated
- **Funds are permanently locked** in stake

### What happens to the funds?
```
Initial Balance: 200 GXC (in UTXOs)
Stake Amount: 100 GXC
Fee: 0.001 GXC

STAKE Transaction:
  Inputs:  200 GXC (consumed)
  Outputs: 99.999 GXC (change to user)
  Locked:  100 GXC (staked, not in outputs)
  Fee:     0.001 GXC (to miner)

After Confirmation:
  Spendable Balance: 99.999 GXC
  Staked Balance:    100 GXC
  Total Balance:     199.999 GXC
```

The staked 100 GXC is **not in the UTXO set** - it's locked in the validator's stake.

## Additional Staking

Users can add more stake to an existing validator:

### Using `addStake` RPC
```json
{
  "method": "addStake",
  "params": ["tGXC_address", 50.0]
}
```

This creates another STAKE transaction that:
1. Consumes more UTXOs
2. Locks additional 50 GXC
3. Updates validator's stake: 100 + 50 = 150 GXC

The system detects this is an **additional stake** (not first stake) because:
- Validator exists in `validatorMap`
- Validator is NOT pending (`isPending = false`)
- Validator is active (`isActive = true`)

## Unstaking

Users can unstake funds to return them to spendable balance:

### Using `unstake` RPC
```json
{
  "method": "unstake",
  "params": ["tGXC_address", 50.0]
}
```

This creates an UNSTAKE transaction that:
1. Has no real inputs (funds come from stake pool)
2. Creates output of 50 GXC to user's address
3. Reduces validator's stake: 150 - 50 = 100 GXC

When confirmed:
- Validator's stake updated
- New UTXO created for 50 GXC
- User can spend the 50 GXC

## Code Changes Summary

### 1. RPCAPI.cpp - registerValidator
**Before:**
```cpp
Validator validator(address, 0.0, stakingDays);  // ❌ Shows 0 stake
```

**After:**
```cpp
Validator validator(address, stakeAmount, stakingDays);  // ✅ Shows actual stake
validator.setPending(true);
validator.setIsActive(false);
```

### 2. Blockchain.cpp - updateUtxoSet (STAKE processing)
**Before:**
```cpp
// Always added stake amount
mapIt->second.addStake(stakedAmount);
```

**After:**
```cpp
if (mapIt->second.getIsPending()) {
    // First stake - just activate (amount already set)
    mapIt->second.setPending(false);
    mapIt->second.setIsActive(true);
} else {
    // Additional stake - add to existing
    mapIt->second.addStake(stakedAmount);
}
```

### 3. RPCAPI.cpp - getValidatorInfo
**Added:**
```cpp
result["is_pending"] = validator.getIsPending();
result["status"] = validator.getIsPending() ? "pending_confirmation" : 
                  (validator.getIsActive() ? "active" : "inactive");
```

## User Experience Improvements

### Before Fix
```
User: "I registered with 100 GXC"
System: "Validator created with 0 GXC stake"
User: "Where are my funds?!" 😰
```

### After Fix
```
User: "I registered with 100 GXC"
System: "Validator created with 100 GXC stake (pending confirmation)"
User: "Great! I can see my stake." 😊
```

## Security Considerations

### Why not activate immediately?
Validators are not activated until the STAKE transaction is confirmed because:
1. **Blockchain consistency**: Only confirmed transactions should affect consensus
2. **Double-spend prevention**: Mempool transactions can be replaced
3. **Network security**: Prevents fake validators from participating in PoS

### Why show stake amount before confirmation?
1. **User experience**: Users need to see their funds are being staked
2. **Transparency**: Clear indication of pending stake
3. **No security risk**: Validator is marked as `pending` and `inactive`

The validator **cannot produce blocks** until:
- STAKE transaction is confirmed
- `isPending = false`
- `isActive = true`

## Testing

Test the flow:
```bash
# 1. Register validator
curl -X POST http://localhost:8545 -d '{
  "jsonrpc": "2.0",
  "method": "registerValidator",
  "params": ["tGXC_your_address", 100.0, 30],
  "id": 1
}'

# Response shows stake immediately:
{
  "stake_amount": 100.0,
  "status": "pending",
  "message": "Stake of 100.0 GXC is pending confirmation"
}

# 2. Check validator info
curl -X POST http://localhost:8545 -d '{
  "jsonrpc": "2.0",
  "method": "getValidatorInfo",
  "params": ["tGXC_your_address"],
  "id": 1
}'

# Response shows pending status:
{
  "stake_amount": 100.0,
  "is_active": false,
  "is_pending": true,
  "status": "pending_confirmation"
}

# 3. Wait for block confirmation...

# 4. Check again
# Response shows active status:
{
  "stake_amount": 100.0,
  "is_active": true,
  "is_pending": false,
  "status": "active"
}
```

## Related Files
- `src/RPCAPI.cpp` - RPC methods for validator registration
- `src/Blockchain.cpp` - STAKE transaction processing
- `src/Validator.cpp` - Validator class implementation
- `include/Validator.h` - Validator class definition

## Summary

The fix ensures that:
1. ✅ Users see their stake amount immediately when registering
2. ✅ Funds are locked in mempool (cannot be double-spent)
3. ✅ Validator shows "pending_confirmation" status
4. ✅ Validator is activated when block is mined
5. ✅ Clear distinction between pending and active validators
6. ✅ No security compromises - pending validators cannot produce blocks
