# Admin API Complete Reference

## Overview

The GXC Admin API provides endpoints for managing fraud reports, admin users, and system operations. This guide covers all admin endpoints needed to build a complete admin dashboard.

## Base URL

```
http://localhost:8080/api/admin
```

## Authentication

All admin endpoints (except login) require a session token in the Authorization header.

### Session Token Format

```
Authorization: Bearer {sessionToken}
```

### Session Management

- Sessions expire after 24 hours of inactivity
- Each login generates a new session token
- Logout invalidates the session token
- Multiple concurrent sessions are allowed per admin

---

## 1. Authentication Endpoints

### 1.1 Admin Login

**Endpoint:** `POST /api/admin/login`

**Purpose:** Authenticate admin and get session token

**Request:**
```json
{
  "username": "admin",
  "password": "admin123"
}
```

**Response (Success):**
```json
{
  "success": true,
  "sessionToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "adminId": "admin_001",
  "username": "admin",
  "role": "super_admin",
  "permissions": [
    "manage_admins",
    "fraud_reviewer",
    "fraud_approver",
    "view_reports",
    "manage_system"
  ],
  "expiresAt": 1735747200
}
```

**Response (Failure):**
```json
{
  "success": false,
  "error": "INVALID_CREDENTIALS",
  "message": "Invalid username or password"
}
```

**Status Codes:**
- `200 OK`: Login successful
- `401 Unauthorized`: Invalid credentials
- `503 Service Unavailable`: Admin system not available

### 1.2 Admin Logout

**Endpoint:** `POST /api/admin/logout`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Response:**
```json
{
  "success": true,
  "message": "Logged out successfully"
}
```

### 1.3 Verify Session

**Endpoint:** `GET /api/admin/verify-session`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Response:**
```json
{
  "valid": true,
  "adminId": "admin_001",
  "username": "admin",
  "role": "super_admin",
  "permissions": ["manage_admins", "fraud_reviewer", "fraud_approver"]
}
```

---

## 2. Fraud Report Management

### 2.1 Get Pending Fraud Reports

**Endpoint:** `GET /api/admin/fraud/pending`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `fraud_reviewer`

**Response:**
```json
{
  "success": true,
  "reports": [
    {
      "reportId": "FR_001",
      "txHash": "abc123...",
      "reporterAddress": "GXC1victim123...",
      "amount": 100.0,
      "email": "victim@example.com",
      "description": "My wallet was hacked and funds stolen",
      "evidence": "Transaction logs, wallet screenshots",
      "timestamp": 1735660800,
      "factsStatus": "PENDING",
      "executionStatus": "NOT_STARTED",
      "assignedTo": ""
    }
  ],
  "count": 1
}
```

### 2.2 Get Fraud Report Details

**Endpoint:** `GET /api/admin/fraud/report/{reportId}`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `fraud_reviewer`

**Response:**
```json
{
  "success": true,
  "report": {
    "reportId": "FR_001",
    "txHash": "abc123...",
    "reporterAddress": "GXC1victim123...",
    "amount": 100.0,
    "email": "victim@example.com",
    "description": "My wallet was hacked and funds stolen",
    "evidence": "Transaction logs, wallet screenshots",
    "timestamp": 1735660800,
    "factsStatus": "PENDING",
    "reviewedBy": "",
    "reviewedAt": 0,
    "reviewNotes": "",
    "assignedTo": "",
    "executionStatus": "NOT_STARTED",
    "proofHash": "",
    "validatedAt": 0,
    "executedAt": 0,
    "recoveredAmount": 0,
    "executionNotes": ""
  }
}
```

### 2.3 Assign Fraud Report to Reviewer

**Endpoint:** `POST /api/admin/fraud/assign`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `fraud_reviewer`

**Request:**
```json
{
  "reportId": "FR_001",
  "assignTo": "admin_002"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Report assigned successfully",
  "reportId": "FR_001",
  "assignedTo": "admin_002"
}
```

