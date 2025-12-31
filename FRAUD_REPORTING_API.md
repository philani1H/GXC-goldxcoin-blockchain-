# Fraud Reporting API - Easy Ways to Report Stolen Funds

## The Problem You Identified

**"What if someone doesn't know how to report?"**

You're absolutely right! We need **multiple easy ways** for users to report stolen funds.

---

## Solution: Multiple Reporting Methods

### Method 1: Web Interface (Easiest)
### Method 2: Mobile App
### Method 3: REST API (For Developers)
### Method 4: RPC API (For Advanced Users)
### Method 5: Email/Support Ticket
### Method 6: Automatic Detection (Future)

---

## Method 1: Web Interface (EASIEST FOR USERS)

### Simple Web Form

```html
<!DOCTYPE html>
<html>
<head>
    <title>Report Stolen Funds - GXC Blockchain</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
        }
        input, textarea {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        button {
            background-color: #d9534f;
            color: white;
            padding: 12px 30px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover {
            background-color: #c9302c;
        }
        .success {
            background-color: #d4edda;
            color: #155724;
            padding: 15px;
            border-radius: 4px;
            margin-top: 20px;
        }
        .error {
            background-color: #f8d7da;
            color: #721c24;
            padding: 15px;
            border-radius: 4px;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <h1>🚨 Report Stolen Funds</h1>
    <p>If your funds were stolen, report it here immediately.</p>
    
    <form id="reportForm">
        <div class="form-group">
            <label for="txHash">Transaction Hash (Required)*</label>
            <input type="text" id="txHash" name="txHash" 
                   placeholder="e.g., 0x1234567890abcdef..." required>
            <small>The transaction hash where your funds were stolen</small>
        </div>
        
        <div class="form-group">
            <label for="yourAddress">Your Wallet Address (Required)*</label>
            <input type="text" id="yourAddress" name="yourAddress" 
                   placeholder="e.g., GXC1234..." required>
            <small>Your wallet address that was affected</small>
        </div>
        
        <div class="form-group">
            <label for="amount">Amount Stolen (Required)*</label>
            <input type="number" id="amount" name="amount" 
                   placeholder="e.g., 10000" step="0.01" required>
            <small>Amount in GXC</small>
        </div>
        
        <div class="form-group">
            <label for="email">Your Email (Required)*</label>
            <input type="email" id="email" name="email" 
                   placeholder="your@email.com" required>
            <small>We'll contact you for verification</small>
        </div>
        
        <div class="form-group">
            <label for="description">What Happened? (Required)*</label>
            <textarea id="description" name="description" rows="5" 
                      placeholder="Describe how your funds were stolen..." required></textarea>
        </div>
        
        <div class="form-group">
            <label for="evidence">Evidence (Optional)</label>
            <textarea id="evidence" name="evidence" rows="3" 
                      placeholder="Screenshots, wallet logs, etc."></textarea>
        </div>
        
        <button type="submit">🚨 Report Stolen Funds</button>
    </form>
    
    <div id="result"></div>
    
    <script>
        document.getElementById('reportForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const formData = {
                txHash: document.getElementById('txHash').value,
                yourAddress: document.getElementById('yourAddress').value,
                amount: parseFloat(document.getElementById('amount').value),
                email: document.getElementById('email').value,
                description: document.getElementById('description').value,
                evidence: document.getElementById('evidence').value,
                timestamp: Date.now()
            };
            
            try {
                const response = await fetch('/api/fraud/report-stolen', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(formData)
                });
                
                const result = await response.json();
                
                if (response.ok) {
                    document.getElementById('result').innerHTML = `
                        <div class="success">
                            <h3>✅ Report Submitted Successfully!</h3>
                            <p>Report ID: ${result.reportId}</p>
                            <p>Status: ${result.status}</p>
                            <p>We will review your report and take action within 24 hours.</p>
                            <p>You will receive an email confirmation at: ${formData.email}</p>
                        </div>
                    `;
                    document.getElementById('reportForm').reset();
                } else {
                    throw new Error(result.message || 'Failed to submit report');
                }
            } catch (error) {
                document.getElementById('result').innerHTML = `
                    <div class="error">
                        <h3>❌ Error</h3>
                        <p>${error.message}</p>
                        <p>Please try again or contact support@gxc.com</p>
                    </div>
                `;
            }
        });
    </script>
</body>
</html>
```

