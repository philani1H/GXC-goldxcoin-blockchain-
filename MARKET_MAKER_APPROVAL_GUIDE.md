# Market Maker Approval Guide

## Overview

The GXC blockchain has a comprehensive market maker verification and approval system. This guide covers the complete process from application submission to final approval.

## Market Maker Approval Process

### Process Flow

```
1. Application Submitted
   ↓
2. Assigned to Reviewer
   ↓
3. Five Verification Steps (in parallel or sequence)
   - License Verification
   - Company Reputation Check
   - Financial Standing Review
   - Technical Capabilities Verification
   - KYC/AML Compliance Check
   ↓
4. All Steps Passed?
   ├─ YES → Super Admin Approves → APPROVED
   └─ NO → Super Admin Rejects → REJECTED
```

### Application Statuses

| Status | Description |
|--------|-------------|
| `PENDING` | Application submitted, awaiting review |
| `UNDER_REVIEW` | Assigned to reviewer, verification in progress |
| `ADDITIONAL_INFO_REQUIRED` | More information needed from applicant |
| `APPROVED` | All checks passed, market maker created |
| `REJECTED` | Application denied |
| `REVOKED` | Previously approved, now revoked |

### Verification Steps

Each application must pass 5 verification steps:

1. **LICENSE_VERIFIED** - License verified with regulatory body
2. **REPUTATION_CHECKED** - Company reputation verified
3. **FINANCIAL_REVIEWED** - Financial standing assessed
4. **TECHNICAL_VERIFIED** - Technical capabilities confirmed
5. **KYC_AML_COMPLETED** - KYC/AML compliance verified

---

## API Endpoints

### Base URL

```
http://localhost:8080/api/admin
```

### Authentication

All endpoints require admin session token:

```
Authorization: Bearer {sessionToken}
```

---

## 1. Application Management

### 1.1 Get All Applications

**Endpoint:** `GET /api/admin/market-makers/applications`

**Query Parameters:**
- `status`: Filter by status (`PENDING`, `UNDER_REVIEW`, `APPROVED`, `REJECTED`)
- `page`: Page number (default: 1)
- `limit`: Results per page (default: 20)

**Required Permission:** `reviewer` or `verifier`

**Request:**
```bash
curl -X GET "http://localhost:8080/api/admin/market-makers/applications?status=PENDING" \
  -H "Authorization: Bearer {sessionToken}"
```

**Response:**
```json
{
  "success": true,
  "applications": [
    {
      "applicationId": "APP_001",
      "companyName": "ABC Trading Ltd",
      "contactEmail": "contact@abctrading.com",
      "licenseNumber": "MM-2025-001",
      "country": "United States",
      "status": "PENDING",
      "submittedAt": 1735660800,
      "assignedTo": "",
      "verificationSteps": [
        {
          "step": "LICENSE_VERIFIED",
          "status": "PENDING",
          "verifiedBy": "",
          "verifiedAt": 0,
          "notes": "",
          "proofDocument": ""
        }
      ]
    }
  ],
  "pagination": {
    "page": 1,
    "limit": 20,
    "total": 5,
    "pages": 1
  }
}
```

### 1.2 Get Application Details

**Endpoint:** `GET /api/admin/market-makers/application/{applicationId}`

**Required Permission:** `reviewer` or `verifier`

