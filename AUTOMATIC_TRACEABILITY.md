# GXC Blockchain - Automatic Traceability

## Overview

The GXC blockchain maintains **complete traceability automatically** for ALL transaction types, just like it does for regular sends. No manual intervention required.

---

## How Traceability Works Automatically

### 1. Regular Send Transaction

**User Action:**
```bash
curl -X POST http://localhost:8545 \
  -d '{"method":"sendtoaddress","params":["recipientAddr", 10.0]}'
```

**What the Node Does Automatically:**
```
1. Wallet selects mature UTXOs
2. Creates transaction inputs:
   - txHash: Previous transaction where user received coins
   - amount: Amount being sent
   - signature: Proves ownership
3. Creates transaction outputs:
   - address: Recipient
   - amount: Amount to send
4. Sets previousTxHash: Links to previous transaction
5. Calculates transaction hash
6. Signs with private key

✅ AUTOMATIC: Complete traceability maintained
✅ AUTOMATIC: Ownership proven
✅ AUTOMATIC: Audit trail created
```

**Traceability Chain:**
```
tx_mining_100 (User receives 50 GXC)
  ↓ [Automatic link via input.txHash]
tx_send_200 (User sends 10 GXC)
  ↓ [Automatic link via input.txHash]
tx_send_300 (Recipient sends 5 GXC)
```

---

### 2. Staking Transaction

**User Action:**
```bash
curl -X POST http://localhost:8545 \
  -d '{"method":"registervalidator","params":["validatorAddr", 100.0]}'
```

**What the Node Does Automatically:**
```
1. Wallet selects mature UTXOs (same as send)
2. Creates transaction inputs:
   - txHash: Previous transaction where user received coins
   - amount: Amount being staked
   - signature: Proves ownership
3. Creates transaction outputs: NONE (coins go to pool)
4. Sets previousTxHash: Links to previous transaction
5. Calculates transaction hash
6. Signs with private key
7. Blockchain processes:
   - Consumes UTXOs
   - Adds to staking pool with traceability:
     * stakeTxHash: This transaction's hash
     * previousTxHash: Where coins came from
     * inputTxHashes: All original UTXOs
     * referencedAmount: Amount staked
8. Saves to LevelDB

✅ AUTOMATIC: Complete traceability maintained
✅ AUTOMATIC: Ownership proven
✅ AUTOMATIC: Original coins tracked
✅ AUTOMATIC: Saved to database
```

**Traceability Chain:**
```
tx_mining_100 (User receives 50 GXC)
  ↓ [Automatic link via input.txHash]
tx_stake_200 (User stakes 50 GXC)
  ↓ [Stored in StakingPool with full traceability]
Pool reserves 50 GXC
```

**StakingPool Entry (Automatic):**
```cpp
StakeEntry {
    stakeTxHash: "tx_stake_200"           // ✅ Automatic
    previousTxHash: "tx_mining_100"       // ✅ Automatic
    inputTxHashes: ["tx_mining_100"]      // ✅ Automatic
    referencedAmount: 50.0                // ✅ Automatic
    amount: 50.0
    ownerAddress: "user"
    validatorAddress: "validator"
}
```

---

### 3. Reward Transaction

**Automatic Trigger:**
```
Block is added to blockchain
  ↓
distributeStakingRewards() called automatically
```

**What the Node Does Automatically:**
```
1. Gets validator address from block
2. Finds all stakes for that validator
3. Calculates proportional rewards
4. For each staker:
   a. Creates REWARD transaction:
      - Input.txHash: Stake transaction hash
      - Input.amount: Reward amount
      - Output.address: Staker address
      - Output.amount: Reward amount (NEW COINS)
   b. Sets previousTxHash: Stake transaction
   c. Sets type: REWARD
   d. Calculates hash
   e. Adds to pending transactions
   f. Updates stake's accumulated rewards
5. UTXO set updated (rewards become spendable)
6. Saves to LevelDB

✅ AUTOMATIC: Links to stake transaction
✅ AUTOMATIC: Proves legitimacy
✅ AUTOMATIC: Mints new coins
✅ AUTOMATIC: Saved to database
```

**Traceability Chain:**
```
tx_stake_200 (User staked 50 GXC)
  ↓ [Automatic link via input.txHash]
tx_reward_300 (User receives 2.5 GXC reward)
  ↓ [NEW COINS minted, linked to stake]
User can spend reward immediately
```

**Reward Transaction (Automatic):**
```cpp
Transaction {
    type: REWARD                          // ✅ Automatic
    inputs: [{
        txHash: "tx_stake_200"           // ✅ Automatic (links to stake)
        amount: 2.5
    }]
    outputs: [{
        address: "user"
        amount: 2.5                       // ✅ NEW COINS
    }]
    previousTxHash: "tx_stake_200"       // ✅ Automatic
    referencedAmount: 2.5                // ✅ Automatic
}
```