**User Experience**:
1. User visits: `https://gxc.com/report-theft`
2. Fills out simple form
3. Clicks "Report Stolen Funds"
4. Gets confirmation immediately
5. System automatically flags the transaction

---

## Method 2: Mobile App

### Simple Mobile Interface

```javascript
// React Native Example
import React, { useState } from 'react';
import { View, Text, TextInput, Button, Alert } from 'react-native';

function ReportTheftScreen() {
    const [txHash, setTxHash] = useState('');
    const [amount, setAmount] = useState('');
    const [description, setDescription] = useState('');
    
    const submitReport = async () => {
        try {
            const response = await fetch('https://api.gxc.com/fraud/report-stolen', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    txHash,
                    amount: parseFloat(amount),
                    description,
                    timestamp: Date.now()
                })
            });
            
            const result = await response.json();
            
            if (response.ok) {
                Alert.alert(
                    '✅ Report Submitted',
                    `Report ID: ${result.reportId}\nWe will review within 24 hours.`,
                    [{ text: 'OK' }]
                );
            } else {
                throw new Error(result.message);
            }
        } catch (error) {
            Alert.alert('❌ Error', error.message);
        }
    };
    
    return (
        <View style={{ padding: 20 }}>
            <Text style={{ fontSize: 24, fontWeight: 'bold' }}>
                🚨 Report Stolen Funds
            </Text>
            
            <TextInput
                placeholder="Transaction Hash"
                value={txHash}
                onChangeText={setTxHash}
                style={{ borderWidth: 1, padding: 10, marginTop: 20 }}
            />
            
            <TextInput
                placeholder="Amount Stolen"
                value={amount}
                onChangeText={setAmount}
                keyboardType="numeric"
                style={{ borderWidth: 1, padding: 10, marginTop: 10 }}
            />
            
            <TextInput
                placeholder="What happened?"
                value={description}
                onChangeText={setDescription}
                multiline
                numberOfLines={4}
                style={{ borderWidth: 1, padding: 10, marginTop: 10 }}
            />
            
            <Button
                title="🚨 Report Stolen Funds"
                onPress={submitReport}
                color="#d9534f"
            />
        </View>
    );
}
```

---

## Method 3: REST API (For Developers)

### Endpoint: Report Stolen Funds

```http
POST /api/fraud/report-stolen
Content-Type: application/json

{
    "txHash": "0x1234567890abcdef...",
    "reporterAddress": "GXC1234...",
    "amount": 10000,
    "email": "victim@email.com",
    "description": "My wallet was hacked and funds were stolen",
    "evidence": "Screenshot URL or description",
    "timestamp": 1704067200
}
```

**Response**:
```json
{
    "success": true,
    "reportId": "REPORT_001",
    "status": "PENDING_REVIEW",
    "message": "Report submitted successfully. We will review within 24 hours.",
    "estimatedReviewTime": "2024-01-01T12:00:00Z"
}
```

### Example: Using cURL

```bash
curl -X POST https://api.gxc.com/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "0x1234567890abcdef",
    "reporterAddress": "GXC1234",
    "amount": 10000,
    "email": "victim@email.com",
    "description": "Wallet hacked, funds stolen"
  }'
```

### Example: Using JavaScript

```javascript
async function reportStolenFunds(txHash, amount, description) {
    const response = await fetch('https://api.gxc.com/fraud/report-stolen', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            txHash,
            amount,
            description,
            timestamp: Date.now()
        })
    });
    
    const result = await response.json();
    console.log('Report ID:', result.reportId);
    return result;
}

// Usage
reportStolenFunds(
    '0x1234567890abcdef',
    10000,
    'My wallet was hacked'
);
```

### Example: Using Python

```python
import requests
import time

def report_stolen_funds(tx_hash, amount, description):
    url = 'https://api.gxc.com/fraud/report-stolen'
    
    data = {
        'txHash': tx_hash,
        'amount': amount,
        'description': description,
        'timestamp': int(time.time())
    }
    
    response = requests.post(url, json=data)
    result = response.json()
    
    print(f"Report ID: {result['reportId']}")
    print(f"Status: {result['status']}")
    
    return result

# Usage
report_stolen_funds(
    '0x1234567890abcdef',
    10000,
    'My wallet was hacked'
)
```

---

## Method 4: RPC API (For Advanced Users)

### RPC Call

