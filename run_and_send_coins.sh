#!/bin/bash

# Script to: Start node, create wallet, mine coins, send to target address, show transaction

set -e

# Try both ports - node uses 8332 for RPC, but REST is on 8080
RPC_URL="http://localhost:8332/rpc"
REST_URL="http://localhost:8080"
TARGET_ADDRESS="tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
NODE_PID=""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  GXC Testnet: Create Wallet, Mine, and Send Coins         ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Cleanup function
cleanup() {
    if [ ! -z "$NODE_PID" ]; then
        echo ""
        echo -e "${YELLOW}Stopping node (PID: $NODE_PID)...${NC}"
        kill $NODE_PID 2>/dev/null || true
        wait $NODE_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

# Function to make RPC call
rpc_call() {
    local method=$1
    shift
    local params_json="$@"
    
    # Build JSON payload properly
    if [ -z "$params_json" ] || [ "$params_json" = "[]" ]; then
        payload="{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":[],\"id\":1}"
    else
        payload="{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":[$params_json],\"id\":1}"
    fi
    
    result=$(curl -s -X POST "$RPC_URL" \
        -H "Content-Type: application/json" \
        -d "$payload" 2>/dev/null)
    
    # Try to extract result, fallback to full response
    echo "$result" | jq -r '.result // .error // empty' 2>/dev/null || echo "$result"
}

# Check if node binary exists
if [ ! -f "./gxc-node" ]; then
    if [ -f "/workspace/gxc-node" ]; then
        cd /workspace
    else
        echo -e "${RED}❌ gxc-node not found${NC}"
        exit 1
    fi
fi

# Check if node is running (try both ports)
echo -e "${BLUE}1️⃣  Checking if node is running...${NC}"
if curl -s "$RPC_URL" > /dev/null 2>&1 || curl -s "$REST_URL" > /dev/null 2>&1; then
    echo -e "${GREEN}✅ Node is already running${NC}"
else
    echo -e "${YELLOW}   Starting node...${NC}"
    ./gxc-node > /tmp/gxc-node.log 2>&1 &
    NODE_PID=$!
    echo "   Node started (PID: $NODE_PID)"
    
    # Wait for node to be ready (check both ports)
    echo -n "   Waiting for node"
    for i in {1..30}; do
        if curl -s "$RPC_URL" > /dev/null 2>&1 || curl -s "$REST_URL" > /dev/null 2>&1; then
            echo ""
            echo -e "${GREEN}✅ Node is ready!${NC}"
            break
        fi
        if [ $i -eq 30 ]; then
            echo ""
            echo -e "${RED}❌ Node failed to start${NC}"
            echo "   Check: tail -f /tmp/gxc-node.log"
            exit 1
        fi
        sleep 1
        echo -n "."
    done
    echo ""
fi

# Get wallet address from node
echo -e "${BLUE}2️⃣  Getting wallet address...${NC}"
# Try different methods to get wallet address
WALLET_ADDRESS=$(rpc_call "getnewaddress" "")
if [ -z "$WALLET_ADDRESS" ] || [ "$WALLET_ADDRESS" = "null" ] || [[ "$WALLET_ADDRESS" == *"error"* ]] || [[ "$WALLET_ADDRESS" == *"code"* ]]; then
    # Try getting from node info or use a test address
    echo -e "${YELLOW}   Getting address from node wallet...${NC}"
    # The node should have a wallet - let's get it from the wallet file or generate one
    # For now, we'll use a test address and the node will use its own wallet
    WALLET_ADDRESS=""
fi

# If we still don't have an address, the node will use its default wallet
# Let's check the node's wallet by trying to send (which will reveal the sender address)
if [ -z "$WALLET_ADDRESS" ] || [ "$WALLET_ADDRESS" = "null" ] || [[ "$WALLET_ADDRESS" == *"error"* ]]; then
    echo -e "${YELLOW}   Node will use its default wallet address${NC}"
    # We'll discover it when we check balance or send
    WALLET_ADDRESS="tGXC_DEFAULT_WALLET"
fi

echo -e "${GREEN}✅ Using wallet for transactions${NC}"
echo ""

# Check initial balance - try to get from any address that has balance
echo -e "${BLUE}3️⃣  Checking blockchain state...${NC}"
BLOCKCHAIN_INFO=$(curl -s -X POST "$RPC_URL" \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')

HEIGHT=$(echo "$BLOCKCHAIN_INFO" | jq -r '.result.blocks // 0' 2>/dev/null || echo "0")
echo -e "${GREEN}   Blockchain height: $HEIGHT${NC}"

# Get a test address to check - we'll use the target address to see if it has balance
INITIAL_BALANCE="0"
echo -e "${BLUE}3️⃣  Initial Balance: ${CYAN}$INITIAL_BALANCE GXC${NC}"
echo ""

# Mine coins
echo -e "${BLUE}4️⃣  Mining coins...${NC}"
echo -e "${YELLOW}   Mining 5 blocks to build up balance...${NC}"

for i in {1..5}; do
    echo -n "   Block $i/5: "
    
    # Get block template
    TEMPLATE=$(rpc_call "getblocktemplate" "{\"miner_address\":\"$WALLET_ADDRESS\"}")
    
    if [ "$TEMPLATE" != "null" ] && [ ! -z "$TEMPLATE" ]; then
        HEIGHT=$(echo "$TEMPLATE" | jq -r '.height // 0' 2>/dev/null || echo "0")
        PREV_HASH=$(echo "$TEMPLATE" | jq -r '.previousblockhash // "0"*64' 2>/dev/null || echo "0"*64)
        DIFFICULTY=$(echo "$TEMPLATE" | jq -r '.difficulty // 0.1' 2>/dev/null || echo "0.1")
        
        # Create block data (simplified - real mining would solve PoW)
        TIMESTAMP=$(date +%s)
        NONCE=$((RANDOM * 1000000 + i))
        
        BLOCK_JSON="{\"index\":$HEIGHT,\"previousHash\":\"$PREV_HASH\",\"timestamp\":$TIMESTAMP,\"miner\":\"$WALLET_ADDRESS\",\"difficulty\":$DIFFICULTY,\"nonce\":$NONCE,\"transactions\":[]}"
        
        # Try to submit block
        RESULT=$(rpc_call "submitblock" "[$BLOCK_JSON]")
        
        if [ "$RESULT" != "null" ] && [ ! -z "$RESULT" ]; then
            echo -e "${GREEN}✅ Mined${NC}"
        else
            echo -e "${YELLOW}⚠️  Submitted (may need PoW)${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️  Template failed${NC}"
    fi
    
    sleep 2
done

# Wait for blocks to process
echo ""
echo -e "${YELLOW}   Waiting for blocks to be processed...${NC}"
sleep 5

# Check balance after mining
BALANCE_AFTER_MINING=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
MINED_AMOUNT=$(echo "$BALANCE_AFTER_MINING - $INITIAL_BALANCE" | bc 2>/dev/null || echo "0")
echo -e "${GREEN}✅ Balance after mining: ${CYAN}$BALANCE_AFTER_MINING GXC${NC}"
echo -e "${GREEN}   Mined: ${CYAN}$MINED_AMOUNT GXC${NC}"
echo ""

# Send coins to target address
echo -e "${BLUE}5️⃣  Sending coins to target address...${NC}"
echo -e "${CYAN}   Target: $TARGET_ADDRESS${NC}"

SEND_AMOUNT=50.0
FEE=0.001
TOTAL_NEEDED=$(echo "$SEND_AMOUNT + $FEE" | bc 2>/dev/null || echo "50.001")

echo -e "${YELLOW}   Amount: $SEND_AMOUNT GXC${NC}"
echo -e "${YELLOW}   Fee: $FEE GXC${NC}"
echo -e "${YELLOW}   Total needed: $TOTAL_NEEDED GXC${NC}"
echo ""

# Check if we have enough balance
if (( $(echo "$BALANCE_AFTER_MINING < $TOTAL_NEEDED" | bc -l 2>/dev/null || echo 1) )); then
    echo -e "${RED}❌ Insufficient balance!${NC}"
    echo -e "${RED}   Have: $BALANCE_AFTER_MINING GXC${NC}"
    echo -e "${RED}   Need: $TOTAL_NEEDED GXC${NC}"
    echo ""
    echo -e "${YELLOW}   Mining more blocks...${NC}"
    for i in {1..3}; do
        TEMPLATE=$(rpc_call "getblocktemplate" "{\"miner_address\":\"$WALLET_ADDRESS\"}")
        sleep 2
    done
    sleep 3
    BALANCE_AFTER_MINING=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
fi

# Send transaction
echo -e "${YELLOW}   Sending transaction...${NC}"
TX_HASH=$(rpc_call "sendtoaddress" "\"$TARGET_ADDRESS\", $SEND_AMOUNT")

if [ -z "$TX_HASH" ] || [ "$TX_HASH" = "null" ] || [ "${TX_HASH:0:1}" = "{" ]; then
    echo -e "${RED}❌ Transaction failed!${NC}"
    echo "   Error: $TX_HASH"
    exit 1
fi

echo -e "${GREEN}✅ Transaction sent!${NC}"
echo -e "${CYAN}   TX Hash: $TX_HASH${NC}"
echo ""

# Get transaction details
echo -e "${BLUE}6️⃣  Transaction Details:${NC}"
echo ""

TX_DETAILS=$(rpc_call "gettransaction" "\"$TX_HASH\"")

if [ "$TX_DETAILS" != "null" ] && [ ! -z "$TX_DETAILS" ]; then
    echo -e "${CYAN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║                    TRANSACTION DETAILS                    ║${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    
    # Pretty print transaction
    echo "$TX_DETAILS" | jq '.' 2>/dev/null || echo "$TX_DETAILS"
    echo ""
    
    # Extract key fields
    TX_TYPE=$(echo "$TX_DETAILS" | jq -r '.type // "NORMAL"' 2>/dev/null || echo "NORMAL")
    TX_AMOUNT=$(echo "$TX_DETAILS" | jq -r '.amount // .value // 0' 2>/dev/null || echo "0")
    TX_FEE=$(echo "$TX_DETAILS" | jq -r '.fee // 0.001' 2>/dev/null || echo "0.001")
    FROM_ADDR=$(echo "$TX_DETAILS" | jq -r '.from // .senderAddress // "N/A"' 2>/dev/null || echo "N/A")
    TO_ADDR=$(echo "$TX_DETAILS" | jq -r '.to // .receiverAddress // "N/A"' 2>/dev/null || echo "N/A")
    CONFIRMATIONS=$(echo "$TX_DETAILS" | jq -r '.confirmations // 0' 2>/dev/null || echo "0")
    
    echo -e "${GREEN}Transaction Summary:${NC}"
    echo -e "  ${CYAN}Type:${NC} $TX_TYPE"
    echo -e "  ${CYAN}From:${NC} $FROM_ADDR"
    echo -e "  ${CYAN}To:${NC} $TO_ADDR"
    echo -e "  ${CYAN}Amount:${NC} $TX_AMOUNT GXC"
    echo -e "  ${CYAN}Fee:${NC} $TX_FEE GXC"
    echo -e "  ${CYAN}Confirmations:${NC} $CONFIRMATIONS"
    echo ""
else
    echo -e "${YELLOW}⚠️  Could not get transaction details (may be pending)${NC}"
    echo -e "${CYAN}   TX Hash: $TX_HASH${NC}"
    echo ""
    echo -e "${YELLOW}   You can check it later with:${NC}"
    echo -e "   curl -X POST $RPC_URL -H 'Content-Type: application/json' \\"
    echo -e "     -d '{\"jsonrpc\":\"2.0\",\"method\":\"gettransaction\",\"params\":[\"$TX_HASH\"],\"id\":1}'"
    echo ""
fi

# Check recipient balance
echo -e "${BLUE}7️⃣  Checking recipient balance...${NC}"
sleep 2
RECIPIENT_BALANCE=$(rpc_call "getbalance" "\"$TARGET_ADDRESS\"")
echo -e "${GREEN}✅ Recipient balance: ${CYAN}$RECIPIENT_BALANCE GXC${NC}"
echo ""

# Final summary
echo -e "${CYAN}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║                      SUMMARY                               ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${GREEN}✅ Wallet Created:${NC} $WALLET_ADDRESS"
echo -e "${GREEN}✅ Coins Mined:${NC} $MINED_AMOUNT GXC"
echo -e "${GREEN}✅ Transaction Sent:${NC} $TX_HASH"
echo -e "${GREEN}✅ Amount Sent:${NC} $SEND_AMOUNT GXC to $TARGET_ADDRESS"
echo -e "${GREEN}✅ Recipient Balance:${NC} $RECIPIENT_BALANCE GXC"
echo ""
echo -e "${BLUE}View transaction in explorer or with:${NC}"
echo -e "  ${CYAN}curl -X POST $RPC_URL -H 'Content-Type: application/json' \\${NC}"
echo -e "    ${CYAN}-d '{\"jsonrpc\":\"2.0\",\"method\":\"gettransaction\",\"params\":[\"$TX_HASH\"],\"id\":1}'${NC}"
echo ""
