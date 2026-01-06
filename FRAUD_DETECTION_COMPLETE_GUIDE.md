# Fraud Detection System - Complete Guide

## Overview

The GXC blockchain has a **production-grade fraud detection system** based on **Proof of Traceability (POT)** that automatically detects and tracks stolen funds through mathematical taint propagation.

## Who Activates Fraud Detection?

### Automatic Activation (Always Running)

The fraud detection system is **automatically initialized** when the node starts:

```cpp
// From src/node_main.cpp
FraudDetection fraudDetection(&blockchain);  // Line 292
```

**The system is ALWAYS active** - it doesn't need manual activation.

## How Fraud Detection Works

### 1. Initialization (Automatic)

When the node starts:
```
Node Startup
    ↓
Initialize Blockchain
    ↓
Initialize FraudDetection(&blockchain)  ← AUTOMATIC
    ↓
Initialize ProofGenerator
    ↓
Initialize ReversalExecutor
    ↓
Connect all systems
    ↓
Node Ready (Fraud Detection Active)
```

### 2. Fraud Report Submission (Manual Trigger)

**Who can report fraud?**
- **Anyone** can submit a fraud report via REST API
- **Admins** review and approve reports
- **System** automatically activates algorithms upon approval

**How to report fraud:**

```bash
# Via REST API
curl -X POST http://localhost:8080/api/fraud/report \
  -H "Content-Type: application/json" \
  -d '{
    "tx_hash": "abc123...",
    "victim_address": "tGXC...",
    "stolen_amount": 1000.0,
    "description": "Phishing attack",
    "evidence": ["screenshot.png", "email.txt"]
  }'
```

**What happens next:**
1. Report stored in database
2. Admin receives notification
3. Admin reviews evidence
4. Admin approves or rejects
5. **If approved**: Fraud algorithms activate automatically

### 3. Algorithm Activation (Automatic Upon Approval)

When an admin approves a fraud report:

```cpp
// From src/RESTServer_AdminAPIs.cpp:378
fraudDetection->markAsStolen(report.txHash);
```

This **automatically triggers**:

#### Algorithm 1: Taint Propagation
```
markAsStolen(txHash)
    ↓
Set τ(T) = 1.0 (100% tainted)
    ↓
propagateTaint(txHash)  ← AUTOMATIC
    ↓
BFS through transaction graph
    ↓
Calculate τ(T_j) = Σ w_i · τ(T_i)
    ↓
Track all descendant transactions
```

#### Algorithm 2: Velocity Analysis
```
For each tainted transaction:
    ↓
Check time between transactions
    ↓
If Δt < 5 minutes (VELOCITY_EPSILON)
    ↓
Generate VELOCITY_ANOMALY alert
```

#### Algorithm 3: Fan-Out Detection (Smurfing)
```
For each tainted transaction:
    ↓
Count number of outputs
    ↓
If outputs > 5 (FAN_OUT_K)
    ↓
Generate FAN_OUT_PATTERN alert
```

#### Algorithm 4: Re-Aggregation Detection (Layering)
```
For each address:
    ↓
Check if receiving from multiple tainted sources
    ↓
If aggregation > 70% (RE_AGG_THETA)
    ↓
Generate RE_AGGREGATION alert
```

#### Algorithm 5: Dormancy Activation
```
For each tainted UTXO:
    ↓
Check time since last movement
    ↓
If dormant > 7 days then suddenly moves
    ↓
Generate DORMANCY_ACTIVATION alert
```

#### Algorithm 6: Clean Zone Entry
```
For each tainted transaction:
    ↓
Check if sending to exchange/pool
    ↓
If entering clean zone with high taint
    ↓
Generate CLEAN_ZONE_ENTRY alert
```

## Conditions for Algorithm Activation

### Condition 1: Transaction Marked as Stolen
```cpp
fraudDetection->markAsStolen(txHash);
```
**Triggers**: All algorithms automatically

### Condition 2: New Transaction Detected
```cpp
// Automatic check on every new transaction
auto alerts = fraudDetection->checkTransaction(tx);
```
**Triggers**: All rule checks if transaction is tainted

### Condition 3: Address Monitoring
```cpp
// Check specific address
auto alerts = fraudDetection->checkAddress(address);
```
**Triggers**: All rules for that address

## What Each Algorithm Does

### 1. Taint Propagation (Core Algorithm)

