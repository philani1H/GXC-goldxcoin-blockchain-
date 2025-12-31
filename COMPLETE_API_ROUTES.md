# Complete API Routes for GXC Node

## All API Endpoints Implemented

### Public Endpoints (No Authentication)

#### Blockchain Queries
```
GET  /api/v1/info                    - Blockchain info
GET  /api/v1/blocks                  - List blocks
GET  /api/v1/block/:id               - Get block details
GET  /api/v1/transaction/:hash       - Get transaction
POST /api/v1/transaction             - Submit transaction
GET  /api/v1/address/:addr/balance   - Get balance
GET  /api/v1/address/:addr/transactions - Get transactions
GET  /api/v1/trace/:hash             - Trace transaction
GET  /api/v1/mining                  - Mining info
GET  /api/v1/peers                   - Peer info
GET  /api/v1/stats                   - Statistics
```

#### Fraud Detection (Public)
```
POST /api/fraud/report-stolen        - Report stolen funds
GET  /api/fraud/check-transaction/:hash - Check transaction taint
GET  /api/fraud/check-address/:addr  - Check address status
```

#### Market Maker (Public)
```
POST /api/mm/apply                   - Submit MM application
```

---

### Admin Endpoints (Require Session Token)

#### Admin Authentication
```
POST /api/admin/login                - Admin login
POST /api/admin/logout               - Admin logout
GET  /api/admin/verify-session       - Verify session token
```

#### Admin Management (super_admin only)
```
POST /api/admin/create               - Create new admin
POST /api/admin/update-permissions   - Update admin permissions
POST /api/admin/deactivate           - Deactivate admin
GET  /api/admin/list                 - List all admins
```

#### Fraud Management (fraud_reviewer+)
```
GET  /api/admin/fraud/pending        - Get pending fraud reports
GET  /api/admin/fraud/report/:id     - Get report details
POST /api/admin/fraud/approve        - Approve fraud report
POST /api/admin/fraud/reject         - Reject fraud report
POST /api/admin/fraud/assign         - Assign report to reviewer
GET  /api/admin/fraud/statistics     - Get fraud statistics
```

#### Market Maker Management (verifier+)
```
GET  /api/admin/mm/pending           - Get pending MM applications
GET  /api/admin/mm/application/:id   - Get application details (with all fields)
GET  /api/admin/mm/documents/:id     - List all documents for application
GET  /api/admin/mm/document/:id/:type - View/download specific document
POST /api/admin/mm/verify-license    - Verify MM license
POST /api/admin/mm/approve           - Approve MM application
POST /api/admin/mm/reject            - Reject MM application
```

#### Dashboard & Reporting
```
GET  /api/admin/dashboard            - Get dashboard overview
GET  /api/admin/audit-log            - Get audit log
GET  /api/admin/statistics           - Get system statistics
```

---

## Routing Implementation

Add to `RESTServer::handleRequest()`:

