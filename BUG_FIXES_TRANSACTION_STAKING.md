# Bug Fixes: Transaction and Staking Issues

## Summary
Fixed critical bugs in the blockchain node that prevented fund transfers and staking from working correctly.

## Bugs Fixed

### Bug 1: Fund Transfer Not Working
**Issue**: When transferring funds from one wallet to another, the transaction would be created but the UTXO set was not properly updated due to a missing closing brace in the staking transaction processing logic.

**Root Cause**: In `src/Blockchain.cpp` line 669, there was an extra closing brace `}` that caused the change output processing to be incorrectly scoped within the validator lookup logic. This meant that if a validator was not found, the change outputs would never be added to the UTXO set.

**Fix**: Removed the extra closing brace and restructured the code to ensure change outputs are always added to the UTXO set regardless of validator lookup success.

**Files Changed**:
- `src/Blockchain.cpp` (lines 576-800)

### Bug 2: Staking Creates Validator But Doesn't Stake Funds
**Issue**: When staking funds, a validator would be created but the staked amount would not be properly locked. The validator would show 0 stake even after the stake transaction was confirmed.

**Root Cause**: The `registerValidator` RPC method creates a validator with 0 stake and pending status, adding it to `validatorMap` but not to the `validators` vector. When `updateUtxoSet` processes the STAKE transaction, it only searches the `validators` vector, not the `validatorMap`, causing the stake update to fail.

**Fix**: 
1. Modified `updateUtxoSet` to check `validatorMap` first (authoritative source) instead of only the `validators` vector
2. When a validator is found in the map but not in the vector, it's added to the vector (activating it from pending state)
3. Updated `registerValidator` to properly sync both the map and vector when updating existing validators

**Files Changed**:
- `src/Blockchain.cpp` (lines 576-800, 1905-1980)

## Technical Details

### STAKE Transaction Processing Flow (Fixed)
1. User calls `registerValidator` RPC → Creates validator with 0 stake, pending status
2. Validator added to `validatorMap` only (not in `validators` vector yet)
3. User creates STAKE transaction → Transaction validated (checks `validatorMap`)
4. STAKE transaction included in block → `updateUtxoSet` called
5. **FIX**: `updateUtxoSet` now checks `validatorMap` first
6. Validator found in map → Stake amount added
7. Validator not in vector → Added to vector (activated from pending)
8. Change outputs properly added to UTXO set

### UNSTAKE Transaction Processing Flow (Fixed)
1. User creates UNSTAKE transaction
2. Transaction included in block → `updateUtxoSet` called
3. **FIX**: `updateUtxoSet` now checks `validatorMap` first
4. Validator found in map → Stake amount reduced
5. Validator in vector → Updated to match map
6. Unstaked funds added as new UTXOs (spendable balance)

## Code Changes

### Change 1: STAKE Processing in updateUtxoSet
```cpp
// BEFORE: Only checked validators vector
for (auto& v : validators) {
    if (v.getAddress() == stakerAddress) {
        v.addStake(stakedAmount);
        // ...
    }
}

// AFTER: Check validatorMap first, sync with vector
auto mapIt = validatorMap.find(stakerAddress);
if (mapIt != validatorMap.end()) {
    mapIt->second.addStake(stakedAmount);
    
    // Update in vector if exists
    for (auto& v : validators) {
        if (v.getAddress() == stakerAddress) {
            v.addStake(stakedAmount);
            found = true;
            break;
        }
    }
    
    // If not in vector, add it (was pending)
    if (!found) {
        validators.push_back(mapIt->second);
    }
}
```

### Change 2: UNSTAKE Processing in updateUtxoSet
```cpp
// BEFORE: Only checked validators vector
for (auto& v : validators) {
    if (v.getAddress() == stakerAddress) {
        v.removeStake(unstakeAmount);
        validatorMap[stakerAddress] = v;
        // ...
    }
}

// AFTER: Check validatorMap first, sync with vector
auto mapIt = validatorMap.find(stakerAddress);
if (mapIt != validatorMap.end()) {
    mapIt->second.removeStake(unstakeAmount);
    
    // Update validator in vector if it exists
    for (auto& v : validators) {
        if (v.getAddress() == stakerAddress) {
            v = mapIt->second;
            break;
        }
    }
}
```

### Change 3: registerValidator Sync Fix
```cpp
// BEFORE: Only updated map when validator exists
if (validatorMap.find(validator.getAddress()) != validatorMap.end()) {
    validatorMap[validator.getAddress()] = validator;
}

// AFTER: Update both map and vector
if (validatorMap.find(validator.getAddress()) != validatorMap.end()) {
    validatorMap[validator.getAddress()] = validator;
    
    // Update in vector as well
    bool foundInVector = false;
    for (auto& v : validators) {
        if (v.getAddress() == validator.getAddress()) {
            v = validator;
            foundInVector = true;
            break;
        }
    }
    
    // If not in vector but in map, add it (was pending)
    if (!foundInVector) {
        validators.push_back(validator);
    }
}
```

## Testing

Created comprehensive test suite in `tests/test_transaction_staking_fixes.cpp` covering:
- Fund transfer creates proper UTXOs
- Staking transaction properly locks funds
- Validator registration and stake confirmation
- Multiple stake transactions to same validator
- Unstake transaction returns funds to spendable balance
- Transaction with insufficient funds fails gracefully
- Transaction chain preserves traceability
- Validator stake amount updates correctly
- Validator deactivates when stake drops below minimum

## Impact

### Before Fix
- ❌ Fund transfers would fail silently or create invalid UTXOs
- ❌ Staking would create validators with 0 stake
- ❌ Staked funds would be lost (not locked, not spendable)
- ❌ Unstaking would fail to return funds

### After Fix
- ✅ Fund transfers work correctly and update UTXO set
- ✅ Staking properly locks funds and updates validator stake
- ✅ Validators are activated from pending state when stake is confirmed
- ✅ Unstaking returns funds to spendable balance
- ✅ Both validatorMap and validators vector stay in sync

## Verification

To verify the fixes:
1. Compile the blockchain node: `cd build && make`
2. Run the test suite: `./test_transaction_staking_fixes`
3. Test fund transfer: Create two wallets, send funds between them
4. Test staking: Register validator, stake funds, verify stake amount
5. Test unstaking: Unstake funds, verify they return to spendable balance

## Related Files
- `src/Blockchain.cpp` - Main blockchain logic
- `src/RPCAPI.cpp` - RPC methods for staking
- `src/Wallet.cpp` - Transaction creation
- `include/Validator.h` - Validator class definition
- `tests/test_transaction_staking_fixes.cpp` - Test suite

## Notes
- The fix maintains backward compatibility
- Database persistence is handled correctly
- Traceability formula is preserved
- No breaking changes to RPC API
