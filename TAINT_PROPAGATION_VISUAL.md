# Taint Propagation: Visual Guide

## The Mathematical Foundation

### Conservation of Taint Formula

```
τ(T_j) = Σ w_i · τ(T_i)

where:
τ(T_j) = taint score of output transaction j
w_i    = weight of input i = (value_from_i / total_input_value)
τ(T_i) = taint score of input transaction i
```

**Key Principle:** Taint is conserved like mass or energy. It cannot be created or destroyed, only diluted or concentrated.

## Scenario 1: Simple Split (9 Wallets)

### Initial Theft

```
┌─────────────────────────────────────┐
│  VICTIM'S WALLET                    │
│  Balance: 100 GXC                   │
│  τ = 0.0 (clean)                    │
└─────────────────────────────────────┘
                 │
                 │ THEFT OCCURS
                 ↓
┌─────────────────────────────────────┐
│  THIEF'S WALLET                     │
│  Stolen: 100 GXC                    │
│  τ = 1.0 (100% stolen)              │
│  Transaction: TX_STOLEN             │
└─────────────────────────────────────┘
```

### Split Transaction

```
┌─────────────────────────────────────┐
│  THIEF'S WALLET                     │
│  100 GXC (τ = 1.0)                  │
└─────────────────────────────────────┘
                 │
                 │ SPLIT TO 9 WALLETS
                 ↓
    ┌────────────┴────────────┐
    │                         │
    ↓                         ↓
┌─────────┐              ┌─────────┐
│ Wallet 1│              │ Wallet 2│
│ 50 GXC  │              │ 20 GXC  │
│ τ = 0.5 │              │ τ = 0.2 │
└─────────┘              └─────────┘
    │                         │
    ├─────────────────────────┼─────────────────────────┐
    ↓                         ↓                         ↓
┌─────────┐              ┌─────────┐              ┌─────────┐
│ Wallet 3│              │ Wallet 4│              │ Wallet 5│
│ 10 GXC  │              │ 5 GXC   │              │ 5 GXC   │
│ τ = 0.1 │              │ τ = 0.05│              │ τ = 0.05│
└─────────┘              └─────────┘              └─────────┘
    │                         │                         │
    ├─────────────────────────┼─────────────────────────┤
    ↓                         ↓                         ↓
┌─────────┐              ┌─────────┐              ┌─────────┐
│ Wallet 6│              │ Wallet 7│              │ Wallet 8│
│ 3 GXC   │              │ 3 GXC   │              │ 2 GXC   │
│ τ = 0.03│              │ τ = 0.03│              │ τ = 0.02│
└─────────┘              └─────────┘              └─────────┘
                                                       │
                                                       ↓
                                                  ┌─────────┐
                                                  │ Wallet 9│
                                                  │ 2 GXC   │
                                                  │ τ = 0.02│
                                                  └─────────┘
```

### Taint Calculation

```
Wallet 1: 50/100 × 1.0 = 0.5  ✓ Above threshold (0.1)
Wallet 2: 20/100 × 1.0 = 0.2  ✓ Above threshold
Wallet 3: 10/100 × 1.0 = 0.1  ✓ At threshold
Wallet 4: 5/100  × 1.0 = 0.05 ✗ Below threshold
Wallet 5: 5/100  × 1.0 = 0.05 ✗ Below threshold
Wallet 6: 3/100  × 1.0 = 0.03 ✗ Below threshold
Wallet 7: 3/100  × 1.0 = 0.03 ✗ Below threshold
Wallet 8: 2/100  × 1.0 = 0.02 ✗ Below threshold
Wallet 9: 2/100  × 1.0 = 0.02 ✗ Below threshold

Total taint: 0.5 + 0.2 + 0.1 + 0.05 + 0.05 + 0.03 + 0.03 + 0.02 + 0.02 = 1.0 ✓
(Conservation verified)
```

### Recovery Potential