### 2.4 Approve Fraud Report (Validate FACTS)

**Endpoint:** `POST /api/admin/fraud/approve`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `fraud_approver`

**Request:**
```json
{
  "reportId": "FR_001",
  "notes": "Evidence reviewed. Fraud claim is legitimate. Transaction was unauthorized."
}
```

**Response:**
```json
{
  "success": true,
  "message": "Fraud FACTS approved. Protocol will now validate feasibility.",
  "reportId": "FR_001",
  "factsStatus": "FACTS_APPROVED",
  "executionStatus": "VALIDATING",
  "note": "Admin approved FACTS only. Protocol will independently validate and execute if feasible."
}
```

**Important Notes:**
- Admin approval validates **FACTS** (fraud legitimacy), NOT execution
- Protocol independently validates feasibility
- Execution is automatic if protocol determines it's feasible
- Admin cannot force or prevent execution

### 2.5 Reject Fraud Report

**Endpoint:** `POST /api/admin/fraud/reject`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `fraud_approver`

**Request:**
```json
{
  "reportId": "FR_001",
  "reason": "Insufficient evidence. Transaction appears to be legitimate."
}
```

**Response:**
```json
{
  "success": true,
  "message": "Fraud report rejected",
  "reportId": "FR_001",
  "factsStatus": "FACTS_REJECTED",
  "executionStatus": "NOT_STARTED"
}
```

### 2.6 Get All Fraud Reports (with filters)

**Endpoint:** `GET /api/admin/fraud/reports`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `fraud_reviewer`

**Query Parameters:**
- `status`: Filter by factsStatus (`PENDING`, `FACTS_APPROVED`, `FACTS_REJECTED`)
- `executionStatus`: Filter by executionStatus (`NOT_STARTED`, `VALIDATING`, `EXECUTED`, `INFEASIBLE`)
- `assignedTo`: Filter by assigned admin
- `page`: Page number (default: 1)
- `limit`: Results per page (default: 20)

**Example:**
```
GET /api/admin/fraud/reports?status=FACTS_APPROVED&executionStatus=EXECUTED&page=1&limit=10
```

**Response:**
```json
{
  "success": true,
  "reports": [...],
  "pagination": {
    "page": 1,
    "limit": 10,
    "total": 45,
    "pages": 5
  }
}
```

---

## 3. Admin User Management

### 3.1 Create Admin User

**Endpoint:** `POST /api/admin/users/create`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `manage_admins` (super_admin only)

**Request:**
```json
{
  "username": "fraud_admin_01",
  "password": "SecurePassword123!",
  "role": "fraud_approver",
  "permissions": ["fraud_reviewer", "fraud_approver"]
}
```

**Response:**
```json
{
  "success": true,
  "message": "Admin user created successfully",
  "adminId": "admin_003",
  "username": "fraud_admin_01",
  "role": "fraud_approver"
}
```

**Available Roles:**
- `super_admin`: Full system access
- `fraud_admin`: Manage fraud detection system
- `fraud_approver`: Approve/reject fraud reports
- `fraud_reviewer`: Review and assign fraud reports
- `verifier`: Verify market maker applications
- `reviewer`: Review applications

**Available Permissions:**
- `manage_admins`: Create/edit/delete admin users
- `fraud_reviewer`: View and assign fraud reports
- `fraud_approver`: Approve/reject fraud reports
- `view_reports`: View fraud reports (read-only)
- `manage_system`: System configuration
- `view_audit_logs`: View audit logs

### 3.2 List Admin Users

**Endpoint:** `GET /api/admin/users`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `manage_admins`

**Response:**
```json
{
  "success": true,
  "admins": [
    {
      "adminId": "admin_001",
      "username": "super_admin",
      "role": "super_admin",
      "permissions": ["manage_admins", "fraud_reviewer", "fraud_approver"],
      "isActive": true,
      "createdAt": 1735660800,
      "lastLoginAt": 1735747200
    }
  ],
  "count": 1
}
```

