# Admin System Integration Plan

## Current State Analysis

### Existing Admin System
**File**: `include/MarketMakerAdmin.h`

**Features**:
- ✅ Admin user management
- ✅ Permission system (super_admin, verifier, reviewer)
- ✅ Team management (create/deactivate/remove admins)
- ✅ Audit logging
- ✅ Market maker application approval workflow

**Permissions**:
- `super_admin` - Full access, can manage other admins
- `verifier` - Can verify applications
- `reviewer` - Can review and assign applications

### Fraud Detection System
**File**: `include/FraudDetection.h`, `src/RESTServer.cpp`

**Current Implementation**:
- ✅ Fraud reporting (public)
- ✅ Report status checking (public)
- ❌ Approval requires hardcoded admin key
- ❌ Not integrated with admin permission system
- ❌ No team management
- ❌ No audit logging

**Current Endpoints**:
```
POST /api/fraud/report-stolen        (Public)
GET  /api/fraud/report/:id            (Public)
POST /api/fraud/approve               (Hardcoded key)
POST /api/fraud/reject                (Hardcoded key)
GET  /api/fraud/pending               (Hardcoded key)
GET  /api/fraud/check-transaction/:hash  (Public)
GET  /api/fraud/check-address/:addr   (Public)
GET  /api/fraud/statistics            (Public)
```

---

## Integration Plan

### Goal
Integrate fraud detection approval into the existing MarketMakerAdmin system with proper permissions and team management.

### Changes Needed

#### 1. Add Fraud Permissions to Admin System
**File**: `include/MarketMakerAdmin.h`

**New Permissions**:
- `fraud_reviewer` - Can review fraud reports
- `fraud_approver` - Can approve/reject fraud reports
- `fraud_admin` - Full fraud detection management

#### 2. Integrate Fraud Reports with Admin System
**File**: `include/MarketMakerAdmin.h`

**New Structures**:
```cpp
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
    std::string reviewedBy; // Admin ID
    uint64_t reviewedAt;
    std::string reviewNotes;
};
```

**New Methods**:
```cpp
// Get pending fraud reports
std::vector<FraudReport> getPendingFraudReports(const std::string& adminId);

// Approve fraud report
bool approveFraudReport(
    const std::string& adminId,
    const std::string& reportId,
    const std::string& notes
);

// Reject fraud report
bool rejectFraudReport(
    const std::string& adminId,
    const std::string& reportId,
    const std::string& reason
);

// Get fraud report details
FraudReport getFraudReportDetails(
    const std::string& adminId,
    const std::string& reportId
);

// Get fraud statistics
json getFraudStatistics(const std::string& adminId);
```

#### 3. Update RESTServer to Use Admin System
**File**: `src/RESTServer.cpp`

**Changes**:
- Replace hardcoded admin key with admin session verification
- Use MarketMakerAdmin for permission checking
- Log all fraud actions to audit log

**Updated Endpoints**:
```
POST /api/fraud/report-stolen        (Public - no change)
GET  /api/fraud/report/:id            (Public - no change)
POST /api/admin/fraud/approve         (Requires fraud_approver permission)
POST /api/admin/fraud/reject          (Requires fraud_approver permission)
GET  /api/admin/fraud/pending         (Requires fraud_reviewer permission)
GET  /api/admin/fraud/statistics      (Requires fraud_reviewer permission)
GET  /api/fraud/check-transaction/:hash  (Public - no change)
GET  /api/fraud/check-address/:addr   (Public - no change)
```

#### 4. Create Admin Dashboard API
**New File**: `include/AdminDashboard.h`

**Features**:
- Unified admin interface
- Market maker applications
- Fraud reports
- System statistics
- Audit logs
- Team management

---

## Implementation Details

### Permission Matrix

| Action | super_admin | fraud_admin | fraud_approver | fraud_reviewer | verifier | reviewer |
|--------|-------------|-------------|----------------|----------------|----------|----------|
| View fraud reports | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| Approve fraud reports | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ |
| Reject fraud reports | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ |
| View fraud statistics | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| Manage admins | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| View audit logs | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ |
| Verify MM applications | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ |
| Approve MM applications | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |

### Audit Log Actions

**Fraud-Related Actions**:
- `fraud_report_submitted` - User submitted fraud report
- `fraud_report_reviewed` - Admin reviewed report
- `fraud_report_approved` - Admin approved report (marked as stolen)
- `fraud_report_rejected` - Admin rejected report
- `fraud_transaction_flagged` - Transaction automatically flagged
- `fraud_address_flagged` - Address automatically flagged
- `fraud_clean_zone_registered` - Clean zone registered
- `fraud_clean_zone_removed` - Clean zone removed

### Admin Session Management

