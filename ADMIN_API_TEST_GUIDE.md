# Admin API Test Guide

## Important Notes

### REST Server Implementation Status

⚠️ **The REST server in the current implementation is a mock/placeholder.**

The `RESTServer` class has:
- ✅ All API endpoint routing logic implemented
- ✅ All handler methods implemented
- ✅ JSON request/response handling
- ❌ **No actual HTTP server listening on a socket**

The `serverLoop()` method is a placeholder that just sleeps - it doesn't bind to a port or accept HTTP connections.

### To Make REST API Functional

You need to integrate an HTTP server library. Options:

1. **cpp-httplib** (Recommended - header-only)
   ```cpp
   #include "httplib.h"
   
   void RESTServer::serverLoop() {
       httplib::Server svr;
       
       svr.Post("/api/admin/login", [this](const httplib::Request& req, httplib::Response& res) {
           res.set_content(this->adminLogin(req.body), "application/json");
       });
       
       // Add all other routes...
       
       svr.listen("0.0.0.0", serverPort);
   }
   ```

2. **Boost.Beast** (More complex but powerful)
3. **Pistache** (Modern C++ REST framework)
4. **Crow** (Flask-like C++ framework)

---

## Fraud Report Submission Requirements

### ⚠️ CRITICAL: Only Wallet Owner Can Report

**Security Rule**: Only the owner of the wallet that was stolen from can submit a fraud report.

### Required Fields for Fraud Report

When submitting a fraud report, the user MUST provide:

#### 1. **Transaction Hash** (Required)
- The hash of the stolen transaction
- Must be a valid transaction on the blockchain
- Example: `"txHash": "abc123def456..."`

#### 2. **Reporter Address** (Required)
- The wallet address of the person reporting
- **MUST be the sender or receiver of the stolen transaction**
- Used to verify ownership
- Example: `"reporterAddress": "GXC1abc123..."`

#### 3. **Amount** (Required)
- The amount of GXC stolen
- Must match the transaction amount
- Example: `"amount": 1000.50`

#### 4. **Description** (Required)
- Detailed explanation of what happened
- Why they believe it was stolen
- How the theft occurred
- Example: `"description": "My wallet was hacked. I did not authorize this transaction. The hacker gained access to my private key through a phishing email."`

#### 5. **Evidence** (Optional but Recommended)
- Supporting evidence
- Screenshots, emails, chat logs, etc.
- Police report number if filed
- Example: `"evidence": "Police report #12345, Screenshot of phishing email attached"`

#### 6. **Email** (Optional)
- Contact email for follow-up
- Example: `"email": "victim@example.com"`

### Verification Process

When a fraud report is submitted:

1. **Ownership Verification**
   - System checks if `reporterAddress` is involved in the transaction
   - Must be either sender or receiver
   - If not, report is rejected immediately

2. **Transaction Validation**
   - Verify transaction exists on blockchain
   - Verify amount matches
   - Check transaction status

3. **Admin Review**
   - Fraud reviewer examines the description
   - Checks evidence provided
   - Verifies the claim is legitimate

4. **Approval/Rejection**
   - If approved: Transaction marked as stolen, taint propagation begins
   - If rejected: Reporter notified with reason

---

## API Endpoint Specifications

### 1. Submit Fraud Report

**Endpoint**: `POST /api/fraud/report-stolen`

**Request Body**:
```json
{
  "txHash": "abc123def456789...",
  "reporterAddress": "GXC1abc123def456...",
  "amount": 1000.50,
  "description": "My wallet was hacked. I did not authorize this transaction. The hacker gained access through a phishing email on Dec 30, 2025. I have filed a police report (#12345) and have screenshots of the phishing email.",
  "evidence": "Police report #12345, Phishing email screenshot, Transaction notification showing unauthorized access",
  "email": "victim@example.com"
}
```

**Success Response**:
```json
{
  "success": true,
  "txHash": "abc123def456789...",
  "status": "PENDING_REVIEW",
  "message": "Report submitted successfully. We will review within 24 hours.",
  "timestamp": 1735689186
}
```

**Error Response (Not Owner)**:
```json
{
  "success": false,
  "error": "UNAUTHORIZED",
  "message": "You are not the owner of this transaction. Only the sender or receiver can report it as stolen."
}
```

**Error Response (Missing Fields)**:
```json
{
  "success": false,
  "error": "MISSING_FIELDS",
  "message": "txHash, reporterAddress, amount, and description are required"
}
```

---

### 2. Admin Login

**Endpoint**: `POST /api/admin/login`

**Request Body**:
```json
{
  "username": "admin",
  "password": "admin123"
}
```

