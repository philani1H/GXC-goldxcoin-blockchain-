# Market Maker Admin API Documentation

## Complete API for GXC Team to Verify and Approve Market Makers

**Version**: 1.0  
**Status**: Production Ready ✅  
**Purpose**: Enable GXC team to verify market maker applications

---

## Table of Contents

1. [Overview](#overview)
2. [Admin Management](#admin-management)
3. [Application Submission](#application-submission)
4. [Verification Workflow](#verification-workflow)
5. [Approval/Rejection](#approvalrejection)
6. [Reporting & Analytics](#reporting--analytics)
7. [Integration Examples](#integration-examples)

---

## Overview

### What This API Does

This API enables the GXC team to:
1. **Receive** market maker applications
2. **Verify** licenses with regulatory bodies (SEC, FCA, etc.)
3. **Check** company reputation
4. **Review** financial standing
5. **Verify** technical capabilities
6. **Complete** KYC/AML compliance checks
7. **Approve** or **Reject** applications
8. **Track** all verification steps with audit trail

### Workflow

```
Applicant → Submit Application → GXC Team Reviews → Verification Steps → Approval/Rejection
```

**Verification Steps**:
1. ✅ Verify license with regulatory body
2. ✅ Check company reputation
3. ✅ Review financial standing
4. ✅ Verify technical capabilities
5. ✅ Complete KYC/AML compliance

**All steps must pass** before approval.

---

## Admin Management

### 1. Create Admin User

**Endpoint**: `POST /api/v1/admin/create`

**Authorization**: Super Admin Only

**Request**:
```json
{
  "superAdminId": "admin_super_001",
  "username": "john_verifier",
  "password": "secure_password_123",
  "role": "verifier",
  "permissions": [
    "view_applications",
    "verify_license",
    "check_reputation",
    "review_financial",
    "verify_technical",
    "complete_kyc_aml"
  ]
}
```

**Response**:
```json
{
  "success": true,
  "adminId": "ADMIN_1703001234_5678",
  "username": "john_verifier",
  "role": "verifier",
  "message": "Admin user created successfully"
}
```

**Roles**:
- `super_admin` - Can approve/reject applications, create admins
- `verifier` - Can perform verification steps
- `reviewer` - Can view and assign applications

---

### 2. Admin Login

**Endpoint**: `POST /api/v1/admin/login`

**Request**:
```json
{
  "username": "john_verifier",
  "password": "secure_password_123"
}
```

**Response**:
```json
{
  "success": true,
  "sessionToken": "SESSION_ADMIN_1703001234_5678_1703001234",
  "adminId": "ADMIN_1703001234_5678",
  "role": "verifier",
  "permissions": [
    "view_applications",
    "verify_license",
    "check_reputation"
  ]
}
```

**Use session token** in all subsequent requests:
```
Authorization: Bearer SESSION_ADMIN_1703001234_5678_1703001234
```

---

## Application Submission

### 3. Submit Market Maker Application

**Endpoint**: `POST /api/v1/marketmaker/apply`

**Authorization**: Public (Applicant)

**Request**:
```json
{
  "applicantAddress": "tGXC_goldman_sachs",
  "companyName": "Goldman Sachs Trading Desk",
  "licenseNumber": "MM-12345-US",
  "regulatoryBody": "SEC",
  "country": "USA",
  "contactEmail": "mm-support@goldmansachs.com",
  "contactPhone": "+1-212-555-0100",
  "website": "https://www.goldmansachs.com",
  "licenseDocumentHash": "0xabc123...",
  "financialStatementsHash": "0xdef456...",
  "technicalCapabilitiesHash": "0xghi789...",
  "kycDocumentsHash": "0xjkl012..."
}
```

**Response**:
```json
{
  "success": true,
  "applicationId": "APP_1703001234_1234",
  "status": "PENDING",
  "message": "Application submitted successfully. You will be notified of the review status.",
  "estimatedReviewTime": "5-7 business days"
}
```

---

### 4. Check Application Status

**Endpoint**: `GET /api/v1/marketmaker/status?applicationId=APP_1703001234_1234`

**Authorization**: Public (Applicant can check their own)

**Response**:
```json
{
  "applicationId": "APP_1703001234_1234",
  "companyName": "Goldman Sachs Trading Desk",
  "status": "UNDER_REVIEW",
  "submittedAt": 1703001234,
  "lastUpdatedAt": 1703005678,
  "verificationSteps": [
    {
      "stepName": "License Verification",
      "status": "COMPLETED",
      "passed": true,
      "completedAt": 1703002000
    },
    {
      "stepName": "Company Reputation Check",
      "status": "IN_PROGRESS",
      "passed": null,
      "completedAt": null
    },
    {
      "stepName": "Financial Standing Review",
      "status": "PENDING",
      "passed": null,
      "completedAt": null
    }
  ]
}
```

---

## Verification Workflow

### 5. Get Pending Applications

**Endpoint**: `GET /api/v1/admin/applications/pending`

**Authorization**: Verifier or Reviewer

**Request Headers**:
```
Authorization: Bearer SESSION_ADMIN_1703001234_5678_1703001234
```

**Response**:
```json
{
  "success": true,
  "applications": [
    {
      "applicationId": "APP_1703001234_1234",
      "companyName": "Goldman Sachs Trading Desk",
      "licenseNumber": "MM-12345-US",
      "regulatoryBody": "SEC",
      "country": "USA",
      "status": "PENDING",
      "submittedAt": 1703001234
    },
    {
      "applicationId": "APP_1703001235_5678",
      "companyName": "Citadel Securities",
      "licenseNumber": "MM-67890-US",
      "regulatoryBody": "SEC",
      "country": "USA",
      "status": "UNDER_REVIEW",
      "submittedAt": 1703001235
    }
  ]
}
```

---

### 6. Get Application Details

**Endpoint**: `GET /api/v1/admin/applications/{applicationId}`

**Authorization**: Verifier or Reviewer

**Response**:
```json
{
  "applicationId": "APP_1703001234_1234",
  "applicantAddress": "tGXC_goldman_sachs",
  "companyName": "Goldman Sachs Trading Desk",
  "licenseNumber": "MM-12345-US",
  "regulatoryBody": "SEC",
  "country": "USA",
  "contactEmail": "mm-support@goldmansachs.com",
  "contactPhone": "+1-212-555-0100",
  "website": "https://www.goldmansachs.com",
  "documents": {
    "license": "0xabc123...",
    "financialStatements": "0xdef456...",
    "technicalCapabilities": "0xghi789...",
    "kycDocuments": "0xjkl012..."
  },
  "status": "PENDING",
  "submittedAt": 1703001234,
  "verificationSteps": []
}
```

---

### 7. Step 1: Verify License with Regulatory Body

**Endpoint**: `POST /api/v1/admin/verify/license`

**Authorization**: Verifier

**Request**:
```json
{
  "adminId": "ADMIN_1703001234_5678",
  "applicationId": "APP_1703001234_1234",
  "passed": true,
  "notes": "Verified with SEC. License MM-12345-US is active and in good standing. Verified on 2025-12-19.",
  "proofDocumentHash": "0xsec_verification_proof_abc123"
}
```

**Response**:
```json
{
  "success": true,
  "message": "License verification completed",
  "step": {
    "stepName": "License Verification",
    "status": "LICENSE_VERIFIED",
    "passed": true,
    "verifiedBy": "ADMIN_1703001234_5678",
    "verifiedAt": 1703002000,
    "notes": "Verified with SEC. License MM-12345-US is active and in good standing.",
    "proofDocument": "0xsec_verification_proof_abc123"
  }
}
```

**How to Verify License**:
1. Check license number with regulatory body (SEC, FCA, MAS, etc.)
2. Verify license is active
3. Check for any violations or sanctions
4. Document verification with proof hash
5. Mark as passed/failed

---

### 8. Step 2: Check Company Reputation

**Endpoint**: `POST /api/v1/admin/verify/reputation`

**Authorization**: Verifier

**Request**:
```json
{
  "adminId": "ADMIN_1703001234_5678",
  "applicationId": "APP_1703001234_1234",
  "passed": true,
  "notes": "Company has excellent reputation. No negative news or regulatory issues found. Checked Bloomberg, Reuters, and regulatory databases.",
  "proofDocumentHash": "0xreputation_check_proof_def456"
}
```

**Response**:
```json
{
  "success": true,
  "message": "Company reputation check completed",
  "step": {
    "stepName": "Company Reputation Check",
    "status": "REPUTATION_CHECKED",
    "passed": true,
    "verifiedBy": "ADMIN_1703001234_5678",
    "verifiedAt": 1703003000,
    "notes": "Company has excellent reputation. No negative news or regulatory issues found.",
    "proofDocument": "0xreputation_check_proof_def456"
  }
}
```

**How to Check Reputation**:
1. Search company on Bloomberg, Reuters
2. Check regulatory databases for violations
3. Review news articles and press releases
4. Check credit ratings (Moody's, S&P, Fitch)
5. Verify no sanctions or legal issues
6. Document findings with proof hash

---

### 9. Step 3: Review Financial Standing

**Endpoint**: `POST /api/v1/admin/verify/financial`

**Authorization**: Verifier

**Request**:
```json
{
  "adminId": "ADMIN_1703001234_5678",
  "applicationId": "APP_1703001234_1234",
  "passed": true,
  "notes": "Reviewed financial statements. Company has strong financial position with $50B+ in assets. Adequate capital for market making activities.",
  "proofDocumentHash": "0xfinancial_review_proof_ghi789"
}
```

**Response**:
```json
{
  "success": true,
  "message": "Financial standing review completed",
  "step": {
    "stepName": "Financial Standing Review",
    "status": "FINANCIAL_REVIEWED",
    "passed": true,
    "verifiedBy": "ADMIN_1703001234_5678",
    "verifiedAt": 1703004000,
    "notes": "Company has strong financial position with $50B+ in assets.",
    "proofDocument": "0xfinancial_review_proof_ghi789"
  }
}
```

**How to Review Financial Standing**:
1. Review financial statements (balance sheet, income statement)
2. Check capital adequacy
3. Verify liquidity ratios
4. Review credit ratings
5. Check for any financial distress indicators
6. Document review with proof hash

---

### 10. Step 4: Verify Technical Capabilities

**Endpoint**: `POST /api/v1/admin/verify/technical`

**Authorization**: Verifier

**Request**:
```json
{
  "adminId": "ADMIN_1703001234_5678",
  "applicationId": "APP_1703001234_1234",
  "passed": true,
  "notes": "Company has proven technical infrastructure. Uses Bloomberg Terminal for price feeds. Has dedicated trading systems and risk management. Passed technical assessment.",
  "proofDocumentHash": "0xtechnical_verification_proof_jkl012"
}
```

**Response**:
```json
{
  "success": true,
  "message": "Technical capabilities verification completed",
  "step": {
    "stepName": "Technical Capabilities Verification",
    "status": "TECHNICAL_VERIFIED",
    "passed": true,
    "verifiedBy": "ADMIN_1703001234_5678",
    "verifiedAt": 1703005000,
    "notes": "Company has proven technical infrastructure. Uses Bloomberg Terminal for price feeds.",
    "proofDocument": "0xtechnical_verification_proof_jkl012"
  }
}
```

**How to Verify Technical Capabilities**:
1. Review technical documentation
2. Check price feed sources (Bloomberg, Reuters, etc.)
3. Verify trading system capabilities
4. Check API integration capabilities
5. Review risk management systems
6. Verify disaster recovery plans
7. Document verification with proof hash

---

### 11. Step 5: Complete KYC/AML Compliance Check

**Endpoint**: `POST /api/v1/admin/verify/kyc-aml`

**Authorization**: Verifier

**Request**:
```json
{
  "adminId": "ADMIN_1703001234_5678",
  "applicationId": "APP_1703001234_1234",
  "passed": true,
  "notes": "KYC/AML compliance verified. Company has robust AML program. No sanctions or watchlist matches. Complies with FATF recommendations.",
  "proofDocumentHash": "0xkyc_aml_proof_mno345"
}
```

**Response**:
```json
{
  "success": true,
  "message": "KYC/AML compliance check completed",
  "step": {
    "stepName": "KYC/AML Compliance Check",
    "status": "KYC_AML_COMPLETED",
    "passed": true,
    "verifiedBy": "ADMIN_1703001234_5678",
    "verifiedAt": 1703006000,
    "notes": "KYC/AML compliance verified. Company has robust AML program.",
    "proofDocument": "0xkyc_aml_proof_mno345"
  }
}
```

**How to Complete KYC/AML Check**:
1. Verify company identity
2. Check beneficial owners
3. Screen against sanctions lists (OFAC, UN, EU)
4. Review AML policies and procedures
5. Check for PEP (Politically Exposed Persons)
6. Verify compliance with FATF recommendations
7. Document check with proof hash

---

## Approval/Rejection

### 12. Approve Application

**Endpoint**: `POST /api/v1/admin/approve`

**Authorization**: Super Admin Only

**Request**:
```json
{
  "adminId": "admin_super_001",
  "applicationId": "APP_1703001234_1234",
  "notes": "All verification steps passed. Goldman Sachs is approved as market maker. Effective immediately."
}
```

**Response**:
```json
{
  "success": true,
  "message": "Market maker application approved",
  "marketMakerId": "MM_Goldman_Sachs_Trading_Desk",
  "marketMakerAddress": "tGXC_goldman_sachs",
  "approvedBy": "admin_super_001",
  "approvedAt": 1703007000,
  "status": "APPROVED",
  "canNow": [
    "Deploy stock contracts",
    "Update stock prices",
    "Provide liquidity",
    "Execute corporate actions"
  ]
}
```

**Requirements for Approval**:
- ✅ All 5 verification steps must be completed
- ✅ All 5 verification steps must pass
- ✅ Only super admin can approve
- ✅ Creates market maker in registry

---

### 13. Reject Application

**Endpoint**: `POST /api/v1/admin/reject`

**Authorization**: Super Admin Only

**Request**:
```json
{
  "adminId": "admin_super_001",
  "applicationId": "APP_1703001235_5678",
  "reason": "Failed financial standing review. Insufficient capital for market making activities."
}
```

**Response**:
```json
{
  "success": true,
  "message": "Market maker application rejected",
  "applicationId": "APP_1703001235_5678",
  "rejectedBy": "admin_super_001",
  "rejectedAt": 1703007100,
  "status": "REJECTED",
  "reason": "Failed financial standing review. Insufficient capital for market making activities."
}
```

---

## Reporting & Analytics

### 14. Get Verification Statistics

**Endpoint**: `GET /api/v1/admin/statistics`

**Authorization**: Verifier or Super Admin

**Response**:
```json
{
  "total": 25,
  "pending": 5,
  "underReview": 8,
  "approved": 10,
  "rejected": 2,
  "averageReviewTime": "6.5 days",
  "approvalRate": "83%"
}
```

---

### 15. Get Audit Log

**Endpoint**: `GET /api/v1/admin/audit-log?startTime=1703000000&endTime=1703010000`

**Authorization**: Super Admin Only

**Response**:
```json
{
  "logs": [
    {
      "logId": "LOG_1703001234_1111",
      "adminId": "ADMIN_1703001234_5678",
      "action": "verify_license",
      "applicationId": "APP_1703001234_1234",
      "details": "License verification: PASSED",
      "timestamp": 1703002000,
      "ipAddress": "192.168.1.100"
    },
    {
      "logId": "LOG_1703001235_2222",
      "adminId": "admin_super_001",
      "action": "approve_application",
      "applicationId": "APP_1703001234_1234",
      "details": "Application approved",
      "timestamp": 1703007000,
      "ipAddress": "192.168.1.101"
    }
  ]
}
```

---

## Integration Examples

### Example 1: Complete Verification Workflow

```javascript
// GXC Admin Dashboard - Complete Workflow

class GXCAdminDashboard {
    constructor(apiUrl, sessionToken) {
        this.apiUrl = apiUrl;
        this.sessionToken = sessionToken;
    }
    
    // Step 1: Get pending applications
    async getPendingApplications() {
        const response = await fetch(`${this.apiUrl}/admin/applications/pending`, {
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`
            }
        });
        return await response.json();
    }
    
    // Step 2: Get application details
    async getApplicationDetails(applicationId) {
        const response = await fetch(
            `${this.apiUrl}/admin/applications/${applicationId}`,
            {
                headers: {
                    'Authorization': `Bearer ${this.sessionToken}`
                }
            }
        );
        return await response.json();
    }
    
    // Step 3: Verify license with SEC
    async verifyLicense(applicationId, passed, notes, proofHash) {
        const response = await fetch(`${this.apiUrl}/admin/verify/license`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                adminId: this.getAdminId(),
                applicationId: applicationId,
                passed: passed,
                notes: notes,
                proofDocumentHash: proofHash
            })
        });
        return await response.json();
    }
    
    // Step 4: Check company reputation
    async checkReputation(applicationId, passed, notes, proofHash) {
        const response = await fetch(`${this.apiUrl}/admin/verify/reputation`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                adminId: this.getAdminId(),
                applicationId: applicationId,
                passed: passed,
                notes: notes,
                proofDocumentHash: proofHash
            })
        });
        return await response.json();
    }
    
    // Step 5: Review financial standing
    async reviewFinancial(applicationId, passed, notes, proofHash) {
        const response = await fetch(`${this.apiUrl}/admin/verify/financial`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                adminId: this.getAdminId(),
                applicationId: applicationId,
                passed: passed,
                notes: notes,
                proofDocumentHash: proofHash
            })
        });
        return await response.json();
    }
    
    // Step 6: Verify technical capabilities
    async verifyTechnical(applicationId, passed, notes, proofHash) {
        const response = await fetch(`${this.apiUrl}/admin/verify/technical`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                adminId: this.getAdminId(),
                applicationId: applicationId,
                passed: passed,
                notes: notes,
                proofDocumentHash: proofHash
            })
        });
        return await response.json();
    }
    
    // Step 7: Complete KYC/AML check
    async completeKycAml(applicationId, passed, notes, proofHash) {
        const response = await fetch(`${this.apiUrl}/admin/verify/kyc-aml`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                adminId: this.getAdminId(),
                applicationId: applicationId,
                passed: passed,
                notes: notes,
                proofDocumentHash: proofHash
            })
        });
        return await response.json();
    }
    
    // Step 8: Approve application (super admin only)
    async approveApplication(applicationId, notes) {
        const response = await fetch(`${this.apiUrl}/admin/approve`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${this.sessionToken}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                adminId: this.getAdminId(),
                applicationId: applicationId,
                notes: notes
            })
        });
        return await response.json();
    }
    
    // Complete workflow for one application
    async processApplication(applicationId) {
        console.log("Processing application:", applicationId);
        
        // Get details
        const app = await this.getApplicationDetails(applicationId);
        console.log("Company:", app.companyName);
        
        // Step 1: Verify license
        console.log("Step 1: Verifying license with", app.regulatoryBody);
        await this.verifyLicense(
            applicationId,
            true,
            `Verified with ${app.regulatoryBody}. License ${app.licenseNumber} is active.`,
            "0xproof_license_123"
        );
        
        // Step 2: Check reputation
        console.log("Step 2: Checking company reputation");
        await this.checkReputation(
            applicationId,
            true,
            "Company has excellent reputation. No negative news found.",
            "0xproof_reputation_456"
        );
        
        // Step 3: Review financial
        console.log("Step 3: Reviewing financial standing");
        await this.reviewFinancial(
            applicationId,
            true,
            "Strong financial position. Adequate capital for market making.",
            "0xproof_financial_789"
        );
        
        // Step 4: Verify technical
        console.log("Step 4: Verifying technical capabilities");
        await this.verifyTechnical(
            applicationId,
            true,
            "Proven technical infrastructure. Uses Bloomberg Terminal.",
            "0xproof_technical_abc"
        );
        
        // Step 5: Complete KYC/AML
        console.log("Step 5: Completing KYC/AML check");
        await this.completeKycAml(
            applicationId,
            true,
            "KYC/AML compliance verified. Robust AML program in place.",
            "0xproof_kyc_def"
        );
        
        // Step 6: Approve (if super admin)
        console.log("Step 6: Approving application");
        const result = await this.approveApplication(
            applicationId,
            "All verification steps passed. Approved as market maker."
        );
        
        console.log("Application approved! Market Maker ID:", result.marketMakerId);
        return result;
    }
}

