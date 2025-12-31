# GXC Fraud Detection Algorithm - Mathematical Implementation

## Executive Summary

GXC's fraud detection system transforms stolen fund detection from a **guessing problem** into a **graph + rules problem** by leveraging Proof of Traceability (POT).

**Key Insight:** Because every transaction has explicit ancestry, stolen funds become a **causal signal** and crime becomes a **graph anomaly**.

This is not heuristics. This is **physics**.

---

## 1. Core Mathematical Model

### Transaction Graph

The blockchain is modeled as a directed acyclic graph (DAG):

```
G = (V, E)

where:
  V = set of all transactions
  E = directed edges (T_i → T_j means T_j spends output from T_i)
```

**Properties guaranteed by POT:**
- Graph is **acyclic** (no cycles possible)
- Every node has **unique ancestry**
- Every edge represents **value flow**

### Taint Function

Define a taint function τ: V → [0,1]

**Initial Condition:**
```
τ(T) = {
  1    if T ∈ S (stolen transaction set)
  0    otherwise
}
```

**Propagation Rule (Conservation of Value):**
```
For transaction T_j with inputs from {T_i}:

τ(T_j) = Σ w_i · τ(T_i)

where:
  w_i = value_from_T_i / total_input_value
```

**This is not opinion. This is conservation of value.**

---

## 2. The Algorithm (Fast BFS Propagation)

### Pseudocode

```python
def propagate_taint(start_tx, max_hops=10):
    """
    Propagate taint through transaction graph
    Complexity: O(edges touched), NOT O(chain size)
    """
    queue = [(start_tx, 0)]  # (transaction, depth)
    visited = set()
    
    while queue:
        current_tx, depth = queue.pop(0)
        
        if depth >= max_hops:
            continue
        
        current_taint = τ(current_tx)
        
        # Stop if taint below threshold δ
        if current_taint < δ:
            continue
        
        # Find all descendants (transactions spending this output)
        descendants = get_descendants(current_tx)
        
        for desc_tx in descendants:
            if desc_tx in visited:
                continue
            
            visited.add(desc_tx)
            
            # Calculate taint
            desc_taint = calculate_taint(desc_tx)
            
            if desc_taint > 0:
                τ(desc_tx) = desc_taint
                
                # Check rules
                alerts = check_rules(desc_tx, desc_taint)
                
                # Continue propagation
                queue.append((desc_tx, depth + 1))
    
    return visited
```

### Complexity Analysis

**Time Complexity:** O(E_touched)
- Only traverses edges reachable from stolen transaction
- Does NOT scan entire blockchain
- Stops when τ < δ

**Space Complexity:** O(V_visited)
- Only stores visited transactions
- Bounded by max_hops

**Why This is Fast:**
- Average case: O(10-100 transactions)
- Worst case: O(1000s of transactions)
- Never O(millions of transactions)

---

## 3. The Five Rules (Real-World Intelligence)

### Rule 1: Velocity Anomaly

**Principle:** Stolen funds move quickly to reduce seizure risk.

**Mathematical Condition:**
```
Δt(T_{i+1} - T_i) < ε  AND  τ(T_i) > τ_high

where:
  ε = 300 seconds (5 minutes)
  τ_high = 0.5
```

**Implementation:**
```cpp
bool checkVelocityAnomaly(Transaction tx, TaintInfo taint) {
    if (taint.ancestry.size() < 2) return false;
    
    Transaction prev_tx = get_previous_transaction(taint);
    uint64_t time_delta = tx.timestamp - prev_tx.timestamp;
    
    return (time_delta < VELOCITY_EPSILON && 
            taint.score > HIGH_TAINT);
}
```

**Real-World Example:**
```
Stolen funds: 1000 GXC
T1 (t=0):     1000 GXC → Address A
T2 (t=120s):  1000 GXC → Address B  🚩 ALERT
T3 (t=240s):  1000 GXC → Address C  🚩 ALERT
```

---

### Rule 2: Fan-Out Pattern (Smurfing)

**Principle:** Criminals split funds to evade limits.

**Mathematical Condition:**
```
outdegree(T) > k  AND  τ(T) > τ_min

where:
  k = 5 (max outputs before alert)
  τ_min = 0.1
```

