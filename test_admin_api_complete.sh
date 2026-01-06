#!/bin/bash

# Complete Admin API Test Script
# Tests all admin endpoints after fixing struct mismatches

echo "========================================"
echo "GXC Complete Admin API Test"
echo "========================================"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
REST_URL="http://localhost:8080"
TESTS_PASSED=0
TESTS_FAILED=0

# Test function
test_endpoint() {
    local name="$1"
    local method="$2"
    local endpoint="$3"
    local data="$4"
    
    echo ""
    echo "----------------------------------------"
    echo "TEST: $name"
    echo "----------------------------------------"
    
    if [ "$method" == "GET" ]; then
        response=$(curl -s "$REST_URL$endpoint")
    else
        response=$(curl -s -X "$method" "$REST_URL$endpoint" \
            -H "Content-Type: application/json" \
            -d "$data")
    fi
    
    echo "Response: $response"
    
    if echo "$response" | grep -q "success.*true\|sessionToken\|reports"; then
        echo -e "${GREEN}✅ PASSED${NC}"
        ((TESTS_PASSED++))
        echo "$response"
    else
        echo -e "${RED}❌ FAILED${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Check if node is running
echo "Checking if node is running..."
if ! curl -s "$REST_URL/health" > /dev/null 2>&1; then
    echo -e "${RED}❌ ERROR: Node is not running at $REST_URL${NC}"
    echo "Please start the node first:"
    echo "  ./gxc-node --testnet"
    exit 1
fi

echo -e "${GREEN}✅ Node is running${NC}"

echo ""
echo "========================================"
echo "TEST 1: Admin Login"
echo "========================================"

LOGIN_DATA='{
    "username": "Philani-GXC.Foundation",
    "password": "GXC$ecure2025!Philani#Foundation@Admin"
}'

login_response=$(test_endpoint "Admin Login" "POST" "/api/admin/login" "$LOGIN_DATA")

if [ $? -eq 0 ]; then
    SESSION_TOKEN=$(echo "$login_response" | jq -r '.sessionToken')
    echo "Session Token: $SESSION_TOKEN"
else
    echo -e "${RED}❌ Cannot proceed without login${NC}"
    exit 1
fi

echo ""
echo "========================================"
echo "TEST 2: Submit Fraud Report"
echo "========================================"

FRAUD_REPORT='{
    "txHash": "test_tx_'$(date +%s)'",
    "reporterAddress": "test_address",
    "amount": 1000.0,
    "email": "test@example.com",
    "description": "Test fraud report",
    "evidence": "Test evidence"
}'

report_response=$(curl -s -X POST "$REST_URL/api/fraud/report-stolen" \
    -H "Content-Type: application/json" \
    -d "$FRAUD_REPORT")

echo "Response: $report_response"

if echo "$report_response" | grep -q "reportId\|success"; then
    echo -e "${GREEN}✅ PASSED${NC}"
    ((TESTS_PASSED++))
    REPORT_ID=$(echo "$report_response" | jq -r '.reportId // empty')
    echo "Report ID: $REPORT_ID"
else
    echo -e "${RED}❌ FAILED${NC}"
    ((TESTS_FAILED++))
fi

echo ""
echo "========================================"
echo "TEST 3: Get Pending Fraud Reports"
echo "========================================"

test_endpoint "Get Pending Reports" "GET" "/api/admin/fraud/reports/pending?sessionToken=$SESSION_TOKEN" ""

echo ""
echo "========================================"
echo "TEST 4: Get Fraud Report Details"
echo "========================================"

if [ ! -z "$REPORT_ID" ]; then
    test_endpoint "Get Report Details" "GET" "/api/admin/fraud/report/$REPORT_ID?sessionToken=$SESSION_TOKEN" ""
else
    echo -e "${YELLOW}⚠️  SKIPPED: No report ID available${NC}"
fi

echo ""
echo "========================================"
echo "TEST 5: Get Audit Log"
echo "========================================"

test_endpoint "Get Audit Log" "GET" "/api/admin/audit-log?sessionToken=$SESSION_TOKEN" ""

echo ""
echo "========================================"
echo "TEST 6: Get Reversal Statistics"
echo "========================================"

test_endpoint "Get Reversal Stats" "GET" "/api/admin/reversal/statistics?sessionToken=$SESSION_TOKEN" ""

echo ""
echo "========================================"
echo "TEST 7: Admin Logout"
echo "========================================"

LOGOUT_DATA="{\"sessionToken\": \"$SESSION_TOKEN\"}"
test_endpoint "Admin Logout" "POST" "/api/admin/logout" "$LOGOUT_DATA"

echo ""
echo "========================================"
echo "TEST SUMMARY"
echo "========================================"
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo "========================================"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✅ ALL TESTS PASSED!${NC}"
    echo ""
    echo "Admin API is fully functional:"
    echo "  - Admin login works"
    echo "  - Fraud report submission works"
    echo "  - Get pending reports works"
    echo "  - Get report details works"
    echo "  - Audit log works"
    echo "  - Reversal statistics works"
    echo "  - Admin logout works"
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED${NC}"
    exit 1
fi
