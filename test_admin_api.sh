#!/bin/bash

# Test Admin System via REST API
# This script tests all admin functions using the REST API

echo "========================================"
echo "GXC Admin System API Test"
echo "========================================"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# API Base URL
API_URL="http://localhost:8332"

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to test API endpoint
test_api() {
    local test_name="$1"
    local method="$2"
    local endpoint="$3"
    local data="$4"
    local expected_status="$5"
    
    echo ""
    echo "----------------------------------------"
    echo "TEST: $test_name"
    echo "----------------------------------------"
    
    if [ "$method" == "GET" ]; then
        response=$(curl -s -w "\n%{http_code}" "$API_URL$endpoint")
    else
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$API_URL$endpoint" \
            -H "Content-Type: application/json" \
            -d "$data")
    fi
    
    # Extract status code (last line)
    status_code=$(echo "$response" | tail -n1)
    # Extract body (all but last line)
    body=$(echo "$response" | head -n-1)
    
    echo "Status: $status_code"
    echo "Response: $body"
    
    if [ "$status_code" == "$expected_status" ]; then
        echo -e "${GREEN}✅ PASSED${NC}"
        ((TESTS_PASSED++))
        # Return the body for further processing
        echo "$body"
    else
        echo -e "${RED}❌ FAILED - Expected $expected_status, got $status_code${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Check if node is running
echo "Checking if GXC node is running..."
if ! curl -s "$API_URL/api/v1/blockchain/info" > /dev/null 2>&1; then
    echo -e "${RED}❌ ERROR: GXC node is not running at $API_URL${NC}"
    echo "Please start the node first:"
    echo "  ./gxc-node --datadir=./test_data --port=9333 --rpc-port=8332"
    exit 1
fi

echo -e "${GREEN}✅ Node is running${NC}"

echo ""
echo "========================================"
echo "TEST 1: Super Admin Login"
echo "========================================"

LOGIN_DATA='{
    "username": "Philani-GXC.Foundation",
    "password": "GXC$ecure2025!Philani#Foundation@Admin"
}'

login_response=$(test_api "Super Admin Login" "POST" "/api/admin/login" "$LOGIN_DATA" "200")

if [ $? -eq 0 ]; then
    # Extract session token from response
    SESSION_TOKEN=$(echo "$login_response" | jq -r '.sessionToken // .token // .')
    echo "Session Token: $SESSION_TOKEN"
else
    echo -e "${RED}❌ Cannot proceed without login${NC}"
    exit 1
fi

echo ""
echo "========================================"
echo "TEST 2: Get Admin Info"
echo "========================================"

test_api "Get Admin Info" "GET" "/api/admin/info?sessionToken=$SESSION_TOKEN" "" "200"

echo ""
echo "========================================"
echo "TEST 3: Submit Fraud Report (Public)"
echo "========================================"

FRAUD_REPORT='{
    "txHash": "test_stolen_tx_' $(date +%s) '",
    "reporterAddress": "test_victim_address",
    "amount": 1000.0,
    "email": "victim@test.com",
    "description": "Test fraud report - wallet hacked",
    "evidence": "Test evidence: screenshots, logs"
}'

report_response=$(test_api "Submit Fraud Report" "POST" "/api/fraud/report-stolen" "$FRAUD_REPORT" "200")

if [ $? -eq 0 ]; then
    REPORT_ID=$(echo "$report_response" | jq -r '.reportId // empty')
    echo "Report ID: $REPORT_ID"
fi

echo ""
echo "========================================"
echo "TEST 4: Get Pending Fraud Reports"
echo "========================================"

test_api "Get Pending Reports" "GET" "/api/admin/fraud/reports/pending?sessionToken=$SESSION_TOKEN" "" "200"

echo ""
echo "========================================"
echo "TEST 5: Check Transaction Taint"
echo "========================================"

test_api "Check Transaction Taint" "GET" "/api/fraud/check-transaction/test_tx_123" "" "200"

echo ""
echo "========================================"
echo "TEST 6: Check Address Fraud Status"
echo "========================================"

test_api "Check Address Fraud" "GET" "/api/fraud/check-address/test_address_123" "" "200"

echo ""
echo "========================================"
echo "TEST 7: Get Blockchain Info"
echo "========================================"

test_api "Get Blockchain Info" "GET" "/api/v1/blockchain/info" "" "200"

echo ""
echo "========================================"
echo "TEST 8: Admin Logout"
echo "========================================"

LOGOUT_DATA="{\"sessionToken\": \"$SESSION_TOKEN\"}"
test_api "Admin Logout" "POST" "/api/admin/logout" "$LOGOUT_DATA" "200"

echo ""
echo "========================================"
echo "TEST 9: Invalid Login Attempt"
echo "========================================"

INVALID_LOGIN='{
    "username": "invalid_user",
    "password": "wrong_password"
}'

test_api "Invalid Login" "POST" "/api/admin/login" "$INVALID_LOGIN" "401"

echo ""
echo "========================================"
echo "TEST SUMMARY"
echo "========================================"
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo "========================================"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✅ ALL TESTS PASSED!${NC}"
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED${NC}"
    exit 1
fi