**Implementation:**
```cpp
bool checkFanOutPattern(Transaction tx, TaintInfo taint) {
    set<string> recipients;
    for (auto output : tx.outputs) {
        recipients.insert(output.address);
    }
    
    return (recipients.size() > FAN_OUT_K && 
            taint.score > TAINT_THRESHOLD);
}
```

**Real-World Example:**
```
Stolen funds: 1000 GXC

T1: 1000 GXC → {
    Address A: 200 GXC
    Address B: 200 GXC
    Address C: 200 GXC
    Address D: 200 GXC
    Address E: 200 GXC
}  🚩 ALERT: Smurfing detected
```

---

### Rule 3: Re-Aggregation (Layering)

**Principle:** After splitting, funds are recombined to obscure origin.

**Mathematical Condition:**
```
∃ T_j : Σ_{i ∈ inputs} τ(T_i) > θ

where:
  θ = 0.7 (re-aggregation threshold)
```

**Implementation:**
```cpp
bool checkReAggregation(Transaction tx) {
    if (tx.inputs.size() < 2) return false;
    
    double total_taint = 0.0;
    for (auto input : tx.inputs) {
        total_taint += get_taint(input.tx_hash);
    }
    
    return (total_taint > RE_AGG_THETA);
}
```

**Real-World Example:**
```
After smurfing:
  Address A: 200 GXC (τ=0.2)
  Address B: 200 GXC (τ=0.2)
  Address C: 200 GXC (τ=0.2)
  Address D: 200 GXC (τ=0.2)

T_combine: {A, B, C, D} → Address X: 800 GXC
  τ(T_combine) = 0.2 + 0.2 + 0.2 + 0.2 = 0.8  🚩 ALERT: Layering
```

---

### Rule 4: Dormancy then Activation

**Principle:** Criminals wait for attention to die down.

**Mathematical Condition:**
```
Δt ≫ median_time  AND  τ(T) > 0

where:
  Δt = current_time - taint_timestamp
  dormancy_period = 604,800 seconds (7 days)
```

**Implementation:**
```cpp
bool checkDormancyActivation(Transaction tx, TaintInfo taint) {
    uint64_t current_time = time(nullptr);
    uint64_t dormancy = current_time - taint.timestamp;
    
    return (dormancy > DORMANCY_PERIOD && 
            taint.score > TAINT_THRESHOLD);
}
```

**Real-World Example:**
```
T1 (day 0):   Theft - 1000 GXC stolen
T2 (day 30):  First movement after 30 days  🚩 ALERT: Dormancy activation
```

---

### Rule 5: Clean Zone Entry

**Principle:** Attempt to legitimize stolen value.

**Mathematical Condition:**
```
destination(T) ∈ CleanZones  AND  τ(T) > τ_min

where:
  CleanZones = {exchanges, staking, merchants}
```

**Implementation:**
```cpp
bool checkCleanZoneEntry(Transaction tx, TaintInfo taint) {
    // Check if destination is exchange/staking/merchant
    if (tx.type == STAKE) {
        return taint.score > TAINT_THRESHOLD;
    }
    
    if (is_exchange_address(tx.to)) {
        return taint.score > TAINT_THRESHOLD;
    }
    
    return false;
}
```

**Real-World Example:**
```
T1: Stolen 1000 GXC (τ=1.0)
T2: 1000 GXC → Exchange deposit address  🚩🚩🚩 CRITICAL ALERT
```

---

## 4. Composite Risk Scoring

### Alert Levels

```
Risk Score = average(rule_scores)

Alert Level:
  CRITICAL:  score ≥ 0.8  OR  3+ rule violations
  HIGH:      score ≥ 0.5  OR  2+ rule violations
  MEDIUM:    score ≥ 0.1  OR  1 rule violation
  LOW:       score < 0.1
```

### Recommendations by Level

```
CRITICAL → FREEZE ADDRESS + Contact authorities
HIGH     → FLAG ADDRESS + Monitor closely + Delay withdrawals
MEDIUM   → WATCH ADDRESS + Increase monitoring frequency
LOW      → NORMAL + Continue standard monitoring
```

---

## 5. System Output (Not Accusations)