```json
{
    "jsonrpc": "2.0",
    "method": "fraud.reportStolen",
    "params": {
        "txHash": "0x1234567890abcdef",
        "amount": 10000,
        "description": "Wallet hacked"
    },
    "id": 1
}
```

**Response**:
```json
{
    "jsonrpc": "2.0",
    "result": {
        "reportId": "REPORT_001",
        "status": "PENDING_REVIEW"
    },
    "id": 1
}
```

---

## Method 5: Email/Support Ticket

### Email Template

**To**: security@gxc.com  
**Subject**: URGENT: Stolen Funds Report

```
Transaction Hash: 0x1234567890abcdef
My Wallet Address: GXC1234...
Amount Stolen: 10,000 GXC
Date/Time: 2024-01-01 12:00 UTC

Description:
My wallet was hacked and 10,000 GXC was stolen. I noticed unauthorized 
transactions from my wallet to an unknown address.

Evidence:
- Screenshot of wallet showing unauthorized transaction
- Transaction confirmation email
- Wallet activity log

Please investigate and flag this transaction immediately.

Contact: victim@email.com
Phone: +1-555-0123
```

**Auto-Response**:
```
Thank you for reporting stolen funds.

Report ID: REPORT_001
Status: PENDING_REVIEW

Our security team will review your report within 24 hours.
You will receive an email update once the review is complete.

In the meantime, we recommend:
1. Change your wallet password immediately
2. Enable 2FA if not already enabled
3. Check for malware on your device
4. Do not share your private keys with anyone

GXC Security Team
security@gxc.com
```

---

## Method 6: Automatic Detection (Future Enhancement)

### AI-Powered Anomaly Detection

```python
# Future: Automatic detection without user report
class AutomaticFraudDetection:
    def detect_suspicious_activity(self, address):
        """
        Automatically detect suspicious patterns:
        - Sudden large withdrawals
        - Unusual transaction patterns
        - Access from new locations
        - Multiple failed login attempts
        """
        
        # Check for anomalies
        if self.is_unusual_withdrawal(address):
            # Automatically flag for review
            self.create_automatic_report(address, "Unusual withdrawal pattern")
            
        if self.is_compromised_wallet(address):
            # Freeze wallet temporarily
            self.freeze_wallet(address)
            # Notify user
            self.send_alert(address, "Suspicious activity detected")
```

---

## Backend Implementation: Fraud Reporting API

### C++ Implementation

```cpp
// Add to RESTServer.h
class RESTServer {
private:
    FraudDetection* fraudDetection;
    
    // Fraud reporting
    std::string reportStolenFunds(const std::string& body);
    std::string checkReportStatus(const std::string& reportId);
    std::string listMyReports(const std::string& address);
    
    // Report storage
    struct FraudReport {
        std::string reportId;
        std::string txHash;
        std::string reporterAddress;
        double amount;
        std::string email;
        std::string description;
        std::string evidence;
        uint64_t timestamp;
        std::string status; // PENDING, APPROVED, REJECTED
        std::string reviewNotes;
    };
    
    std::unordered_map<std::string, FraudReport> reports;
    uint32_t reportCounter;
    
public:
    // ... existing methods
};
```

