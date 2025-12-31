# Third-Party Wallet Integration Guide

## Overview

Third-party wallets can integrate GXC's fraud detection system to protect their users. The system provides public API endpoints that require **no authentication** for checking transaction safety.

## How the System Tracks Original Victim

### Automatic Victim Address Detection

**Q: How does the system know which address to reverse funds to?**

**A: The system automatically extracts the victim address from the stolen transaction.**

```cpp
// From ProofGenerator.cpp line 169
std::string origin_owner = stolen_tx_obj->getSenderAddress();
```

**Flow:**
```
1. Victim reports theft with stolen TX hash
2. System retrieves the stolen transaction
3. Extracts sender address from that transaction
4. That sender = original victim (origin_owner)
5. Reversal sends funds back to origin_owner
```

**Example:**
```
Stolen Transaction:
  Hash: abc123...
  From: victim_address_xyz  ← This is automatically extracted
  To: thief_address_123
  Amount: 100 GXC

When reversal executes:
  From: current_holder_address
  To: victim_address_xyz  ← Automatically set from stolen TX
  Amount: recoverable_amount
```

### Why This Works

The blockchain is **immutable** - the stolen transaction permanently records:
- Who sent the funds (victim)
- Who received them (thief)
- When it happened
- How much was stolen

The system simply reads this data from the blockchain. No manual input needed.

## Public API Endpoints (No Auth Required)

### 1. Check Transaction Taint

**Endpoint:** `GET /api/fraud/check-transaction/{txHash}`

**Purpose:** Check if a transaction involves stolen funds

**Use Case:** Before accepting a transaction, check if funds are tainted

**Request:**
```bash
curl http://node.gxc.network:8080/api/fraud/check-transaction/abc123...
```

**Response:**
```json
{
  "txHash": "abc123...",
  "taintScore": 0.65,
  "isStolen": false,
  "riskLevel": "HIGH"
}
```

**Risk Levels:**
- `CRITICAL`: τ ≥ 0.8 (80%+ stolen)
- `HIGH`: τ ≥ 0.5 (50-79% stolen)
- `MEDIUM`: τ ≥ 0.1 (10-49% stolen)
- `LOW`: τ < 0.1 (< 10% stolen)

**Integration Example:**
```javascript
async function checkTransactionSafety(txHash) {
    const response = await fetch(
        `http://node.gxc.network:8080/api/fraud/check-transaction/${txHash}`
    );
    const data = await response.json();
    
    if (data.riskLevel === 'CRITICAL' || data.riskLevel === 'HIGH') {
        // Warn user or block transaction
        alert('Warning: This transaction involves potentially stolen funds!');
        return false;
    }
    
    return true;
}
```

### 2. Check Address Fraud Status

**Endpoint:** `GET /api/fraud/check-address/{address}`

**Purpose:** Check if an address is flagged for fraud

**Use Case:** Before sending funds to an address, check if it's suspicious

**Request:**
```bash
curl http://node.gxc.network:8080/api/fraud/check-address/GXC1234...
```

**Response:**
```json
{
  "address": "GXC1234...",
  "isFlagged": true,
  "alertCount": 3,
  "alerts": [
    {
      "level": "HIGH",
      "description": "Fan-out pattern detected: 8 outputs",
      "taintScore": 0.65,
      "timestamp": 1735660800
    },
    {
      "level": "MEDIUM",
      "description": "Velocity anomaly: Funds moved in 120 seconds",
      "taintScore": 0.65,
      "timestamp": 1735660920
    }
  ]
}
```

**Integration Example:**
```javascript
async function checkAddressSafety(address) {
    const response = await fetch(
        `http://node.gxc.network:8080/api/fraud/check-address/${address}`
    );
    const data = await response.json();
    
    if (data.isFlagged) {
        // Show warning to user
        const alertSummary = data.alerts
            .map(a => `${a.level}: ${a.description}`)
            .join('\n');
        
        alert(`Warning: This address has fraud alerts:\n${alertSummary}`);
        return false;
    }
    
    return true;
}
```

### 3. Report Stolen Funds

**Endpoint:** `POST /api/fraud/report-stolen`

**Purpose:** Allow users to report theft directly from wallet

**Use Case:** User discovers theft, reports it immediately

**Request:**
```bash
curl -X POST http://node.gxc.network:8080/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "abc123...",
    "reporterAddress": "victim_address",
    "amount": 100.0,
    "description": "Unauthorized transaction - my wallet was compromised"
  }'
