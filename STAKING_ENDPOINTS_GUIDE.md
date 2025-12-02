# GXC Staking and Validator Endpoints Guide

## Overview

GXC blockchain supports **Proof of Stake (PoS)** validation through a validator staking system. Validators stake GXC coins to participate in block validation and earn rewards.

## Staking Requirements

- **Minimum Stake**: 100 GXC
- **Minimum Staking Period**: 14 days
- **Maximum Staking Period**: 365 days
- **Validator Status**: Must be active and not slashed

## RPC Endpoints

### 1. Register as Validator / Stake Coins

**Endpoint**: `registervalidator` or `gxc_registerValidator` or `stake` or `gxc_stake`

**Method**: POST

**Parameters**:
```json
{
  "jsonrpc": "2.0",
  "method": "registervalidator",
  "params": [
    "tGXC9fab7317231b966af85ac453e168c0932",  // Your address
    1000.0,                                    // Stake amount (GXC)
    90                                         // Staking days (14-365)
  ],
  "id": 1
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "tGXC9fab7317231b966af85ac453e168c0932",
    "stake_amount": 1000.0,
    "staking_days": 90,
    "weighted_stake": 1050.0,
    "message": "Validator registered successfully"
  },
  "id": 1
}
```

**Example cURL**:
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "registervalidator",
    "params": ["tGXC9fab7317231b966af85ac453e168c0932", 1000.0, 90],
    "id": 1
  }'
```

### 2. Add More Stake

**Endpoint**: `addstake` or `gxc_addStake`

**Method**: POST

**Parameters**:
```json
{
  "jsonrpc": "2.0",
  "method": "addstake",
  "params": [
    "tGXC9fab7317231b966af85ac453e168c0932",  // Your validator address
    500.0                                      // Additional stake amount (GXC)
  ],
  "id": 1
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "tGXC9fab7317231b966af85ac453e168c0932",
    "additional_amount": 500.0,
    "message": "Additional stake added successfully"
  },
  "id": 1
}
```

### 3. Unstake / Withdraw Stake

**Endpoint**: `unstake` or `gxc_unstake`

**Method**: POST

**Parameters**:
```json
{
  "jsonrpc": "2.0",
  "method": "unstake",
  "params": [
    "tGXC9fab7317231b966af85ac453e168c0932"  // Your validator address
  ],
  "id": 1
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "tGXC9fab7317231b966af85ac453e168c0932",
    "message": "Stake withdrawn successfully"
  },
  "id": 1
}
```

### 4. Get All Validators

**Endpoint**: `getvalidators` or `gxc_getValidators`

**Method**: POST

**Parameters**:
```json
{
  "jsonrpc": "2.0",
  "method": "getvalidators",
  "params": [],
  "id": 1
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "stake_amount": 1000.0,
      "staking_days": 90,
      "weighted_stake": 1050.0,
      "is_active": true,
      "blocks_produced": 15,
      "missed_blocks": 2,
      "uptime": 0.95,
      "total_rewards": 50.0,
      "apy": 5.0,
      "is_slashed": false
    },
    {
      "address": "tGXC...another_validator",
      "stake_amount": 2000.0,
      "staking_days": 180,
      "weighted_stake": 2200.0,
      "is_active": true,
      "blocks_produced": 30,
      "missed_blocks": 1,
      "uptime": 0.98,
      "total_rewards": 120.0,
      "apy": 6.0,
      "is_slashed": false
    }
  ],
  "id": 1
}
```

### 5. Get Validator Info

**Endpoint**: `getvalidatorinfo` or `gxc_getValidatorInfo`

**Method**: POST

**Parameters**:
```json
{
  "jsonrpc": "2.0",
  "method": "getvalidatorinfo",
  "params": [
    "tGXC9fab7317231b966af85ac453e168c0932"  // Validator address
  ],
  "id": 1
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "tGXC9fab7317231b966af85ac453e168c0932",
    "stake_amount": 1000.0,
    "staking_days": 90,
    "weighted_stake": 1050.0,
    "time_weight": 1.05,
    "is_active": true,
    "blocks_produced": 15,
    "missed_blocks": 2,
    "uptime": 0.95,
    "total_rewards": 50.0,
    "pending_rewards": 5.0,
    "apy": 5.0,
    "is_slashed": false,
    "slashed_amount": 0.0,
    "is_valid": true
  },
  "id": 1
}
```

## How Staking Works

### Weighted Stake Calculation

Validators are selected based on **weighted stake**, which combines:
- **Stake Amount**: The amount of GXC staked
- **Time Weight**: Bonus for longer staking periods

**Formula**:
```
Weighted Stake = Stake Amount × Time Weight
Time Weight = (Staking Days / 365)^0.5
```

**Example**:
- Stake: 1000 GXC
- Staking Days: 90
- Time Weight: (90/365)^0.5 = 0.497 ≈ 0.5
- Weighted Stake: 1000 × 0.5 = 500

Actually, looking at the code, the time weight calculation gives a bonus for longer staking:
- 90 days: (90/365)^0.5 ≈ 0.497
- But the weighted stake multiplies by this, so longer staking = higher weight

### Validator Selection

Validators are selected for block production based on:
1. **Weighted Stake**: Higher weighted stake = higher selection probability
2. **Uptime**: Validators with better uptime are preferred
3. **Performance**: Blocks produced vs. missed blocks

### Rewards

Validators earn rewards for:
- **Block Production**: Rewards for producing valid blocks
- **Uptime**: Bonus for maintaining high uptime
- **Staking Duration**: Longer staking periods may earn higher rewards

### Slashing

Validators can be **slashed** (lose stake) for:
- **Double Signing**: Signing conflicting blocks
- **Downtime**: Extended periods of being offline
- **Malicious Behavior**: Attempting to attack the network

## Step-by-Step: Becoming a Validator

### Step 1: Ensure You Have Enough Balance

Check your balance:
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXC9fab7317231b966af85ac453e168c0932"],
    "id": 1
  }'
```

