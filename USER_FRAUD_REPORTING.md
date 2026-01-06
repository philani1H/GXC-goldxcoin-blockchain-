# User Fraud Reporting: Complete Guide

## Your Question: Must Maingin User Report?

### Answer: **YES - Users (Victims) Must Report Fraud**

The fraud detection system **requires a victim to report** the theft before any action can be taken. Here's why and how:

---

## The Complete User Reporting Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    STEP 1: THEFT OCCURS                         │
│                    (Blockchain Event)                           │
└─────────────────────────────────────────────────────────────────┘
                             │
                             ▼
                    Hacker steals funds
                    Transaction recorded on blockchain
                    Victim loses access to funds
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    STEP 2: VICTIM REPORTS                       │
│                    (User Action - REQUIRED)                     │
└─────────────────────────────────────────────────────────────────┘
                             │
                             ▼
                    Victim submits fraud report:
                    POST /api/fraud/report-stolen
                    {
                        "txHash": "TX_STOLEN",
                        "reporterAddress": "victim_address",
                        "amount": 1000,
                        "email": "victim@email.com",
                        "description": "My wallet was hacked",
                        "evidence": "Screenshots, logs, etc."
                    }
                             │
                             ▼
                    System creates FraudReport
                    Status: PENDING
                    Assigned report ID
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    STEP 3: ADMIN REVIEW                         │
│                    (Admin Action)                               │
└─────────────────────────────────────────────────────────────────┘
                             │
                             ▼
                    Admin receives notification
                    Reviews report details:
                    - Transaction hash
                    - Victim address
                    - Amount claimed
                    - Evidence provided
                             │
                             ▼
                    Admin investigates:
                    - Is victim legitimate?
                    - Is evidence valid?
                    - Was theft real?
                             │
                    ┌────────┴────────┐
                    │                 │
                    ▼                 ▼
            ┌──────────────┐  ┌──────────────┐
            │ Admin        │  │ Admin        │
            │ APPROVES     │  │ REJECTS      │
            └──────┬───────┘  └──────┬───────┘
                   │                 │
                   │                 ▼
                   │          ┌──────────────┐
                   │          │ Report       │
                   │          │ REJECTED     │
                   │          │ No action    │
                   │          └──────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────────┐
│                    STEP 4: ADMIN MARKS AS STOLEN                │
│                    (Triggers Fraud Detection)                   │
└─────────────────────────────────────────────────────────────────┘
                             │
                             ▼
                    Admin approves report
                    System calls:
                    fraudDetection->markAsStolen(txHash)
                             │
                             ▼
                    Fraud detection activates:
                    - Sets taint τ = 1.0
                    - Starts BFS propagation
                    - Applies formula to descendants
                    - Generates alerts
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    STEP 5: AUTOMATIC DETECTION                  │
│                    (System Action)                              │
└─────────────────────────────────────────────────────────────────┘
                             │
                             ▼
                    System tracks stolen funds
                    Generates alerts for patterns
                    Monitors all movements
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    STEP 6: REVERSAL PROCESS                     │
│                    (If Feasible)                                │
└─────────────────────────────────────────────────────────────────┘
                             │
                             ▼
                    Proof generation
                    Protocol validation
                    Reversal execution
                    Funds returned to victim
