# Staking Functionality Test Guide

This document provides comprehensive testing instructions for the GXC blockchain staking and PoS (Proof of Stake) functionality.

## Prerequisites

1. **Running Blockchain Node**: The blockchain node must be running and accessible via RPC
   - Default RPC port: `18332` (testnet) or `8332` (mainnet)
   - Test RPC connection: `curl -X POST http://localhost:18332 -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'`

2. **Sufficient Balance**: To register as a validator, you need:
   - Minimum stake: **100 GXC**
   - Recommended: At least **200 GXC** (100 for staking + 100 for transactions)

3. **Python Dependencies**: 
   ```bash
   pip3 install requests
   ```

## Test Scripts

### Quick Test (Bash)
```bash
./test_staking_quick.sh [RPC_URL]
# Example:
./test_staking_quick.sh http://localhost:18332
```

### Comprehensive Test (Python)
```bash
python3 test_staking.py [--rpc-url RPC_URL]
# Example:
python3 test_staking.py --rpc-url http://localhost:18332
```

## Manual Testing Steps

### Step 1: Verify Node Connection
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "blocks": 123,
    "difficulty": 0.1,
    ...
  },
  "id": 1
}
```

### Step 2: Generate or Use an Address
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}'
```

**Save the address** - you'll need it for all staking operations.

### Step 3: Check Balance
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["YOUR_ADDRESS"],"id":1}'
```

**Ensure balance ≥ 100 GXC** before proceeding.

### Step 4: Register as Validator (Stake)
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "registervalidator",
    "params": [
      "YOUR_ADDRESS",
      100.0,
      90
    ],
    "id": 1
  }'
```

**Parameters:**
- `address`: Your validator address
- `stakeAmount`: Amount to stake (minimum 100 GXC)
- `stakingDays`: Staking period (14-365 days)

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "YOUR_ADDRESS",
    "stake_amount": 100.0,
    "staking_days": 90,
    "weighted_stake": 150.0,
    "message": "Validator registered successfully"
  },
  "id": 1
}
```

### Step 5: Verify Validator Registration
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getvalidatorinfo","params":["YOUR_ADDRESS"],"id":1}'
```

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "YOUR_ADDRESS",
    "stake_amount": 100.0,
    "staking_days": 90,
    "weighted_stake": 150.0,
    "is_active": true,
    "is_valid": true
  },
  "id": 1
}
```

### Step 6: List All Validators
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getvalidators","params":[],"id":1}'
```

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "address": "YOUR_ADDRESS",
      "stake_amount": 100.0,
      "staking_days": 90,
      "weighted_stake": 150.0,
      "is_active": true,
      "is_valid": true
    }
  ],
  "id": 1
}
```

### Step 7: Check Next Block Type
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

**Block Type Logic:**
- **Even heights** (0, 2, 4, ...) → **PoW blocks**
- **Odd heights** (1, 3, 5, ...) → **PoS blocks** (if validators exist)

### Step 8: Get Block Template (Check PoS Info)
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblocktemplate","params":[],"id":1}'
```

**If next block is PoS, you should see:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "consensus_type": "pos",
    "block_type": "pos",
    "height": 125,
    "validator_address": "YOUR_ADDRESS",
    "validator_stake": 100.0,
    "validator_weighted_stake": 150.0,
    ...
  },
  "id": 1
}
```

### Step 9: Create PoS Block (When Next Block is PoS)
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "createposblock",
    "params": ["YOUR_ADDRESS"],
    "id": 1
  }'
```

**Note:** This only works when:
- Current height is **odd** (next block will be PoS)
- Validator is **active** and **selected** for this block

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "height": 125,
    "hash": "abc123...",
    "consensus_type": "pos",
    "validator_address": "YOUR_ADDRESS",
    "validator_signature": "sig123...",
    "block_reward": 50.0,
    "transactions": [...]
  },
  "id": 1
}
```

### Step 10: Submit PoS Block
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "submitposblock",
    "params": [{
      "height": 125,
      "hash": "abc123...",
      "previousblockhash": "prev123...",
      "validator_address": "YOUR_ADDRESS",
      "validator_signature": "sig123...",
      "transactions": [...]
    }],
    "id": 1
  }'
```