**Response:**
```json
{
  "success": true,
  "application": {
    "applicationId": "APP_001",
    "companyName": "ABC Trading Ltd",
    "contactEmail": "contact@abctrading.com",
    "contactPhone": "+1-555-0123",
    "licenseNumber": "MM-2025-001",
    "country": "United States",
    "tradingVolume": 1000000.0,
    "yearsInBusiness": 5,
    "technicalInfrastructure": "AWS-based trading platform with 99.9% uptime",
    "status": "UNDER_REVIEW",
    "submittedAt": 1735660800,
    "assignedTo": "admin_002",
    "verificationSteps": [
      {
        "step": "LICENSE_VERIFIED",
        "status": "PASSED",
        "verifiedBy": "admin_003",
        "verifiedAt": 1735664400,
        "notes": "License verified with SEC. Valid until 2026.",
        "proofDocument": "hash_of_license_document"
      },
      {
        "step": "REPUTATION_CHECKED",
        "status": "PENDING",
        "verifiedBy": "",
        "verifiedAt": 0,
        "notes": "",
        "proofDocument": ""
      }
    ],
    "documents": [
      {
        "type": "LICENSE",
        "hash": "doc_hash_123",
        "uploadedAt": 1735660800
      },
      {
        "type": "FINANCIAL_STATEMENT",
        "hash": "doc_hash_456",
        "uploadedAt": 1735660800
      }
    ]
  }
}
```

### 1.3 Assign Application to Reviewer

**Endpoint:** `POST /api/admin/market-makers/assign`

**Required Permission:** `reviewer`

**Request:**
```json
{
  "applicationId": "APP_001",
  "reviewerAdminId": "admin_002"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Application assigned successfully",
  "applicationId": "APP_001",
  "assignedTo": "admin_002"
}
```

---

## 2. Verification Steps

### 2.1 Verify License

**Endpoint:** `POST /api/admin/market-makers/verify-license`

**Required Permission:** `verifier`

**Request:**
```json
{
  "applicationId": "APP_001",
  "passed": true,
  "notes": "License verified with SEC. Valid until 2026. License number MM-2025-001 confirmed.",
  "proofDocumentHash": "hash_of_verification_document"
}
```

**Response:**
```json
{
  "success": true,
  "message": "License verification completed",
  "applicationId": "APP_001",
  "step": "LICENSE_VERIFIED",
  "status": "PASSED"
}
```

### 2.2 Check Company Reputation

**Endpoint:** `POST /api/admin/market-makers/check-reputation`

**Required Permission:** `verifier`

**Request:**
```json
{
  "applicationId": "APP_001",
  "passed": true,
  "notes": "Company has excellent reputation. No complaints found. BBB rating: A+",
  "proofDocumentHash": "hash_of_reputation_report"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Reputation check completed",
  "applicationId": "APP_001",
  "step": "REPUTATION_CHECKED",
  "status": "PASSED"
}
```

### 2.3 Review Financial Standing

**Endpoint:** `POST /api/admin/market-makers/review-financial`

**Required Permission:** `verifier`

**Request:**
```json
{
  "applicationId": "APP_001",
  "passed": true,
  "notes": "Financial statements reviewed. Company has $5M in liquid assets. Debt-to-equity ratio: 0.3",
  "proofDocumentHash": "hash_of_financial_analysis"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Financial review completed",
  "applicationId": "APP_001",
  "step": "FINANCIAL_REVIEWED",
  "status": "PASSED"
}
```

### 2.4 Verify Technical Capabilities

**Endpoint:** `POST /api/admin/market-makers/verify-technical`

**Required Permission:** `verifier`

**Request:**
```json
{
  "applicationId": "APP_001",
  "passed": true,
  "notes": "Technical infrastructure verified. AWS-based platform with 99.9% uptime. API latency < 50ms.",
  "proofDocumentHash": "hash_of_technical_audit"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Technical verification completed",
  "applicationId": "APP_001",
  "step": "TECHNICAL_VERIFIED",
  "status": "PASSED"
}
```

### 2.5 Complete KYC/AML Check

**Endpoint:** `POST /api/admin/market-makers/complete-kyc-aml`

**Required Permission:** `verifier`

**Request:**
```json
{
  "applicationId": "APP_001",
  "passed": true,
  "notes": "KYC/AML compliance verified. All directors passed background checks. AML procedures in place.",
  "proofDocumentHash": "hash_of_kyc_aml_report"
}
```

