#!/bin/bash

# GXC Mining Pool - RPC Connection Test Script
# Tests all critical RPC endpoints needed by mining pools

# Configuration
NODE_URL="${NODE_URL:-http://localhost:8332}"
POOL_ADDRESS="${POOL_ADDRESS:-tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Helper function for RPC calls
rpc_call() {
    local method=$1
    local params=$2
    local id=${3:-1}

    curl -s -X POST "$NODE_URL" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":$params,\"id\":$id}"
}

# Test function
test_endpoint() {
    local name=$1
    local method=$2
    local params=$3
    local check_field=$4

    echo -n "Testing $name... "

    result=$(rpc_call "$method" "$params")

    # Check if we got a valid response
    if echo "$result" | jq -e . >/dev/null 2>&1; then
        # Valid JSON received
        if echo "$result" | jq -e '.result' >/dev/null 2>&1; then
            echo -e "${GREEN}✓ PASS${NC}"
            if [ -n "$check_field" ]; then
                value=$(echo "$result" | jq -r ".result.$check_field")
                echo "  → $check_field: $value"
            else
                echo "$result" | jq -C '.result' | head -5
            fi
            ((TESTS_PASSED++))
            return 0
        elif echo "$result" | jq -e '.error' >/dev/null 2>&1; then
            error_msg=$(echo "$result" | jq -r '.error.message')
            echo -e "${YELLOW}⚠ ERROR${NC}"
            echo "  → $error_msg"
            ((TESTS_FAILED++))
            return 1
        fi
    else
        echo -e "${RED}✗ FAIL - Invalid JSON or empty response${NC}"
        echo "  → Response: $result"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Banner
echo "================================================"
echo "  GXC Mining Pool RPC Connection Test"
echo "================================================"
echo "Node URL: $NODE_URL"
echo "Pool Address: $POOL_ADDRESS"
echo "================================================"
echo ""

# Test 0: Health check (GET request)
echo -n "Testing Health Endpoint (GET /health)... "
health_result=$(curl -s "$NODE_URL/health")
if echo "$health_result" | jq -e '.status == "ok"' >/dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS${NC}"
    echo "$health_result" | jq -C '.'
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL - Node may not be running${NC}"
    echo "  → Response: $health_result"
    echo ""
    echo "CRITICAL: Node is not responding. Please start the node:"
    echo "  ./gxc-node --testnet --rpc-port=18332"
    echo ""
    exit 1
fi
echo ""

# Test 1: getinfo
test_endpoint \
    "getinfo (Node Information)" \
    "getinfo" \
    "[]" \
    "blocks"
echo ""

# Test 2: getblockchaininfo
test_endpoint \
    "getblockchaininfo (Blockchain Status)" \
    "getblockchaininfo" \
    "[]" \
    "blocks"
echo ""

# Test 3: getdifficulty
test_endpoint \
    "getdifficulty (Current Difficulty)" \
    "getdifficulty" \
    "[]"
echo ""

# Test 4: getbalance
test_endpoint \
    "getbalance (Pool Wallet Balance)" \
    "getbalance" \
    "[\"$POOL_ADDRESS\"]"
echo ""

# Test 5: listunspent
test_endpoint \
    "listunspent (UTXOs for Pool Address)" \
    "listunspent" \
    "[\"$POOL_ADDRESS\"]"
echo ""

# Test 6: getblocktemplate
echo -n "Testing getblocktemplate (Mining Work)... "
result=$(rpc_call "getblocktemplate" "[{\"minerAddress\":\"$POOL_ADDRESS\"}]")
if echo "$result" | jq -e '.result' >/dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS${NC}"
    height=$(echo "$result" | jq -r '.result.height')
    reward=$(echo "$result" | jq -r '.result.coinbasevalue')
    difficulty=$(echo "$result" | jq -r '.result.difficulty')
    txcount=$(echo "$result" | jq '.result.transactions | length')
    echo "  → Height: $height"
    echo "  → Reward: $reward GXC"
    echo "  → Difficulty: $difficulty"
    echo "  → Transactions in template: $txcount"
    ((TESTS_PASSED++))
elif echo "$result" | jq -e '.error' >/dev/null 2>&1; then
    error_msg=$(echo "$result" | jq -r '.error.message')
    echo -e "${YELLOW}⚠ ERROR${NC}"
    echo "  → $error_msg"
    ((TESTS_FAILED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi
echo ""

# Test 7: getvalidators
test_endpoint \
    "getvalidators (Validator List)" \
    "getvalidators" \
    "[]"
echo ""

# Test 8: getmininginfo
test_endpoint \
    "getmininginfo (Mining Status)" \
    "getmininginfo" \
    "[]"
echo ""

# Test 9: getnetworkhashps
test_endpoint \
    "getnetworkhashps (Network Hash Rate)" \
    "getnetworkhashps" \
    "[]"
echo ""

# Test 10: getnewaddress
test_endpoint \
    "getnewaddress (Generate New Address)" \
    "getnewaddress" \
    "[]"
echo ""

# Summary
echo "================================================"
echo "  Test Summary"
echo "================================================"
echo -e "Tests Passed:  ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed:  ${RED}$TESTS_FAILED${NC}"
echo "Total Tests:   $((TESTS_PASSED + TESTS_FAILED))"
echo "================================================"
echo ""

# Exit code
if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed! Pool can connect to node.${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Review MINING_POOL_RPC_GUIDE.md for detailed API documentation"
    echo "2. Implement work distribution to miners"
    echo "3. Test block submission with submitblock"
    echo "4. Implement payment distribution"
    exit 0
else
    echo -e "${RED}✗ Some tests failed. Please check node configuration.${NC}"
    echo ""
    echo "Troubleshooting:"
    echo "1. Verify node is running: ps aux | grep gxc-node"
    echo "2. Check node logs: tail -f ./gxc_data/debug.log"
    echo "3. Test manually: curl $NODE_URL/health"
    exit 1
fi