```

---

## Why User Reporting is Required

### 1. **System Cannot Know Automatically**

The blockchain cannot automatically detect theft because:

❌ **Cannot distinguish legitimate vs stolen:**
```
Transaction A: Alice → Bob (1000 GXC)
Transaction B: Hacker → Bob (1000 GXC)
```
Both look identical on the blockchain!

❌ **Cannot read intent:**
- Was it a gift?
- Was it a payment?
- Was it stolen?
- Was it a mistake?

❌ **Cannot verify ownership:**
- Who is the real owner?
- Was the private key compromised?
- Was it authorized?

### 2. **Legal and Ethical Requirements**

✅ **Victim must claim loss:**
- Proves they are the legitimate owner
- Provides evidence of theft
- Confirms they want reversal

✅ **Prevents false accusations:**
- Anyone could claim any transaction is stolen
- Requires victim to come forward
- Requires evidence

✅ **Protects innocent holders:**
- Current holder might be innocent
- Needs investigation before reversal
- Prevents abuse

### 3. **Technical Requirements**

✅ **Provides starting point:**
- Which transaction was stolen?
- When did theft occur?
- Who is the victim?

✅ **Enables tracing:**
- System needs to know where to start
- BFS propagation starts from stolen TX
- Formula calculates taint from that point

✅ **Triggers detection:**
- `markAsStolen()` activates fraud detection
- Without it, system doesn't know to track
- Automatic detection only works after marking

---

## How Users Report Fraud

### Method 1: REST API (Public Endpoint)

**Endpoint:** `POST /api/fraud/report-stolen`

**Request:**
```json
{
    "txHash": "abc123...",
    "reporterAddress": "GXC_victim_address",
    "amount": 1000.0,
    "email": "victim@example.com",
    "description": "My wallet was hacked on 2025-01-01. Hacker transferred all my funds.",
    "evidence": "Screenshots: [link], Transaction logs: [link], Police report: [link]"
}
```

**Response:**
```json
{
    "success": true,
    "reportId": "REPORT_1704556800_1234",
    "txHash": "abc123...",
    "status": "PENDING_REVIEW",
    "message": "Report submitted successfully. We will review within 24 hours.",
    "timestamp": 1704556800
}
```

**Implementation:**
```cpp
std::string RESTServer::reportStolenFunds(const std::string& body) {
    auto json = nlohmann::json::parse(body);
    
    std::string txHash = json["txHash"];
    std::string reporterAddress = json["reporterAddress"];
    double amount = json["amount"];
    std::string description = json["description"];
    
    // Log security event
    LOG_SECURITY(LogLevel::CRITICAL, 
        "Fraud report received for TX: " + txHash,
        "FraudDetection");
    
    // Create response
    return {
        "success": true,
        "status": "PENDING_REVIEW",
        "message": "Report submitted. Admin will review."
    };
}
```

### Method 2: Admin System (Internal)

**For admins to submit on behalf of users:**

```cpp
std::string reportId = marketMakerAdmin->submitFraudReport(
    "TX_STOLEN",           // txHash
    "victim_address",      // reporterAddress
    1000.0,                // amount
    "victim@email.com",    // email
    "Wallet hacked",       // description
    "Evidence links"       // evidence
);
```

---

## What Happens After User Reports

### Stage 1: Report Submission (Immediate)

**User submits report:**
```cpp
FraudReport {
    reportId: "REPORT_001",
    txHash: "TX_STOLEN",
    reporterAddress: "victim_address",
    amount: 1000.0,
    email: "victim@email.com",
    description: "Wallet hacked",
    evidence: "Screenshots, logs",
    timestamp: 1704556800,
    
    // Admin review status
    factsStatus: "PENDING",
    reviewedBy: "",
    reviewedAt: 0,
    
    // Protocol execution status
    executionStatus: "NOT_STARTED",
    proofHash: "",
    recoveredAmount: 0
}
```

**Status:** ⏳ Waiting for admin review

---

### Stage 2: Admin Review (24-48 hours)

**Admin views pending reports:**
```cpp
auto reports = admin->getPendingFraudReports(sessionToken);
// Shows all PENDING reports
```

**Admin investigates:**
- Verifies victim identity
- Checks transaction on blockchain
- Reviews evidence
- Confirms theft occurred

**Admin decision:**
```cpp
// If legitimate:
admin->approveFraudReport(sessionToken, reportId, "Confirmed theft");

// If not legitimate:
admin->rejectFraudReport(sessionToken, reportId, "Insufficient evidence");
```

**Status:** ✅ Approved OR ❌ Rejected

---

### Stage 3: Fraud Detection Activation (Automatic)

**If admin approves, system automatically:**

```cpp
// 1. Mark transaction as stolen
fraudDetection->markAsStolen(report.txHash);

// This triggers:
// - Taint initialization (τ = 1.0)
// - BFS propagation
// - Formula application
// - Alert generation
```

**Status:** 🔍 Tracking stolen funds

---

### Stage 4: Proof Generation (Automatic)

**System generates proof:**
```cpp
ProofOfFeasibility proof = proofGenerator->generateProof(
    report.txHash,
    current_holder,
    admin_id,
    admin_signature
);
```

**Status:** 📋 Proof created

---

### Stage 5: Protocol Validation (Automatic)

**Protocol checks feasibility:**
```cpp
bool feasible = validateFeasibility(proof);
// Checks: taint score, balance, time window, etc.
```

**Status:** ✅ Feasible OR ❌ Infeasible

---

### Stage 6: Reversal Execution (Automatic)

**If feasible, system executes:**
```cpp
bool success = executeReversal(proof);
// Debits holder, credits victim, pays fee
```

**Status:** 🎉 Funds returned!

---

## Real Example: User Reports Theft

### Scenario

**Day 1 - Theft:**
```
Alice has 1000 GXC in wallet
Hacker steals private key
Hacker transfers 1000 GXC to his address
Alice discovers theft
```

**Day 1 - Report:**
```
Alice submits fraud report:
POST /api/fraud/report-stolen
{
    "txHash": "TX_STOLEN_123",
    "reporterAddress": "alice_address",
    "amount": 1000,
    "email": "alice@example.com",
    "description": "My wallet was hacked. I lost access to my private key.",
    "evidence": "Screenshots of wallet before/after, transaction logs"
}