**Purpose**: Track stolen funds through the blockchain

**How it works**:
```
Stolen TX (τ = 1.0)
    ↓
TX1 spends 50% → τ(TX1) = 0.5
    ↓
TX2 spends 30% of TX1 → τ(TX2) = 0.5 × 0.3 = 0.15
    ↓
TX3 spends 100% of TX2 → τ(TX3) = 0.15
    ↓
Stops when τ < 0.1 (TAINT_THRESHOLD)
```

**Mathematical Formula**:
```
τ(T_j) = Σ w_i · τ(T_i)

where:
  w_i = value_from_T_i / total_input_value
  τ(T_i) = taint score of input transaction
```

**Example**:
```
Stolen: 1000 GXC (τ = 1.0)
    ↓
Split into:
  - 600 GXC → Address A (τ = 0.6)
  - 400 GXC → Address B (τ = 0.4)
    ↓
Address A sends 300 GXC → Address C
  τ(C) = 0.6 × (300/600) = 0.3
```

### 2. Velocity Analysis

**Purpose**: Detect rapid movement (money laundering)

**Condition**: Time between transactions < 5 minutes

**Example**:
```
10:00:00 - Stolen TX
10:02:30 - TX1 (2.5 min later) ← ALERT!
10:03:45 - TX2 (1.25 min later) ← ALERT!
```

**Alert Level**: HIGH (rapid movement indicates laundering)

### 3. Fan-Out Detection (Smurfing)

**Purpose**: Detect splitting funds to hide trail

**Condition**: Transaction has > 5 outputs

**Example**:
```
Stolen: 1000 GXC
    ↓
Split into 10 outputs of 100 GXC each ← ALERT!
    ↓
Trying to hide by spreading funds
```

**Alert Level**: MEDIUM to HIGH

### 4. Re-Aggregation Detection (Layering)

**Purpose**: Detect collecting split funds back together

**Condition**: Address receives from multiple tainted sources

**Example**:
```
10 addresses (each with 100 GXC tainted)
    ↓
All send to Address X ← ALERT!
    ↓
Re-aggregating after smurfing
```

**Alert Level**: HIGH (indicates sophisticated laundering)

### 5. Dormancy Activation

**Purpose**: Detect "wait and move" strategy

**Condition**: Tainted funds dormant > 7 days, then suddenly move

**Example**:
```
Day 0: Funds stolen
Day 1-7: No movement
Day 8: Suddenly moves ← ALERT!
```

**Alert Level**: MEDIUM (trying to wait out detection)

### 6. Clean Zone Entry

**Purpose**: Detect attempts to legitimize stolen funds

**Condition**: High taint (>0.5) entering exchange/pool

**Example**:
```
Stolen funds (τ = 0.8)
    ↓
Sent to Binance ← ALERT!
    ↓
Trying to cash out or mix
```

**Alert Level**: CRITICAL (immediate action needed)

## Admin Functions

### 1. Review Fraud Reports

**Who**: Fraud admins, fraud reviewers

**How**:
```bash
# Get pending reports
curl http://localhost:8080/api/admin/fraud/pending \
  -H "Authorization: Bearer <token>"
```

**Response**:
```json
{
  "reports": [
    {
      "report_id": "FR001",
      "tx_hash": "abc123...",
      "victim": "tGXC...",
      "amount": 1000.0,
      "status": "pending",
      "submitted_at": 1704556800,
      "evidence": ["proof1.pdf", "proof2.png"]
    }
  ]
}
```

### 2. Approve Fraud Report

**Who**: Fraud approvers, super admins

**How**:
```bash
curl -X POST http://localhost:8080/api/admin/fraud/approve \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "report_id": "FR001",
    "notes": "Evidence verified, phishing confirmed"
  }'
```

**What happens**:
1. Report marked as approved
2. `fraudDetection->markAsStolen(txHash)` called
3. **All algorithms activate automatically**
4. Taint propagation starts
5. Alerts generated for all tainted transactions
6. Reversal proof generated (if feasible)

### 3. View Fraud Statistics

**Who**: All admins

**How**:
```bash
curl http://localhost:8080/api/admin/fraud/stats \
  -H "Authorization: Bearer <token>"
```