```
┌──────────┬──────────┬──────────┬──────────────┬────────────┐
│ Wallet   │ Balance  │ Taint    │ Recoverable  │ Status     │
├──────────┼──────────┼──────────┼──────────────┼────────────┤
│ Wallet 1 │ 50 GXC   │ 0.5      │ 25 GXC       │ ✓ FEASIBLE │
│ Wallet 2 │ 20 GXC   │ 0.2      │ 4 GXC        │ ✓ FEASIBLE │
│ Wallet 3 │ 10 GXC   │ 0.1      │ 1 GXC        │ ✓ FEASIBLE │
│ Wallet 4 │ 5 GXC    │ 0.05     │ 0.25 GXC     │ ✗ INFEASIBLE│
│ Wallet 5 │ 5 GXC    │ 0.05     │ 0.25 GXC     │ ✗ INFEASIBLE│
│ Wallet 6 │ 3 GXC    │ 0.03     │ 0.09 GXC     │ ✗ INFEASIBLE│
│ Wallet 7 │ 3 GXC    │ 0.03     │ 0.09 GXC     │ ✗ INFEASIBLE│
│ Wallet 8 │ 2 GXC    │ 0.02     │ 0.04 GXC     │ ✗ INFEASIBLE│
│ Wallet 9 │ 2 GXC    │ 0.02     │ 0.04 GXC     │ ✗ INFEASIBLE│
├──────────┼──────────┼──────────┼──────────────┼────────────┤
│ TOTAL    │ 100 GXC  │ 1.0      │ 30 GXC       │ 30% RECOVERY│
└──────────┴──────────┴──────────┴──────────────┴────────────┘
```

## Scenario 2: Mixing with Clean Funds

### Wallet 2 Mixes with Clean Funds

```
┌─────────────────────────────────────┐
│  WALLET 2                           │
│  20 GXC (τ = 0.2)                   │
│  [From stolen funds]                │
└─────────────────────────────────────┘
                 │
                 │ MIXED WITH
                 ↓
┌─────────────────────────────────────┐
│  CLEAN SOURCE                       │
│  80 GXC (τ = 0.0)                   │
│  [Legitimate earnings]              │
└─────────────────────────────────────┘
                 │
                 │ COMBINED
                 ↓
┌─────────────────────────────────────┐
│  WALLET 10                          │
│  100 GXC (τ = ?)                    │
│                                     │
│  Calculation:                       │
│  τ = (20/100 × 0.2) + (80/100 × 0.0)│
│  τ = 0.04 + 0.0                     │
│  τ = 0.04 (4% tainted)              │
│                                     │
│  Status: ✗ BELOW THRESHOLD (0.1)    │
│  Reason: 96% of funds are clean     │
└─────────────────────────────────────┘
```

### Why This Protects Innocent Parties

```
Wallet 10 owner:
- Earned 80 GXC legitimately
- Received 20 GXC unknowingly from tainted source
- 96% of their funds are clean
- Reversing would harm innocent party
- Protocol rejects: τ < 0.1
```

## Scenario 3: Multi-Hop Propagation

### Funds Move Through Multiple Wallets

```
THEFT (100 GXC, τ=1.0)
    │
    ↓
WALLET A (100 GXC, τ=1.0)
    │
    ├─→ WALLET B (60 GXC, τ=0.6)
    │       │
    │       ├─→ WALLET D (30 GXC, τ=0.3)
    │       │       │
    │       │       └─→ WALLET F (15 GXC, τ=0.15) ✓
    │       │
    │       └─→ WALLET E (30 GXC, τ=0.3)
    │               │
    │               └─→ WALLET G (10 GXC, τ=0.1) ✓
    │
    └─→ WALLET C (40 GXC, τ=0.4)
            │
            ├─→ WALLET H (20 GXC, τ=0.2) ✓
            │
            └─→ WALLET I (20 GXC, τ=0.2) ✓

Recoverable wallets (τ ≥ 0.1):
- Wallet F: 15 × 0.15 = 2.25 GXC
- Wallet G: 10 × 0.1 = 1 GXC
- Wallet H: 20 × 0.2 = 4 GXC
- Wallet I: 20 × 0.2 = 4 GXC
Total: 11.25 GXC
```

### Taint Propagation Tree

```
                    TX_STOLEN
                    100 GXC
                    τ = 1.0
                       │
        ┌──────────────┴──────────────┐
        │                             │
    WALLET_A                      WALLET_B
    60 GXC                        40 GXC
    τ = 0.6                       τ = 0.4
        │                             │
    ┌───┴───┐                     ┌───┴───┐
    │       │                     │       │
WALLET_D WALLET_E             WALLET_H WALLET_I
30 GXC   30 GXC               20 GXC   20 GXC
τ=0.3    τ=0.3                τ=0.2    τ=0.2
    │       │                     ✓       ✓
    │       │
WALLET_F WALLET_G
15 GXC   10 GXC
τ=0.15   τ=0.1
   ✓       ✓

Legend:
✓ = Above threshold, can be reversed
✗ = Below threshold, cannot be reversed
```