Response:
{
    "reportId": "REPORT_001",
    "status": "PENDING_REVIEW",
    "message": "Report submitted. We will review within 24 hours."
}
```

**Day 2 - Admin Review:**
```
Admin logs in
Views pending reports
Sees Alice's report
Investigates:
- Checks TX_STOLEN_123 on blockchain ✓
- Verifies Alice owns alice_address ✓
- Reviews evidence ✓
- Confirms theft ✓

Admin approves:
approveFraudReport("REPORT_001", "Confirmed theft")
```

**Day 2 - Automatic Detection:**
```
System marks TX_STOLEN_123 as stolen
Fraud detection activates:
- Sets taint τ = 1.0
- Starts BFS propagation
- Tracks all descendants
- Generates alerts

Hacker moves funds:
TX1: Hacker → Bob (800 GXC)
System detects: τ(TX1) = 0.8

TX2: Bob → Exchange (800 GXC)
System alerts: CRITICAL - Clean zone entry!
```

**Day 2 - Automatic Reversal:**
```
System generates proof:
- Stolen TX: TX_STOLEN_123
- Current holder: Bob
- Taint score: 0.8
- Recoverable: 640 GXC (0.8 × 800)

Protocol validates:
- Proof valid ✓
- Taint ≥ 0.1 ✓
- Bob has balance ✓
- Within time window ✓

System executes:
- Debit Bob: -640 GXC
- Credit Alice: +640 GXC
- Status: EXECUTED

Alice receives notification:
"Your report has been processed. 640 GXC recovered."
```

---

## User Reporting Requirements

### What Users Must Provide

**Required Information:**
- ✅ Transaction hash (stolen TX)
- ✅ Victim address (reporter)
- ✅ Amount stolen
- ✅ Contact email

**Recommended Information:**
- ✅ Detailed description
- ✅ Evidence (screenshots, logs)
- ✅ Timeline of events
- ✅ Police report (if filed)

### What Makes a Valid Report

**Valid Report:**
- ✅ Transaction exists on blockchain
- ✅ Reporter is the victim (owns address)
- ✅ Evidence supports theft claim
- ✅ Amount matches transaction
- ✅ Reported within reasonable time

**Invalid Report:**
- ❌ Transaction doesn't exist
- ❌ Reporter is not the victim
- ❌ No evidence provided
- ❌ Amount doesn't match
- ❌ Too old (> 30 days)

---

## Can System Detect Without User Report?

### NO - Here's Why

**Scenario 1: Legitimate Transfer**
```
Alice → Bob: 1000 GXC
```
Is this:
- A gift? ✓
- A payment? ✓
- Stolen? ✓
- A mistake? ✓

**System cannot know without Alice reporting!**

**Scenario 2: Hacker Transfer**
```
Hacker → Exchange: 1000 GXC
```
Is this:
- Hacker laundering? ✓
- Legitimate user? ✓
- Exchange deposit? ✓

**System cannot know without victim reporting!**

**Scenario 3: Multiple Hops**
```
TX1 → TX2 → TX3 → TX4
```
Which one was stolen?
- TX1? Maybe
- TX2? Maybe
- TX3? Maybe
- TX4? Maybe

**System cannot know without starting point!**

---

## What If User Doesn't Report?

### Consequences

**No Report = No Action:**
- ❌ System doesn't know theft occurred
- ❌ Fraud detection not activated
- ❌ No taint propagation
- ❌ No alerts generated
- ❌ No reversal possible
- ❌ Funds remain with thief

**Why:**
- System respects privacy
- Cannot assume intent
- Requires victim to claim loss
- Prevents false accusations

**Exception:**
- If admin discovers theft independently
- Admin can mark as stolen directly
- But still requires investigation

---

## Summary

### Question: Must Maingin User Report?

**Answer: YES - User (Victim) MUST Report**

**The Process:**

1. **User Reports** (Required)
   - Submits fraud report via API
   - Provides transaction hash, evidence
   - Status: PENDING

2. **Admin Reviews** (Required)
   - Investigates claim
   - Verifies evidence
   - Approves or rejects

3. **System Activates** (Automatic)
   - Marks TX as stolen
   - Starts fraud detection
   - Tracks stolen funds

4. **Proof Generated** (Automatic)
   - Uses fraud detection data
   - Creates Proof of Feasibility
   - Validates constraints

5. **Reversal Executes** (Automatic)
   - If feasible
   - Returns funds to victim
   - Victim receives notification

**Key Points:**

✅ **User reporting is REQUIRED**
- System cannot detect theft automatically
- Victim must come forward
- Evidence must be provided

✅ **Admin approval is REQUIRED**
- Prevents false accusations
- Verifies legitimacy
- Protects innocent parties

✅ **After approval, everything is AUTOMATIC**
- Fraud detection activates
- Proof generation
- Protocol validation
- Reversal execution

**Without user report, there is NO fraud detection!**

The fraud detection algorithms only work AFTER a transaction is marked as stolen, which only happens AFTER a user reports and admin approves.

---

## How to Report (Step-by-Step)

### For Victims

**Step 1: Gather Information**
- Transaction hash of theft
- Your wallet address
- Amount stolen
- Evidence (screenshots, logs)

**Step 2: Submit Report**
```bash
curl -X POST https://gxc-blockchain.com/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "your_stolen_tx_hash",
    "reporterAddress": "your_wallet_address",
    "amount": 1000.0,
    "email": "your@email.com",
    "description": "Detailed description of theft",
    "evidence": "Links to evidence"
  }'