**Success Response**:
```json
{
  "success": true,
  "sessionToken": "SESSION_abc123def456...",
  "adminId": "ADMIN_001",
  "username": "admin",
  "role": "super_admin",
  "permissions": ["*"],
  "expiresAt": 1735775586
}
```

---

### 3. List Fraud Reports (Admin Only)

**Endpoint**: `GET /api/admin/fraud/reports`

**Headers**:
```
Authorization: Bearer SESSION_abc123def456...
```

**Query Parameters**:
- `status` (optional): Filter by status (PENDING_REVIEW, APPROVED, REJECTED)
- `page` (optional): Page number (default: 1)
- `limit` (optional): Results per page (default: 20)

**Success Response**:
```json
{
  "success": true,
  "reports": [
    {
      "reportId": "REPORT_001",
      "txHash": "abc123def456789...",
      "reporterAddress": "GXC1abc123def456...",
      "amount": 1000.50,
      "description": "My wallet was hacked...",
      "evidence": "Police report #12345...",
      "email": "victim@example.com",
      "status": "PENDING_REVIEW",
      "submittedAt": 1735689186,
      "reviewedBy": null,
      "reviewedAt": null
    }
  ],
  "total": 1,
  "page": 1,
  "limit": 20
}
```

---

### 4. Review Fraud Report (fraud_reviewer role)

**Endpoint**: `POST /api/admin/fraud/review/:reportId`

**Headers**:
```
Authorization: Bearer SESSION_abc123def456...
```

**Request Body**:
```json
{
  "notes": "Reviewed evidence. Transaction appears to be unauthorized. Phishing email confirmed. Police report verified. Recommend approval.",
  "recommendation": "APPROVE"
}
```

**Success Response**:
```json
{
  "success": true,
  "reportId": "REPORT_001",
  "status": "UNDER_REVIEW",
  "reviewedBy": "fraud_reviewer_001",
  "reviewedAt": 1735689286,
  "notes": "Reviewed evidence..."
}
```

---

### 5. Approve Fraud Report (fraud_approver role)

**Endpoint**: `POST /api/admin/fraud/approve/:reportId`

**Headers**:
```
Authorization: Bearer SESSION_abc123def456...
```

**Request Body**:
```json
{
  "notes": "Approved. Evidence is sufficient. Transaction will be marked as stolen and taint propagation will begin."
}
```

**Success Response**:
```json
{
  "success": true,
  "reportId": "REPORT_001",
  "status": "APPROVED",
  "approvedBy": "fraud_approver_001",
  "approvedAt": 1735689386,
  "txHash": "abc123def456789...",
  "taintScore": 1.0,
  "message": "Report approved. Transaction marked as stolen. Taint propagation initiated."
}
```

**Actions Taken**:
1. Transaction marked as stolen in FraudDetection system
2. Taint score set to 1.0 (100% tainted)
3. Taint propagation begins to descendant transactions
4. Address flagged if suspicious activity detected
5. Alerts generated for affected addresses

---

### 6. Reject Fraud Report (fraud_approver role)

**Endpoint**: `POST /api/admin/fraud/reject/:reportId`

**Headers**:
```
Authorization: Bearer SESSION_abc123def456...
```

**Request Body**:
```json
{
  "reason": "Insufficient evidence. Transaction appears to be authorized. No signs of unauthorized access.",
  "notes": "Reviewed all evidence. Transaction was initiated from user's registered device. No phishing indicators found."
}
```

**Success Response**:
```json
{
  "success": true,
  "reportId": "REPORT_001",
  "status": "REJECTED",
  "rejectedBy": "fraud_approver_001",
  "rejectedAt": 1735689386,
  "reason": "Insufficient evidence...",
  "message": "Report rejected. No action taken."
}
```

---

## Example Fraud Report Workflow

### Scenario: User's Wallet Hacked

#### Step 1: User Discovers Theft
- User logs into wallet
- Sees unauthorized transaction of 5000 GXC
- Transaction hash: `tx_hack_12345`
- User's address: `GXC1victim123`

#### Step 2: User Submits Report

```bash
curl -X POST http://localhost:18080/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "tx_hack_12345",
    "reporterAddress": "GXC1victim123",
    "amount": 5000,
    "description": "My wallet was hacked on Dec 30, 2025 at 3:45 PM. I received a phishing email pretending to be from GXC support asking me to verify my wallet. I clicked the link and entered my credentials. Within minutes, 5000 GXC was transferred out without my authorization. I have filed a police report (#67890) and have screenshots of the phishing email and the unauthorized transaction notification.",
    "evidence": "Police report #67890 filed with City Police Dept. Screenshot of phishing email (attached). Screenshot of transaction notification showing unauthorized access. Email headers showing phishing domain. My registered device logs showing no activity at time of transaction.",
    "email": "victim@example.com"
  }'
```