## Scenario 4: Re-Aggregation (Layering)

### Thief Recombines Split Funds

```
STEP 1: SPLIT
┌─────────────┐
│ 100 GXC     │
│ τ = 1.0     │
└──────┬──────┘
       │
   ┌───┴───┬───────┬───────┐
   ↓       ↓       ↓       ↓
┌──────┐┌──────┐┌──────┐┌──────┐
│25 GXC││25 GXC││25 GXC││25 GXC│
│τ=0.25││τ=0.25││τ=0.25││τ=0.25│
└──────┘└──────┘└──────┘└──────┘
  W1      W2      W3      W4

STEP 2: RE-AGGREGATE
┌──────┐┌──────┐┌──────┐
│25 GXC││25 GXC││25 GXC│
│τ=0.25││τ=0.25││τ=0.25│
└───┬──┘└───┬──┘└───┬──┘
    │       │       │
    └───────┼───────┘
            ↓
      ┌──────────┐
      │ 75 GXC   │
      │ τ = 0.75 │  ← HIGH TAINT!
      │ WALLET 5 │
      └──────────┘

FRAUD ALERT:
✓ Rule 3: Re-Aggregation detected
✓ 3 out of 3 inputs tainted (100%)
✓ Taint score: 0.75 (HIGH)
✓ Alert Level: HIGH
```

### Detection Logic

```cpp
bool checkReAggregation(const Transaction& tx) {
    uint32_t taintedInputs = 0;
    uint32_t totalInputs = tx.getInputs().size();
    
    for (const auto& input : tx.getInputs()) {
        if (getTaintScore(input.txHash) > 0.1) {
            taintedInputs++;
        }
    }
    
    double ratio = (double)taintedInputs / totalInputs;
    
    // Alert if > 70% of inputs are tainted
    return (ratio > 0.7);
}

// In this case:
// ratio = 3/3 = 1.0 (100%)
// 1.0 > 0.7 → ALERT!
```

## Scenario 5: Fan-Out Pattern (Smurfing)

### Thief Splits to Many Addresses

```
┌─────────────────────────────────────┐
│  WALLET WITH STOLEN FUNDS           │
│  100 GXC (τ = 1.0)                  │
└─────────────────────────────────────┘
                 │
                 │ SPLIT TO 10 ADDRESSES
                 ↓
    ┌────────────┴────────────┬────────────┬────────────┐
    ↓            ↓            ↓            ↓            ↓
┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐
│10 GXC  │  │10 GXC  │  │10 GXC  │  │10 GXC  │  │10 GXC  │
│τ = 0.1 │  │τ = 0.1 │  │τ = 0.1 │  │τ = 0.1 │  │τ = 0.1 │
└────────┘  └────────┘  └────────┘  └────────┘  └────────┘
    ↓            ↓            ↓            ↓            ↓
┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐
│10 GXC  │  │10 GXC  │  │10 GXC  │  │10 GXC  │  │10 GXC  │
│τ = 0.1 │  │τ = 0.1 │  │τ = 0.1 │  │τ = 0.1 │  │τ = 0.1 │
└────────┘  └────────┘  └────────┘  └────────┘  └────────┘

FRAUD ALERT:
✓ Rule 2: Fan-Out Pattern detected
✓ 10 outputs (threshold: 5)
✓ Classic smurfing technique
✓ Alert Level: MEDIUM
```

## Scenario 6: Velocity Anomaly

### Funds Moved Too Quickly

```
TIME: T0
┌─────────────────────────────────────┐
│  THEFT OCCURS                       │
│  100 GXC stolen                     │
│  Timestamp: 12:00:00                │
└─────────────────────────────────────┘

TIME: T0 + 2 minutes
┌─────────────────────────────────────┐
│  FIRST TRANSFER                     │
│  100 GXC → Wallet A                 │
│  Timestamp: 12:02:00                │
│  Time diff: 120 seconds             │
└─────────────────────────────────────┘

TIME: T0 + 4 minutes
┌─────────────────────────────────────┐
│  SECOND TRANSFER                    │
│  100 GXC → Wallet B                 │
│  Timestamp: 12:04:00                │
│  Time diff: 120 seconds             │
└─────────────────────────────────────┘

FRAUD ALERT:
✓ Rule 1: Velocity Anomaly detected
✓ Time between transactions: 120s < 300s
✓ Thief trying to move funds quickly
✓ Alert Level: HIGH
```

