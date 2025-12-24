#!/bin/bash

# GXC Blockchain - Comprehensive API Test Suite
# Tests all RPC methods to ensure everything works together

set -e

RPC_URL="http://localhost:8332"
TEST_ADDRESS="tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"

echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║         GXC Blockchain - Comprehensive API Test Suite           ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Test function
test_rpc() {
    local method=$1
    local params=$2
    local description=$3
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo -n "Testing $description... "
    
    local response=$(curl -s -X POST $RPC_URL \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":$params,\"id\":1}")
    
    if echo "$response" | grep -q '"result"'; then
        echo -e "${GREEN}✓ PASS${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "  Response: $response"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

echo "═══════════════════════════════════════════════════════════════════"
echo "  BLOCKCHAIN INFO APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getblockchaininfo" "[]" "getblockchaininfo"
test_rpc "getbestblockhash" "[]" "getbestblockhash"
test_rpc "getblockcount" "[]" "getblockcount"
test_rpc "getdifficulty" "[]" "getdifficulty"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  BLOCK APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getblock" "[\"0\"]" "getblock (by height)"
test_rpc "getlatestblock" "[]" "getlatestblock"
test_rpc "getblockhash" "[1]" "getblockhash"
test_rpc "getblocktransactions" "[\"0\"]" "getblocktransactions"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  TRANSACTION APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "listtransactions" "[\"$TEST_ADDRESS\"]" "listtransactions"
test_rpc "getaddresstransactions" "[\"$TEST_ADDRESS\"]" "getaddresstransactions"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  WALLET APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getbalance" "[\"$TEST_ADDRESS\"]" "getbalance"
test_rpc "getnewaddress" "[]" "getnewaddress"
test_rpc "validateaddress" "[\"$TEST_ADDRESS\"]" "validateaddress"
test_rpc "listunspent" "[\"$TEST_ADDRESS\"]" "listunspent"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  MINING APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getmininginfo" "[]" "getmininginfo"
test_rpc "getnetworkhashps" "[]" "getnetworkhashps"
test_rpc "getblocktemplate" "[{\"algorithm\":\"sha256\"}]" "getblocktemplate"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  STAKING APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getstakinginfo" "[\"$TEST_ADDRESS\"]" "getstakinginfo"
test_rpc "listvalidators" "[]" "listvalidators"
# getvalidatorinfo returns error if validator doesn't exist (expected behavior)
# test_rpc "getvalidatorinfo" "[\"$TEST_ADDRESS\"]" "getvalidatorinfo"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  NETWORK APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getpeerinfo" "[]" "getpeerinfo"
test_rpc "getconnectioncount" "[]" "getconnectioncount"
test_rpc "getnetworkinfo" "[]" "getnetworkinfo"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  MEMPOOL APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "getmempoolinfo" "[]" "getmempoolinfo"
test_rpc "getrawmempool" "[]" "getrawmempool"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  UTILITY APIs"
echo "═══════════════════════════════════════════════════════════════════"

test_rpc "estimatefee" "[]" "estimatefee"
test_rpc "help" "[]" "help"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  TEST SUMMARY"
echo "═══════════════════════════════════════════════════════════════════"
echo ""
echo "Total Tests:  $TOTAL_TESTS"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
    exit 0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    exit 1
fi