**Response**:
```json
{
  "success": true,
  "txHash": "tx_hack_12345",
  "status": "PENDING_REVIEW",
  "message": "Report submitted successfully. We will review within 24 hours.",
  "timestamp": 1735689186
}
```

#### Step 3: Admin Reviews Report

**Fraud Reviewer logs in**:
```bash
curl -X POST http://localhost:18080/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "fraud_reviewer_1",
    "password": "secure_password"
  }'
```

**Lists pending reports**:
```bash
curl -X GET http://localhost:18080/api/admin/fraud/reports?status=PENDING_REVIEW \
  -H "Authorization: Bearer SESSION_TOKEN"
```

**Reviews the report**:
```bash
curl -X POST http://localhost:18080/api/admin/fraud/review/REPORT_001 \
  -H "Authorization: Bearer SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "notes": "Reviewed all evidence:\n- Police report verified with City Police Dept\n- Phishing email confirmed as fraudulent (domain: gxc-support-verify.com - not official)\n- Email headers show origin from known phishing server\n- Device logs confirm user was not active at time of transaction\n- Transaction originated from different IP address (Russia) than user'\''s registered location (USA)\n- User has clean history, no prior suspicious activity\n\nRECOMMENDATION: APPROVE\nThis is a clear case of phishing attack and unauthorized access.",
    "recommendation": "APPROVE"
  }'
```

#### Step 4: Fraud Approver Makes Decision

**Fraud Approver logs in**:
```bash
curl -X POST http://localhost:18080/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "fraud_approver_1",
    "password": "secure_password"
  }'
```

**Reviews the case and approves**:
```bash
curl -X POST http://localhost:18080/api/admin/fraud/approve/REPORT_001 \
  -H "Authorization: Bearer SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "notes": "Approved based on reviewer recommendation. Evidence is comprehensive and conclusive. Transaction will be marked as stolen. Taint propagation will begin immediately to track the stolen funds."
  }'
```

**Response**:
```json
{
  "success": true,
  "reportId": "REPORT_001",
  "status": "APPROVED",
  "approvedBy": "fraud_approver_1",
  "approvedAt": 1735689386,
  "txHash": "tx_hack_12345",
  "taintScore": 1.0,
  "message": "Report approved. Transaction marked as stolen. Taint propagation initiated."
}
```

#### Step 5: System Actions

**Automatic actions taken**:
1. ✅ Transaction `tx_hack_12345` marked as stolen
2. ✅ Taint score set to 1.0 (100% tainted)
3. ✅ Taint propagation begins:
   - All descendant transactions receive taint scores
   - Taint decreases with each hop (0.9, 0.81, 0.729...)
4. ✅ Hacker's address `GXC1hacker456` flagged
5. ✅ Alerts generated for exchanges receiving tainted funds
6. ✅ Victim notified via email

---

## Why Detailed Description is Required

### Legal and Compliance Reasons

1. **Evidence for Law Enforcement**
   - Detailed reports help police investigations
   - Can be used in court proceedings
   - Establishes timeline of events

2. **Fraud Pattern Detection**
   - Helps identify common attack vectors
   - Enables proactive security measures
   - Improves fraud detection algorithms

3. **False Report Prevention**
   - Detailed descriptions deter false claims
   - Allows verification of claims
   - Protects innocent parties

4. **Regulatory Compliance**
   - AML/KYC requirements
   - Financial crime reporting obligations
   - Audit trail for regulators

### What Makes a Good Description

#### ✅ Good Description Example:
```
"On December 30, 2025 at 3:45 PM EST, I received an email claiming to be from GXC support (from: support@gxc-verify-wallet.com) asking me to verify my wallet due to 'suspicious activity'. The email looked legitimate with GXC branding. I clicked the link and entered my wallet credentials on what appeared to be the official GXC website. Within 5 minutes, I received a notification that 5000 GXC was transferred to address GXC1hacker456 without my authorization. I immediately checked my wallet and confirmed the unauthorized transaction (hash: tx_hack_12345). I have filed a police report (#67890) with City Police Department. I have screenshots of the phishing email, the fake website, and the transaction notification. My device logs show I was not active on my wallet at the time of the transaction. The transaction originated from an IP address in Russia, while I am located in the USA and have never accessed my wallet from Russia."
```

#### ❌ Bad Description Example:
```
"My coins were stolen. Please help."
```

