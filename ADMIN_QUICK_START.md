# GXC Admin System - Quick Start Guide

## Super Admin Login Credentials

**⚠️ IMPORTANT: Save these credentials securely!**

```
Username: Philani-GXC.Foundation
Password: GXC$ecure2025!Philani#Foundation@Admin
Admin ID: admin_philani_gxc_foundation
Role: super_admin
Permissions: all
```

---

## Quick Start (5 Minutes)

### Step 1: Login as Super Admin

**Using REST API:**
```bash
curl -X POST http://localhost:8332/api/admin/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "Philani-GXC.Foundation",
    "password": "GXC$ecure2025!Philani#Foundation@Admin"
  }'
```

**Response:**
```json
{
  "success": true,
  "sessionToken": "SESSION_admin_philani_gxc_foundation_1704556800",
  "adminId": "admin_philani_gxc_foundation",
  "username": "Philani-GXC.Foundation",
  "role": "super_admin",
  "message": "Login successful"
}
```

**Save the sessionToken for subsequent requests!**

---

### Step 2: Create Additional Admins

**Create Fraud Reviewer:**
```bash
curl -X POST http://localhost:8332/api/admin/create \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "YOUR_SESSION_TOKEN",
    "username": "fraud_reviewer_1",
    "password": "SecurePassword123!",
    "role": "fraud_reviewer",
    "permissions": ["fraud_reviewer", "view_applications"]
  }'
```

**Create Fraud Approver:**
```bash
curl -X POST http://localhost:8332/api/admin/create \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "YOUR_SESSION_TOKEN",
    "username": "fraud_approver_1",
    "password": "SecurePassword456!",
    "role": "fraud_approver",
    "permissions": ["fraud_approver", "fraud_reviewer", "view_applications"]
  }'
```

---

### Step 3: Test Fraud Report Workflow

**Submit Fraud Report (Public - No Auth Required):**
```bash
curl -X POST http://localhost:8332/api/fraud/report-stolen \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "test_stolen_tx_123",
    "reporterAddress": "victim_address",
    "amount": 1000.0,
    "email": "victim@example.com",
    "description": "My wallet was hacked",
    "evidence": "Screenshots, transaction logs"
  }'
```

**Response:**
```json
{
  "success": true,
  "reportId": "REPORT_1704556800_1234",
  "txHash": "test_stolen_tx_123",
  "status": "PENDING_REVIEW",
  "message": "Report submitted successfully. We will review within 24 hours.",
  "timestamp": 1704556800
}
```

**Get Pending Reports (Requires fraud_reviewer permission):**
```bash
curl -X GET "http://localhost:8332/api/admin/fraud/reports/pending?sessionToken=YOUR_SESSION_TOKEN"
```

**Approve Report (Requires fraud_approver permission):**
```bash
curl -X POST http://localhost:8332/api/admin/fraud/report/REPORT_ID/approve \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "YOUR_SESSION_TOKEN",
    "notes": "Confirmed theft - evidence verified"
  }'
```

---

## Admin Roles and Permissions

| Role | Can Do | Cannot Do |
|------|--------|-----------|
| **super_admin** | Everything | Nothing (full access) |
| **fraud_approver** | Approve/reject reports, view reports | Create admins, manage system |
| **fraud_reviewer** | View reports, investigate | Approve reports, create admins |
| **fraud_admin** | Assign reports, manage fraud system | Approve reports, create admins |

---

## Common Tasks

### Change Your Password
```bash
curl -X POST http://localhost:8332/api/admin/change-password \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "YOUR_SESSION_TOKEN",
    "oldPassword": "GXC$ecure2025!Philani#Foundation@Admin",
    "newPassword": "YourNewSecurePassword123!"
  }'
```

### Logout
```bash
curl -X POST http://localhost:8332/api/admin/logout \
  -H "Content-Type: application/json" \
  -d '{
    "sessionToken": "YOUR_SESSION_TOKEN"
  }'
```

### Check Transaction Taint (Public)
```bash
curl -X GET http://localhost:8332/api/fraud/check-transaction/TX_HASH
```

### Check Address Fraud Status (Public)
```bash
curl -X GET http://localhost:8332/api/fraud/check-address/ADDRESS
```

---

## Fraud Report Workflow