**Response:**
```json
{
  "success": true,
  "message": "KYC/AML check completed",
  "applicationId": "APP_001",
  "step": "KYC_AML_COMPLETED",
  "status": "PASSED"
}
```

---

## 3. Approval/Rejection

### 3.1 Approve Application

**Endpoint:** `POST /api/admin/market-makers/approve`

**Required Permission:** `super_admin` only

**Prerequisites:**
- All 5 verification steps must be PASSED
- Application status must be UNDER_REVIEW

**Request:**
```json
{
  "applicationId": "APP_001",
  "notes": "All verification steps passed. Application approved. Market maker created."
}
```

**Response:**
```json
{
  "success": true,
  "message": "Market maker application approved",
  "applicationId": "APP_001",
  "marketMakerId": "MM_001",
  "status": "APPROVED",
  "approvedAt": 1735747200,
  "note": "Market maker has been created and can now operate on the platform"
}
```

**What Happens:**
1. Application status changed to APPROVED
2. Market maker created in registry
3. Market maker ID assigned
4. Permissions granted
5. Audit log entry created

### 3.2 Reject Application

**Endpoint:** `POST /api/admin/market-makers/reject`

**Required Permission:** `super_admin` only

**Request:**
```json
{
  "applicationId": "APP_001",
  "reason": "Failed financial standing review. Insufficient liquid assets."
}
```

**Response:**
```json
{
  "success": true,
  "message": "Market maker application rejected",
  "applicationId": "APP_001",
  "status": "REJECTED",
  "rejectedAt": 1735747200
}
```

### 3.3 Request Additional Information

**Endpoint:** `POST /api/admin/market-makers/request-info`

**Required Permission:** `verifier`

**Request:**
```json
{
  "applicationId": "APP_001",
  "requestedInfo": "Please provide updated financial statements for Q4 2024 and proof of insurance coverage."
}
```

**Response:**
```json
{
  "success": true,
  "message": "Additional information requested",
  "applicationId": "APP_001",
  "status": "ADDITIONAL_INFO_REQUIRED"
}
```

---

## 4. Market Maker Management

### 4.1 Get Approved Market Makers

**Endpoint:** `GET /api/admin/market-makers/approved`

**Required Permission:** `reviewer`

**Response:**
```json
{
  "success": true,
  "marketMakers": [
    {
      "marketMakerId": "MM_001",
      "companyName": "ABC Trading Ltd",
      "status": "ACTIVE",
      "approvedAt": 1735747200,
      "tradingVolume": 1000000.0,
      "lastActivity": 1735833600
    }
  ],
  "count": 1
}
```

### 4.2 Revoke Market Maker

**Endpoint:** `POST /api/admin/market-makers/revoke`

**Required Permission:** `super_admin` only

**Request:**
```json
{
  "marketMakerId": "MM_001",
  "reason": "Violation of trading rules. Multiple instances of market manipulation detected."
}
```

**Response:**
```json
{
  "success": true,
  "message": "Market maker authorization revoked",
  "marketMakerId": "MM_001",
  "status": "REVOKED",
  "revokedAt": 1735833600
}
```

**What Happens:**
1. Market maker status changed to REVOKED
2. Trading permissions removed
3. All open orders cancelled
4. Audit log entry created
5. Notification sent to market maker

### 4.3 Get Market Maker History

**Endpoint:** `GET /api/admin/market-makers/{marketMakerId}/history`

**Required Permission:** `reviewer`

**Response:**
```json
{
  "success": true,
  "history": [
    {
      "action": "APPLICATION_SUBMITTED",
      "timestamp": 1735660800,
      "details": "Application submitted"
    },
    {
      "action": "LICENSE_VERIFIED",
      "timestamp": 1735664400,
      "verifiedBy": "admin_003",
      "details": "License verified with SEC"
    },
    {
      "action": "APPLICATION_APPROVED",
      "timestamp": 1735747200,
      "approvedBy": "admin_001",
      "details": "All verification steps passed"
    }
  ]
}
```

