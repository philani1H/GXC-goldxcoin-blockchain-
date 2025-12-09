#!/bin/bash

echo "========================================="
echo "GXC External Wallet Helper Methods Test"
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

# Start node in background
echo "Starting testnet node..."
cd /workspaces/GXC-goldxcoin-blockchain-/build
./gxc-node --testnet > testnet_helper_test.log 2>&1 &
NODE_PID=$!
echo "Node PID: $NODE_PID"

# Wait for node to start
echo "Waiting for node to initialize..."
sleep 5

# Check if node is running
if curl -s http://localhost:8332/rpc -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' > /dev/null 2>&1; then
    echo -e "${GREEN}✓ Node started successfully${NC}"
else
    echo -e "${RED}✗ Node failed to start${NC}"
    kill $NODE_PID 2>/dev/null
    exit 1
fi

echo ""

# Test 1: List Unspent (listunspent)
echo "========================================="
echo "Test 1: List Unspent UTXOs"
echo "========================================="
echo -n "Testing: listunspent... "

response=$(curl -s -X POST http://localhost:8332/rpc \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"listunspent","params":[],"id":1}')

if echo "$response" | grep -q "\"result\""; then
    echo -e "${GREEN}✓ PASS${NC}"
    echo "Response: $response" | head -c 200
    echo "..."
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAIL${NC}"
    echo "Response: $response"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# Test 2: Get Signing Message (getsigningmessage)
echo "========================================="
echo "Test 2: Get Signing Message Format"
echo "========================================="
echo -n "Testing: getsigningmessage... "

response=$(curl -s -X POST http://localhost:8332/rpc \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getsigningmessage","params":["abc123",0,100.0],"id":1}')

if echo "$response" | grep -q "\"message\""; then
    echo -e "${GREEN}✓ PASS${NC}"
    echo "Response: $response"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAIL${NC}"
    echo "Response: $response"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# Test 3: Create Transaction (createtransaction)
echo "========================================="
echo "Test 3: Create Transaction Template"
echo "========================================="
echo -n "Testing: createtransaction... "

# Get a valid address first
address_response=$(curl -s -X POST http://localhost:8332/rpc \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}')

address=$(echo "$address_response" | grep -o '"result":"[^"]*"' | cut -d'"' -f4)

if [ -z "$address" ]; then
    echo -e "${YELLOW}⚠ SKIP (no address available)${NC}"
else
    echo "Using address: $address"
    
    # Try to create transaction (will fail if no UTXOs, but method should work)
    response=$(curl -s -X POST http://localhost:8332/rpc \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"createtransaction\",\"params\":[\"$address\",\"$address\",1.0,0.001],\"id\":1}")
    
    # Check if method exists (even if it fails due to insufficient funds)
    if echo "$response" | grep -q -E "(\"result\"|\"error\".*insufficient)"; then
        echo -e "${GREEN}✓ PASS${NC}"
        echo "Response: $response" | head -c 300
        echo "..."
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "Response: $response"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
fi

echo ""

# Test 4: List Unspent for Specific Address
echo "========================================="
echo "Test 4: List Unspent for Address"
echo "========================================="
echo -n "Testing: listunspent with address filter... "

if [ -z "$address" ]; then
    echo -e "${YELLOW}⚠ SKIP (no address available)${NC}"
else
    response=$(curl -s -X POST http://localhost:8332/rpc \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"listunspent\",\"params\":[\"$address\"],\"id\":1}")
    
    if echo "$response" | grep -q "\"result\""; then
        echo -e "${GREEN}✓ PASS${NC}"
        echo "Response: $response" | head -c 200
        echo "..."
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "Response: $response"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
fi

echo ""

# Test 5: Verify Method Registration
echo "========================================="
echo "Test 5: Verify All Methods Registered"
echo "========================================="

methods=("listunspent" "getsigningmessage" "createtransaction")
all_registered=true

for method in "${methods[@]}"; do
    echo -n "Checking: $method... "
    
    response=$(curl -s -X POST http://localhost:8332/rpc \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":[],\"id\":1}")
    
    # Method should exist (even if params are wrong)
    if echo "$response" | grep -q -v "Method not found"; then
        echo -e "${GREEN}✓ Registered${NC}"
    else
        echo -e "${RED}✗ Not Found${NC}"
        all_registered=false
    fi
done

if [ "$all_registered" = true ]; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# Summary
echo "========================================="
echo "Test Summary"
echo "========================================="
echo "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo "Total Tests: $((TESTS_PASSED + TESTS_FAILED))"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
fi

echo ""

# Shutdown node
echo "Shutting down node..."
kill $NODE_PID 2>/dev/null
wait $NODE_PID 2>/dev/null

echo "Test complete!"