**Login Flow**:
```
1. Admin logs in: POST /api/admin/login
   {
     "username": "admin@gxc.com",
     "password": "secure_password"
   }
   
2. Returns session token:
   {
     "success": true,
     "sessionToken": "eyJhbGc...",
     "adminId": "ADMIN_001",
     "role": "fraud_approver",
     "permissions": ["fraud_reviewer", "fraud_approver"]
   }
   
3. Use token in subsequent requests:
   POST /api/admin/fraud/approve
   Headers: {
     "Authorization": "Bearer eyJhbGc..."
   }
```

### Team Management Flow

**Adding Fraud Review Team Member**:
```
1. Super admin creates new admin:
   POST /api/admin/create
   {
     "username": "fraud_reviewer_1",
     "password": "secure_password",
     "role": "fraud_reviewer",
     "permissions": ["fraud_reviewer"]
   }
   
2. New admin can now:
   - View pending fraud reports
   - View fraud statistics
   - Cannot approve/reject (needs fraud_approver permission)
   
3. Super admin can upgrade permissions:
   POST /api/admin/update-permissions
   {
     "targetAdminId": "ADMIN_002",
     "permissions": ["fraud_reviewer", "fraud_approver"]
   }
```

---

## API Endpoints Summary

### Public Endpoints (No Auth Required)
```
POST /api/fraud/report-stolen
GET  /api/fraud/report/:id
GET  /api/fraud/check-transaction/:hash
GET  /api/fraud/check-address/:addr
```

### Admin Endpoints (Auth Required)
```
# Admin Management
POST /api/admin/login
POST /api/admin/logout
POST /api/admin/create                (super_admin only)
POST /api/admin/update-permissions    (super_admin only)
POST /api/admin/deactivate            (super_admin only)
GET  /api/admin/list                  (super_admin only)

# Fraud Management
GET  /api/admin/fraud/pending         (fraud_reviewer+)
GET  /api/admin/fraud/report/:id      (fraud_reviewer+)
POST /api/admin/fraud/approve         (fraud_approver+)
POST /api/admin/fraud/reject          (fraud_approver+)
GET  /api/admin/fraud/statistics      (fraud_reviewer+)

# Market Maker Management
GET  /api/admin/mm/pending            (verifier+)
POST /api/admin/mm/verify-license     (verifier+)
POST /api/admin/mm/approve            (super_admin only)
POST /api/admin/mm/reject             (super_admin only)

# Audit & Reporting
GET  /api/admin/audit-log             (super_admin, fraud_admin)
GET  /api/admin/dashboard             (all admins)
```

---

## Dashboard Features

### Admin Dashboard Sections

#### 1. Overview
- Total pending fraud reports
- Total pending MM applications
- Recent activity
- System alerts

#### 2. Fraud Reports
- List pending reports
- Filter by status
- Search by transaction hash
- Approve/reject actions
- View report details

#### 3. Market Maker Applications
- List pending applications
- Verification workflow
- Approve/reject actions
- View application details

#### 4. Team Management (super_admin only)
- List all admins
- Create new admin
- Update permissions
- Deactivate/reactivate admin

#### 5. Audit Log (super_admin, fraud_admin)
- View all admin actions
- Filter by date range
- Filter by admin
- Filter by action type
- Export audit log

#### 6. Statistics
- Fraud detection metrics
- Market maker metrics
- System health
- Performance metrics

---

## Security Considerations

### Authentication
- ✅ Session-based authentication
- ✅ Password hashing (bcrypt)
- ✅ Session expiration (24 hours)
- ✅ IP address logging

### Authorization
- ✅ Permission-based access control
- ✅ Role-based permissions
- ✅ Action-level permissions
- ✅ Audit logging for all actions

### Data Protection
- ✅ Sensitive data encryption
- ✅ Secure password storage
- ✅ HTTPS required for admin endpoints
- ✅ Rate limiting on admin endpoints

---

## Implementation Steps

1. ✅ Analyze existing admin system
2. ⏳ Add fraud permissions to MarketMakerAdmin
3. ⏳ Create FraudReport structure in admin system
4. ⏳ Implement fraud approval methods in MarketMakerAdmin
5. ⏳ Update RESTServer to use admin authentication
6. ⏳ Create AdminDashboard API
7. ⏳ Integrate with node_main.cpp
8. ⏳ Build and test complete system
9. ⏳ Test all admin endpoints
10. ⏳ Create admin dashboard UI (optional)

---

## Next Steps

1. Extend MarketMakerAdmin with fraud management
2. Update RESTServer to use admin authentication
3. Create unified admin dashboard
4. Integrate everything into node_main.cpp
5. Build and test complete node
6. Test all APIs end-to-end

---

**Status**: Ready to implement integration