```

**Step 3: Wait for Review**
- Admin will review within 24-48 hours
- You'll receive email notification
- Check status via report ID

**Step 4: Track Progress**
```bash
curl https://gxc-blockchain.com/api/fraud/report-status/REPORT_001
```

**Step 5: Receive Funds**
- If approved and feasible
- Funds automatically returned
- Email notification sent

---

**Remember: Without your report, the system cannot help you!**

---

## Roles and Responsibilities

### User (Victim) Role

**Responsibilities:**
- ✅ Report theft immediately
- ✅ Provide accurate information
- ✅ Submit evidence
- ✅ Respond to admin inquiries
- ✅ Verify recovered funds

**Permissions:**
- ✅ Submit fraud reports (public API)
- ✅ Check transaction taint status
- ✅ Check address fraud status
- ✅ Track report status
- ❌ Cannot mark TX as stolen directly
- ❌ Cannot approve reversals
- ❌ Cannot access admin functions

**API Access:**
```
Public Endpoints (No Auth Required):
- POST /api/fraud/report-stolen
- GET /api/fraud/check-transaction/{txHash}
- GET /api/fraud/check-address/{address}
- GET /api/fraud/report-status/{reportId}
```

---

### Admin Role

**Responsibilities:**
- ✅ Review fraud reports
- ✅ Investigate claims
- ✅ Verify evidence
- ✅ Approve/reject reports
- ✅ Mark transactions as stolen
- ✅ Monitor fraud detection system

**Permissions:**
- ✅ View all fraud reports
- ✅ Approve/reject reports
- ✅ Mark TX as stolen (triggers detection)
- ✅ Assign reports to reviewers
- ✅ View fraud detection alerts
- ✅ Access admin dashboard
- ❌ Cannot execute reversals directly
- ❌ Cannot override protocol validation

**API Access:**
```
Admin Endpoints (Auth Required):
- GET /api/admin/fraud/reports/pending
- GET /api/admin/fraud/report/{reportId}
- POST /api/admin/fraud/report/{reportId}/approve
- POST /api/admin/fraud/report/{reportId}/reject
- POST /api/admin/fraud/mark-stolen
- GET /api/admin/fraud/alerts
```

**Admin Types:**

| Role | Permissions | Can Approve Reports? | Can Mark Stolen? |
|------|-------------|---------------------|------------------|
| `fraud_reviewer` | View reports, investigate | ❌ No | ❌ No |
| `fraud_approver` | Review + approve reports | ✅ Yes | ✅ Yes (via approval) |
| `fraud_admin` | All fraud functions | ✅ Yes | ✅ Yes |
| `super_admin` | All system functions | ✅ Yes | ✅ Yes |

---

### Protocol Role (Automatic)

**Responsibilities:**
- ✅ Validate Proof of Feasibility
- ✅ Check mathematical constraints
- ✅ Execute reversals if feasible
- ✅ Reject reversals if infeasible
- ✅ Maintain system integrity

**Permissions:**
- ✅ Validate proofs independently
- ✅ Execute reversals automatically
- ✅ Reject reversals (even if admin approved)
- ✅ Enforce mathematical rules
- ❌ Cannot override math constraints

**Validation Checks:**
```cpp
Protocol validates independently:
1. Proof cryptographically valid?
2. Taint score ≥ 0.1?
3. Holder has sufficient balance?
4. Fee pool has funds?
5. Within time window (30 days)?