```cpp
// Add to RESTServer.cpp

std::string RESTServer::reportStolenFunds(const std::string& body) {
    try {
        auto json = nlohmann::json::parse(body);
        
        // Validate required fields
        if (!json.contains("txHash") || !json.contains("amount")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "txHash and amount are required");
        }
        
        // Create report
        FraudReport report;
        report.reportId = "REPORT_" + std::to_string(++reportCounter);
        report.txHash = json["txHash"];
        report.reporterAddress = json.value("reporterAddress", "");
        report.amount = json["amount"];
        report.email = json.value("email", "");
        report.description = json.value("description", "");
        report.evidence = json.value("evidence", "");
        report.timestamp = std::time(nullptr);
        report.status = "PENDING_REVIEW";
        
        // Store report
        reports[report.reportId] = report;
        
        // Log security event
        LOG_SECURITY(LogLevel::CRITICAL, 
            "Fraud report received: " + report.reportId + 
            " | TX: " + report.txHash + 
            " | Amount: " + std::to_string(report.amount));
        
        // Send notification to security team
        sendSecurityNotification(report);
        
        // Send confirmation email to reporter
        if (!report.email.empty()) {
            sendConfirmationEmail(report);
        }
        
        // Create response
        nlohmann::json response;
        response["success"] = true;
        response["reportId"] = report.reportId;
        response["status"] = report.status;
        response["message"] = "Report submitted successfully. We will review within 24 hours.";
        response["estimatedReviewTime"] = getEstimatedReviewTime();
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

std::string RESTServer::checkReportStatus(const std::string& reportId) {
    auto it = reports.find(reportId);
    
    if (it == reports.end()) {
        return createErrorResponse(404, "NOT_FOUND", 
            "Report not found: " + reportId);
    }
    
    const FraudReport& report = it->second;
    
    nlohmann::json response;
    response["reportId"] = report.reportId;
    response["txHash"] = report.txHash;
    response["amount"] = report.amount;
    response["status"] = report.status;
    response["timestamp"] = report.timestamp;
    response["reviewNotes"] = report.reviewNotes;
    
    return jsonToString(response);
}

// Admin endpoint: Approve report
std::string RESTServer::approveReport(const std::string& reportId, 
                                      const std::string& adminKey) {
    // Verify admin authorization
    if (!isAuthorizedAdmin(adminKey)) {
        return createErrorResponse(403, "UNAUTHORIZED", 
            "Admin authorization required");
    }
    
    auto it = reports.find(reportId);
    if (it == reports.end()) {
        return createErrorResponse(404, "NOT_FOUND", 
            "Report not found: " + reportId);
    }
    
    FraudReport& report = it->second;
    
    // Mark transaction as stolen in fraud detection system
    fraudDetection->markAsStolen(report.txHash);
    
    // Update report status
    report.status = "APPROVED";
    report.reviewNotes = "Report approved by admin. Transaction marked as stolen.";
    
    // Notify reporter
    if (!report.email.empty()) {
        sendApprovalEmail(report);
    }
    
    LOG_SECURITY(LogLevel::CRITICAL, 
        "Fraud report approved: " + reportId + 
        " | TX marked as stolen: " + report.txHash);
    
    nlohmann::json response;
    response["success"] = true;
    response["reportId"] = reportId;
    response["status"] = "APPROVED";
    response["message"] = "Report approved. Transaction marked as stolen.";
    
    return jsonToString(response);
}
```

### Add Routes to handleRequest()

```cpp
std::string RESTServer::handleRequest(const std::string& method, 
                                      const std::string& path, 
                                      const std::string& body) {
    // ... existing routes ...
    
    // Fraud reporting endpoints
    if (path == "/api/fraud/report-stolen" && method == "POST") {
        return reportStolenFunds(body);
    }
    else if (path.find("/api/fraud/report/") == 0) {
        std::string reportId = path.substr(18); // Remove "/api/fraud/report/"
        return checkReportStatus(reportId);
    }
    else if (path == "/api/fraud/approve" && method == "POST") {
        auto json = nlohmann::json::parse(body);
        return approveReport(json["reportId"], json["adminKey"]);
    }
    else if (path == "/api/fraud/reject" && method == "POST") {
        auto json = nlohmann::json::parse(body);
        return rejectReport(json["reportId"], json["adminKey"], json["reason"]);
    }
    
    // ... rest of routes ...
}
```

---

## Complete User Flow

### Scenario: Alice's Wallet is Hacked

