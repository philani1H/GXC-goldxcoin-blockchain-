# Build and Test Complete GXC Node

## Step-by-Step Build Instructions

### Step 1: Integrate Admin APIs into RESTServer.cpp

Copy the methods from `src/RESTServer_AdminAPIs.cpp` to the end of `src/RESTServer.cpp` (before the closing brace).

**Location**: After the existing methods, before the final `}`

**What to copy**: All methods from RESTServer_AdminAPIs.cpp

### Step 2: Update handleRequest() Routing

Add the new routes to `RESTServer::handleRequest()` method.

**File**: `src/RESTServer.cpp`

**Add after existing routes**:

```cpp
// ===== ADMIN AUTHENTICATION =====
else if (path == "/api/admin/login" && method == "POST") {
    return adminLogin(body);
}
else if (path == "/api/admin/logout" && method == "POST") {
    // Extract session token from headers or body
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return adminLogout(sessionToken);
}
else if (path == "/api/admin/verify-session" && method == "GET") {
    // Extract session token from headers
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return verifySession(sessionToken);
}

// ===== ADMIN MANAGEMENT =====
else if (path == "/api/admin/create" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return createAdmin(sessionToken, body);
}
else if (path == "/api/admin/update-permissions" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return updateAdminPermissions(sessionToken, body);
}
else if (path == "/api/admin/deactivate" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return deactivateAdmin(sessionToken, body);
}
else if (path == "/api/admin/list" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return listAllAdmins(sessionToken);
}

// ===== FRAUD MANAGEMENT (ADMIN) =====
else if (path == "/api/admin/fraud/pending" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getPendingFraudReports(sessionToken);
}
else if (path.find("/api/admin/fraud/report/") == 0) {
    std::string reportId = path.substr(24);
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getFraudReportDetails(sessionToken, reportId);
}
else if (path == "/api/admin/fraud/approve" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return approveFraudReport(sessionToken, body);
}
else if (path == "/api/admin/fraud/reject" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return rejectFraudReport(sessionToken, body);
}
else if (path == "/api/admin/fraud/assign" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return assignFraudReport(sessionToken, body);
}
else if (path == "/api/admin/fraud/statistics" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getFraudStatistics(sessionToken);
}

// ===== MARKET MAKER MANAGEMENT (ADMIN) =====
else if (path == "/api/admin/mm/pending" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getPendingMMApplications(sessionToken);
}
else if (path.find("/api/admin/mm/application/") == 0) {
    std::string appId = path.substr(27);
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getMMApplicationDetails(sessionToken, appId);
}
else if (path.find("/api/admin/mm/documents/") == 0) {
    std::string appId = path.substr(24);
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return listMMDocuments(sessionToken, appId);
}
else if (path.find("/api/admin/mm/document/") == 0) {
    // Parse: /api/admin/mm/document/:appId/:docType
    std::string remainder = path.substr(23);
    size_t slashPos = remainder.find('/');
    if (slashPos != std::string::npos) {
        std::string appId = remainder.substr(0, slashPos);
        std::string docType = remainder.substr(slashPos + 1);
        auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
        std::string sessionToken = json.value("sessionToken", "");
        return getMMDocument(sessionToken, appId, docType);
    }
}
else if (path == "/api/admin/mm/verify-license" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return verifyMMLicense(sessionToken, body);
}
else if (path == "/api/admin/mm/approve" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return approveMMApplication(sessionToken, body);
}
else if (path == "/api/admin/mm/reject" && method == "POST") {
    auto json = nlohmann::json::parse(body);
    std::string sessionToken = json.value("sessionToken", "");
    return rejectMMApplication(sessionToken, body);
}

// ===== DASHBOARD & REPORTING =====
else if (path == "/api/admin/dashboard" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getAdminDashboard(sessionToken);
}
else if (path == "/api/admin/audit-log" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getAuditLog(sessionToken);
}
else if (path == "/api/admin/statistics" && method == "GET") {
    auto json = nlohmann::json::parse(body.empty() ? "{}" : body);
    std::string sessionToken = json.value("sessionToken", "");
    return getSystemStatistics(sessionToken);
}
```

### Step 3: Update node_main.cpp

Add initialization of all systems:

