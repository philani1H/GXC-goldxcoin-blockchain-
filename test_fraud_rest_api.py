#!/usr/bin/env python3
"""
Fraud Detection REST API Integration Test

Tests all fraud detection API endpoints to ensure they work correctly.
"""

import requests
import json
import time

class FraudAPITest:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.admin_key = "gxc_admin_secret_key_2024"
        self.test_results = []
        
    def log(self, message):
        print(f"[API TEST] {message}")
        
    def test_passed(self, test_name):
        print(f"✅ {test_name}")
        self.test_results.append({"test": test_name, "status": "PASSED"})
        
    def test_failed(self, test_name, reason):
        print(f"❌ {test_name}: {reason}")
        self.test_results.append({"test": test_name, "status": "FAILED", "reason": reason})
        
    def test_1_report_stolen_funds(self):
        """Test: Report stolen funds endpoint"""
        self.log("\nTest 1: Report Stolen Funds")
        
        url = f"{self.base_url}/api/fraud/report-stolen"
        data = {
            "txHash": "test_api_stolen_001",
            "reporterAddress": "GXC_test_reporter",
            "amount": 10000,
            "email": "test@example.com",
            "description": "API integration test - stolen funds"
        }
        
        try:
            response = requests.post(url, json=data, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if result.get("success") and result.get("reportId"):
                    self.test_passed(f"Report stolen funds - Report ID: {result['reportId']}")
                    return result["reportId"]
                else:
                    self.test_failed("Report stolen funds", "Invalid response format")
            else:
                self.test_failed("Report stolen funds", f"HTTP {response.status_code}")
        except requests.exceptions.ConnectionError:
            self.log("⚠️  Server not running - skipping API tests")
            self.log("   To test APIs, start the server first:")
            self.log("   ./gxc-node")
            return None
        except Exception as e:
            self.test_failed("Report stolen funds", str(e))
            
        return None
        
    def test_2_check_report_status(self, report_id):
        """Test: Check report status endpoint"""
        if not report_id:
            self.log("Skipping test 2 (no report ID)")
            return
            
        self.log("\nTest 2: Check Report Status")
        
        url = f"{self.base_url}/api/fraud/report/{report_id}"
        
        try:
            response = requests.get(url, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if result.get("reportId") == report_id:
                    self.test_passed(f"Check report status - Status: {result.get('status')}")
                else:
                    self.test_failed("Check report status", "Report ID mismatch")
            else:
                self.test_failed("Check report status", f"HTTP {response.status_code}")
        except Exception as e:
            self.test_failed("Check report status", str(e))
            
    def test_3_approve_report(self, report_id):
        """Test: Approve report endpoint (admin)"""
        if not report_id:
            self.log("Skipping test 3 (no report ID)")
            return
            
        self.log("\nTest 3: Approve Report (Admin)")
        
        url = f"{self.base_url}/api/fraud/approve"
        data = {
            "reportId": report_id,
            "adminKey": self.admin_key
        }
        
        try:
            response = requests.post(url, json=data, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if result.get("success") and result.get("status") == "APPROVED":
                    self.test_passed(f"Approve report - TX marked as stolen")
                else:
                    self.test_failed("Approve report", "Invalid response")
            else:
                self.test_failed("Approve report", f"HTTP {response.status_code}")
        except Exception as e:
            self.test_failed("Approve report", str(e))
            
    def test_4_check_transaction_taint(self):
        """Test: Check transaction taint endpoint"""
        self.log("\nTest 4: Check Transaction Taint")
        
        url = f"{self.base_url}/api/fraud/check-transaction/test_api_stolen_001"
        
        try:
            response = requests.get(url, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if "taintScore" in result and "riskLevel" in result:
                    self.test_passed(f"Check transaction taint - Taint: {result['taintScore']}, Risk: {result['riskLevel']}")
                else:
                    self.test_failed("Check transaction taint", "Missing fields")
            else:
                self.test_failed("Check transaction taint", f"HTTP {response.status_code}")
        except Exception as e:
            self.test_failed("Check transaction taint", str(e))
            
    def test_5_check_address_fraud(self):
        """Test: Check address fraud status endpoint"""
        self.log("\nTest 5: Check Address Fraud Status")
        
        url = f"{self.base_url}/api/fraud/check-address/GXC_test_reporter"
        
        try:
            response = requests.get(url, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if "isFlagged" in result and "alertCount" in result:
                    self.test_passed(f"Check address fraud - Flagged: {result['isFlagged']}, Alerts: {result['alertCount']}")
                else:
                    self.test_failed("Check address fraud", "Missing fields")
            else:
                self.test_failed("Check address fraud", f"HTTP {response.status_code}")
        except Exception as e:
            self.test_failed("Check address fraud", str(e))
            
    def test_6_get_fraud_statistics(self):
        """Test: Get fraud statistics endpoint"""
        self.log("\nTest 6: Get Fraud Statistics")
        
        url = f"{self.base_url}/api/fraud/statistics"
        
        try:
            response = requests.get(url, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if "totalStolenTransactions" in result and "totalAlerts" in result:
                    self.test_passed(f"Get fraud statistics - Stolen: {result['totalStolenTransactions']}, Alerts: {result['totalAlerts']}")
                else:
                    self.test_failed("Get fraud statistics", "Missing fields")
            else:
                self.test_failed("Get fraud statistics", f"HTTP {response.status_code}")
        except Exception as e:
            self.test_failed("Get fraud statistics", str(e))
            
    def test_7_list_pending_reports(self):
        """Test: List pending reports endpoint (admin)"""
        self.log("\nTest 7: List Pending Reports (Admin)")
        
        url = f"{self.base_url}/api/fraud/pending"
        data = {
            "adminKey": self.admin_key
        }
        
        try:
            response = requests.get(url, json=data, timeout=5)
            
            if response.status_code == 200:
                result = response.json()
                if "reports" in result and "count" in result:
                    self.test_passed(f"List pending reports - Count: {result['count']}")
                else:
                    self.test_failed("List pending reports", "Missing fields")
            else:
                self.test_failed("List pending reports", f"HTTP {response.status_code}")
        except Exception as e:
            self.test_failed("List pending reports", str(e))
            
    def test_8_reject_report(self):
        """Test: Reject report endpoint (admin)"""
        self.log("\nTest 8: Reject Report (Admin)")
        
        # First create a report to reject
        url = f"{self.base_url}/api/fraud/report-stolen"
        data = {
            "txHash": "test_api_reject_001",
            "amount": 5000,
            "description": "Test report for rejection"
        }
        
        try:
            response = requests.post(url, json=data, timeout=5)
            if response.status_code == 200:
                result = response.json()
                report_id = result.get("reportId")
                
                if report_id:
                    # Now reject it
                    reject_url = f"{self.base_url}/api/fraud/reject"
                    reject_data = {
                        "reportId": report_id,
                        "adminKey": self.admin_key,
                        "reason": "Test rejection"
                    }
                    
                    reject_response = requests.post(reject_url, json=reject_data, timeout=5)
                    
                    if reject_response.status_code == 200:
                        reject_result = reject_response.json()
                        if reject_result.get("status") == "REJECTED":
                            self.test_passed(f"Reject report - Report {report_id} rejected")
                        else:
                            self.test_failed("Reject report", "Invalid response")
                    else:
                        self.test_failed("Reject report", f"HTTP {reject_response.status_code}")
                else:
                    self.test_failed("Reject report", "Could not create test report")
            else:
                self.test_failed("Reject report", "Could not create test report")
        except Exception as e:
            self.test_failed("Reject report", str(e))
            
    def run_all_tests(self):
        self.log("=== Starting Fraud Detection REST API Tests ===\n")
        
        # Test 1: Report stolen funds
        report_id = self.test_1_report_stolen_funds()
        
        if report_id:
            # Test 2: Check report status
            self.test_2_check_report_status(report_id)
            
            # Test 3: Approve report
            self.test_3_approve_report(report_id)
            
            # Test 4: Check transaction taint
            self.test_4_check_transaction_taint()
            
            # Test 5: Check address fraud
            self.test_5_check_address_fraud()
            
            # Test 6: Get fraud statistics
            self.test_6_get_fraud_statistics()
            
            # Test 7: List pending reports
            self.test_7_list_pending_reports()
            
            # Test 8: Reject report
            self.test_8_reject_report()
        
        self.log("\n=== API Test Results ===")
        
        passed = sum(1 for r in self.test_results if r["status"] == "PASSED")
        failed = sum(1 for r in self.test_results if r["status"] == "FAILED")
        
        print(f"\nTotal: {len(self.test_results)} tests")
        print(f"✅ Passed: {passed}")
        print(f"❌ Failed: {failed}")
        
        if failed == 0 and passed > 0:
            print("\n🎉 ALL API TESTS PASSED! Fraud detection REST API is working.\n")
            return True
        elif passed == 0:
            print("\n⚠️  Server not running. Start the server to test APIs.\n")
            return False
        else:
            print("\n⚠️  Some API tests failed. Review implementation.\n")
            return False

if __name__ == "__main__":
    test = FraudAPITest()
    success = test.run_all_tests()
    exit(0 if success else 1)