---

### 4. Unstaking Transaction

**User Action:**
```bash
curl -X POST http://localhost:8545 \
  -d '{"method":"unstake","params":["userAddr"]}'
```

**What the Node Does Automatically:**
```
1. Finds user's stake in StakingPool
2. Verifies stake is mature (lock period expired)
3. Wallet creates UNSTAKE transaction:
   - Input.txHash: Stake transaction hash
   - Input.amount: Original staked amount
   - Output.address: User address
   - Output.amount: Original staked amount
   - Signature: Proves ownership
4. Sets previousTxHash: Stake transaction
5. Sets type: UNSTAKE
6. Calculates hash
7. Signs with private key
8. Blockchain processes:
   - Removes from staking pool
   - Creates UTXO for returned coins
   - Updates validator pool
9. Saves to LevelDB

✅ AUTOMATIC: Links to stake transaction
✅ AUTOMATIC: Proves ownership
✅ AUTOMATIC: Returns ORIGINAL coins (doesn't mint)
✅ AUTOMATIC: Saved to database
```

**Traceability Chain:**
```
tx_stake_200 (User staked 50 GXC)
  ↓ [Automatic link via input.txHash]
tx_unstake_400 (User unstakes 50 GXC)
  ↓ [ORIGINAL COINS returned]
User receives same 50 GXC back
```

**Unstake Transaction (Automatic):**
```cpp
Transaction {
    type: UNSTAKE                         // ✅ Automatic
    inputs: [{
        txHash: "tx_stake_200"           // ✅ Automatic (links to stake)
        amount: 50.0
        signature: "..."                  // ✅ Automatic (proves ownership)
    }]
    outputs: [{
        address: "user"
        amount: 50.0                      // ✅ ORIGINAL COINS (not minted)
    }]
    previousTxHash: "tx_stake_200"       // ✅ Automatic
    referencedAmount: 50.0               // ✅ Automatic
}
```

---

## Complete Flow Example

### Step-by-Step with Automatic Traceability

```
1. User Mines Block
   ↓
   Coinbase Transaction (AUTOMATIC)
   - Type: COINBASE
   - Output: 50 GXC to user
   - TxHash: tx_mining_100
   ✅ User has 50 GXC

2. User Stakes 50 GXC
   ↓
   Wallet Creates Transaction (AUTOMATIC)
   - Input.txHash: tx_mining_100 ← Links to mining
   - Input.amount: 50 GXC
   - Input.signature: Proves ownership
   ↓
   Blockchain Processes (AUTOMATIC)
   - Consumes UTXO tx_mining_100
   - Adds to StakingPool:
     * stakeTxHash: tx_stake_200
     * previousTxHash: tx_mining_100 ← Traceability
     * amount: 50 GXC
   - Saves to LevelDB
   ✅ 50 GXC reserved in pool

3. Validator Produces Block
   ↓
   distributeStakingRewards() (AUTOMATIC)
   - Finds user's stake
   - Calculates reward: 2.5 GXC
   - Creates REWARD transaction:
     * Input.txHash: tx_stake_200 ← Links to stake
     * Output: 2.5 GXC to user
     * Type: REWARD
   - Adds to pending transactions
   - Updates UTXO set
   - Saves to LevelDB
   ✅ User receives 2.5 GXC reward (NEW COINS)

4. User Unstakes After Lock Period
   ↓
   Wallet Creates Transaction (AUTOMATIC)
   - Input.txHash: tx_stake_200 ← Links to stake
   - Input.amount: 50 GXC
   - Input.signature: Proves ownership
   ↓
   Blockchain Processes (AUTOMATIC)
   - Verifies stake exists and is mature
   - Removes from StakingPool
   - Creates UTXO: 50 GXC to user
   - Saves to LevelDB
   ✅ User receives original 50 GXC back

Final State:
  User has: 50 GXC (returned) + 2.5 GXC (reward) = 52.5 GXC
  Supply: 50 GXC (mining) + 2.5 GXC (reward) = 52.5 GXC ✅
```

---

## Traceability Verification

### How to Verify Traceability

```bash
# 1. Check transaction details
curl -X POST http://localhost:8545 \
  -d '{"method":"gettransaction","params":["tx_hash"]}'

Response:
{
  "txHash": "tx_stake_200",
  "type": "STAKE",
  "inputs": [{
    "txHash": "tx_mining_100",  ← Links to previous
    "amount": 50.0
  }],
  "previousTxHash": "tx_mining_100",  ← Traceability
  "referencedAmount": 50.0
}

# 2. Check staking info
curl -X POST http://localhost:8545 \
  -d '{"method":"getstakinginfo","params":["address"]}'

Response:
{
  "activeStakes": [{
    "stakeTxHash": "tx_stake_200",
    "previousTxHash": "tx_mining_100",  ← Traceability
    "inputTxHashes": ["tx_mining_100"],
    "referencedAmount": 50.0
  }]
}

# 3. Trace complete chain
tx_mining_100 → tx_stake_200 → tx_reward_300 (NEW)
                             → tx_unstake_400 (RETURNED)
```