// Usage
const dashboard = new GXCAdminDashboard(
    "https://api.gxc.io/v1",
    "SESSION_ADMIN_1703001234_5678_1703001234"
);

// Process Goldman Sachs application
await dashboard.processApplication("APP_1703001234_1234");
```

---

### Example 2: Python Admin Script

```python
# GXC Admin Verification Script

import requests
import json

class GXCAdminAPI:
    def __init__(self, api_url, session_token):
        self.api_url = api_url
        self.session_token = session_token
        self.headers = {
            'Authorization': f'Bearer {session_token}',
            'Content-Type': 'application/json'
        }
    
    def get_pending_applications(self):
        """Get all pending applications"""
        response = requests.get(
            f'{self.api_url}/admin/applications/pending',
            headers=self.headers
        )
        return response.json()
    
    def verify_license(self, application_id, passed, notes, proof_hash):
        """Verify license with regulatory body"""
        data = {
            'adminId': self.get_admin_id(),
            'applicationId': application_id,
            'passed': passed,
            'notes': notes,
            'proofDocumentHash': proof_hash
        }
        response = requests.post(
            f'{self.api_url}/admin/verify/license',
            headers=self.headers,
            json=data
        )
        return response.json()
    
    def approve_application(self, application_id, notes):
        """Approve market maker application"""
        data = {
            'adminId': self.get_admin_id(),
            'applicationId': application_id,
            'notes': notes
        }
        response = requests.post(
            f'{self.api_url}/admin/approve',
            headers=self.headers,
            json=data
        )
        return response.json()
    
    def process_all_pending(self):
        """Process all pending applications"""
        applications = self.get_pending_applications()
        
        for app in applications['applications']:
            print(f"Processing: {app['companyName']}")
            
            # Verify all steps
            self.verify_license(app['applicationId'], True, "License verified", "0xproof1")
            # ... other steps
            
            # Approve
            result = self.approve_application(app['applicationId'], "Approved")
            print(f"Approved: {result['marketMakerId']}")

# Usage
api = GXCAdminAPI(
    "https://api.gxc.io/v1",
    "SESSION_ADMIN_1703001234_5678_1703001234"
)

api.process_all_pending()
```

---

## Summary

### What You Can Build

With these APIs, you can build:

1. **Admin Dashboard** - Web interface for GXC team
2. **Verification Workflow** - Step-by-step verification process
3. **Approval System** - Approve/reject applications
4. **Audit System** - Track all actions
5. **Reporting System** - Statistics and analytics

### Key Features

- ✅ Complete verification workflow
- ✅ 5-step verification process
- ✅ Admin authentication and permissions
- ✅ Audit trail for all actions
- ✅ Application status tracking
- ✅ Approval/rejection system
- ✅ Integration with market maker registry

### Next Steps

1. Build admin dashboard using these APIs
2. Implement verification workflow
3. Test with sample applications
4. Deploy to production
5. Start accepting market maker applications

---

**Questions?**
- Technical: Review `include/MarketMakerAdmin.h`
- Implementation: Review `src/MarketMakerAdmin.cpp`
- Integration: Review examples above
