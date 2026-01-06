# GXC Admin API - Complete Testing Guide

## Base URLs

**Railway Production Node:**
```
https://gxc-chain112-blockchain-node-production.up.railway.app
```

**REST API Port:** 8080 (if enabled)
**RPC API Port:** 8332

---

## 🔐 Authentication Endpoints

### 1. Admin Login
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/login`

**Request:**
```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "admin123"
  }'
```

**Response:**
```json
{
  "success": true,
  "sessionToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "adminId": "admin_001",
  "username": "admin",
  "role": "super_admin",
  "permissions": ["manage_admins", "fraud_reviewer", "fraud_approver"],
  "expiresAt": 1735747200
}
```

**Save the sessionToken for subsequent requests!**

---

### 2. Verify Session
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/verify-session`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/verify-session \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

---

### 3. Admin Logout
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/logout`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/logout \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

---

## 🚨 Fraud Detection Endpoints

### 4. Submit Fraud Report (Public)
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/fraud/report-stolen`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "abc123def456...",
    "reporterAddress": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
    "amount": 1000.50,
    "description": "My wallet was hacked. I did not authorize this transaction.",
    "evidence": "Police report #12345",
    "email": "victim@example.com"
  }'
```

**Response:**
```json
{
  "success": true,
  "reportId": "FR_1234567890",
  "message": "Fraud report submitted successfully",
  "status": "pending_review"
}
```

---

### 5. Check Transaction Taint
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/fraud/check-transaction/{txHash}`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/fraud/check-transaction/abc123def456...
```

**Response:**
```json
{
  "txHash": "abc123def456...",
  "isTainted": false,
  "taintScore": 0.0,
  "flaggedReports": [],
  "status": "clean"
}
```

---

### 6. Check Address Status
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/fraud/check-address/{address}`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/fraud/check-address/tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
```

**Response:**
```json
{
  "address": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
  "isFlagged": false,
  "riskLevel": "low",
  "associatedReports": [],
  "totalTaintedAmount": 0.0
}
```

---

## 📋 Admin Fraud Management

### 7. List All Fraud Reports
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/reports`

```bash
curl -X GET "https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/reports?status=pending&page=1&limit=20" \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

**Query Parameters:**
- `status`: `pending`, `under_review`, `approved`, `rejected`, `all`
- `page`: Page number (default: 1)
- `limit`: Results per page (default: 20)

**Response:**
```json
{
  "success": true,
  "reports": [
    {
      "reportId": "FR_1234567890",
      "txHash": "abc123...",
      "reporterAddress": "tGXC3tz6...",
      "amount": 1000.50,
      "status": "pending_review",
      "submittedAt": 1735747200,
      "description": "Wallet hacked...",
      "assignedTo": null
    }
  ],
  "pagination": {
    "page": 1,
    "limit": 20,
    "total": 45,
    "totalPages": 3
  }
}
```

---

### 8. Get Fraud Report Details
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/report/{reportId}`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/report/FR_1234567890 \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

---

### 9. Assign Fraud Report to Reviewer
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/assign`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/assign \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "reportId": "FR_1234567890",
    "reviewerId": "admin_002"
  }'
```

---

### 10. Approve Fraud Report (Initiate Reversal)
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/approve`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/approve \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "reportId": "FR_1234567890",
    "notes": "Verified with police report. Initiating reversal.",
    "reversalAmount": 1000.50
  }'
```

**Response:**
```json
{
  "success": true,
  "reportId": "FR_1234567890",
  "status": "approved",
  "reversalTxHash": "xyz789...",
  "message": "Fraud report approved. Reversal transaction created."
}
```

---

### 11. Reject Fraud Report
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/reject`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/reject \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "reportId": "FR_1234567890",
    "reason": "Insufficient evidence. Transaction appears legitimate.",
    "notes": "Reporter could not provide proof of unauthorized access."
  }'
```

---

### 12. Get Fraud Statistics
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/stats`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/stats \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

**Response:**
```json
{
  "success": true,
  "stats": {
    "totalReports": 150,
    "pending": 25,
    "underReview": 10,
    "approved": 80,
    "rejected": 35,
    "totalAmountReported": 125000.50,
    "totalAmountReversed": 95000.00,
    "averageProcessingTime": 48.5
  }
}
```

---

## 👥 Admin User Management

### 13. List All Admins
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/list`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/list \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

---