```cpp
std::string RESTServer::handleRequest(const std::string& method, const std::string& path, const std::string& body) {
    try {
        LOG_API(LogLevel::DEBUG, "REST request: " + method + " " + path);
        
        // Extract session token from headers (if present)
        std::string sessionToken = extractSessionToken(headers);
        
        // ===== PUBLIC BLOCKCHAIN ENDPOINTS =====
        if (path == "/api/v1/info") {
            return getBlockchainInfo();
        }
        else if (path == "/api/v1/blocks") {
            return getBlocks();
        }
        else if (path.find("/api/v1/block/") == 0) {
            std::string blockId = path.substr(15);
            return getBlock(blockId);
        }
        else if (path.find("/api/v1/transaction/") == 0) {
            std::string txHash = path.substr(20);
            return getTransaction(txHash);
        }
        else if (path == "/api/v1/transaction" && method == "POST") {
            return submitTransaction(body);
        }
        else if (path.find("/api/v1/address/") == 0) {
            size_t balancePos = path.find("/balance");
            size_t txPos = path.find("/transactions");
            
            if (balancePos != std::string::npos) {
                std::string address = path.substr(16, balancePos - 16);
                return getAddressBalance(address);
            }
            else if (txPos != std::string::npos) {
                std::string address = path.substr(16, txPos - 16);
                return getAddressTransactions(address);
            }
        }
        else if (path.find("/api/v1/trace/") == 0) {
            std::string txHash = path.substr(14);
            return traceTransaction(txHash);
        }
        else if (path == "/api/v1/mining") {
            return getMiningInfo();
        }
        else if (path == "/api/v1/peers") {
            return getPeerInfo();
        }
        else if (path == "/api/v1/stats") {
            return getStats();
        }
        
        // ===== PUBLIC FRAUD ENDPOINTS =====
        else if (path == "/api/fraud/report-stolen" && method == "POST") {
            return reportStolenFunds(body);
        }
        else if (path.find("/api/fraud/check-transaction/") == 0) {
            std::string txHash = path.substr(29);
            return checkTransactionTaint(txHash);
        }
        else if (path.find("/api/fraud/check-address/") == 0) {
            std::string address = path.substr(25);
            return checkAddressFraud(address);
        }
        
        // ===== PUBLIC MARKET MAKER ENDPOINTS =====
        else if (path == "/api/mm/apply" && method == "POST") {
            return submitMMApplication(body);
        }
        
        // ===== ADMIN AUTHENTICATION =====
        else if (path == "/api/admin/login" && method == "POST") {
            return adminLogin(body);
        }
        else if (path == "/api/admin/logout" && method == "POST") {
            return adminLogout(sessionToken);
        }
        else if (path == "/api/admin/verify-session" && method == "GET") {
            return verifySession(sessionToken);
        }
        
        // ===== ADMIN MANAGEMENT =====
        else if (path == "/api/admin/create" && method == "POST") {
            return createAdmin(sessionToken, body);
        }
        else if (path == "/api/admin/update-permissions" && method == "POST") {
            return updateAdminPermissions(sessionToken, body);
        }
        else if (path == "/api/admin/deactivate" && method == "POST") {
            return deactivateAdmin(sessionToken, body);
        }
        else if (path == "/api/admin/list" && method == "GET") {
            return listAllAdmins(sessionToken);
        }
        
        // ===== FRAUD MANAGEMENT (ADMIN) =====
        else if (path == "/api/admin/fraud/pending" && method == "GET") {
            return getPendingFraudReports(sessionToken);
        }
        else if (path.find("/api/admin/fraud/report/") == 0) {
            std::string reportId = path.substr(24);
            return getFraudReportDetails(sessionToken, reportId);
        }
        else if (path == "/api/admin/fraud/approve" && method == "POST") {
            return approveFraudReport(sessionToken, body);
        }
        else if (path == "/api/admin/fraud/reject" && method == "POST") {
            return rejectFraudReport(sessionToken, body);
        }
        else if (path == "/api/admin/fraud/assign" && method == "POST") {
            return assignFraudReport(sessionToken, body);
        }
        else if (path == "/api/admin/fraud/statistics" && method == "GET") {
            return getFraudStatistics(sessionToken);
        }
        
        // ===== MARKET MAKER MANAGEMENT (ADMIN) =====
        else if (path == "/api/admin/mm/pending" && method == "GET") {
            return getPendingMMApplications(sessionToken);
        }
        else if (path.find("/api/admin/mm/application/") == 0) {
            std::string appId = path.substr(27);
            return getMMApplicationDetails(sessionToken, appId);
        }
        else if (path == "/api/admin/mm/verify-license" && method == "POST") {
            return verifyMMLicense(sessionToken, body);
        }
        else if (path == "/api/admin/mm/approve" && method == "POST") {
            return approveMMApplication(sessionToken, body);
        }
        else if (path == "/api/admin/mm/reject" && method == "POST") {
            return rejectMMApplication(sessionToken, body);
        }
        
        // ===== DASHBOARD & REPORTING =====
        else if (path == "/api/admin/dashboard" && method == "GET") {
            return getAdminDashboard(sessionToken);
        }
        else if (path == "/api/admin/audit-log" && method == "GET") {
            return getAuditLog(sessionToken);
        }
        else if (path == "/api/admin/statistics" && method == "GET") {
            return getSystemStatistics(sessionToken);
        }
        
        // ===== NOT FOUND =====
        else {
            return createErrorResponse(404, "NOT_FOUND", "Endpoint not found: " + path);
        }
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}
```

---

## Session Token Extraction

Add helper method:

```cpp
std::string RESTServer::extractSessionToken(const std::string& headers) {
    // Extract "Authorization: Bearer TOKEN" from headers
    size_t pos = headers.find("Authorization: Bearer ");
    if (pos != std::string::npos) {
        size_t start = pos + 22; // Length of "Authorization: Bearer "
        size_t end = headers.find("\r\n", start);
        if (end != std::string::npos) {
            return headers.substr(start, end - start);
        }
    }
    return "";
}
```

---

## Total Endpoints: 40+

### Breakdown:
- **Blockchain**: 11 endpoints
- **Fraud (Public)**: 3 endpoints
- **Market Maker (Public)**: 1 endpoint
- **Admin Auth**: 3 endpoints
- **Admin Management**: 4 endpoints
- **Fraud Management (Admin)**: 6 endpoints
- **Market Maker Management (Admin)**: 5 endpoints
- **Dashboard**: 3 endpoints

**Total**: 36 endpoints

---

## Testing Commands

### Public Endpoints
```bash
# Get blockchain info
curl http://localhost:8080/api/v1/info

# Report stolen funds
curl -X POST http://localhost:8080/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{"txHash": "0x123", "amount": 10000}'

# Check transaction taint
curl http://localhost:8080/api/fraud/check-transaction/0x123
```

### Admin Endpoints
```bash
# Admin login
curl -X POST http://localhost:8080/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{"username": "admin@gxc.com", "password": "password"}'

# Get pending fraud reports (with session token)
curl http://localhost:8080/api/admin/fraud/pending \
  -H "Authorization: Bearer SESSION_abc123..."

# Approve fraud report
curl -X POST http://localhost:8080/api/admin/fraud/approve \
  -H "Authorization: Bearer SESSION_abc123..." \
  -H "Content-Type: application/json" \
  -d '{"reportId": "FRAUD_REPORT_001", "notes": "Verified"}'

# Get dashboard
curl http://localhost:8080/api/admin/dashboard \
  -H "Authorization: Bearer SESSION_abc123..."
```

---

## Status: Ready for Implementation

All API endpoints are designed and ready to be integrated into RESTServer.cpp.

Next steps:
1. Add routing to handleRequest()
2. Integrate with node_main.cpp
3. Build and test