---

## Key Points

### 1. Automatic Like Regular Sends
```
Regular Send:
  ✅ Wallet selects UTXOs automatically
  ✅ Creates inputs with previous tx hash
  ✅ Signs transaction automatically
  ✅ Blockchain validates traceability

Staking:
  ✅ Wallet selects UTXOs automatically (SAME)
  ✅ Creates inputs with previous tx hash (SAME)
  ✅ Signs transaction automatically (SAME)
  ✅ Blockchain validates traceability (SAME)
  ✅ PLUS: Saves to StakingPool with full traceability
```

### 2. No Manual Work Required
```
❌ User does NOT need to:
   - Manually specify previous tx hash
   - Manually calculate amounts
   - Manually create signatures
   - Manually maintain traceability

✅ Node does EVERYTHING automatically:
   - Selects appropriate UTXOs
   - Links to previous transactions
   - Creates proper signatures
   - Maintains complete audit trail
   - Saves to database
```

### 3. Fraud Prevention Built-In
```
✅ Can't stake coins you don't own
   → Signature verification automatic

✅ Can't claim rewards you didn't earn
   → Reward links to stake automatic

✅ Can't unstake more than you staked
   → Amount verification automatic

✅ Can't double-spend staked coins
   → UTXO consumption automatic
```

### 4. Complete Audit Trail
```
Every transaction maintains:
  ✅ Previous transaction hash
  ✅ Referenced amount
  ✅ Signature proof
  ✅ Complete input chain

Stored in LevelDB:
  ✅ Stake entries with traceability
  ✅ Validator pool stats
  ✅ Accumulated rewards
  ✅ All transaction history
```

---

## Implementation Details

### Wallet Layer (Automatic)
```cpp
Transaction Wallet::createStakeTransaction(...) {
    // 1. Select mature UTXOs (AUTOMATIC)
    auto utxos = selectMatureUTXOs(amount);
    
    // 2. Create inputs with traceability (AUTOMATIC)
    for (const auto& utxo : utxos) {
        TransactionInput input;
        input.txHash = utxo.txHash;  // ← Links to previous
        input.amount = utxo.amount;
        input.signature = sign(input);  // ← Proves ownership
        inputs.push_back(input);
    }
    
    // 3. Set previous tx hash (AUTOMATIC)
    tx.setPreviousTxHash(lastTxHash);
    
    // 4. Calculate hash and sign (AUTOMATIC)
    tx.calculateHash();
    tx.signInputs(privateKey);
    
    return tx;  // ✅ Complete traceability
}
```

### Blockchain Layer (Automatic)
```cpp
bool Blockchain::processStakeTransaction(const Transaction& tx) {
    // 1. Validate traceability (AUTOMATIC)
    if (!validateTraceability(tx)) {
        return false;  // Reject if traceability broken
    }
    
    // 2. Consume UTXOs (AUTOMATIC)
    for (const auto& input : tx.getInputs()) {
        utxoSet.erase(input.txHash + "_" + input.outputIndex);
    }
    
    // 3. Add to staking pool with traceability (AUTOMATIC)
    std::vector<std::string> inputHashes;
    for (const auto& input : tx.getInputs()) {
        inputHashes.push_back(input.txHash);
    }
    
    stakingPool->addStake(
        tx.getSenderAddress(),
        tx.getMetadata("validatorAddress"),
        tx.getReferencedAmount(),
        lockDays,
        tx.getHash(),              // ← Stake tx hash
        tx.getPreviousTxHash(),    // ← Previous tx hash
        inputHashes                // ← All input tx hashes
    );
    
    // 4. Save to LevelDB (AUTOMATIC)
    stakingPool->saveToDatabase();
    
    return true;  // ✅ Complete traceability maintained
}
```

---

## Summary

### Traceability is AUTOMATIC

**Just like regular sends:**
1. ✅ Wallet handles UTXO selection
2. ✅ Wallet creates proper inputs
3. ✅ Wallet signs transactions
4. ✅ Blockchain validates traceability
5. ✅ Database saves everything

**No manual work required:**
- ❌ No manual tx hash specification
- ❌ No manual signature creation
- ❌ No manual traceability maintenance
- ✅ Everything handled by the node

**Complete audit trail:**
- ✅ Every transaction linked
- ✅ Every stake tracked
- ✅ Every reward proven
- ✅ Every unstake verified

**The blockchain maintains traceability automatically, exactly like it does for regular send transactions!**

---

**Last Updated:** 2025-12-26 07:50 UTC
**Status:** PRODUCTION READY
**Traceability:** AUTOMATIC ✅
