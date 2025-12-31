# Fraud Detection System: Complete Explanation

## Overview

The GXC fraud detection system uses **Proof of Traceability (POT)** to automatically track stolen funds through the blockchain, even when they're split across multiple wallets.

## How It Works: The Complete Flow

### 1. Initial Trigger: Marking a Transaction as Stolen

**Who triggers it:** Victim or Admin
**When:** After a theft is discovered
**How:** Via fraud report submission

```
Victim discovers theft
    ↓
Submits fraud report with evidence
    ↓
Admin reviews evidence
    ↓
Admin approves FACTS (confirms theft is legitimate)
    ↓
System marks transaction as STOLEN
    ↓
Fraud detection automatically activates
```

**Code trigger:**
```cpp
// When admin approves fraud report
fraudDetection->markAsStolen(stolenTxHash);

// This sets taint score to 1.0 (100% stolen)
taintMap[stolenTxHash] = TaintInfo(1.0, stolenTxHash, timestamp);
```

### 2. Automatic Taint Propagation

**What happens:** System automatically traces funds forward through the blockchain

**Mathematical Formula:**
```
τ(T_j) = Σ w_i · τ(T_i)

where:
- τ(T_j) = taint score of transaction j
- w_i = weight from input i = (value_from_i / total_input_value)
- τ(T_i) = taint score of input transaction i
```

**Example: Funds Split to 9 Wallets**

```
Original Theft: 100 GXC (τ = 1.0)
        ↓
Split Transaction:
├─ Wallet 1: 50 GXC  (τ = 0.5)  [50/100 = 50%]
├─ Wallet 2: 20 GXC  (τ = 0.2)  [20/100 = 20%]
├─ Wallet 3: 10 GXC  (τ = 0.1)  [10/100 = 10%]
├─ Wallet 4: 5 GXC   (τ = 0.05) [5/100 = 5%]
├─ Wallet 5: 5 GXC   (τ = 0.05)
├─ Wallet 6: 3 GXC   (τ = 0.03)
├─ Wallet 7: 3 GXC   (τ = 0.03)
├─ Wallet 8: 2 GXC   (τ = 0.02)
└─ Wallet 9: 2 GXC   (τ = 0.02)

Total: 100 GXC (conservation of taint)
```

**Propagation continues:**
```
Wallet 1 (50 GXC, τ=0.5) sends to Wallet 10:
    Wallet 10: 50 GXC (τ = 0.5)

Wallet 2 (20 GXC, τ=0.2) mixes with clean funds:
    Input 1: 20 GXC (τ = 0.2) [stolen]
    Input 2: 80 GXC (τ = 0.0) [clean]
    Output: 100 GXC (τ = 0.04) [4% tainted]
    
    Calculation: (20/100 × 0.2) + (80/100 × 0.0) = 0.04
```

**Code implementation:**
```cpp
void FraudDetection::propagateTaint(const std::string& startTxHash, uint32_t maxHops) {
    // Start with stolen transaction (τ = 1.0)
    std::queue<std::pair<std::string, uint32_t>> queue;
    queue.push({startTxHash, 0});
    
    while (!queue.empty()) {
        auto [currentTxHash, hops] = queue.front();
        queue.pop();
        
        if (hops >= maxHops) continue;
        
        // Get all transactions that spend this one
        std::vector<Transaction> spendingTxs = blockchain->getSpendingTransactions(currentTxHash);
        
        for (const auto& tx : spendingTxs) {
            // Calculate taint for this transaction
            double taint = calculateTaint(tx);
            
            // Only propagate if above threshold (10%)
            if (taint >= TAINT_THRESHOLD) {
                taintMap[tx.getHash()] = TaintInfo(taint, startTxHash, time(nullptr));
                
                // Continue propagation
                queue.push({tx.getHash(), hops + 1});
                
                // Check for fraud patterns
                checkTransaction(tx);
            }
        }
    }
}
```

### 3. Fraud Pattern Detection (5 Rules)

**Triggered:** Automatically on every tainted transaction

**Rule 1: Velocity Anomaly**
- **What:** Funds moved too quickly after theft
- **Threshold:** < 5 minutes between transactions
- **Why suspicious:** Thief trying to move funds fast

```cpp
bool checkVelocityAnomaly(const Transaction& tx, const TaintInfo& taint) {
    uint64_t timeDiff = tx.getTimestamp() - taint.timestamp;
    return (timeDiff < VELOCITY_EPSILON); // 300 seconds
}
```