### 3.3 Update Admin User

**Endpoint:** `PUT /api/admin/users/{adminId}`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `manage_admins`

**Request:**
```json
{
  "role": "fraud_admin",
  "permissions": ["fraud_reviewer", "fraud_approver", "view_audit_logs"],
  "isActive": true
}
```

**Response:**
```json
{
  "success": true,
  "message": "Admin user updated successfully",
  "adminId": "admin_003"
}
```

### 3.4 Delete Admin User

**Endpoint:** `DELETE /api/admin/users/{adminId}`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `manage_admins`

**Response:**
```json
{
  "success": true,
  "message": "Admin user deleted successfully",
  "adminId": "admin_003"
}
```

### 3.5 Change Admin Password

**Endpoint:** `POST /api/admin/users/change-password`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Request:**
```json
{
  "currentPassword": "OldPassword123!",
  "newPassword": "NewPassword456!"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Password changed successfully"
}
```

---

## 4. Dashboard & Statistics

### 4.1 Get Dashboard Overview

**Endpoint:** `GET /api/admin/dashboard/overview`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Response:**
```json
{
  "success": true,
  "overview": {
    "pendingFraudReports": 5,
    "approvedFraudReports": 23,
    "rejectedFraudReports": 7,
    "executedReversals": 18,
    "infeasibleReversals": 5,
    "totalAmountReported": 2500.0,
    "totalAmountRecovered": 1200.0,
    "systemPoolBalance": 150.5,
    "activeAdmins": 4,
    "recentActivity": [
      {
        "action": "fraud_report_approved",
        "adminId": "admin_001",
        "reportId": "FR_045",
        "timestamp": 1735747200
      }
    ]
  }
}
```

### 4.2 Get Fraud Statistics

**Endpoint:** `GET /api/admin/dashboard/fraud-stats`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Query Parameters:**
- `period`: Time period (`day`, `week`, `month`, `year`, `all`)

**Response:**
```json
{
  "success": true,
  "stats": {
    "period": "month",
    "totalReports": 35,
    "pendingReports": 5,
    "approvedReports": 23,
    "rejectedReports": 7,
    "executedReversals": 18,
    "infeasibleReversals": 5,
    "totalAmountReported": 2500.0,
    "totalAmountRecovered": 1200.0,
    "recoveryRate": 0.48,
    "averageProcessingTime": 14400,
    "byStatus": {
      "PENDING": 5,
      "FACTS_APPROVED": 23,
      "FACTS_REJECTED": 7
    },
    "byExecutionStatus": {
      "NOT_STARTED": 7,
      "VALIDATING": 0,
      "EXECUTED": 18,
      "INFEASIBLE": 5
    }
  }
}
```

### 4.3 Get System Pool Status

**Endpoint:** `GET /api/admin/dashboard/pool-status`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Response:**
```json
{
  "success": true,
  "pool": {
    "address": "GXC_SYSTEM_POOL_ADDRESS",
    "currentBalance": 150.5,
    "totalFunded": 500.0,
    "totalSpent": 349.5,
    "totalReversals": 18,
    "averageFee": 19.42,
    "isBalanceLow": false,
    "fundingSources": {
      "transactionFees": 450.0,
      "reversalFees": 50.0,
      "manualFunding": 0.0
    },
    "recentDeposits": [
      {
        "amount": 0.15,
        "source": "SYSTEM_TX_FEE_SPLIT",
        "timestamp": 1735747200
      }
    ]
  }
}
```

### 4.4 Get Audit Logs

**Endpoint:** `GET /api/admin/audit/logs`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `view_audit_logs`

**Query Parameters:**
- `adminId`: Filter by admin
- `action`: Filter by action type
- `startDate`: Start date (Unix timestamp)
- `endDate`: End date (Unix timestamp)
- `page`: Page number
- `limit`: Results per page

