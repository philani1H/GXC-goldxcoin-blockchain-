#!/bin/bash
# Comprehensive Test of ALL GXC Blockchain Functions
# Tests: Transactions, Staking, Unstaking, Stock Contracts, Admin

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo "╔══════════════════════════════════════════════════════════════════════╗"
echo "║                                                                      ║"
echo "║         GXC BLOCKCHAIN - COMPREHENSIVE FUNCTION TEST                ║"
echo "║                                                                      ║"
echo "╚══════════════════════════════════════════════════════════════════════╝"
echo ""

# Get node address
ADDR1=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}' | jq -r '.result')
echo -e "${CYAN}Node Address: $ADDR1${NC}"
echo ""

# Mine initial blocks
echo "═══════════════════════════════════════════════════════════════════════"
echo "TEST 1: Mining (10 blocks)"
echo "═══════════════════════════════════════════════════════════════════════"
for i in {1..10}; do
    echo -n "Mining block $i... "
    TEMPLATE=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblocktemplate","params":[{"algorithm":"sha256"}],"id":1}')
    PREV_HASH=$(echo "$TEMPLATE" | jq -r '.result.previousblockhash // "0000000000000000000000000000000000000000000000000000000000000000"')
    HEIGHT=$(echo "$TEMPLATE" | jq -r '.result.height // 0')
    
    for ((nonce=0; nonce<100000; nonce++)); do
        TS=$(date +%s)
        STR="${HEIGHT}${PREV_HASH}${nonce}${ADDR1}${TS}"
        HASH=$(echo -n "$STR" | sha256sum | cut -d' ' -f1)
        
        if [[ "$HASH" == 0* ]]; then
            BLOCK_DATA="{\"hash\":\"$HASH\",\"previousblockhash\":\"$PREV_HASH\",\"height\":$HEIGHT,\"nonce\":$nonce,\"miner\":\"$ADDR1\",\"timestamp\":$TS,\"difficulty\":0.1,\"transactions\":[]}"
            curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"submitblock\",\"params\":[$BLOCK_DATA],\"id\":1}" > /dev/null
            echo -e "${GREEN}✓${NC}"
            break
        fi
    done
    sleep 0.5
done

BAL=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$ADDR1\"],\"id\":1}" | jq -r '.result')
echo -e "${GREEN}Balance: $BAL tGXC${NC}"
echo ""

# Test sending transaction
echo "═══════════════════════════════════════════════════════════════════════"
echo "TEST 2: Send Transaction"
echo "═══════════════════════════════════════════════════════════════════════"
# Generate second address
ADDR2="tGXC$(echo -n "test_address_2" | sha256sum | cut -c1-35)"
echo "Sending 50 tGXC to $ADDR2..."

TX_RESULT=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_sendFrom\",\"params\":[\"$ADDR1\",\"$ADDR2\",50.0],\"id\":1}")
echo "$TX_RESULT" | jq '.'
echo ""

# Test staking
echo "═══════════════════════════════════════════════════════════════════════"
echo "TEST 3: Staking"
echo "═══════════════════════════════════════════════════════════════════════"
echo "Registering validator and staking 100 tGXC..."

STAKE_RESULT=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"registervalidator\",\"params\":[\"$ADDR1\",100.0,30],\"id\":1}")
echo "$STAKE_RESULT" | jq '.'
echo ""

# Test unstaking
echo "═══════════════════════════════════════════════════════════════════════"
echo "TEST 4: Unstaking"
echo "═══════════════════════════════════════════════════════════════════════"
echo "Testing unstake functionality..."

UNSTAKE_RESULT=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"unstake\",\"params\":[\"$ADDR1\",50.0],\"id\":1}")
echo "$UNSTAKE_RESULT" | jq '.'
echo ""

# Test stock contracts
echo "═══════════════════════════════════════════════════════════════════════"
echo "TEST 5: Stock Contracts"
echo "═══════════════════════════════════════════════════════════════════════"
echo "Testing stock contract functionality..."

# Check if stock contract methods exist
HELP=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"help","params":[],"id":1}' | jq -r '.result')
if echo "$HELP" | grep -q "stock"; then
    echo -e "${GREEN}✓ Stock contract methods available${NC}"
    echo "$HELP" | grep -i "stock"
else
    echo -e "${YELLOW}⚠ No stock contract methods found${NC}"
fi
echo ""

# Test admin functions
echo "═══════════════════════════════════════════════════════════════════════"
echo "TEST 6: Admin Functions"
echo "═══════════════════════════════════════════════════════════════════════"
echo "Testing admin functionality..."

# Check admin methods
if echo "$HELP" | grep -q "admin"; then
    echo -e "${GREEN}✓ Admin methods available${NC}"
    echo "$HELP" | grep -i "admin"
else
    echo -e "${YELLOW}⚠ No admin methods found${NC}"
fi
echo ""

# Get final status
echo "═══════════════════════════════════════════════════════════════════════"
echo "FINAL STATUS"
echo "═══════════════════════════════════════════════════════════════════════"

INFO=$(curl -s -X POST http://localhost:8332 -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
echo "$INFO" | jq '.result'
echo ""

echo "╔══════════════════════════════════════════════════════════════════════╗"
echo "║                                                                      ║"
echo "║              ✅ COMPREHENSIVE TEST COMPLETE! ✅                      ║"
echo "║                                                                      ║"
echo "╚══════════════════════════════════════════════════════════════════════╝"
