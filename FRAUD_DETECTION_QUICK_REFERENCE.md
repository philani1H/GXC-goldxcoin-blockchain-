# Fraud Detection: Quick Reference

## TL;DR

**Q: Who triggers fraud detection?**
A: Admin marks transaction as stolen → System automatically traces funds

**Q: What if funds split to 9 wallets?**
A: System traces ALL wallets automatically, recovers from each one independently

**Q: When are reversals approved?**
A: Admin approves FACTS → Protocol validates feasibility → Automatic execution if possible

## The 5-Second Explanation

```
1. Victim reports theft
2. Admin confirms it's legitimate
3. System automatically traces funds (even through 100 wallets)
4. Protocol checks if reversal is mathematically possible
5. If yes: automatic execution, if no: automatic rejection
```

## Key Numbers

| Parameter | Value | Meaning |
|-----------|-------|---------|
| Taint Threshold | 10% | Minimum taint to reverse |
| Max Hops | 10 | How far to trace funds |
| Time Window | 30 days | Maximum age for reversal |
| Velocity Threshold | 5 minutes | Too fast = suspicious |
| Fan-Out Threshold | 5 outputs | Too many = suspicious |
| Re-Agg Threshold | 70% | Tainted inputs = suspicious |

## The 9-Wallet Example

**Stolen:** 100 GXC

**Split to:**
- Wallet 1: 50 GXC (τ=0.5) → Recover 25 GXC ✓
- Wallet 2: 20 GXC (τ=0.2) → Recover 4 GXC ✓
- Wallet 3: 10 GXC (τ=0.1) → Recover 1 GXC ✓
- Wallets 4-9: Below threshold → Cannot recover ✗

**Total Recovery:** 30 GXC (30%)

**Why not 100%?**
- Wallets 4-9 have < 10% taint
- Majority of their funds are clean
- Reversing would harm innocent parties

## Taint Formula (Simple)

```
Taint = (Stolen Amount / Total Amount) × Previous Taint

Example:
- Input: 50 GXC with τ=1.0 (100% stolen)
- Mixed with: 50 GXC with τ=0.0 (clean)
- Output: 100 GXC with τ=0.5 (50% stolen)

Calculation: (50/100 × 1.0) + (50/100 × 0.0) = 0.5
```

## Fraud Detection Rules

| Rule | What It Detects | Threshold |
|------|----------------|-----------|
| 1. Velocity | Moving funds too fast | < 5 minutes |
| 2. Fan-Out | Splitting to many wallets | > 5 outputs |
| 3. Re-Aggregation | Recombining split funds | > 70% tainted inputs |
| 4. Dormancy | Waiting then moving | > 7 days idle |
| 5. Clean Zone | Sending to exchange | Registered clean zone |

## Approval Process

### Phase 1: Admin (Human Judgment)
```
✓ Is the theft claim legitimate?
✓ Does evidence support it?
✓ Is victim identity verified?

✗ NOT checking if reversal is possible
✗ NOT checking balances
✗ NOT checking taint scores
```

### Phase 2: Protocol (Mathematical)
```
✓ Is taint score ≥ 10%?
✓ Does holder have balance?
✓ Is system pool funded?
✓ Within time window?
✓ Is proof valid?

→ If ALL yes: Execute automatically
→ If ANY no: Reject automatically
```

## Common Scenarios

### Scenario 1: Simple Theft
```
Stolen: 100 GXC
Thief keeps it: 100 GXC (τ=1.0)
Recovery: 100 GXC (100%)
```

### Scenario 2: Split to 2 Wallets
```
Stolen: 100 GXC
Split: 60 GXC (τ=0.6) + 40 GXC (τ=0.4)
Recovery: 36 GXC + 16 GXC = 52 GXC (52%)
```

### Scenario 3: Mixed with Clean Funds
```
Stolen: 20 GXC (τ=1.0)
Mixed with: 80 GXC clean
Result: 100 GXC (τ=0.2)
Recovery: 20 GXC (20%)
```

### Scenario 4: Below Threshold
```
Stolen: 5 GXC (τ=1.0)
Mixed with: 95 GXC clean
Result: 100 GXC (τ=0.05)
Recovery: 0 GXC (τ < 0.1)
```

### Scenario 5: Funds Already Moved
```
Stolen: 100 GXC
Moved to Wallet A: 100 GXC (τ=1.0)
Wallet A sends to Wallet B: 100 GXC (τ=1.0)
Wallet A balance: 0 GXC

Recovery from A: 0 GXC (no balance)
Recovery from B: 100 GXC (100%)
```

## API Quick Reference