If ANY check fails → INFEASIBLE (no reversal)
If ALL checks pass → EXECUTE (automatic reversal)
```

---

## The Three-Layer System

### Layer 1: User (Initiates)
```
User reports theft
    ↓
Provides evidence
    ↓
Waits for review
```

**Power:** Can initiate process
**Limitation:** Cannot execute directly

---

### Layer 2: Admin (Validates)
```
Admin reviews report
    ↓
Investigates evidence
    ↓
Approves if legitimate
```

**Power:** Can approve/reject
**Limitation:** Cannot force execution

---

### Layer 3: Protocol (Executes)
```
Protocol validates math
    ↓
Checks feasibility
    ↓
Executes if possible
```

**Power:** Final decision on execution
**Limitation:** Must follow math rules

---

## Why This Three-Layer System?

### Prevents Abuse

**Without User Layer:**
- ❌ Anyone could claim any TX is stolen
- ❌ No victim to verify
- ❌ False accusations

**Without Admin Layer:**
- ❌ Automatic execution without review
- ❌ No human judgment
- ❌ Easy to abuse

**Without Protocol Layer:**
- ❌ Admin could reverse anything
- ❌ No mathematical constraints
- ❌ Centralized control

### Provides Checks and Balances

**User checks Admin:**
- User must report (admin can't act alone)
- User provides evidence
- User verifies recovery

**Admin checks User:**
- Admin verifies legitimacy
- Admin prevents false claims
- Admin investigates evidence

**Protocol checks Admin:**
- Protocol validates math
- Protocol enforces constraints
- Protocol can reject admin approval

---

## Example: All Three Layers Working

### Scenario: Legitimate Theft

**User Layer:**
```
Alice reports: "My 1000 GXC was stolen"
Evidence: Screenshots, transaction logs
Status: PENDING
```

**Admin Layer:**
```
Admin reviews: Evidence is valid ✓
Admin approves: "Confirmed theft"
Status: APPROVED
```

**Protocol Layer:**
```
Protocol validates:
- Taint score: 0.8 ✓
- Balance: 800 GXC ✓
- Time: 2 days ✓
Status: EXECUTED
Result: 640 GXC returned to Alice
```

**Outcome:** ✅ Success (all layers agree)

---

### Scenario: False Claim

**User Layer:**
```
Bob reports: "My 1000 GXC was stolen"
Evidence: None provided
Status: PENDING
```

**Admin Layer:**
```
Admin reviews: No evidence ✗
Admin rejects: "Insufficient proof"
Status: REJECTED
```

**Protocol Layer:**
```
Not reached (admin rejected)
```

**Outcome:** ❌ Rejected (admin caught false claim)

---

### Scenario: Infeasible Reversal

**User Layer:**
```
Carol reports: "My 1000 GXC was stolen"
Evidence: Valid
Status: PENDING
```

**Admin Layer:**
```
Admin reviews: Evidence valid ✓
Admin approves: "Confirmed theft"
Status: APPROVED
```

**Protocol Layer:**
```
Protocol validates:
- Taint score: 0.05 ✗ (too low)
- Balance: 0 GXC ✗ (funds moved)
- Time: 35 days ✗ (too old)
Status: INFEASIBLE
Result: Cannot execute reversal
```

**Outcome:** ❌ Infeasible (protocol caught impossibility)

---

## Summary: Who Does What

| Action | User | Admin | Protocol |
|--------|------|-------|----------|
| Report theft | ✅ Required | ❌ No | ❌ No |
| Provide evidence | ✅ Required | ❌ No | ❌ No |
| Review report | ❌ No | ✅ Required | ❌ No |
| Approve/reject | ❌ No | ✅ Required | ❌ No |
| Mark as stolen | ❌ No | ✅ Yes | ❌ No |
| Activate detection | ❌ No | ✅ Triggers | ✅ Automatic |
| Generate proof | ❌ No | ❌ No | ✅ Automatic |
| Validate feasibility | ❌ No | ❌ No | ✅ Automatic |
| Execute reversal | ❌ No | ❌ No | ✅ Automatic |
| Receive funds | ✅ Yes | ❌ No | ❌ No |

**Key Insight:**
- **User** = Initiator (must report)
- **Admin** = Validator (must approve)
- **Protocol** = Executor (must validate math)

**All three are required for successful reversal!**
