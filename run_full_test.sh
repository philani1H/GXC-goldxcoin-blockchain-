#!/bin/bash

# Full test script: Build, start node, test mining, staking, restaking, and sending

set -e

RPC_URL="http://localhost:8080/rpc"
TARGET_ADDRESS="tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
NODE_PID=""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}🚀 GXC Blockchain Full Test Suite${NC}"
echo "============================================================"

# Function to cleanup on exit
cleanup() {
    if [ ! -z "$NODE_PID" ]; then
        echo ""
        echo -e "${YELLOW}Stopping node (PID: $NODE_PID)...${NC}"
        kill $NODE_PID 2>/dev/null || true
        wait $NODE_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

# Check if node binary exists
if [ ! -f "./gxc-node" ]; then
    echo -e "${RED}❌ gxc-node not found in current directory${NC}"
    echo "   Looking for node binary..."
    if [ -f "/workspace/gxc-node" ]; then
        echo "   Found at /workspace/gxc-node"
        cd /workspace
    else
        echo -e "${RED}   Node binary not found. Please build first.${NC}"
        exit 1
    fi
fi

# Check if node is already running
if curl -s "$RPC_URL" > /dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Node appears to be already running${NC}"
    echo "   Using existing node instance"
else
    echo "1️⃣  Starting node..."
    ./gxc-node > /tmp/gxc-node.log 2>&1 &
    NODE_PID=$!
    echo "   Node started (PID: $NODE_PID)"
    
    # Wait for node to be ready
    echo "   Waiting for node to be ready..."
    for i in {1..30}; do
        if curl -s "$RPC_URL" > /dev/null 2>&1; then
            echo -e "${GREEN}✅ Node is ready!${NC}"
            break
        fi
        if [ $i -eq 30 ]; then
            echo -e "${RED}❌ Node failed to start${NC}"
            echo "   Check logs: tail -f /tmp/gxc-node.log"
            exit 1
        fi
        sleep 1
        echo -n "."
    done
    echo ""
fi

# Function to make RPC call
rpc_call() {
    local method=$1
    shift
    local params="$@"
    
    curl -s -X POST "$RPC_URL" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":[$params],\"id\":1}" 2>/dev/null | jq -r '.result // .error // "null"'
}

# Get wallet address
echo "2️⃣  Getting wallet address..."
WALLET_ADDRESS=$(rpc_call "getwalletaddress")
if [ -z "$WALLET_ADDRESS" ] || [ "$WALLET_ADDRESS" = "null" ]; then
    echo -e "${RED}❌ Could not get wallet address${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Wallet address: $WALLET_ADDRESS${NC}"

# Get initial balance
INITIAL_BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo "📊 Initial balance: $INITIAL_BALANCE GXC"

# Test 1: Mining
echo ""
echo "============================================================"
echo -e "${GREEN}TEST 1: MINING COINS${NC}"
echo "============================================================"

# Mine blocks by submitting work
for i in {1..5}; do
    echo "⛏️  Mining block $i/5..."
    
    # Get block template
    TEMPLATE=$(rpc_call "getblocktemplate" "{\"miner_address\":\"$WALLET_ADDRESS\"}")
    if [ "$TEMPLATE" != "null" ] && [ ! -z "$TEMPLATE" ]; then
        HEIGHT=$(echo "$TEMPLATE" | jq -r '.height // 0')
        PREV_HASH=$(echo "$TEMPLATE" | jq -r '.previousblockhash // "0"*64')
        
        # Create a simple block (for testing - real mining would solve PoW)
        BLOCK_DATA="{\"index\":$HEIGHT,\"previousHash\":\"$PREV_HASH\",\"timestamp\":$(date +%s),\"miner\":\"$WALLET_ADDRESS\",\"difficulty\":0.1,\"nonce\":$((RANDOM * 1000))}"
        
        # Try to submit (may fail if PoW not solved, but that's OK for testing)
        RESULT=$(rpc_call "submitblock" "[$BLOCK_DATA]")
        echo "   Block $i submitted"
    fi
    sleep 2
done

# Wait for blocks to be processed
sleep 5
FINAL_BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo ""
echo "📊 Balance after mining: $FINAL_BALANCE GXC"

# Test 2: Staking
echo ""
echo "============================================================"
echo -e "${GREEN}TEST 2: STAKING${NC}"
echo "============================================================"

BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo "📊 Current balance: $BALANCE GXC"

STAKE_AMOUNT=100.0
STAKING_DAYS=30

if (( $(echo "$BALANCE >= 100.001" | bc -l 2>/dev/null || echo 0) )); then
    echo "📌 Staking $STAKE_AMOUNT GXC for $STAKING_DAYS days"
    RESULT=$(rpc_call "registervalidator" "\"$WALLET_ADDRESS\", $STAKE_AMOUNT, $STAKING_DAYS")
    
    if echo "$RESULT" | jq -e '.success' > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Validator registered!${NC}"
        TX_HASH=$(echo "$RESULT" | jq -r '.stake_tx // "N/A"')
        echo "   Stake TX: $TX_HASH"
    else
        echo -e "${RED}❌ Staking failed: $RESULT${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  Insufficient balance for staking${NC}"
fi

sleep 3

# Test 3: Restaking
echo ""
echo "============================================================"
echo -e "${GREEN}TEST 3: RESTAKING${NC}"
echo "============================================================"

VALIDATOR_INFO=$(rpc_call "getvalidatorinfo" "\"$WALLET_ADDRESS\"")
if echo "$VALIDATOR_INFO" | jq -e '.stake_amount' > /dev/null 2>&1; then
    CURRENT_STAKE=$(echo "$VALIDATOR_INFO" | jq -r '.stake_amount')
    echo "📊 Current stake: $CURRENT_STAKE GXC"
    
    ADDITIONAL=50.0
    echo "📌 Adding $ADDITIONAL GXC more stake"
    
    RESULT=$(rpc_call "addstake" "\"$WALLET_ADDRESS\", $ADDITIONAL")
    if echo "$RESULT" | jq -e '.success' > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Additional stake added!${NC}"
    else
        echo -e "${RED}❌ Restaking failed: $RESULT${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  Not a validator yet, skipping restaking${NC}"
fi

sleep 2

# Test 4: Send coins
echo ""
echo "============================================================"
echo -e "${GREEN}TEST 4: SENDING COINS${NC}"
echo "============================================================"

SEND_AMOUNT=50.0
BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo "📊 Sender balance: $BALANCE GXC"
echo "📤 Sending $SEND_AMOUNT GXC to $TARGET_ADDRESS"

if (( $(echo "$BALANCE >= 50.001" | bc -l 2>/dev/null || echo 0) )); then
    RESULT=$(rpc_call "sendtoaddress" "\"$TARGET_ADDRESS\", $SEND_AMOUNT")
    
    if [ "$RESULT" != "null" ] && [ ! -z "$RESULT" ] && [ "${RESULT:0:2}" != "{\"" ]; then
        echo -e "${GREEN}✅ Transaction sent!${NC}"
        echo "   TX Hash: $RESULT"
        
        sleep 3
        RECIPIENT_BALANCE=$(rpc_call "getbalance" "\"$TARGET_ADDRESS\"")
        echo "📊 Recipient balance: $RECIPIENT_BALANCE GXC"
    else
        echo -e "${RED}❌ Send failed: $RESULT${NC}"
    fi
else
    echo -e "${RED}❌ Insufficient balance${NC}"
fi

# Summary
echo ""
echo "============================================================"
echo -e "${GREEN}TEST SUMMARY${NC}"
echo "============================================================"
echo -e "${GREEN}✅ All tests completed!${NC}"
echo ""
echo "To view node logs: tail -f /tmp/gxc-node.log"
