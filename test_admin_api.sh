#!/bin/bash

# GXC Admin API - Comprehensive Test Suite
# Tests all admin functions on Railway deployment

NODE_URL="https://gxc-chain112-blockchain-node-production.up.railway.app"

echo "=========================================="
echo "GXC Admin API - Comprehensive Test Suite"
echo "=========================================="
echo ""
echo "Node URL: $NODE_URL"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to print test result
print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✅ PASS${NC}: $2"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}❌ FAIL${NC}: $2"
        ((TESTS_FAILED++))
    fi
}

echo "=========================================="
echo "TEST 1: Admin Login (Valid Credentials)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/admin/login?username=admin&password=admin123")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if login was successful
SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
if [ "$SUCCESS" == "true" ]; then
    print_result 0 "Admin login with valid credentials"
    TOKEN=$(echo "$RESPONSE" | jq -r '.sessionToken')
    echo "Session Token: $TOKEN"
else
    print_result 1 "Admin login with valid credentials"
fi
echo ""

echo "=========================================="
echo "TEST 2: Admin Login (Invalid Credentials)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/admin/login?username=admin&password=wrongpassword")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if login failed as expected
SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
if [ "$SUCCESS" == "false" ]; then
    print_result 0 "Admin login rejection with invalid credentials"
else
    print_result 1 "Admin login rejection with invalid credentials"
fi
echo ""

echo "=========================================="
echo "TEST 3: Admin Login (Missing Parameters)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/admin/login?username=admin")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if error is returned
ERROR=$(echo "$RESPONSE" | jq -r '.error')
if [ "$ERROR" != "null" ]; then
    print_result 0 "Admin login error handling for missing parameters"
else
    print_result 1 "Admin login error handling for missing parameters"
fi
echo ""

echo "=========================================="
echo "TEST 4: Submit Fraud Report (Valid)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/fraud/report?txHash=test_tx_001&reporterAddress=tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf&amount=100.5&description=Test%20fraud%20report")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if report was submitted
SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
REPORT_ID=$(echo "$RESPONSE" | jq -r '.reportId')
if [ "$SUCCESS" == "true" ] && [ "$REPORT_ID" != "null" ]; then
    print_result 0 "Fraud report submission"
    echo "Report ID: $REPORT_ID"
else
    print_result 1 "Fraud report submission"
fi
echo ""

echo "=========================================="
echo "TEST 5: Submit Fraud Report (Missing Params)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/fraud/report?txHash=test_tx_002&amount=50")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if error is returned
ERROR=$(echo "$RESPONSE" | jq -r '.error')
if [ "$ERROR" != "null" ]; then
    print_result 0 "Fraud report error handling for missing parameters"
else
    print_result 1 "Fraud report error handling for missing parameters"
fi
echo ""

echo "=========================================="
echo "TEST 6: Submit Multiple Fraud Reports"
echo "=========================================="
echo ""

for i in {1..3}; do
    echo "Submitting report $i..."
    RESPONSE=$(curl -s "$NODE_URL/api/fraud/report?txHash=test_tx_00$i&reporterAddress=tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf&amount=$((100 * i))&description=Fraud%20report%20$i")
    REPORT_ID=$(echo "$RESPONSE" | jq -r '.reportId')
    echo "Report ID: $REPORT_ID"
done
echo ""
print_result 0 "Multiple fraud report submissions"
echo ""

echo "=========================================="
echo "TEST 7: Approve Fraud Report (Valid)"
echo "=========================================="
echo ""

# Use the token from TEST 1
RESPONSE=$(curl -s "$NODE_URL/api/admin/fraud/approve?token=$TOKEN&reportId=$REPORT_ID&notes=Approved%20for%20testing")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if approval was successful
SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
if [ "$SUCCESS" == "true" ]; then
    print_result 0 "Fraud report approval"
else
    print_result 1 "Fraud report approval"
fi
echo ""

echo "=========================================="
echo "TEST 8: Approve Fraud Report (Missing Token)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/admin/fraud/approve?reportId=FR_123&notes=Test")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if error is returned
ERROR=$(echo "$RESPONSE" | jq -r '.error')
if [ "$ERROR" != "null" ]; then
    print_result 0 "Fraud approval error handling for missing token"
else
    print_result 1 "Fraud approval error handling for missing token"
fi
echo ""