## Scenario 7: Clean Zone Entry

### Attempting to Cash Out

```
┌─────────────────────────────────────┐
│  WALLET WITH TAINTED FUNDS          │
│  50 GXC (τ = 0.5)                   │
└─────────────────────────────────────┘
                 │
                 │ SEND TO
                 ↓
┌─────────────────────────────────────┐
│  EXCHANGE (Clean Zone)              │
│  Registered: exchange.com           │
│  Type: EXCHANGE                     │
│  Purpose: Cash out to fiat          │
└─────────────────────────────────────┘

FRAUD ALERT:
✓ Rule 5: Clean Zone Entry detected
✓ Destination: Registered exchange
✓ Taint score: 0.5 (HIGH)
✓ Attempting to legitimize funds
✓ Alert Level: CRITICAL
✓ Action: Exchange can freeze funds
```

## Recovery Timeline

### Complete Flow from Theft to Recovery

```
DAY 1: THEFT
┌─────────────────────────────────────┐
│ 12:00 - Theft occurs                │
│ 12:05 - Funds split to 9 wallets    │
│ 12:10 - Victim discovers theft      │
└─────────────────────────────────────┘

DAY 1: REPORTING
┌─────────────────────────────────────┐
│ 13:00 - Victim submits fraud report │
│ 13:05 - Report enters queue         │
│ Status: PENDING                     │
└─────────────────────────────────────┘

DAY 2: ADMIN REVIEW
┌─────────────────────────────────────┐
│ 10:00 - Admin reviews evidence      │
│ 10:30 - Admin approves FACTS        │
│ Status: FACTS_APPROVED              │
│ Action: System marks TX as stolen   │
└─────────────────────────────────────┘

DAY 2: AUTOMATIC PROPAGATION
┌─────────────────────────────────────┐
│ 10:31 - Taint propagation starts    │
│ 10:31 - Traces to 9 wallets         │
│ 10:31 - Calculates taint scores     │
│ 10:31 - Identifies 3 recoverable    │
│ Status: VALIDATING                  │
└─────────────────────────────────────┘

DAY 2: PROTOCOL VALIDATION
┌─────────────────────────────────────┐
│ 10:32 - Generates proof for Wallet 1│
│ 10:32 - Validates feasibility       │
│ 10:32 - Executes reversal           │
│ 10:32 - Recovers 25 GXC             │
│                                     │
│ 10:33 - Generates proof for Wallet 2│
│ 10:33 - Validates feasibility       │
│ 10:33 - Executes reversal           │
│ 10:33 - Recovers 4 GXC              │
│                                     │
│ 10:34 - Generates proof for Wallet 3│
│ 10:34 - Validates feasibility       │
│ 10:34 - Executes reversal           │
│ 10:34 - Recovers 1 GXC              │
│                                     │
│ Status: EXECUTED                    │
│ Total recovered: 30 GXC             │
└─────────────────────────────────────┘

DAY 2: COMPLETION
┌─────────────────────────────────────┐
│ 10:35 - Victim receives 30 GXC      │
│ 10:35 - System pool receives fees   │
│ 10:35 - Report marked complete      │
│ Recovery rate: 30%                  │
└─────────────────────────────────────┘
```

## Key Takeaways

### 1. Taint is Conserved
```
Total taint before = Total taint after
1.0 = 0.5 + 0.2 + 0.1 + 0.05 + 0.05 + 0.03 + 0.03 + 0.02 + 0.02
```

### 2. Threshold Protects Innocents
```
τ < 0.1 → Cannot reverse
Reason: Majority of funds are clean
```

### 3. Multiple Wallets = Multiple Reversals
```
Each wallet gets independent proof
Each reversal executed separately
Total recovery = sum of all reversals
```

### 4. Mixing Dilutes Taint
```
Stolen funds + Clean funds = Lower taint
Lower taint = Harder to reverse
Protects innocent parties
```

### 5. Automatic Detection
```
No manual intervention needed
System traces all paths
Generates alerts automatically
Protocol validates independently
```

This visual guide demonstrates how the system handles complex scenarios while maintaining mathematical rigor and protecting innocent parties.