**Response:**
```json
{
  "success": true,
  "logs": [
    {
      "logId": "LOG_001",
      "adminId": "admin_001",
      "action": "fraud_report_approved",
      "reportId": "FR_045",
      "details": "Approved fraud report for TX: abc123...",
      "timestamp": 1735747200,
      "ipAddress": "192.168.1.100"
    }
  ],
  "pagination": {
    "page": 1,
    "limit": 50,
    "total": 234,
    "pages": 5
  }
}
```

---

## 5. System Configuration

### 5.1 Get System Configuration

**Endpoint:** `GET /api/admin/system/config`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `manage_system`

**Response:**
```json
{
  "success": true,
  "config": {
    "feePoolSplitPercentage": 0.15,
    "reversalExecutionFeePercentage": 0.002,
    "minTaintThreshold": 0.1,
    "maxReversalAge": 2592000,
    "minReversalAmount": 0.01,
    "fraudDetectionEnabled": true,
    "reversalSystemEnabled": true
  }
}
```

### 5.2 Update System Configuration

**Endpoint:** `PUT /api/admin/system/config`

**Headers:**
```
Authorization: Bearer {sessionToken}
```

**Required Permission:** `manage_system` (super_admin only)

**Request:**
```json
{
  "feePoolSplitPercentage": 0.20,
  "reversalExecutionFeePercentage": 0.003
}
```

**Response:**
```json
{
  "success": true,
  "message": "System configuration updated successfully",
  "updated": {
    "feePoolSplitPercentage": 0.20,
    "reversalExecutionFeePercentage": 0.003
  }
}
```

---

## 6. Error Responses

All endpoints return consistent error responses:

```json
{
  "success": false,
  "error": "ERROR_CODE",
  "message": "Human-readable error message"
}
```

### Common Error Codes

| Code | HTTP Status | Description |
|------|-------------|-------------|
| `INVALID_CREDENTIALS` | 401 | Invalid username or password |
| `INVALID_SESSION` | 401 | Session token invalid or expired |
| `PERMISSION_DENIED` | 403 | Insufficient permissions |
| `NOT_FOUND` | 404 | Resource not found |
| `MISSING_FIELDS` | 400 | Required fields missing |
| `INVALID_INPUT` | 400 | Invalid input data |
| `SERVICE_UNAVAILABLE` | 503 | Service not available |
| `INTERNAL_ERROR` | 500 | Internal server error |

---

## 7. Rate Limiting

Admin API endpoints are rate-limited to prevent abuse:

- **Authentication endpoints**: 5 requests per minute per IP
- **Read endpoints**: 100 requests per minute per session
- **Write endpoints**: 20 requests per minute per session

**Rate Limit Headers:**
```
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
X-RateLimit-Reset: 1735747260
```

---

## 8. Webhooks (Optional)

Configure webhooks to receive real-time notifications:

### 8.1 Configure Webhook

**Endpoint:** `POST /api/admin/webhooks/configure`

**Request:**
```json
{
  "url": "https://your-dashboard.com/webhooks/fraud",
  "events": ["fraud_report_submitted", "fraud_report_approved", "reversal_executed"],
  "secret": "your_webhook_secret"
}
```

### 8.2 Webhook Events

**Event: fraud_report_submitted**
```json
{
  "event": "fraud_report_submitted",
  "timestamp": 1735747200,
  "data": {
    "reportId": "FR_046",
    "txHash": "abc123...",
    "amount": 100.0
  }
}
```

**Event: fraud_report_approved**
```json
{
  "event": "fraud_report_approved",
  "timestamp": 1735747200,
  "data": {
    "reportId": "FR_046",
    "approvedBy": "admin_001",
    "factsStatus": "FACTS_APPROVED"
  }
}
```

**Event: reversal_executed**
```json
{
  "event": "reversal_executed",
  "timestamp": 1735747200,
  "data": {
    "reportId": "FR_046",
    "recoveredAmount": 50.0,
    "executionStatus": "EXECUTED"
  }
}
```