**Rule 2: Fan-Out Pattern (Smurfing)**
- **What:** Splitting funds to many addresses
- **Threshold:** > 5 outputs in one transaction
- **Why suspicious:** Classic money laundering technique

```cpp
bool checkFanOutPattern(const Transaction& tx, const TaintInfo& taint) {
    return (tx.getOutputs().size() > FAN_OUT_K); // K = 5
}
```

**Rule 3: Re-Aggregation (Layering)**
- **What:** Multiple tainted inputs combined
- **Threshold:** > 70% of inputs are tainted
- **Why suspicious:** Thief recombining split funds

```cpp
bool checkReAggregation(const Transaction& tx) {
    uint32_t taintedInputs = 0;
    for (const auto& input : tx.getInputs()) {
        if (getTaintScore(input.txHash) > TAINT_THRESHOLD) {
            taintedInputs++;
        }
    }
    double ratio = (double)taintedInputs / tx.getInputs().size();
    return (ratio > RE_AGG_THETA); // 0.7
}
```

**Rule 4: Dormancy Activation**
- **What:** Tainted funds sitting idle, then suddenly moved
- **Threshold:** > 7 days dormant
- **Why suspicious:** Waiting for heat to die down

```cpp
bool checkDormancyActivation(const Transaction& tx, const TaintInfo& taint) {
    uint64_t dormancyPeriod = tx.getTimestamp() - taint.timestamp;
    return (dormancyPeriod > DORMANCY_PERIOD); // 7 days
}
```

**Rule 5: Clean Zone Entry**
- **What:** Tainted funds sent to exchange/merchant
- **Threshold:** Destination is registered clean zone
- **Why suspicious:** Attempting to cash out or legitimize

```cpp
bool checkCleanZoneEntry(const Transaction& tx, const TaintInfo& taint) {
    for (const auto& output : tx.getOutputs()) {
        if (isCleanZone(output.address)) {
            return true;
        }
    }
    return false;
}
```

### 4. Alert Generation

**Triggered:** When fraud rules are violated

**Alert Levels:**
```cpp
enum class AlertLevel {
    LOW,      // τ < 0.5, 1 rule violation
    MEDIUM,   // τ < 0.5, 2+ rules OR τ ≥ 0.5, 1 rule
    HIGH,     // τ ≥ 0.5, 2+ rules OR τ ≥ 0.8, 1 rule
    CRITICAL  // τ ≥ 0.8, 2+ rules
};
```

**Example Alert:**
```json
{
    "transactionHash": "abc123...",
    "address": "wallet_10",
    "level": "HIGH",
    "rule": "FAN_OUT_PATTERN",
    "taintScore": 0.65,
    "description": "Tainted funds split to 8 addresses",
    "evidence": [
        "Input: 65 GXC (τ=0.65)",
        "Outputs: 8 addresses",
        "Pattern: Smurfing detected"
    ]
}
```

### 5. Reversal Approval Conditions

**When reversals are approved:**

#### Phase 1: Admin Validates FACTS
```
Conditions for admin approval:
✓ Evidence supports theft claim
✓ Transaction was unauthorized
✓ Victim identity verified
✓ Timeline makes sense

Admin does NOT check:
✗ Whether funds are still available
✗ Whether reversal is mathematically possible
✗ Current holder balance
```

#### Phase 2: Protocol Validates FEASIBILITY
```
Conditions for protocol execution:
✓ Taint score ≥ 10% (TAINT_THRESHOLD)
✓ Current holder has sufficient balance
✓ System pool has funds for fees
✓ Within reversal time window (30 days)
✓ Cryptographic proof is valid

Protocol automatically:
✓ Executes if all conditions met
✗ Rejects if any condition fails
```

**Code:**
```cpp
bool ReversalExecutor::validateFeasibility(const ProofOfFeasibility& proof) {
    // Check 1: Taint score
    if (proof.taint_score < 0.1) {
        return false; // Less than 10% tainted
    }
    
    // Check 2: Balance
    double holderBalance = blockchain->getBalance(proof.current_holder);
    if (holderBalance < proof.recoverable_amount / 100000000.0) {
        return false; // Insufficient balance
    }
    
    // Check 3: System pool
    if (!feePool->hasSufficientFunds(calculateFee(proof.recoverable_amount))) {
        return false; // Pool underfunded
    }
    
    // Check 4: Time window
    if (currentTime - proof.timestamp > 30 * 24 * 60 * 60) {
        return false; // Too old
    }
    
    // Check 5: Proof validity
    if (!proofGenerator->validateProof(proof)) {
        return false; // Invalid proof
    }
    
    return true; // All checks passed
}
```