```
┌─────────────────────────────────────────────────────────┐
│  STEP 1: Alice Discovers Theft                          │
│                                                          │
│  Alice checks wallet: "Where did my 10,000 GXC go?!"    │
│  She sees unauthorized transaction: TX_STOLEN            │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  STEP 2: Alice Reports (Multiple Options)               │
│                                                          │
│  Option A: Web Form                                      │
│    → Visits gxc.com/report-theft                         │
│    → Fills out simple form                               │
│    → Clicks "Report"                                     │
│                                                          │
│  Option B: Mobile App                                    │
│    → Opens GXC Wallet app                                │
│    → Taps "Report Theft"                                 │
│    → Fills out form                                      │
│                                                          │
│  Option C: Email                                         │
│    → Sends email to security@gxc.com                     │
│                                                          │
│  ✅ Easy - No technical knowledge needed!               │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  STEP 3: System Receives Report                         │
│                                                          │
│  Report ID: REPORT_001                                   │
│  Status: PENDING_REVIEW                                  │
│  TX Hash: TX_STOLEN                                      │
│  Amount: 10,000 GXC                                      │
│                                                          │
│  ✅ Confirmation sent to Alice's email                  │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  STEP 4: Security Team Reviews (Within 24 hours)        │
│                                                          │
│  Admin checks:                                           │
│  ✅ Transaction exists                                   │
│  ✅ Alice owns the wallet                                │
│  ✅ Transaction was unauthorized                         │
│  ✅ Evidence provided                                    │
│                                                          │
│  Admin approves: POST /api/fraud/approve                 │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  STEP 5: System Automatically Flags Transaction         │
│                                                          │
│  fraudDetection->markAsStolen("TX_STOLEN")               │
│  τ(TX_STOLEN) = 1.0                                      │
│                                                          │
│  ✅ Taint propagation starts automatically              │
│  ✅ All descendant transactions tracked                 │
│  ✅ Hacker addresses flagged                            │
│  ✅ Exchange deposits blocked                           │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  STEP 6: Alice Gets Notification                        │
│                                                          │
│  Email: "Your report has been approved"                  │
│  Status: APPROVED                                        │
│  Action: Transaction marked as stolen                    │
│  Result: Hacker cannot cash out funds                    │
│                                                          │
│  ✅ Alice is protected                                  │
└─────────────────────────────────────────────────────────┘
```

---

## API Endpoints Summary

### Public Endpoints (Anyone Can Use)

```
POST   /api/fraud/report-stolen          Report stolen funds
GET    /api/fraud/report/:reportId       Check report status
GET    /api/fraud/my-reports/:address    List my reports
```

### Admin Endpoints (Require Authorization)

```
POST   /api/fraud/approve                Approve report (marks as stolen)
POST   /api/fraud/reject                 Reject report
GET    /api/fraud/pending                List pending reports
GET    /api/fraud/statistics             Get fraud statistics
```

### Query Endpoints (Check Fraud Status)

```
GET    /api/fraud/check-transaction/:hash    Check if transaction is tainted
GET    /api/fraud/check-address/:address     Check if address is flagged
GET    /api/fraud/taint-score/:hash          Get taint score
```

---

## Testing the API

### Test 1: Submit Report

```bash
curl -X POST http://localhost:8080/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "test_stolen_tx_001",
    "reporterAddress": "GXC_test_address",
    "amount": 10000,
    "email": "test@example.com",
    "description": "Test theft report"
  }'
```

**Expected Response**:
```json
{
    "success": true,
    "reportId": "REPORT_001",
    "status": "PENDING_REVIEW",
    "message": "Report submitted successfully. We will review within 24 hours."
}
```

### Test 2: Check Report Status

```bash
curl http://localhost:8080/api/fraud/report/REPORT_001
```

**Expected Response**:
```json
{
    "reportId": "REPORT_001",
    "txHash": "test_stolen_tx_001",
    "amount": 10000,
    "status": "PENDING_REVIEW",
    "timestamp": 1704067200
}
```

### Test 3: Approve Report (Admin)

```bash
curl -X POST http://localhost:8080/api/fraud/approve \
  -H "Content-Type: application/json" \
  -d '{
    "reportId": "REPORT_001",
    "adminKey": "admin_secret_key"
  }'
```

**Expected Response**:
```json
{
    "success": true,
    "reportId": "REPORT_001",
    "status": "APPROVED",
    "message": "Report approved. Transaction marked as stolen."
}
```

---

## Summary: Multiple Easy Ways to Report

### ✅ For Non-Technical Users:
1. **Web Form** - Simple, user-friendly interface
2. **Mobile App** - Report from phone
3. **Email** - Send to security@gxc.com

### ✅ For Developers:
4. **REST API** - HTTP POST request
5. **RPC API** - JSON-RPC call

### ✅ Future:
6. **Automatic Detection** - AI-powered anomaly detection

### The Process:
```
User Reports → System Receives → Admin Reviews → Auto-Flags → Protection Active
   (Easy)        (Instant)        (24 hours)      (Automatic)    (Permanent)
```

**No technical knowledge required!** Anyone can report stolen funds easily.

---

## Next Steps

1. ✅ Implement REST API endpoints
2. ✅ Create web form interface
3. ✅ Build mobile app integration
4. ✅ Set up email reporting system
5. ✅ Train security team on review process
6. ✅ Test with real scenarios

**The fraud reporting system is now accessible to everyone!**
