#!/usr/bin/env python3
"""
Comprehensive Admin & Fraud Detection API Test Suite
Tests all admin endpoints and fraud detection algorithms
"""

import requests
import json
import time

# API Base URLs
ADMIN_API_URL = "http://localhost:5002/api/v1"

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def print_test(name):
    print(f"\n{Colors.BLUE}{'='*80}{Colors.END}")
    print(f"{Colors.BLUE}Testing: {name}{Colors.END}")
    print(f"{Colors.BLUE}{'='*80}{Colors.END}")

def print_success(msg):
    print(f"{Colors.GREEN}✓ {msg}{Colors.END}")

def print_error(msg):
    print(f"{Colors.RED}✗ {msg}{Colors.END}")

def print_info(msg):
    print(f"{Colors.YELLOW}ℹ {msg}{Colors.END}")

# Global session token
session_token = None

def test_admin_login():
    """Test admin login"""
    global session_token
    print_test("Admin Login")

    data = {
        "username": "Philani-GXC.Foundation",
        "password": "GXC_SuperAdmin_2024!"
    }

    try:
        response = requests.post(f"{ADMIN_API_URL}/admin/login", json=data)
        result = response.json()

        if result.get('success'):
            session_token = result.get('sessionToken')
            print_success(f"Login successful! Session token: {session_token[:20]}...")
            print_info(f"Role: {result.get('role')}")
            print_info(f"Permissions: {result.get('permissions')}")
            return True
        else:
            print_error(f"Login failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"Login error: {e}")
        return False

def test_create_admin():
    """Test creating a new admin"""
    print_test("Create New Admin")

    data = {
        "username": "test_fraud_reviewer",
        "password": "TestPassword123!",
        "role": "fraud_reviewer",
        "permissions": ["fraud_reviewer", "view_applications"]
    }

    headers = {"Authorization": f"Bearer {session_token}"}

    try:
        response = requests.post(f"{ADMIN_API_URL}/admin/create", json=data, headers=headers)
        result = response.json()

        if result.get('success'):
            print_success(f"Admin created: {result.get('adminId')}")
            print_info(f"Username: {result.get('username')}")
            return True
        else:
            print_error(f"Create admin failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"Create admin error: {e}")
        return False

def test_list_admins():
    """Test listing all admins"""
    print_test("List All Admins")

    headers = {"Authorization": f"Bearer {session_token}"}

    try:
        response = requests.get(f"{ADMIN_API_URL}/admin/list", headers=headers)
        result = response.json()

        if result.get('success'):
            admins = result.get('admins', [])
            print_success(f"Found {len(admins)} admins")
            for admin in admins:
                print_info(f"  - {admin['username']} ({admin['role']})")
            return True
        else:
            print_error(f"List admins failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"List admins error: {e}")
        return False

def test_submit_mm_application():
    """Test submitting market maker application"""
    print_test("Submit Market Maker Application")

    data = {
        "applicantAddress": "GXC1TestMarketMakerAddress123",
        "companyName": "Test Trading LLC",
        "licenseNumber": "SEC-12345-2024",
        "regulatoryBody": "SEC",
        "country": "United States",
        "contactEmail": "compliance@testtrading.com",
        "contactPhone": "+1-555-0123",
        "website": "https://testtrading.com",
        "licenseDocumentHash": "QmTestLicenseHash123",
        "financialStatementsHash": "QmTestFinancialHash456",
        "technicalCapabilitiesHash": "QmTestTechnicalHash789",
        "kycDocumentsHash": "QmTestKYCHash012"
    }

    try:
        response = requests.post(f"{ADMIN_API_URL}/marketmaker/apply", json=data)
        result = response.json()

        if result.get('success'):
            print_success(f"Application submitted: {result.get('applicationId')}")
            print_info(f"Status: {result.get('status')}")
            print_info(f"Estimated review time: {result.get('estimatedReviewTime')}")
            return result.get('applicationId')
        else:
            print_error(f"Submit application failed: {result.get('error')}")
            return None
    except Exception as e:
        print_error(f"Submit application error: {e}")
        return None

def test_get_pending_applications():
    """Test getting pending applications"""
    print_test("Get Pending Applications")

    headers = {"Authorization": f"Bearer {session_token}"}

    try:
        response = requests.get(f"{ADMIN_API_URL}/admin/applications/pending", headers=headers)
        result = response.json()

        if result.get('success'):
            apps = result.get('applications', [])
            print_success(f"Found {len(apps)} pending applications")
            for app in apps:
                print_info(f"  - {app['companyName']} ({app['applicationId']})")
            return True
        else:
            print_error(f"Get pending applications failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"Get pending applications error: {e}")
        return False

def test_verify_steps(app_id):
    """Test verifying application steps"""
    print_test("Verify Application Steps")

    headers = {"Authorization": f"Bearer {session_token}"}

    steps = ['license', 'reputation', 'financial', 'technical', 'kyc']

    for step in steps:
        data = {
            "applicationId": app_id,
            "passed": True,
            "notes": f"{step.capitalize()} verification passed successfully"
        }

        try:
            response = requests.post(f"{ADMIN_API_URL}/admin/verify/{step}",
                                    json=data, headers=headers)
            result = response.json()

            if result.get('success'):
                print_success(f"Verified: {step}")
            else:
                print_error(f"Verify {step} failed: {result.get('error')}")
                return False
        except Exception as e:
            print_error(f"Verify {step} error: {e}")
            return False

    return True

def test_approve_application(app_id):
    """Test approving application"""
    print_test("Approve Application")

    headers = {"Authorization": f"Bearer {session_token}"}
    data = {"applicationId": app_id}

    try:
        response = requests.post(f"{ADMIN_API_URL}/admin/approve", json=data, headers=headers)
        result = response.json()

        if result.get('success'):
            print_success(f"Application approved: {app_id}")
            return True
        else:
            print_error(f"Approve failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"Approve error: {e}")
        return False

def test_fraud_report_submission():
    """Test submitting fraud report"""
    print_test("Submit Fraud Report")

    data = {
        "txHash": "test_stolen_tx_hash_12345",
        "reporterAddress": "GXC1VictimAddress789",
        "amount": 10000.0,
        "email": "victim@example.com",
        "description": "My funds were stolen in a phishing attack",
        "evidence": "Screenshots and transaction logs attached"
    }

    # This endpoint should be public (no auth required)
    try:
        response = requests.post(f"{ADMIN_API_URL}/fraud/report", json=data)
        result = response.json()

        if result.get('success'):
            print_success(f"Fraud report submitted: {result.get('reportId')}")
            print_info(f"Status: {result.get('status')}")
            return result.get('reportId')
        else:
            print_error(f"Submit fraud report failed: {result.get('error')}")
            return None
    except Exception as e:
        print_error(f"Submit fraud report error: {e}")
        return None

def test_get_pending_fraud_reports():
    """Test getting pending fraud reports"""
    print_test("Get Pending Fraud Reports")

    headers = {"Authorization": f"Bearer {session_token}"}

    try:
        response = requests.get(f"{ADMIN_API_URL}/admin/fraud/pending", headers=headers)
        result = response.json()

        if result.get('success'):
            reports = result.get('reports', [])
            print_success(f"Found {len(reports)} pending fraud reports")
            for report in reports:
                print_info(f"  - {report['txHash']} (${report['amount']})")
            return True
        else:
            print_error(f"Get pending fraud reports failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"Get pending fraud reports error: {e}")
        return False

def test_fraud_statistics():
    """Test getting fraud statistics"""
    print_test("Get Fraud Statistics")

    headers = {"Authorization": f"Bearer {session_token}"}

    try:
        response = requests.get(f"{ADMIN_API_URL}/admin/fraud/statistics", headers=headers)
        result = response.json()

        if result.get('success'):
            stats = result.get('statistics', {})
            print_success("Fraud statistics retrieved:")
            print_info(f"  Total reports: {stats.get('totalReports', 0)}")
            print_info(f"  Pending: {stats.get('pendingReports', 0)}")
            print_info(f"  Approved: {stats.get('approvedReports', 0)}")
            print_info(f"  Rejected: {stats.get('rejectedReports', 0)}")
            print_info(f"  Total amount reported: ${stats.get('totalAmountReported', 0)}")
            return True
        else:
            print_error(f"Get fraud statistics failed: {result.get('error')}")
            return False
    except Exception as e:
        print_error(f"Get fraud statistics error: {e}")
        return False

def main():
    """Run all tests"""
    print(f"\n{Colors.BLUE}{'='*80}{Colors.END}")
    print(f"{Colors.BLUE}GXC Admin & Fraud Detection API Test Suite{Colors.END}")
    print(f"{Colors.BLUE}{'='*80}{Colors.END}\n")

    results = {}

    # Test admin functions
    results['login'] = test_admin_login()

    if not results['login']:
        print_error("\nCannot proceed without admin login. Make sure admin API is running on port 5002")
        return

    time.sleep(0.5)
    results['create_admin'] = test_create_admin()

    time.sleep(0.5)
    results['list_admins'] = test_list_admins()

    # Test market maker application flow
    time.sleep(0.5)
    app_id = test_submit_mm_application()
    results['submit_application'] = app_id is not None

    if app_id:
        time.sleep(0.5)
        results['get_pending_apps'] = test_get_pending_applications()

        time.sleep(0.5)
        results['verify_steps'] = test_verify_steps(app_id)

        time.sleep(0.5)
        results['approve_app'] = test_approve_application(app_id)

    # Test fraud detection
    time.sleep(0.5)
    report_id = test_fraud_report_submission()
    results['submit_fraud_report'] = report_id is not None

    time.sleep(0.5)
    results['get_pending_fraud'] = test_get_pending_fraud_reports()

    time.sleep(0.5)
    results['fraud_statistics'] = test_fraud_statistics()

    # Print summary
    print(f"\n{Colors.BLUE}{'='*80}{Colors.END}")
    print(f"{Colors.BLUE}Test Summary{Colors.END}")
    print(f"{Colors.BLUE}{'='*80}{Colors.END}\n")

    passed = sum(1 for v in results.values() if v)
    total = len(results)

    for test_name, result in results.items():
        status = Colors.GREEN + "PASS" + Colors.END if result else Colors.RED + "FAIL" + Colors.END
        print(f"{test_name}: {status}")

    print(f"\n{Colors.BLUE}Total: {passed}/{total} tests passed{Colors.END}\n")

    if passed == total:
        print(f"{Colors.GREEN}✓ All tests passed!{Colors.END}\n")
    else:
        print(f"{Colors.YELLOW}⚠ Some tests failed{Colors.END}\n")

if __name__ == "__main__":
    main()