## Multi-Wallet Recovery Example

### Scenario: Funds Split to 9 Wallets

**Initial State:**
```
Stolen: 100 GXC from Victim
Split to 9 wallets as shown above
```

**Recovery Process:**

#### Step 1: Mark as Stolen
```
fraudDetection->markAsStolen("original_theft_tx");
```

#### Step 2: Automatic Taint Propagation
```
System traces funds:
- Wallet 1: 50 GXC (τ=0.5) ✓ Above threshold
- Wallet 2: 20 GXC (τ=0.2) ✓ Above threshold
- Wallet 3: 10 GXC (τ=0.1) ✓ At threshold
- Wallet 4: 5 GXC (τ=0.05) ✗ Below threshold
- Wallets 5-9: All below threshold
```

#### Step 3: Generate Proofs for Each Wallet
```
For Wallet 1:
  Proof {
    stolen_tx: "original_theft_tx"
    current_holder: "wallet_1"
    taint_score: 0.5
    recoverable_amount: 50 GXC × 0.5 = 25 GXC
    trace_path: ["original_theft_tx", "split_tx", "wallet_1_tx"]
  }

For Wallet 2:
  Proof {
    stolen_tx: "original_theft_tx"
    current_holder: "wallet_2"
    taint_score: 0.2
    recoverable_amount: 20 GXC × 0.2 = 4 GXC
    trace_path: ["original_theft_tx", "split_tx", "wallet_2_tx"]
  }

For Wallet 3:
  Proof {
    stolen_tx: "original_theft_tx"
    current_holder: "wallet_3"
    taint_score: 0.1
    recoverable_amount: 10 GXC × 0.1 = 1 GXC
    trace_path: ["original_theft_tx", "split_tx", "wallet_3_tx"]
  }
```

#### Step 4: Protocol Validates Each Proof
```
Wallet 1: ✓ Feasible (τ=0.5, balance=50 GXC)
  → Execute reversal: Recover 25 GXC

Wallet 2: ✓ Feasible (τ=0.2, balance=20 GXC)
  → Execute reversal: Recover 4 GXC

Wallet 3: ✓ Feasible (τ=0.1, balance=10 GXC)
  → Execute reversal: Recover 1 GXC

Wallets 4-9: ✗ Infeasible (τ < 0.1)
  → Cannot recover (below threshold)
```

#### Step 5: Total Recovery
```
Recovered:
- From Wallet 1: 25 GXC
- From Wallet 2: 4 GXC
- From Wallet 3: 1 GXC
Total: 30 GXC out of 100 GXC stolen

Unrecoverable:
- Wallets 4-9: 20 GXC (taint too low)
- Lost to fees: ~0.5 GXC
- Unrecoverable: 49.5 GXC
```

### Why Not 100% Recovery?

**Taint Threshold (10%):**
- Protects innocent parties
- Prevents false positives
- Ensures mathematical certainty

**Example:**
```
Wallet 4 has 5 GXC with τ=0.05 (5% tainted)
- 95% of funds are clean (from legitimate sources)
- Reversing would harm innocent party
- Protocol rejects: τ < 0.1
```

**Mixing with Clean Funds:**
```
If Wallet 2 (20 GXC, τ=0.2) mixes with clean funds:
  Input 1: 20 GXC (τ=0.2) [stolen]
  Input 2: 80 GXC (τ=0.0) [clean]
  Output: 100 GXC (τ=0.04) [4% tainted]
  
Result: τ < 0.1, cannot reverse
Reason: 96% of funds are clean
```

## Automatic vs Manual Triggers

### Automatic Triggers

**1. Taint Propagation**
- Triggered: When transaction marked as stolen
- Frequency: Runs once, traces all paths
- Scope: Up to 10 hops (configurable)

**2. Fraud Pattern Checking**
- Triggered: On every tainted transaction
- Frequency: Real-time as transactions occur
- Scope: All 5 rules checked

**3. Alert Generation**
- Triggered: When rules violated
- Frequency: Real-time
- Scope: All tainted transactions

### Manual Triggers

**1. Marking as Stolen**
- Triggered: Admin approves fraud report
- Frequency: Once per theft
- Scope: Single transaction