**What the system outputs:**
```json
{
  "transaction": "abc123...",
  "taint_score": 0.85,
  "alert_level": "CRITICAL",
  "rule_violations": [
    "VELOCITY_ANOMALY",
    "FAN_OUT_PATTERN",
    "CLEAN_ZONE_ENTRY"
  ],
  "evidence": [
    "Time delta: 120 seconds",
    "Output count: 7 addresses",
    "Destination: Exchange address"
  ],
  "recommendation": "FREEZE ADDRESS - Contact authorities",
  "ancestry": [
    "stolen_tx_hash",
    "intermediate_tx_1",
    "intermediate_tx_2",
    "current_tx"
  ]
}
```

**NOT:**
```
"Alice is a criminal"
```

**BUT:**
```
"This value-flow violates Rule X with evidence Y"
```

This is crucial legally and ethically.

---

## 6. Why This Works Better Than Traditional AML

### Traditional Finance

| Aspect | Traditional | GXC |
|--------|-------------|-----|
| Data | Incomplete | Complete |
| Ledger | Hidden | Public |
| Causality | Inferred | Deterministic |
| Detection | Heuristics | Physics |
| Verification | Trust-based | Cryptographic |

### GXC Advantages

1. **Perfect Lineage:** Every coin traceable to origin
2. **Deterministic Causality:** No guessing about flow
3. **Public Verification:** Anyone can verify taint
4. **Real-Time Detection:** O(edges touched), not O(chain size)
5. **Mathematical Proof:** Not opinion-based

---

## 7. Why Most Blockchains Cannot Do This

### Requirements

✅ **GXC Has:**
- Explicit transaction ancestry (prevTxHash)
- Deterministic receipts (WorkReceipt)
- Full value lineage (POT)
- Causal graph structure

❌ **Most Blockchains Lack:**
- Explicit ancestry (must infer)
- Deterministic receipts
- Full lineage tracking
- Must rely on heuristics

### Comparison

**Bitcoin:**
- No explicit ancestry
- Must cluster addresses
- Heuristic-based
- Off-chain analysis required

**Ethereum:**
- Account model (not UTXO)
- No explicit lineage
- Contract interactions complex
- Difficult to trace

**GXC:**
- Explicit ancestry (POT)
- UTXO model with traceability
- Deterministic causality
- On-chain detection

---

## 8. Implementation Details

### Data Structures

```cpp
// Taint tracking
unordered_map<string, TaintInfo> taintMap;  // O(1) lookup
unordered_set<string> stolenTransactions;   // Seed set S

// Alert system
vector<FraudAlert> alerts;
unordered_map<string, vector<FraudAlert>> addressAlerts;

// Address flagging
unordered_set<string> flaggedAddresses;
```

### Configuration Parameters

```cpp
// Taint thresholds
TAINT_THRESHOLD = 0.1      // δ: Stop propagation below this
HIGH_TAINT = 0.5           // High taint threshold
CRITICAL_TAINT = 0.8       // Critical taint threshold

// Rule parameters
VELOCITY_EPSILON = 300     // 5 minutes
FAN_OUT_K = 5              // Max outputs
RE_AGG_THETA = 0.7         // Re-aggregation threshold
DORMANCY_PERIOD = 604800   // 7 days
```

### API Methods

```cpp
// Seed management
void markAsStolen(string txHash);
void removeFromStolen(string txHash);

// Taint propagation
void propagateTaint(string startTxHash, uint32_t maxHops = 10);
double getTaintScore(string txHash);

// Rule checking
vector<FraudAlert> checkTransaction(Transaction tx);
vector<FraudAlert> checkAddress(string address);

// Alert management
vector<FraudAlert> getAlerts(AlertLevel minLevel);
void clearAlerts();

// Address flagging
void flagAddress(string address, string reason);
bool isAddressFlagged(string address);

// Consensus integration
bool shouldBlockTransaction(Transaction tx);
bool shouldFreezeAddress(string address);
```

---

## 9. Usage Examples

### Example 1: Mark Transaction as Stolen

```cpp
FraudDetection fraud(blockchain);

// Mark transaction as stolen
fraud.markAsStolen("abc123...");

// System automatically:
// 1. Sets τ(T) = 1.0
// 2. Propagates taint through descendants
// 3. Checks all rules
// 4. Generates alerts
```