### 14. Create New Admin
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/create`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/create \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "reviewer1",
    "password": "SecurePass123!",
    "email": "reviewer1@gxc.com",
    "role": "fraud_reviewer",
    "permissions": ["fraud_reviewer", "view_reports"]
  }'
```

**Roles:**
- `super_admin`: Full system access
- `fraud_reviewer`: Can review and assign reports
- `fraud_approver`: Can approve/reject reversals
- `viewer`: Read-only access

---

### 15. Update Admin Permissions
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/update-permissions`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/update-permissions \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "adminId": "admin_002",
    "permissions": ["fraud_reviewer", "fraud_approver", "view_reports"]
  }'
```

---

### 16. Deactivate Admin
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/deactivate`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/deactivate \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "adminId": "admin_002",
    "reason": "Employee left company"
  }'
```

---

### 17. Change Admin Password
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/change-password`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/users/change-password \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "currentPassword": "OldPass123!",
    "newPassword": "NewSecurePass456!"
  }'
```

---

## 📊 System Monitoring

### 18. Get System Health
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/system/health`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/system/health \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

**Response:**
```json
{
  "success": true,
  "health": {
    "status": "healthy",
    "uptime": 86400,
    "blockHeight": 12345,
    "peers": 8,
    "mempool": 25,
    "fraudSystem": "operational",
    "database": "connected"
  }
}
```

---

### 19. Get Audit Logs
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/audit/logs`

```bash
curl -X GET "https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/audit/logs?page=1&limit=50" \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

**Response:**
```json
{
  "success": true,
  "logs": [
    {
      "timestamp": 1735747200,
      "adminId": "admin_001",
      "action": "APPROVE_FRAUD_REPORT",
      "details": {
        "reportId": "FR_1234567890",
        "amount": 1000.50
      },
      "ipAddress": "192.168.1.1"
    }
  ]
}
```

---

## 🏦 Market Maker Admin (If Enabled)

### 20. List Market Maker Applications
**URL:** `GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/mm/applications`

```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/mm/applications \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN"
```

---

### 21. Approve Market Maker
**URL:** `POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/mm/approve`

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/mm/approve \
  -H "Authorization: Bearer YOUR_SESSION_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "MM_APP_001",
    "notes": "Verified credentials and liquidity requirements"
  }'
```

---

## 🧪 Testing Workflow

### Complete Test Sequence

1. **Login and Get Session Token**
```bash
TOKEN=$(curl -s -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}' | jq -r '.sessionToken')

echo "Session Token: $TOKEN"
```

2. **Verify Session**
```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/verify-session \
  -H "Authorization: Bearer $TOKEN"
```

3. **Get Fraud Reports**
```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/fraud/reports \
  -H "Authorization: Bearer $TOKEN"
```

4. **Get System Health**
```bash
curl -X GET https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/system/health \
  -H "Authorization: Bearer $TOKEN"
```

5. **Logout**
```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/api/admin/logout \
  -H "Authorization: Bearer $TOKEN"
```

---

## 📝 Important Notes

### REST Server Status

⚠️ **The REST API server (port 8080) may not be fully functional on Railway.**

The current implementation has:
- ✅ All endpoint routing and handlers implemented
- ✅ JSON request/response handling
- ❌ May not have HTTP server binding on port 8080

### Alternative: Use RPC API

If REST endpoints don't work, use JSON-RPC on port 8332:

```bash
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "admin_login",
    "params": {
      "username": "admin",
      "password": "admin123"
    },
    "id": 1
  }'
```

### Default Credentials

**Username:** `admin`
**Password:** `admin123`

⚠️ **Change these immediately in production!**

---

## 📚 Full Documentation

For complete API reference, see:
- `ADMIN_API_COMPLETE_REFERENCE.md` - Full endpoint documentation
- `ADMIN_API_TEST_GUIDE.md` - Testing procedures
- `ADMIN_QUICK_START.md` - Quick start guide
- `COMPLETE_ADMIN_FRAUD_SYSTEM.md` - System architecture

---

## 🔗 Quick Links

**Railway Node:** [https://gxc-chain112-blockchain-node-production.up.railway.app](https://gxc-chain112-blockchain-node-production.up.railway.app)

**Health Check:** [https://gxc-chain112-blockchain-node-production.up.railway.app/health](https://gxc-chain112-blockchain-node-production.up.railway.app/health)

**Blockchain Info:** [https://gxc-chain112-blockchain-node-production.up.railway.app/api/getinfo](https://gxc-chain112-blockchain-node-production.up.railway.app/api/getinfo)