```

**Response:**
```json
{
  "success": true,
  "txHash": "abc123...",
  "status": "PENDING_REVIEW",
  "message": "Report submitted successfully. We will review within 24 hours.",
  "timestamp": 1735660800
}
```

**Integration Example:**
```javascript
async function reportTheft(txHash, amount, description) {
    const response = await fetch(
        'http://node.gxc.network:8080/api/fraud/report-stolen',
        {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                txHash: txHash,
                reporterAddress: getCurrentUserAddress(),
                amount: amount,
                description: description
            })
        }
    );
    
    const data = await response.json();
    
    if (data.success) {
        alert('Theft reported successfully. Admins will review within 24 hours.');
    }
    
    return data;
}
```

## Wallet Integration Patterns

### Pattern 1: Pre-Transaction Check

**When:** Before sending a transaction

**Purpose:** Warn user if recipient is suspicious

```javascript
class GXCWallet {
    async sendTransaction(to, amount) {
        // Check recipient address
        const addressCheck = await this.checkAddressSafety(to);
        
        if (!addressCheck.safe) {
            const confirm = await this.showWarning(
                'Warning: Recipient address has fraud alerts. Continue?',
                addressCheck.alerts
            );
            
            if (!confirm) {
                return { cancelled: true };
            }
        }
        
        // Proceed with transaction
        return await this.executeTransaction(to, amount);
    }
    
    async checkAddressSafety(address) {
        const response = await fetch(
            `${this.nodeUrl}/api/fraud/check-address/${address}`
        );
        const data = await response.json();
        
        return {
            safe: !data.isFlagged,
            alerts: data.alerts
        };
    }
}
```

### Pattern 2: Post-Transaction Monitoring

**When:** After receiving a transaction

**Purpose:** Alert user if received funds are tainted

```javascript
class GXCWallet {
    async onTransactionReceived(txHash) {
        // Check transaction taint
        const taintCheck = await this.checkTransactionTaint(txHash);
        
        if (taintCheck.riskLevel === 'HIGH' || taintCheck.riskLevel === 'CRITICAL') {
            this.showAlert(
                'Warning: Received transaction may involve stolen funds',
                `Taint Score: ${taintCheck.taintScore}\n` +
                `Risk Level: ${taintCheck.riskLevel}\n\n` +
                'These funds may be subject to reversal if theft is confirmed.'
            );
        }
    }
    
    async checkTransactionTaint(txHash) {
        const response = await fetch(
            `${this.nodeUrl}/api/fraud/check-transaction/${txHash}`
        );
        return await response.json();
    }
}
```

### Pattern 3: Theft Detection & Reporting

**When:** User discovers unauthorized transaction

**Purpose:** Quick theft reporting

```javascript
class GXCWallet {
    async detectUnauthorizedTransactions() {
        const recentTxs = await this.getRecentTransactions();
        
        for (const tx of recentTxs) {
            if (tx.from === this.address && !this.wasAuthorized(tx)) {
                // Unauthorized transaction detected
                const report = await this.promptTheftReport(tx);
                
                if (report) {
                    await this.reportTheft(tx.hash, tx.amount, report.description);
                }
            }
        }
    }
    
    async reportTheft(txHash, amount, description) {
        const response = await fetch(
            `${this.nodeUrl}/api/fraud/report-stolen`,
            {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    txHash: txHash,
                    reporterAddress: this.address,
                    amount: amount,
                    description: description
                })
            }
        );
        
        const data = await response.json();
        
        if (data.success) {
            this.showNotification(
                'Theft Reported',
                'Your report has been submitted. Admins will review within 24 hours.'
            );
        }
        
        return data;
    }
}
```

### Pattern 4: Real-Time Transaction Validation

**When:** Every transaction (send/receive)

**Purpose:** Continuous fraud protection

```javascript
class GXCWallet {
    constructor(nodeUrl) {
        this.nodeUrl = nodeUrl;
        this.fraudCheckEnabled = true;
    }
    