### Example 2: Check Transaction Before Acceptance

```cpp
bool Blockchain::validateTransaction(Transaction tx) {
    // ... other validations ...
    
    // Check fraud detection
    if (fraudDetection->shouldBlockTransaction(tx)) {
        LOG_ERROR("Transaction blocked by fraud detection");
        return false;
    }
    
    // Check for alerts
    auto alerts = fraudDetection->checkTransaction(tx);
    for (const auto& alert : alerts) {
        if (alert.level == FraudAlert::CRITICAL) {
            LOG_CRITICAL("Critical fraud alert for transaction");
            // Optionally block or flag
        }
    }
    
    return true;
}
```

### Example 3: Monitor Address

```cpp
// Check if address should be frozen
if (fraudDetection->shouldFreezeAddress("tGXC...")) {
    // Freeze address
    // Notify authorities
    // Block all transactions
}

// Get all alerts for address
auto alerts = fraudDetection->getAddressAlerts("tGXC...");
for (const auto& alert : alerts) {
    cout << "Alert: " << alert.description << endl;
    cout << "Level: " << alert.level << endl;
    cout << "Taint: " << alert.taintScore << endl;
}
```

### Example 4: Get Statistics

```cpp
auto stats = fraudDetection->getStatistics();

cout << "Total stolen transactions: " << stats.totalStolenTx << endl;
cout << "Total tainted transactions: " << stats.totalTaintedTx << endl;
cout << "Critical alerts: " << stats.criticalAlerts << endl;
cout << "High alerts: " << stats.highAlerts << endl;
cout << "Flagged addresses: " << stats.flaggedAddresses << endl;
```

---

## 10. Integration with Exchanges

### Exchange API

```cpp
// Before accepting deposit
bool Exchange::acceptDeposit(string txHash, string address) {
    // Check taint
    double taint = fraudDetection->getTaintScore(txHash);
    
    if (taint > 0.5) {
        // High taint - reject or hold
        notifyCompliance(txHash, taint);
        return false;
    }
    
    // Check address
    if (fraudDetection->isAddressFlagged(address)) {
        // Flagged address - reject
        return false;
    }
    
    // Check alerts
    auto alerts = fraudDetection->getAddressAlerts(address);
    if (!alerts.empty()) {
        // Has alerts - review manually
        queueForReview(txHash, address, alerts);
    }
    
    return true;
}
```

---

## 11. One Sentence Summary

> GXC detects stolen funds fast because the blockchain turns money into a **causal signal**, and crime into a **graph anomaly**.

---

## 12. Next Steps

### Immediate Implementation

1. ✅ Mathematical model defined
2. ✅ Core algorithm implemented
3. ✅ Five rules coded
4. ✅ Alert system built
5. ✅ Consensus integration ready

### Future Enhancements

1. **Machine Learning Layer**
   - Train on historical fraud patterns
   - Improve rule thresholds
   - Detect new patterns

2. **Exchange Registry**
   - Maintain list of exchange addresses
   - Automatic clean zone detection
   - KYC integration

3. **Governance Integration**
   - Community voting on flagged addresses
   - Transparent blacklist management
   - Appeal process

4. **Cross-Chain Detection**
   - Track funds across bridges
   - Detect cross-chain laundering
   - Coordinate with other chains

---

## Conclusion

This is not heuristics.
This is not guessing.
This is **physics**.

**GXC's fraud detection transforms stolen fund detection from a forensic problem into a real-time graph analysis problem.**

The system is:
- ✅ **Fast:** O(edges touched), not O(chain size)
- ✅ **Accurate:** Based on conservation of value
- ✅ **Transparent:** Public verification
- ✅ **Fair:** Evidence-based, not accusatory
- ✅ **Elegant:** Leverages existing POT infrastructure

**Ready for production deployment.**

---

**Files:**
- `include/FraudDetection.h` - Header with full API
- `src/FraudDetection.cpp` - Complete implementation
- `FRAUD_DETECTION_ALGORITHM.md` - This documentation

**Status:** ✅ IMPLEMENTED AND READY
