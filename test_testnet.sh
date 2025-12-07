#!/bin/bash

echo "========================================="
echo "GXC Testnet Comprehensive Test"
echo "========================================="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to test RPC call
test_rpc() {
    local method=$1
    local params=$2
    local description=$3
    
    echo -n "Testing: $description... "
    
    response=$(curl -s -X POST http://localhost:8332 \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":$params,\"id\":1}")
    
    if echo "$response" | grep -q "\"result\""; then
        echo -e "${GREEN}✓ PASS${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "Response: $response"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Start node in background
echo "Starting testnet node..."
cd /workspaces/GXC-goldxcoin-blockchain-/build
./gxc-node --testnet > testnet_test.log 2>&1 &
NODE_PID=$!
echo "Node PID: $NODE_PID"

# Wait for node to start
echo "Waiting for node to initialize..."
sleep 5

# Check if node is running
if ! kill -0 $NODE_PID 2>/dev/null; then
    echo -e "${RED}✗ Node failed to start!${NC}"
    cat testnet_test.log
    exit 1
fi

echo -e "${GREEN}✓ Node started successfully${NC}"
echo ""

# Test 1: Blockchain Info
echo "========================================="
echo "Test 1: Blockchain Information"
echo "========================================="
test_rpc "getblockchaininfo" "[]" "Get blockchain info"
echo ""

# Test 2: Block Count
echo "========================================="
echo "Test 2: Block Operations"
echo "========================================="
test_rpc "getblockcount" "[]" "Get block count"
test_rpc "getbestblockhash" "[]" "Get best block hash"
test_rpc "getdifficulty" "[]" "Get difficulty"
echo ""

# Test 3: Network Info
echo "========================================="
echo "Test 3: Network Information"
echo "========================================="
test_rpc "getpeerinfo" "[]" "Get peer info"
test_rpc "getconnectioncount" "[]" "Get connection count"
echo ""

# Test 4: Mining Info
echo "========================================="
echo "Test 4: Mining Information"
echo "========================================="
test_rpc "getmininginfo" "[]" "Get mining info"
test_rpc "getnetworkhashps" "[]" "Get network hashrate"
echo ""

# Test 5: Wallet Operations
echo "========================================="
echo "Test 5: Wallet Operations"
echo "========================================="

# Get node wallet address
WALLET_RESPONSE=$(curl -s -X POST http://localhost:8332 \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}')

WALLET_ADDR=$(echo $WALLET_RESPONSE | grep -o '"result":"[^"]*"' | cut -d'"' -f4)

if [ -n "$WALLET_ADDR" ]; then
    echo -e "${GREEN}✓ PASS${NC} - Created wallet address: $WALLET_ADDR"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    
    # Test balance
    test_rpc "getbalance" "[\"$WALLET_ADDR\"]" "Get balance for address"
    
    # Test address validation
    test_rpc "validateaddress" "[\"$WALLET_ADDR\"]" "Validate address"
else
    echo -e "${RED}✗ FAIL${NC} - Failed to create wallet address"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi
echo ""

# Test 6: Validator Operations
echo "========================================="
echo "Test 6: Validator Operations"
echo "========================================="
test_rpc "getvalidators" "[]" "Get all validators"
echo ""

# Test 7: Transaction Operations
echo "========================================="
echo "Test 7: Transaction Operations"
echo "========================================="
test_rpc "getmempoolinfo" "[]" "Get mempool info"
echo ""

# Test 8: Block Template (for mining)
echo "========================================="
echo "Test 8: Mining Template"
echo "========================================="
test_rpc "getblocktemplate" "[]" "Get block template"
echo ""

# Summary
echo "========================================="
echo "Test Summary"
echo "========================================="
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo "Total Tests: $((TESTS_PASSED + TESTS_FAILED))"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
    EXIT_CODE=0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    EXIT_CODE=1
fi

# Show node info
echo ""
echo "========================================="
echo "Node Information"
echo "========================================="
curl -s -X POST http://localhost:8332 \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' | python3 -m json.tool 2>/dev/null || echo "Node info unavailable"

# Cleanup
echo ""
echo "Shutting down node..."
kill $NODE_PID 2>/dev/null
wait $NODE_PID 2>/dev/null

echo "Test complete!"
exit $EXIT_CODE