    async validateTransaction(tx) {
        if (!this.fraudCheckEnabled) {
            return { valid: true };
        }
        
        const checks = await Promise.all([
            this.checkTransactionTaint(tx.hash),
            this.checkAddressSafety(tx.to),
            this.checkAddressSafety(tx.from)
        ]);
        
        const [taintCheck, toCheck, fromCheck] = checks;
        
        const warnings = [];
        
        if (taintCheck.riskLevel === 'HIGH' || taintCheck.riskLevel === 'CRITICAL') {
            warnings.push({
                type: 'TAINTED_FUNDS',
                severity: taintCheck.riskLevel,
                message: `Transaction involves ${taintCheck.taintScore * 100}% stolen funds`
            });
        }
        
        if (toCheck.isFlagged) {
            warnings.push({
                type: 'FLAGGED_RECIPIENT',
                severity: 'HIGH',
                message: `Recipient has ${toCheck.alertCount} fraud alerts`
            });
        }
        
        if (fromCheck.isFlagged) {
            warnings.push({
                type: 'FLAGGED_SENDER',
                severity: 'HIGH',
                message: `Sender has ${fromCheck.alertCount} fraud alerts`
            });
        }
        
        return {
            valid: warnings.length === 0,
            warnings: warnings
        };
    }
}
```

## Complete Wallet Integration Example

```javascript
class GXCWalletWithFraudProtection {
    constructor(nodeUrl, privateKey) {
        this.nodeUrl = nodeUrl;
        this.privateKey = privateKey;
        this.address = this.deriveAddress(privateKey);
    }
    
    // Send transaction with fraud checks
    async send(to, amount) {
        console.log(`Sending ${amount} GXC to ${to}`);
        
        // Step 1: Check recipient address
        console.log('Checking recipient address...');
        const addressCheck = await this.checkAddress(to);
        
        if (addressCheck.isFlagged) {
            const confirm = confirm(
                `Warning: Recipient has ${addressCheck.alertCount} fraud alerts.\n` +
                'Continue anyway?'
            );
            
            if (!confirm) {
                return { cancelled: true, reason: 'User cancelled due to fraud warning' };
            }
        }
        
        // Step 2: Create and sign transaction
        const tx = await this.createTransaction(to, amount);
        
        // Step 3: Broadcast transaction
        const result = await this.broadcastTransaction(tx);
        
        console.log(`Transaction sent: ${result.txHash}`);
        return result;
    }
    
    // Receive transaction with fraud checks
    async onReceive(txHash) {
        console.log(`Received transaction: ${txHash}`);
        
        // Check transaction taint
        const taintCheck = await this.checkTransaction(txHash);
        
        if (taintCheck.riskLevel !== 'LOW') {
            this.showWarning(
                'Received Potentially Tainted Funds',
                `Risk Level: ${taintCheck.riskLevel}\n` +
                `Taint Score: ${(taintCheck.taintScore * 100).toFixed(1)}%\n\n` +
                'These funds may be subject to reversal if theft is confirmed.'
            );
        }
    }
    
    // Report theft
    async reportTheft(txHash, description) {
        const tx = await this.getTransaction(txHash);
        
        const response = await fetch(
            `${this.nodeUrl}/api/fraud/report-stolen`,
            {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    txHash: txHash,
                    reporterAddress: this.address,
                    amount: tx.amount,
                    description: description
                })
            }
        );
        
        return await response.json();
    }
    
    // Helper: Check transaction
    async checkTransaction(txHash) {
        const response = await fetch(
            `${this.nodeUrl}/api/fraud/check-transaction/${txHash}`
        );
        return await response.json();
    }
    
    // Helper: Check address
    async checkAddress(address) {
        const response = await fetch(
            `${this.nodeUrl}/api/fraud/check-address/${address}`
        );
        return await response.json();
    }
}

// Usage
const wallet = new GXCWalletWithFraudProtection(
    'http://node.gxc.network:8080',
    'user_private_key'
);