**Why it's bad**:
- No details about how theft occurred
- No timeline
- No evidence mentioned
- Cannot be verified
- Insufficient for investigation

---

## Market Maker Application Requirements

### Required Documents

When applying to become a market maker, applicants must provide:

1. **License Document**
   - Financial services license
   - Issued by recognized regulatory body (SEC, FCA, MAS, etc.)
   - Must be current and valid

2. **Financial Statements**
   - Last 2 years of audited financial statements
   - Proof of sufficient capital
   - Bank references

3. **Technical Capabilities Document**
   - Trading infrastructure description
   - API integration capabilities
   - Risk management systems
   - Disaster recovery plans

4. **KYC Documents**
   - Company registration documents
   - Beneficial ownership information
   - Director identification
   - Proof of address

### Application Process

1. **Submit Application** with all documents
2. **License Verification** - GXC team verifies with regulatory body
3. **Reputation Check** - Company background check
4. **Financial Review** - Capital adequacy assessment
5. **Technical Verification** - Infrastructure assessment
6. **KYC/AML Compliance** - Identity verification
7. **Final Approval** - Super admin approves
8. **Activation** - Market maker ID issued

---

## Testing Without HTTP Server

Since the REST server is not actually listening, you can test the logic by:

### Option 1: Unit Tests

Create C++ unit tests that directly call the handler methods:

```cpp
#include "RESTServer.h"
#include "FraudDetection.h"
#include "MarketMakerAdmin.h"

void testFraudReportSubmission() {
    Blockchain blockchain;
    FraudDetection fraud(&blockchain);
    MarketMakerAdmin admin;
    RESTServer server(&blockchain, &fraud, &admin, 8080);
    
    std::string request = R"({
        "txHash": "test_tx_123",
        "reporterAddress": "GXC1test123",
        "amount": 1000,
        "description": "Test fraud report with detailed description...",
        "evidence": "Test evidence",
        "email": "test@example.com"
    })";
    
    std::string response = server.reportStolenFunds(request);
    
    // Parse and verify response
    auto json = nlohmann::json::parse(response);
    assert(json["success"] == true);
    assert(json["status"] == "PENDING_REVIEW");
}
```

### Option 2: Python Test Script

Create a Python script that simulates the API calls:

```python
import json

class MockRESTClient:
    def __init__(self):
        self.session_token = None
    
    def login(self, username, password):
        # Simulate login
        print(f"Login: {username}")
        self.session_token = "MOCK_SESSION_TOKEN"
        return {"success": True, "sessionToken": self.session_token}
    
    def submit_fraud_report(self, tx_hash, reporter_address, amount, description, evidence, email):
        # Simulate fraud report submission
        print(f"Submitting fraud report for TX: {tx_hash}")
        print(f"Reporter: {reporter_address}")
        print(f"Amount: {amount}")
        print(f"Description: {description[:100]}...")
        return {"success": True, "status": "PENDING_REVIEW"}
    
    def list_fraud_reports(self):
        # Simulate listing reports
        print("Listing fraud reports...")
        return {"success": True, "reports": []}

# Run tests
client = MockRESTClient()
client.login("admin", "admin123")
client.submit_fraud_report(
    "tx_123",
    "GXC1test",
    1000,
    "Detailed description of theft...",
    "Evidence documents",
    "test@example.com"
)
```

---

## Next Steps

### To Make REST API Functional:

1. **Add HTTP Server Library**
   - Install cpp-httplib: `git clone https://github.com/yhirose/cpp-httplib.git third_party/cpp-httplib`
   - Or use Boost.Beast (already have Boost)

2. **Update RESTServer.cpp**
   - Replace `serverLoop()` with actual HTTP server
   - Bind to port and listen for connections
   - Route requests to handler methods

3. **Rebuild Node**
   ```bash
   cd build
   cmake ..
   make -j$(nproc)
   ```

4. **Test APIs**
   - Start node
   - Use curl to test endpoints
   - Verify responses

---

## Summary

### ✅ What's Implemented
- All API endpoint routing logic
- All handler methods
- JSON request/response handling
- Admin authentication
- Fraud report management
- Market maker application management
- Role-based access control

### ❌ What's Missing
- Actual HTTP server listening on socket
- Need to integrate HTTP library (cpp-httplib recommended)

### 🔒 Security Confirmed
- ✅ Only wallet owner can report fraud
- ✅ Detailed description required
- ✅ Evidence encouraged
- ✅ Multi-level approval workflow
- ✅ NO balance viewing in admin system
- ✅ Fraud detection uses addresses only

---

**The admin system logic is complete and ready. Just needs HTTP server integration to make it accessible via REST API.**