**Response**:
```json
{
  "total_reports": 15,
  "pending": 3,
  "approved": 10,
  "rejected": 2,
  "total_stolen_tx": 10,
  "total_tainted_tx": 247,
  "total_alerts": 89,
  "critical_alerts": 12,
  "total_recovered": 8500.0
}
```

### 4. View Taint Trace

**Who**: All admins

**How**:
```bash
curl http://localhost:8080/api/admin/fraud/trace/<tx_hash> \
  -H "Authorization: Bearer <token>"
```

**Response**:
```json
{
  "tx_hash": "abc123...",
  "taint_score": 0.65,
  "source_tx": "stolen_tx_hash",
  "hops": 3,
  "path": [
    {"tx": "stolen_tx", "taint": 1.0},
    {"tx": "tx1", "taint": 0.8},
    {"tx": "tx2", "taint": 0.7},
    {"tx": "abc123", "taint": 0.65}
  ],
  "alerts": [
    {"type": "VELOCITY_ANOMALY", "level": "HIGH"},
    {"type": "FAN_OUT_PATTERN", "level": "MEDIUM"}
  ]
}
```

### 5. Execute Reversal

**Who**: Super admins only

**How**:
```bash
curl -X POST http://localhost:8080/api/admin/fraud/reverse \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "report_id": "FR001",
    "current_holder": "tGXC...",
    "amount": 650.0
  }'
```

**What happens**:
1. System generates Proof of Feasibility
2. Validates:
   - Causality (funds came from stolen TX)
   - Conservation (amount ≤ recoverable)
   - Authorization (admin signature)
   - Finality (within reversal window)
3. Creates reversal transaction
4. Debits current holder
5. Credits original victim
6. Records in blockchain

## Alert Levels

### LOW
- Taint score: 0.1 - 0.3
- Action: Monitor
- Example: Distant descendant of stolen funds

### MEDIUM
- Taint score: 0.3 - 0.5
- Action: Flag for review
- Example: Fan-out pattern detected

### HIGH
- Taint score: 0.5 - 0.8
- Action: Alert admins
- Example: Velocity anomaly + high taint

### CRITICAL
- Taint score: 0.8 - 1.0
- Action: Immediate intervention
- Example: High taint entering exchange

## Testing Fraud Detection

### Test 1: Mark Transaction as Stolen
```python
import requests

# Via RPC
response = requests.post('http://localhost:8332', json={
    'jsonrpc': '2.0',
    'method': 'markfraudulent',
    'params': ['tx_hash_here'],
    'id': 1
})
```

### Test 2: Check Taint Score
```python
response = requests.post('http://localhost:8332', json={
    'jsonrpc': '2.0',
    'method': 'gettaintscore',
    'params': ['tx_hash_here'],
    'id': 1
})
print(f"Taint Score: {response.json()['result']}")
```

### Test 3: Get Alerts
```python
response = requests.post('http://localhost:8332', json={
    'jsonrpc': '2.0',
    'method': 'getfraudalerts',
    'params': ['address_here'],
    'id': 1
})
```

## Security Features

### 1. Automatic Detection
- No manual intervention needed
- Algorithms run automatically
- Real-time monitoring

### 2. Mathematical Proof
- Taint propagation is mathematically sound
- Conservation of value principle
- Cannot be gamed

### 3. Multi-Layer Defense
- 6 different detection algorithms
- Multiple alert levels
- Comprehensive coverage

### 4. Reversal Safety
- Proof of Feasibility required
- Admin approval needed
- Cryptographic validation

## Summary

### Activation
- ✅ **Automatic**: System always running
- ✅ **Triggered**: When fraud report approved
- ✅ **Real-time**: Checks every transaction

### Algorithms (6 Total)
1. ✅ Taint Propagation (Core)
2. ✅ Velocity Analysis
3. ✅ Fan-Out Detection
4. ✅ Re-Aggregation Detection
5. ✅ Dormancy Activation
6. ✅ Clean Zone Entry

### Admin Functions
- ✅ Review reports
- ✅ Approve/reject reports
- ✅ View statistics
- ✅ Trace tainted funds
- ✅ Execute reversals

### Protection
- ✅ Prevents money laundering
- ✅ Tracks stolen funds
- ✅ Enables recovery
- ✅ Deters fraud

**The fraud detection system is production-ready and fully automated!**

---

**Status**: ✅ ACTIVE
**Algorithms**: 6 running
**Activation**: Automatic
**Admin Control**: Full