// Send with fraud protection
await wallet.send('recipient_address', 10.0);

// Report theft
await wallet.reportTheft('stolen_tx_hash', 'My wallet was hacked');
```

## UI/UX Recommendations

### 1. Transaction Warning Dialog

```
┌─────────────────────────────────────────────┐
│  ⚠️  Fraud Warning                          │
├─────────────────────────────────────────────┤
│                                             │
│  This address has fraud alerts:             │
│                                             │
│  • HIGH: Fan-out pattern detected          │
│  • MEDIUM: Velocity anomaly                │
│                                             │
│  Taint Score: 65%                          │
│                                             │
│  Funds sent to this address may be         │
│  associated with theft.                    │
│                                             │
│  [ Cancel ]  [ Continue Anyway ]           │
└─────────────────────────────────────────────┘
```

### 2. Received Funds Alert

```
┌─────────────────────────────────────────────┐
│  ⚠️  Tainted Funds Received                 │
├─────────────────────────────────────────────┤
│                                             │
│  You received 10 GXC                       │
│  Risk Level: HIGH                          │
│  Taint Score: 65%                          │
│                                             │
│  These funds may be subject to reversal    │
│  if theft is confirmed by admins.          │
│                                             │
│  [ Learn More ]  [ OK ]                    │
└─────────────────────────────────────────────┘
```

### 3. Theft Report Form

```
┌─────────────────────────────────────────────┐
│  Report Stolen Funds                        │
├─────────────────────────────────────────────┤
│                                             │
│  Transaction Hash:                         │
│  [abc123...]                               │
│                                             │
│  Amount Stolen:                            │
│  [100.0] GXC                               │
│                                             │
│  Description:                              │
│  ┌─────────────────────────────────────┐  │
│  │ My wallet was compromised and       │  │
│  │ funds were stolen without my        │  │
│  │ authorization...                    │  │
│  └─────────────────────────────────────┘  │
│                                             │
│  [ Cancel ]  [ Submit Report ]             │
└─────────────────────────────────────────────┘
```

## Testing Your Integration

### Test Endpoints

```bash
# Test transaction check
curl http://localhost:8080/api/fraud/check-transaction/test_tx_hash

# Test address check
curl http://localhost:8080/api/fraud/check-address/test_address

# Test fraud report
curl -X POST http://localhost:8080/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{"txHash":"test","reporterAddress":"test","amount":1.0,"description":"test"}'
```

### Expected Responses

All endpoints return JSON with appropriate HTTP status codes:
- `200 OK`: Success
- `400 Bad Request`: Invalid input
- `503 Service Unavailable`: Fraud detection not available

## Security Considerations

### 1. No Authentication Required

These endpoints are **public** by design:
- Anyone can check transaction safety
- Anyone can report theft
- No API keys needed
- No rate limiting (currently)

### 2. Privacy

- Only transaction hashes and addresses are exposed
- No personal information required
- Reports are logged but not publicly visible

### 3. False Positives

- Taint scores are probabilistic
- Low taint (< 10%) may be false positive
- Always show warnings, never block automatically
- Let users make final decision

## Summary

### Key Points

1. **No Auth Required**: All fraud check endpoints are public
2. **Automatic Victim Detection**: System extracts victim address from stolen transaction
3. **Real-Time Checks**: Check transactions and addresses before sending
4. **Easy Reporting**: Users can report theft directly from wallet
5. **Risk Levels**: Clear indication of fraud risk (LOW/MEDIUM/HIGH/CRITICAL)

### Integration Checklist

- [ ] Add transaction taint checking before send
- [ ] Add address fraud checking before send
- [ ] Add taint checking on receive
- [ ] Add theft reporting functionality
- [ ] Add warning dialogs for high-risk transactions
- [ ] Add user education about taint scores
- [ ] Test all endpoints with your node
- [ ] Handle API errors gracefully

### Support

For integration support:
- Check API documentation
- Test with local node first
- Monitor logs for errors
- Report issues on GitHub

The fraud detection system is designed to be easy to integrate while providing maximum protection for users.