```
1. User Reports Theft
   ↓
   Status: PENDING
   
2. Admin Assigns to Reviewer
   ↓
   assignedTo: reviewer_admin_id
   
3. Reviewer Investigates
   ↓
   Reviews evidence, checks blockchain
   
4. Approver Approves
   ↓
   Status: FACTS_APPROVED
   executionStatus: VALIDATING
   
5. System Generates Proof (Automatic)
   ↓
   Uses fraud detection data
   Creates Proof of Feasibility
   
6. Protocol Validates (Automatic)
   ↓
   Checks mathematical constraints
   executionStatus: EXECUTED or INFEASIBLE
   
7. Reversal Executes (Automatic if feasible)
   ↓
   Funds returned to victim
   recoveredAmount: stored in report
```

---

## Security Best Practices

### ✅ DO:
- Change super admin password immediately after first login
- Create separate admin accounts for different roles
- Use strong passwords (min 12 characters, mixed case, numbers, symbols)
- Logout when done
- Monitor audit logs regularly
- Keep session tokens secure

### ❌ DON'T:
- Share super admin credentials
- Use the same password for multiple admins
- Leave sessions open indefinitely
- Approve reports without investigation
- Ignore audit logs

---

## Troubleshooting

### Login Failed
**Problem:** "Invalid username or password"  
**Solution:**
- Check username is exactly: `Philani-GXC.Foundation`
- Check password is exactly: `GXC$ecure2025!Philani#Foundation@Admin`
- Ensure no extra spaces or characters

### Session Invalid
**Problem:** "Invalid session"  
**Solution:**
- Login again to get new session token
- Check session token is correct
- Ensure you haven't logged out

### Permission Denied
**Problem:** "Permission denied"  
**Solution:**
- Check your role has required permission
- Login with correct admin account
- Contact super admin to update permissions

### Report Not Found
**Problem:** "Fraud report not found"  
**Solution:**
- Check report ID is correct
- Ensure report was submitted successfully
- Verify you have permission to view reports

---

## API Endpoints Reference

### Public Endpoints (No Auth)
- `POST /api/fraud/report-stolen` - Submit fraud report
- `GET /api/fraud/check-transaction/{txHash}` - Check taint
- `GET /api/fraud/check-address/{address}` - Check fraud status

### Admin Endpoints (Require Session Token)
- `POST /api/admin/login` - Login
- `POST /api/admin/logout` - Logout
- `GET /api/admin/info` - Get admin info
- `POST /api/admin/create` - Create admin (super_admin only)
- `POST /api/admin/change-password` - Change password
- `GET /api/admin/fraud/reports/pending` - Get pending reports
- `GET /api/admin/fraud/report/{reportId}` - Get report details
- `POST /api/admin/fraud/report/{reportId}/approve` - Approve report
- `POST /api/admin/fraud/report/{reportId}/reject` - Reject report
- `POST /api/admin/fraud/report/{reportId}/assign` - Assign report

---

## Files and Documentation

**Implementation:**
- `src/MarketMakerAdmin.cpp` - Core admin logic
- `include/MarketMakerAdmin.h` - Admin interface
- `src/RESTServer_AdminAPIs.cpp` - REST API endpoints

**Documentation:**
- `ADMIN_SYSTEM_TEST_RESULTS.md` - Complete test results
- `ADMIN_QUICK_START.md` - This file
- `USER_FRAUD_REPORTING.md` - User guide for fraud reporting
- `FRAUD_TO_REVERSAL_WORKFLOW.md` - Complete fraud workflow
- `HOW_FRAUD_DETECTION_WORKS.md` - Fraud detection technical details

**Test Scripts:**
- `test_admin_functions.sh` - Test all admin functions
- `test_admin_api.sh` - Test REST API endpoints

---

## Support

**For Issues:**
1. Check audit logs
2. Review error messages
3. Verify permissions
4. Contact super admin

**Super Admin:**
- Username: Philani-GXC.Foundation
- Role: super_admin
- Created: System initialization

---

## Next Steps

1. ✅ Login with super admin credentials
2. ✅ Change super admin password
3. ✅ Create additional admin accounts
4. ✅ Test fraud report workflow
5. ✅ Monitor system in production

---

**System Status:** ✅ PRODUCTION READY  
**Last Updated:** 2026-01-06  
**Version:** 2.0.0