---

## 9. Testing

### Test Credentials

**Default Admin Account:**
```
Username: admin
Password: admin123
Role: super_admin
```

**Note:** Change default credentials in production!

### Test Script

```bash
#!/bin/bash

BASE_URL="http://localhost:8080/api/admin"

# 1. Login
echo "1. Admin Login"
RESPONSE=$(curl -s -X POST "$BASE_URL/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}')

TOKEN=$(echo $RESPONSE | jq -r '.sessionToken')
echo "Session Token: $TOKEN"
echo ""

# 2. Get Pending Reports
echo "2. Get Pending Fraud Reports"
curl -s "$BASE_URL/fraud/pending" \
  -H "Authorization: Bearer $TOKEN" | jq '.'
echo ""

# 3. Get Dashboard Overview
echo "3. Get Dashboard Overview"
curl -s "$BASE_URL/dashboard/overview" \
  -H "Authorization: Bearer $TOKEN" | jq '.'
echo ""

# 4. Logout
echo "4. Admin Logout"
curl -s -X POST "$BASE_URL/logout" \
  -H "Authorization: Bearer $TOKEN" | jq '.'
```

---

## 10. Integration Examples

### React Dashboard Example

```javascript
import axios from 'axios';

class AdminAPI {
    constructor(baseURL = 'http://localhost:8080/api/admin') {
        this.baseURL = baseURL;
        this.sessionToken = localStorage.getItem('adminToken');
    }
    
    // Authentication
    async login(username, password) {
        const response = await axios.post(`${this.baseURL}/login`, {
            username,
            password
        });
        
        if (response.data.success) {
            this.sessionToken = response.data.sessionToken;
            localStorage.setItem('adminToken', this.sessionToken);
        }
        
        return response.data;
    }
    
    async logout() {
        await axios.post(`${this.baseURL}/logout`, {}, {
            headers: { 'Authorization': `Bearer ${this.sessionToken}` }
        });
        
        this.sessionToken = null;
        localStorage.removeItem('adminToken');
    }
    
    // Fraud Reports
    async getPendingReports() {
        const response = await axios.get(`${this.baseURL}/fraud/pending`, {
            headers: { 'Authorization': `Bearer ${this.sessionToken}` }
        });
        
        return response.data.reports;
    }
    
    async getReportDetails(reportId) {
        const response = await axios.get(
            `${this.baseURL}/fraud/report/${reportId}`,
            {
                headers: { 'Authorization': `Bearer ${this.sessionToken}` }
            }
        );
        
        return response.data.report;
    }
    
    async approveReport(reportId, notes) {
        const response = await axios.post(
            `${this.baseURL}/fraud/approve`,
            { reportId, notes },
            {
                headers: { 'Authorization': `Bearer ${this.sessionToken}` }
            }
        );
        
        return response.data;
    }
    
    async rejectReport(reportId, reason) {
        const response = await axios.post(
            `${this.baseURL}/fraud/reject`,
            { reportId, reason },
            {
                headers: { 'Authorization': `Bearer ${this.sessionToken}` }
            }
        );
        
        return response.data;
    }
    
    // Dashboard
    async getDashboardOverview() {
        const response = await axios.get(
            `${this.baseURL}/dashboard/overview`,
            {
                headers: { 'Authorization': `Bearer ${this.sessionToken}` }
            }
        );
        
        return response.data.overview;
    }
    
    async getFraudStats(period = 'month') {
        const response = await axios.get(
            `${this.baseURL}/dashboard/fraud-stats?period=${period}`,
            {
                headers: { 'Authorization': `Bearer ${this.sessionToken}` }
            }
        );
        
        return response.data.stats;
    }
}

export default AdminAPI;
```

### Vue.js Dashboard Example