---

## 5. Statistics and Reporting

### 5.1 Get Verification Statistics

**Endpoint:** `GET /api/admin/market-makers/statistics`

**Required Permission:** `reviewer`

**Response:**
```json
{
  "success": true,
  "statistics": {
    "totalApplications": 50,
    "pending": 5,
    "underReview": 10,
    "approved": 30,
    "rejected": 5,
    "averageProcessingTime": 172800,
    "byVerificationStep": {
      "LICENSE_VERIFIED": {
        "passed": 40,
        "failed": 5,
        "pending": 5
      },
      "REPUTATION_CHECKED": {
        "passed": 38,
        "failed": 7,
        "pending": 5
      }
    },
    "byCountry": {
      "United States": 20,
      "United Kingdom": 15,
      "Singapore": 10,
      "Other": 5
    }
  }
}
```

### 5.2 Get Audit Log

**Endpoint:** `GET /api/admin/market-makers/audit-log`

**Required Permission:** `super_admin`

**Query Parameters:**
- `applicationId`: Filter by application
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
      "applicationId": "APP_001",
      "adminId": "admin_003",
      "action": "LICENSE_VERIFIED",
      "details": "License verified with SEC. Valid until 2026.",
      "timestamp": 1735664400,
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

## 6. Admin Roles and Permissions

### Role Hierarchy

| Role | Permissions | Can Do |
|------|-------------|--------|
| `super_admin` | All permissions | Everything |
| `verifier` | Verify steps, request info | Perform verification steps |
| `reviewer` | View applications, assign | Review and assign applications |

### Permission Matrix

| Action | super_admin | verifier | reviewer |
|--------|-------------|----------|----------|
| View applications | ✅ | ✅ | ✅ |
| Assign to reviewer | ✅ | ✅ | ✅ |
| Verify license | ✅ | ✅ | ❌ |
| Check reputation | ✅ | ✅ | ❌ |
| Review financial | ✅ | ✅ | ❌ |
| Verify technical | ✅ | ✅ | ❌ |
| Complete KYC/AML | ✅ | ✅ | ❌ |
| Request info | ✅ | ✅ | ❌ |
| Approve application | ✅ | ❌ | ❌ |
| Reject application | ✅ | ❌ | ❌ |
| Revoke market maker | ✅ | ❌ | ❌ |
| View audit log | ✅ | ❌ | ❌ |

---

## 7. Complete Workflow Example

### Step-by-Step Process

```bash
#!/bin/bash

BASE_URL="http://localhost:8080/api/admin"
TOKEN="your_session_token"

# Step 1: Get pending applications
echo "Step 1: Get pending applications"
curl -X GET "$BASE_URL/market-makers/applications?status=PENDING" \
  -H "Authorization: Bearer $TOKEN"

# Step 2: Assign to reviewer
echo "Step 2: Assign to reviewer"
curl -X POST "$BASE_URL/market-makers/assign" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "reviewerAdminId": "admin_002"
  }'

# Step 3: Verify license
echo "Step 3: Verify license"
curl -X POST "$BASE_URL/market-makers/verify-license" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "passed": true,
    "notes": "License verified with SEC",
    "proofDocumentHash": "hash_123"
  }'

# Step 4: Check reputation
echo "Step 4: Check reputation"
curl -X POST "$BASE_URL/market-makers/check-reputation" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "passed": true,
    "notes": "Excellent reputation verified",
    "proofDocumentHash": "hash_456"
  }'

# Step 5: Review financial
echo "Step 5: Review financial"
curl -X POST "$BASE_URL/market-makers/review-financial" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "passed": true,
    "notes": "Strong financial standing",
    "proofDocumentHash": "hash_789"
  }'

# Step 6: Verify technical
echo "Step 6: Verify technical"
curl -X POST "$BASE_URL/market-makers/verify-technical" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "passed": true,
    "notes": "Technical infrastructure verified",
    "proofDocumentHash": "hash_abc"
  }'

# Step 7: Complete KYC/AML
echo "Step 7: Complete KYC/AML"
curl -X POST "$BASE_URL/market-makers/complete-kyc-aml" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "passed": true,
    "notes": "KYC/AML compliance verified",
    "proofDocumentHash": "hash_def"
  }'

# Step 8: Approve application (super_admin only)
echo "Step 8: Approve application"
curl -X POST "$BASE_URL/market-makers/approve" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_001",
    "notes": "All verification steps passed. Approved."
  }'
```