```cpp
#include "../include/blockchain.h"
#include "../include/FraudDetection.h"
#include "../include/MarketMakerAdmin.h"
#include "../include/RESTServer.h"
// ... other includes

int main(int argc, char* argv[]) {
    try {
        LOG_INFO("Starting GXC Node...");
        
        // Initialize blockchain
        Blockchain* blockchain = new Blockchain();
        LOG_INFO("Blockchain initialized");
        
        // Initialize fraud detection
        FraudDetection* fraudDetection = new FraudDetection(blockchain);
        LOG_INFO("Fraud Detection initialized");
        
        // Initialize admin system
        MarketMakerAdmin* adminSystem = new MarketMakerAdmin();
        LOG_INFO("Admin System initialized");
        
        // Create default super admin
        std::vector<std::string> superAdminPerms = {"all"};
        std::string superAdminId = adminSystem->createAdmin(
            "", // No parent admin for first admin
            "superadmin@gxc.com",
            "ChangeThisPassword123!", // Change in production!
            "super_admin",
            superAdminPerms
        );
        LOG_INFO("Super admin created: " + superAdminId);
        
        // Register some clean zones
        fraudDetection->registerExchange("binance_deposit_001", "Binance");
        fraudDetection->registerExchange("kraken_deposit_001", "Kraken");
        fraudDetection->registerStakingPool("gxc_pool_001", "GXC Official Pool");
        LOG_INFO("Clean zones registered");
        
        // Initialize REST API with all systems
        RESTServer* restServer = new RESTServer(
            blockchain,
            fraudDetection,
            adminSystem,
            8080
        );
        LOG_INFO("REST API initialized on port 8080");
        
        // Start REST server
        if (restServer->start()) {
            LOG_INFO("REST API server started successfully");
        } else {
            LOG_ERROR("Failed to start REST API server");
            return 1;
        }
        
        // Keep node running
        LOG_INFO("GXC Node is running. Press Ctrl+C to stop.");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // Cleanup
        restServer->stop();
        delete restServer;
        delete adminSystem;
        delete fraudDetection;
        delete blockchain;
        
        return 0;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: " + std::string(e.what()));
        return 1;
    }
}
```

### Step 4: Build the Node

```bash
cd /workspaces/GXC-goldxcoin-blockchain-

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Or use the build script if available
cd ..
./build-blockchain.ps1
```

### Step 5: Run the Node

```bash
# Run the node
./build/gxc-node

# Or if using different binary name
./gxc-node
```

---

## Testing All APIs

### Test 1: Admin Login

```bash
curl -X POST http://localhost:8080/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "superadmin@gxc.com",
    "password": "ChangeThisPassword123!"
  }'
```

**Expected Response**:
```json
{
  "success": true,
  "sessionToken": "SESSION_abc123...",
  "adminId": "ADMIN_001",
  "username": "superadmin@gxc.com",
  "role": "super_admin",
  "permissions": ["all"],
  "expiresAt": 1704153600
}
```

**Save the sessionToken for subsequent requests!**

### Test 2: Create Fraud Reviewer

```bash
curl -X POST http://localhost:8080/api/admin/create \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123...",
    "username": "fraud_reviewer@gxc.com",
    "password": "SecurePassword123!",
    "role": "fraud_reviewer",
    "permissions": ["fraud_reviewer"]
  }'
```

### Test 3: Report Stolen Funds (Public)

```bash
curl -X POST http://localhost:8080/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "0x1234567890abcdef",
    "reporterAddress": "GXC_test_address",
    "amount": 10000,
    "email": "victim@example.com",
    "description": "My wallet was hacked and funds were stolen"
  }'
```

**Expected Response**:
```json
{
  "success": true,
  "reportId": "FRAUD_REPORT_001",
  "status": "PENDING",
  "message": "Report submitted successfully. We will review within 24 hours."
}
```

### Test 4: View Pending Fraud Reports (Admin)

```bash
curl -X GET http://localhost:8080/api/admin/fraud/pending \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

### Test 5: Approve Fraud Report (Admin)

```bash
curl -X POST http://localhost:8080/api/admin/fraud/approve \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123...",
    "reportId": "FRAUD_REPORT_001",
    "notes": "Verified with blockchain explorer. Transaction confirmed as unauthorized."
  }'
```

**Expected Response**:
```json
{
  "success": true,
  "reportId": "FRAUD_REPORT_001",
  "status": "APPROVED",
  "message": "Report approved. Transaction marked as stolen.",
  "txHash": "0x1234567890abcdef",
  "taintScore": 1.0
}
```

### Test 6: Check Transaction Taint (Public)

```bash
curl http://localhost:8080/api/fraud/check-transaction/0x1234567890abcdef
```

**Expected Response**:
```json
{
  "txHash": "0x1234567890abcdef",
  "taintScore": 1.0,
  "isStolen": true,
  "riskLevel": "CRITICAL"
}
```

### Test 7: Submit Market Maker Application (Public)

```bash
curl -X POST http://localhost:8080/api/mm/apply \
  -H "Content-Type: application/json" \
  -d '{
    "companyName": "ABC Trading Ltd",
    "licenseNumber": "MM-12345",
    "contactEmail": "contact@abctrading.com",
    "contactPhone": "+1-555-0123",
    "website": "https://abctrading.com"
  }'