### Submit Fraud Report
```bash
POST /api/fraud/report
{
  "txHash": "stolen_tx_hash",
  "reporterAddress": "victim_address",
  "amount": 100.0,
  "evidence": "proof of theft"
}
```

### Check Taint Score
```bash
GET /api/fraud/taint/{txHash}

Response:
{
  "taintScore": 0.5,
  "sourceTransaction": "original_theft_tx",
  "tracePath": ["tx1", "tx2", "tx3"]
}
```

### Get Fraud Alerts
```bash
GET /api/fraud/alerts?level=HIGH

Response:
{
  "alerts": [
    {
      "transactionHash": "abc123",
      "level": "HIGH",
      "rule": "FAN_OUT_PATTERN",
      "taintScore": 0.65
    }
  ]
}
```

### Trace Tainted Flow
```bash
GET /api/fraud/trace/{txHash}

Response:
{
  "paths": [
    {
      "transactions": ["tx1", "tx2", "tx3"],
      "addresses": ["addr1", "addr2", "addr3"],
      "taintScores": [1.0, 0.5, 0.25],
      "finalTaint": 0.25
    }
  ]
}
```

## Decision Tree

```
Is transaction marked as stolen?
├─ NO → No action
└─ YES → Propagate taint
         │
         ├─ Taint score ≥ 0.1?
         │  ├─ NO → Stop propagation
         │  └─ YES → Continue tracing
         │           │
         │           ├─ Check fraud rules
         │           │  └─ Generate alerts
         │           │
         │           └─ Admin approves FACTS?
         │              ├─ NO → Stop
         │              └─ YES → Protocol validates
         │                       │
         │                       ├─ All checks pass?
         │                       │  ├─ NO → Reject (INFEASIBLE)
         │                       │  └─ YES → Execute (EXECUTED)
         │                       │
         │                       └─ Recover funds
```

## Limitations

### Cannot Recover If:
1. **Taint < 10%** - Majority of funds are clean
2. **No Balance** - Funds already moved
3. **Too Old** - > 30 days since theft
4. **Pool Empty** - System pool underfunded (rare)
5. **Invalid Proof** - Cryptographic verification fails

### Can Recover If:
1. **Taint ≥ 10%** - Sufficient evidence
2. **Has Balance** - Funds still in wallet
3. **Within Window** - ≤ 30 days
4. **Pool Funded** - System has fees
5. **Valid Proof** - Cryptography checks out

## Best Practices

### For Victims
1. **Report immediately** - Don't wait
2. **Provide evidence** - Transaction logs, screenshots
3. **Be patient** - Admin review takes time
4. **Understand limits** - May not recover 100%

### For Admins
1. **Review evidence carefully** - Only approve legitimate claims
2. **Don't worry about feasibility** - Protocol handles that
3. **Document reasoning** - Add notes to approval
4. **Trust the protocol** - It validates independently

### For Developers
1. **Don't bypass protocol** - Never skip validation
2. **Trust the math** - Taint formula is proven
3. **Log everything** - Audit trail is critical
4. **Test edge cases** - Mixing, splitting, etc.

## Troubleshooting

### "Why wasn't my reversal executed?"

Check:
1. **Taint score** - Is it ≥ 10%?
2. **Balance** - Does holder still have funds?
3. **Time** - Is it within 30 days?
4. **Admin approval** - Did admin approve FACTS?
5. **Protocol logs** - What did validation say?

### "Why only 30% recovery?"

Reasons:
1. **Funds split** - Some wallets below threshold
2. **Mixed with clean** - Diluted taint
3. **Already moved** - Some wallets empty
4. **Multiple hops** - Taint decreases over distance

### "Can I increase recovery rate?"

No, because:
1. **Math is fixed** - Taint formula is conservation law
2. **Threshold protects innocents** - Can't lower below 10%
3. **Time window is safety** - Can't extend indefinitely
4. **Protocol is autonomous** - No manual overrides

## Summary

**Fraud detection is:**
- ✓ Automatic (no manual tracing)
- ✓ Mathematical (based on conservation)
- ✓ Fair (protects innocent parties)
- ✓ Transparent (all traces auditable)
- ✓ Efficient (handles complex patterns)

**Key insight:** The system traces funds through ANY number of wallets automatically. You don't need to manually track each one. Just mark the original theft, and the system does the rest.

**Recovery rate depends on:**
1. How funds were split
2. Whether mixed with clean funds
3. How quickly reported
4. Whether funds still in wallets

**Typical recovery rates:**
- Simple theft (no split): 90-100%
- Split to 2-3 wallets: 50-80%
- Split to 9+ wallets: 20-40%
- Mixed with clean funds: 10-30%
- Multiple hops + mixing: 5-15%

The system maximizes recovery while protecting innocent parties. It's a balance between justice and fairness.