---

## 8. Dashboard Integration

### React Component Example

```javascript
import React, { useState, useEffect } from 'react';
import AdminAPI from '../api/AdminAPI';

function MarketMakerApplications() {
    const [applications, setApplications] = useState([]);
    const [selectedApp, setSelectedApp] = useState(null);
    
    useEffect(() => {
        loadApplications();
    }, []);
    
    const loadApplications = async () => {
        const data = await AdminAPI.getMarketMakerApplications('PENDING');
        setApplications(data);
    };
    
    const verifyLicense = async (applicationId) => {
        const result = await AdminAPI.verifyLicense(
            applicationId,
            true,
            "License verified with SEC",
            "hash_123"
        );
        
        if (result.success) {
            alert('License verified successfully');
            loadApplications();
        }
    };
    
    const approveApplication = async (applicationId) => {
        const result = await AdminAPI.approveMarketMaker(
            applicationId,
            "All verification steps passed"
        );
        
        if (result.success) {
            alert('Market maker approved!');
            loadApplications();
        }
    };
    
    return (
        <div className="market-maker-applications">
            <h1>Market Maker Applications</h1>
            
            <table>
                <thead>
                    <tr>
                        <th>Application ID</th>
                        <th>Company</th>
                        <th>Status</th>
                        <th>Progress</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                    {applications.map(app => (
                        <tr key={app.applicationId}>
                            <td>{app.applicationId}</td>
                            <td>{app.companyName}</td>
                            <td>{app.status}</td>
                            <td>
                                {app.verificationSteps.filter(s => s.status === 'PASSED').length} / 5
                            </td>
                            <td>
                                <button onClick={() => setSelectedApp(app)}>
                                    Review
                                </button>
                            </td>
                        </tr>
                    ))}
                </tbody>
            </table>
            
            {selectedApp && (
                <ApplicationDetail
                    application={selectedApp}
                    onVerifyLicense={verifyLicense}
                    onApprove={approveApplication}
                    onClose={() => setSelectedApp(null)}
                />
            )}
        </div>
    );
}

export default MarketMakerApplications;
```

---

## Summary

### Complete Market Maker Approval Process

1. **Application Submitted** → System receives application
2. **Assigned to Reviewer** → Admin assigns to verifier
3. **5 Verification Steps** → Each step verified independently
4. **All Steps Passed** → Ready for approval
5. **Super Admin Approves** → Market maker created
6. **Market Maker Active** → Can now trade on platform

### Key Points

- ✅ 5 independent verification steps
- ✅ Role-based permissions (super_admin, verifier, reviewer)
- ✅ Complete audit trail
- ✅ Can request additional information
- ✅ Can revoke approved market makers
- ✅ Comprehensive statistics and reporting

### API Endpoints Summary

| Category | Endpoints | Count |
|----------|-----------|-------|
| Applications | list, details, assign | 3 |
| Verification | 5 steps (license, reputation, financial, technical, KYC/AML) | 5 |
| Approval | approve, reject, request-info | 3 |
| Management | approved list, revoke, history | 3 |
| Reporting | statistics, audit-log | 2 |

**Total:** 16 market maker endpoints

All endpoints are production-ready and fully documented!