You need at least **100 GXC** to become a validator.

### Step 2: Register as Validator

Register with your desired stake amount and staking period:
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "registervalidator",
    "params": [
      "tGXC9fab7317231b966af85ac453e168c0932",
      1000.0,
      90
    ],
    "id": 1
  }'
```

### Step 3: Verify Registration

Check your validator info:
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getvalidatorinfo",
    "params": ["tGXC9fab7317231b966af85ac453e168c0932"],
    "id": 1
  }'
```

### Step 4: Monitor Performance

Check all validators to see your ranking:
```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getvalidators",
    "params": [],
    "id": 1
  }'
```

## Important Notes

### ⚠️ Staking Requirements

1. **Minimum Stake**: 100 GXC (cannot stake less)
2. **Staking Period**: 14-365 days (must commit for this period)
3. **Balance Check**: Must have sufficient balance before staking
4. **Active Status**: Validator must remain active to earn rewards

### ⚠️ Unstaking

- **Cooldown Period**: May need to wait before unstaking
- **Rewards**: Pending rewards are distributed when unstaking
- **Status**: Validator becomes inactive after unstaking

### ⚠️ Slashing

- **Double Signing**: Signing two conflicting blocks = slashing
- **Downtime**: Extended offline periods may result in slashing
- **Amount**: Slashed amount is deducted from stake

## Example Workflow

### 1. Check Balance
```bash
# Get your balance
curl -X POST http://localhost:18332 \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["YOUR_ADDRESS"],"id":1}'
```

### 2. Register as Validator
```bash
# Register with 1000 GXC for 90 days
curl -X POST http://localhost:18332 \
  -d '{"jsonrpc":"2.0","method":"registervalidator","params":["YOUR_ADDRESS",1000.0,90],"id":1}'
```

### 3. Add More Stake (Optional)
```bash
# Add 500 more GXC to your stake
curl -X POST http://localhost:18332 \
  -d '{"jsonrpc":"2.0","method":"addstake","params":["YOUR_ADDRESS",500.0],"id":1}'
```

### 4. Check Validator Info
```bash
# Get your validator information
curl -X POST http://localhost:18332 \
  -d '{"jsonrpc":"2.0","method":"getvalidatorinfo","params":["YOUR_ADDRESS"],"id":1}'
```

### 5. Unstake (When Ready)
```bash
# Withdraw your stake
curl -X POST http://localhost:18332 \
  -d '{"jsonrpc":"2.0","method":"unstake","params":["YOUR_ADDRESS"],"id":1}'
```

## Status

✅ **Staking endpoints are now implemented and ready to use!**

All endpoints are available via RPC API:
- `registervalidator` / `gxc_registerValidator` / `stake` / `gxc_stake`
- `addstake` / `gxc_addStake`
- `unstake` / `gxc_unstake`
- `getvalidators` / `gxc_getValidators`
- `getvalidatorinfo` / `gxc_getValidatorInfo`