echo "=========================================="
echo "TEST 9: Blockchain Info (Sanity Check)"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/getinfo")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if blockchain info is returned
HEIGHT=$(echo "$RESPONSE" | jq -r '.result.height')
if [ "$HEIGHT" != "null" ] && [ "$HEIGHT" -ge 0 ]; then
    print_result 0 "Blockchain info retrieval"
    echo "Current Height: $HEIGHT"
else
    print_result 1 "Blockchain info retrieval"
fi
echo ""

echo "=========================================="
echo "TEST 10: Health Check"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/health")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if health check returns OK
STATUS=$(echo "$RESPONSE" | jq -r '.status')
if [ "$STATUS" == "ok" ]; then
    print_result 0 "Health check endpoint"
else
    print_result 1 "Health check endpoint"
fi
echo ""

echo "=========================================="
echo "TEST 11: Get Balance"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/getbalance?address=tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if balance is returned
RESULT=$(echo "$RESPONSE" | jq -r '.result')
if [ "$RESULT" != "null" ]; then
    print_result 0 "Get balance endpoint"
else
    print_result 1 "Get balance endpoint"
fi
echo ""

echo "=========================================="
echo "TEST 12: List Unspent"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/listunspent?address=tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if unspent list is returned
RESULT=$(echo "$RESPONSE" | jq -r '.result')
if [ "$RESULT" != "null" ]; then
    print_result 0 "List unspent endpoint"
else
    print_result 1 "List unspent endpoint"
fi
echo ""

echo "=========================================="
echo "TEST 13: Get Block Count"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/getblockcount")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if block count is returned
RESULT=$(echo "$RESPONSE" | jq -r '.result')
if [ "$RESULT" != "null" ] && [ "$RESULT" -ge 0 ]; then
    print_result 0 "Get block count endpoint"
    echo "Block Count: $RESULT"
else
    print_result 1 "Get block count endpoint"
fi
echo ""

echo "=========================================="
echo "TEST 14: Get Best Block Hash"
echo "=========================================="
echo ""

RESPONSE=$(curl -s "$NODE_URL/api/getbestblockhash")
echo "Response:"
echo "$RESPONSE" | jq .
echo ""

# Check if block hash is returned
RESULT=$(echo "$RESPONSE" | jq -r '.result')
if [ "$RESULT" != "null" ] && [ "$RESULT" != "" ]; then
    print_result 0 "Get best block hash endpoint"
else
    print_result 1 "Get best block hash endpoint"
fi
echo ""

echo "=========================================="
echo "TEST 15: Complete Admin Workflow"
echo "=========================================="
echo ""

echo "Step 1: Admin Login..."
LOGIN_RESPONSE=$(curl -s "$NODE_URL/api/admin/login?username=admin&password=admin123")
WORKFLOW_TOKEN=$(echo "$LOGIN_RESPONSE" | jq -r '.sessionToken')
echo "Token: $WORKFLOW_TOKEN"
echo ""

echo "Step 2: Submit Fraud Report..."
REPORT_RESPONSE=$(curl -s "$NODE_URL/api/fraud/report?txHash=workflow_test_tx&reporterAddress=tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf&amount=250&description=Workflow%20test")
WORKFLOW_REPORT_ID=$(echo "$REPORT_RESPONSE" | jq -r '.reportId')
echo "Report ID: $WORKFLOW_REPORT_ID"
echo ""

echo "Step 3: Approve Fraud Report..."
APPROVE_RESPONSE=$(curl -s "$NODE_URL/api/admin/fraud/approve?token=$WORKFLOW_TOKEN&reportId=$WORKFLOW_REPORT_ID&notes=Workflow%20test%20approved")
APPROVE_STATUS=$(echo "$APPROVE_RESPONSE" | jq -r '.status')
echo "Status: $APPROVE_STATUS"
echo ""

if [ "$APPROVE_STATUS" == "approved" ]; then
    print_result 0 "Complete admin workflow (login → report → approve)"
else
    print_result 1 "Complete admin workflow (login → report → approve)"
fi
echo ""

echo "=========================================="
echo "TEST SUMMARY"
echo "=========================================="
echo ""
echo -e "${GREEN}Tests Passed: $TESTS_PASSED${NC}"
echo -e "${RED}Tests Failed: $TESTS_FAILED${NC}"
echo ""

TOTAL_TESTS=$((TESTS_PASSED + TESTS_FAILED))
SUCCESS_RATE=$((TESTS_PASSED * 100 / TOTAL_TESTS))

echo "Total Tests: $TOTAL_TESTS"
echo "Success Rate: $SUCCESS_RATE%"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL TESTS PASSED!${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠️  Some tests failed. Review the output above.${NC}"
    exit 1
fi