### Step 11: Add Additional Stake (Optional)
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "addstake",
    "params": [
      "YOUR_ADDRESS",
      50.0
    ],
    "id": 1
  }'
```

### Step 12: Unstake (Optional - Deactivates Validator)
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "unstake",
    "params": ["YOUR_ADDRESS"],
    "id": 1
  }'
```

## Test Scenarios

### Scenario 1: Basic Staking Flow
1. ✅ Register validator with 100 GXC for 90 days
2. ✅ Verify validator appears in `getvalidators`
3. ✅ Check `getvalidatorinfo` returns correct data
4. ✅ Wait for odd block height
5. ✅ Verify `getblocktemplate` shows PoS info
6. ✅ Create and submit PoS block

### Scenario 2: Multiple Validators
1. ✅ Register 3 validators with different stakes
2. ✅ Verify all appear in `getvalidators`
3. ✅ Check validator selection is weighted by stake
4. ✅ Verify PoS blocks alternate between validators

### Scenario 3: Edge Cases
1. ✅ Try to stake with insufficient balance → Should fail
2. ✅ Try to stake less than 100 GXC → Should fail
3. ✅ Try to stake for < 14 days → Should fail
4. ✅ Try to stake for > 365 days → Should fail
5. ✅ Try to create PoS block at even height → Should fail
6. ✅ Try to create PoS block with inactive validator → Should fail

### Scenario 4: Block Type Alternation
1. ✅ Mine PoW block at even height (0, 2, 4, ...)
2. ✅ Create PoS block at odd height (1, 3, 5, ...)
3. ✅ Verify blocks alternate correctly
4. ✅ Verify PoW blocks still work when no validators

## Expected Behavior

### Validator Selection
- Validators are selected **proportionally** based on weighted stake
- Weighted stake = `stake_amount * (1 + time_weight)`
- Selection is **deterministic** based on block height (seed)
- Higher stake = higher probability of selection

### Block Creation
- **PoW blocks**: Created by miners at even heights
- **PoS blocks**: Created by validators at odd heights
- PoS blocks require:
  - Valid validator signature
  - Minimal hash difficulty (0.01)
  - Active validator status

### Security
- Validators must stake minimum 100 GXC
- Staking period: 14-365 days
- Misbehavior can result in slashing (when implemented)
- PoS blocks are validated before acceptance

## Troubleshooting

### "No validators found"
- Ensure at least one validator is registered
- Check validator is active: `getvalidatorinfo`
- Verify validator has sufficient stake

### "Next block should be PoW, not PoS"
- PoS blocks are only created at **odd heights**
- Wait for next block or check current height
- Use `getblockchaininfo` to check height

### "Validator not selected"
- Selection is based on weighted stake
- Validator with higher stake has higher probability
- Multiple validators may need to wait for their turn

### "Insufficient balance"
- Ensure address has ≥ stake amount + transaction fees
- Check balance: `getbalance`
- Mine more coins if needed

## Code Verification

The staking implementation includes:

1. **Blockchain.cpp**:
   - `validateProofOfStake()`: Validates PoS blocks
   - `selectValidatorForBlock()`: Selects validator based on weighted stake
   - `getNextBlockType()`: Determines PoW vs PoS for next block

2. **RPCAPI.cpp**:
   - `registerValidator()`: Registers new validator
   - `stake()`: Alias for registerValidator
   - `unstake()`: Deactivates validator
   - `addStake()`: Adds additional stake
   - `getValidators()`: Lists all validators
   - `getValidatorInfo()`: Gets validator details
   - `createPoSBlock()`: Creates PoS block template
   - `submitPoSBlock()`: Submits PoS block

3. **Validator.h/cpp**:
   - Validator class with stake management
   - Weighted stake calculation
   - Signature verification
   - Validation checks

## Success Criteria

✅ All RPC endpoints respond correctly  
✅ Validators can register and stake  
✅ Validator selection works proportionally  
✅ PoS blocks are created at odd heights  
✅ PoS blocks are validated correctly  
✅ PoW and PoS blocks alternate correctly  
✅ Edge cases are handled properly  

## Next Steps

After successful testing:
1. Deploy to testnet
2. Test with multiple nodes
3. Monitor validator performance
4. Test slashing (when implemented)
5. Test validator rewards (when implemented)