```

### Test 8: View Pending MM Applications (Admin)

```bash
curl -X GET http://localhost:8080/api/admin/mm/pending \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

### Test 9: View MM Application Details with Documents (Admin)

```bash
curl -X GET http://localhost:8080/api/admin/mm/application/MM_APP_001 \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

**Expected Response** (with all fields):
```json
{
  "applicationId": "MM_APP_001",
  "applicantAddress": "GXC_applicant_address",
  "companyName": "ABC Trading Ltd",
  "licenseNumber": "MM-12345",
  "regulatoryBody": "SEC",
  "country": "USA",
  "contactEmail": "contact@abctrading.com",
  "contactPhone": "+1-555-0123",
  "website": "https://abctrading.com",
  "documents": {
    "licenseDocument": "QmHash123...",
    "financialStatements": "QmHash456...",
    "technicalCapabilities": "QmHash789...",
    "kycDocuments": "QmHashABC..."
  },
  "status": "PENDING",
  "submittedAt": 1704067200,
  "lastUpdatedAt": 1704067200,
  "verificationSteps": [],
  "approvedBy": "",
  "decidedAt": 0,
  "decisionNotes": ""
}
```

### Test 10: List All Documents for Application (Admin)

```bash
curl -X GET http://localhost:8080/api/admin/mm/documents/MM_APP_001 \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

**Expected Response**:
```json
{
  "applicationId": "MM_APP_001",
  "companyName": "ABC Trading Ltd",
  "documents": [
    {
      "type": "license",
      "name": "Business License",
      "hash": "QmHash123...",
      "downloadUrl": "/api/admin/mm/document/MM_APP_001/license"
    },
    {
      "type": "financial",
      "name": "Financial Statements",
      "hash": "QmHash456...",
      "downloadUrl": "/api/admin/mm/document/MM_APP_001/financial"
    },
    {
      "type": "technical",
      "name": "Technical Capabilities Document",
      "hash": "QmHash789...",
      "downloadUrl": "/api/admin/mm/document/MM_APP_001/technical"
    },
    {
      "type": "kyc",
      "name": "KYC/AML Documents",
      "hash": "QmHashABC...",
      "downloadUrl": "/api/admin/mm/document/MM_APP_001/kyc"
    }
  ],
  "count": 4
}
```

### Test 11: View Specific Document (Admin)

```bash
curl -X GET http://localhost:8080/api/admin/mm/document/MM_APP_001/license \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

**Expected Response**:
```json
{
  "applicationId": "MM_APP_001",
  "documentType": "license",
  "documentName": "Business License",
  "documentHash": "QmHash123...",
  "companyName": "ABC Trading Ltd",
  "metadata": {
    "uploadedAt": 1704067200,
    "verified": false
  }
}
```

### Test 12: Approve MM Application (Admin)

```bash
curl -X POST http://localhost:8080/api/admin/mm/approve \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123...",
    "applicationId": "MM_APP_001",
    "notes": "All documents verified. Application approved."
  }'
```

### Test 13: Get Admin Dashboard

```bash
curl -X GET http://localhost:8080/api/admin/dashboard \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

### Test 14: Get System Statistics

```bash
curl -X GET http://localhost:8080/api/admin/statistics \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "SESSION_abc123..."
  }'
```

---

## Testing Stock Contract

Once the node is running and all admin APIs are working:

### Test 1: Deploy Stock Contract (Admin)

```bash
# This would be through the stock contract API
# Exact endpoint depends on stock contract implementation
```

### Test 2: Verify Admin Can Manage Stock Contracts

```bash
# Test admin permissions for stock contract management
```

### Test 3: Test Stock Contract Functions

```bash
# Test all stock contract functions
# Verify they work with the admin system
```

---

## Troubleshooting

### Issue: Compilation Errors

**Solution**: Check that all includes are correct:
```cpp
#include "../include/MarketMakerAdmin.h"
#include "../include/FraudDetection.h"
```

### Issue: Session Token Not Working

**Solution**: Make sure session token is being passed correctly in requests.

### Issue: Permission Denied

**Solution**: Verify the admin has the correct permissions for the action.

### Issue: Documents Not Found

**Solution**: Check that the application has documents uploaded.

---

## Success Criteria

✅ Node compiles without errors
✅ Node starts successfully
✅ Admin can login
✅ Admin can create other admins
✅ Users can report stolen funds
✅ Admin can approve fraud reports
✅ Fraud detection automatically flags transactions
✅ Users can submit MM applications
✅ Admin can view all MM application details
✅ Admin can view all uploaded documents
✅ Admin can approve MM applications
✅ Dashboard shows correct data
✅ Audit log records all actions
✅ Stock contract can be deployed and managed

---

**Status**: Ready to build and test!