```javascript
// store/admin.js
import axios from 'axios';

export default {
    namespaced: true,
    
    state: {
        sessionToken: localStorage.getItem('adminToken'),
        currentAdmin: null,
        pendingReports: [],
        dashboardStats: null
    },
    
    mutations: {
        SET_SESSION(state, token) {
            state.sessionToken = token;
            localStorage.setItem('adminToken', token);
        },
        
        CLEAR_SESSION(state) {
            state.sessionToken = null;
            state.currentAdmin = null;
            localStorage.removeItem('adminToken');
        },
        
        SET_CURRENT_ADMIN(state, admin) {
            state.currentAdmin = admin;
        },
        
        SET_PENDING_REPORTS(state, reports) {
            state.pendingReports = reports;
        },
        
        SET_DASHBOARD_STATS(state, stats) {
            state.dashboardStats = stats;
        }
    },
    
    actions: {
        async login({ commit }, { username, password }) {
            const response = await axios.post('/api/admin/login', {
                username,
                password
            });
            
            if (response.data.success) {
                commit('SET_SESSION', response.data.sessionToken);
                commit('SET_CURRENT_ADMIN', {
                    adminId: response.data.adminId,
                    username: response.data.username,
                    role: response.data.role,
                    permissions: response.data.permissions
                });
            }
            
            return response.data;
        },
        
        async logout({ commit, state }) {
            await axios.post('/api/admin/logout', {}, {
                headers: { 'Authorization': `Bearer ${state.sessionToken}` }
            });
            
            commit('CLEAR_SESSION');
        },
        
        async fetchPendingReports({ commit, state }) {
            const response = await axios.get('/api/admin/fraud/pending', {
                headers: { 'Authorization': `Bearer ${state.sessionToken}` }
            });
            
            commit('SET_PENDING_REPORTS', response.data.reports);
        },
        
        async fetchDashboardStats({ commit, state }) {
            const response = await axios.get('/api/admin/dashboard/overview', {
                headers: { 'Authorization': `Bearer ${state.sessionToken}` }
            });
            
            commit('SET_DASHBOARD_STATS', response.data.overview);
        },
        
        async approveReport({ state }, { reportId, notes }) {
            return await axios.post(
                '/api/admin/fraud/approve',
                { reportId, notes },
                {
                    headers: { 'Authorization': `Bearer ${state.sessionToken}` }
                }
            );
        }
    },
    
    getters: {
        isAuthenticated: state => !!state.sessionToken,
        hasPermission: state => permission => {
            return state.currentAdmin?.permissions.includes(permission);
        }
    }
};
```

---

## 11. Security Best Practices

### For Dashboard Developers

1. **Always use HTTPS** in production
2. **Store session tokens securely** (httpOnly cookies preferred)
3. **Implement CSRF protection**
4. **Validate all user inputs**
5. **Use rate limiting** on client side
6. **Implement session timeout** warnings
7. **Log all admin actions** for audit trail
8. **Use strong password policies**
9. **Implement 2FA** for admin accounts (recommended)
10. **Regular security audits**

### Session Management

```javascript
// Implement automatic session refresh
setInterval(async () => {
    try {
        const response = await axios.get('/api/admin/verify-session', {
            headers: { 'Authorization': `Bearer ${sessionToken}` }
        });
        
        if (!response.data.valid) {
            // Session expired, redirect to login
            window.location.href = '/admin/login';
        }
    } catch (error) {
        // Handle error
    }
}, 5 * 60 * 1000); // Check every 5 minutes
```

---

## Summary

This complete API reference provides everything needed to build a full admin dashboard:

- ✅ Authentication and session management
- ✅ Fraud report management (review, approve, reject)
- ✅ Admin user management
- ✅ Dashboard statistics and analytics
- ✅ System configuration
- ✅ Audit logging
- ✅ Error handling
- ✅ Rate limiting
- ✅ Integration examples (React, Vue.js)
- ✅ Security best practices

All endpoints are production-ready and follow REST API best practices.