**2. Reversal Execution**
- Triggered: Admin approves FACTS
- Frequency: Once per fraud report
- Scope: Protocol validates and executes

## Edge Cases and Limitations

### Edge Case 1: Funds Already Moved

**Scenario:**
```
Stolen: 100 GXC
Split to Wallet 1: 50 GXC (τ=0.5)
Wallet 1 sends to Wallet 10: 50 GXC (τ=0.5)
Wallet 1 current balance: 0 GXC
```

**Result:**
```
Protocol validation:
✗ Wallet 1 balance: 0 GXC
✗ Required: 25 GXC (50 × 0.5)
Status: INFEASIBLE

Solution: Generate proof for Wallet 10 instead
```

### Edge Case 2: Taint Dilution

**Scenario:**
```
Stolen: 100 GXC (τ=1.0)
Mixed with 900 GXC clean funds
Result: 1000 GXC (τ=0.1)
Further mixed with 9000 GXC clean
Result: 10000 GXC (τ=0.01)
```

**Result:**
```
Protocol validation:
✗ Taint score: 0.01
✗ Threshold: 0.1
Status: INFEASIBLE

Reason: 99% of funds are clean
Cannot harm innocent parties
```

### Edge Case 3: Time Window Expired

**Scenario:**
```
Theft occurred: Day 1
Fraud report: Day 35
Admin approval: Day 36
```

**Result:**
```
Protocol validation:
✗ Time elapsed: 36 days
✗ Maximum: 30 days
Status: INFEASIBLE

Reason: Too much time passed
Finality must be respected
```

### Edge Case 4: Multiple Reversals

**Scenario:**
```
Same stolen funds traced to 3 wallets:
- Wallet A: 30 GXC (τ=0.6)
- Wallet B: 40 GXC (τ=0.4)
- Wallet C: 30 GXC (τ=0.3)
```

**Result:**
```
Generate 3 separate proofs:
1. Proof for Wallet A → Recover 18 GXC
2. Proof for Wallet B → Recover 16 GXC
3. Proof for Wallet C → Recover 9 GXC

Total recovered: 43 GXC
Each reversal is independent
```

## System Integration

### Blockchain Integration

**When blocks are added:**
```cpp
bool Blockchain::addBlock(const Block& block) {
    // ... block validation ...
    
    // Check all transactions for fraud patterns
    for (const auto& tx : block.getTransactions()) {
        if (fraudDetection) {
            // Automatic fraud checking
            std::vector<FraudAlert> alerts = fraudDetection->checkTransaction(tx);
            
            // Log alerts
            for (const auto& alert : alerts) {
                LOG_SECURITY("Fraud alert: " + alert.description);
            }
        }
    }
    
    // ... continue block processing ...
}
```

### API Integration

**Endpoints:**
```
POST /api/fraud/report
  → Submit fraud report
  → Returns: report_id

POST /api/fraud/mark-stolen
  → Mark transaction as stolen (admin only)
  → Triggers: Automatic taint propagation

GET /api/fraud/taint/{txHash}
  → Get taint score for transaction
  → Returns: taint_score, trace_path

GET /api/fraud/alerts
  → Get all fraud alerts
  → Returns: list of alerts

GET /api/fraud/trace/{txHash}
  → Trace tainted flow from transaction
  → Returns: all paths and taint scores
```

## Summary

### How Fraud Detection Works

1. **Trigger:** Admin marks transaction as stolen
2. **Propagation:** System automatically traces funds forward
3. **Detection:** 5 fraud rules check every tainted transaction
4. **Alerts:** System generates alerts for suspicious patterns
5. **Approval:** Admin validates FACTS (fraud legitimacy)
6. **Validation:** Protocol validates FEASIBILITY (mathematical constraints)
7. **Execution:** Automatic if feasible, rejected if not

### Multi-Wallet Recovery

- System traces funds to ALL wallets automatically
- Each wallet gets independent proof
- Recovery amount = balance × taint_score
- Only wallets with τ ≥ 0.1 can be reversed
- Multiple reversals executed independently

### Key Principles

- **Automatic:** Taint propagation is automatic
- **Mathematical:** Based on conservation of value
- **Fair:** Protects innocent parties (10% threshold)
- **Transparent:** All traces are auditable
- **Efficient:** Handles complex splitting patterns

The system is designed to recover as much as mathematically possible while protecting innocent parties from false positives.
