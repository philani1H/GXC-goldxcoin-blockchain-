#!/bin/bash

# Comprehensive test script for mining, staking, restaking, and sending coins
# Target address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf

RPC_URL="http://localhost:8080/rpc"
TARGET_ADDRESS="tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"

echo "🚀 GXC Blockchain Test Suite"
echo "============================================================"
echo "RPC URL: $RPC_URL"
echo "Target Address: $TARGET_ADDRESS"
echo "============================================================"

# Function to make RPC call
rpc_call() {
    local method=$1
    shift
    local params="$@"
    
    curl -s -X POST "$RPC_URL" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"$method\",\"params\":[$params],\"id\":1}" | jq -r '.result // .error'
}

# Check if node is running
echo ""
echo "1️⃣  Checking node connection..."
INFO=$(rpc_call "getblockchaininfo")
if [ "$INFO" = "null" ] || [ -z "$INFO" ]; then
    echo "❌ Cannot connect to node. Is gxc-node running?"
    echo "   Start it with: ./gxc-node"
    exit 1
fi
echo "✅ Node is running"
HEIGHT=$(echo "$INFO" | jq -r '.blocks // 0')
echo "   Current height: $HEIGHT"

# Get wallet address
echo ""
echo "2️⃣  Getting wallet address..."
WALLET_ADDRESS=$(rpc_call "getwalletaddress")
if [ -z "$WALLET_ADDRESS" ] || [ "$WALLET_ADDRESS" = "null" ]; then
    echo "❌ Could not get wallet address"
    exit 1
fi
echo "✅ Wallet address: $WALLET_ADDRESS"

# Get initial balance
echo ""
echo "3️⃣  Checking initial balance..."
INITIAL_BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo "📊 Initial balance: $INITIAL_BALANCE GXC"

# Test 1: Mining
echo ""
echo "============================================================"
echo "TEST 1: MINING COINS"
echo "============================================================"
echo "⛏️  Mining blocks to address: $WALLET_ADDRESS"

# Mine a few blocks by getting block templates and submitting
for i in {1..3}; do
    echo ""
    echo "--- Mining block $i/3 ---"
    
    # Get block template
    TEMPLATE=$(rpc_call "getblocktemplate" "{\"miner_address\":\"$WALLET_ADDRESS\"}")
    if [ "$TEMPLATE" != "null" ] && [ ! -z "$TEMPLATE" ]; then
        echo "✅ Block template received"
        PREV_HASH=$(echo "$TEMPLATE" | jq -r '.previousblockhash // "0"*64')
        HEIGHT=$(echo "$TEMPLATE" | jq -r '.height // 0')
        
        # For testing, we'll just log that we got the template
        # In a real scenario, the miner would solve PoW and submit
        echo "   Previous hash: ${PREV_HASH:0:16}..."
        echo "   Height: $HEIGHT"
    else
        echo "⚠️  Could not get block template"
    fi
    
    sleep 1
done

# Wait a bit for any blocks to be processed
sleep 3

# Check balance after mining
FINAL_BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo ""
echo "📊 Balance after mining: $FINAL_BALANCE GXC"
MINED=$(echo "$FINAL_BALANCE - $INITIAL_BALANCE" | bc 2>/dev/null || echo "0")
echo "💰 Mined: $MINED GXC"

# Test 2: Staking
echo ""
echo "============================================================"
echo "TEST 2: STAKING"
echo "============================================================"

BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo "📊 Current balance: $BALANCE GXC"

STAKE_AMOUNT=100.0
STAKING_DAYS=30

# Check if we have enough balance
REQUIRED=$(echo "$STAKE_AMOUNT + 0.001" | bc 2>/dev/null || echo "100.001")
if (( $(echo "$BALANCE < $REQUIRED" | bc -l 2>/dev/null || echo 1) )); then
    echo "⚠️  Insufficient balance for staking. Need $REQUIRED GXC, have $BALANCE GXC"
    echo "   Skipping staking test..."
    STAKE_RESULT="SKIPPED"
else
    echo "📌 Staking $STAKE_AMOUNT GXC for $STAKING_DAYS days"
    STAKE_RESULT=$(rpc_call "registervalidator" "\"$WALLET_ADDRESS\", $STAKE_AMOUNT, $STAKING_DAYS")
    
    if [ "$STAKE_RESULT" != "null" ] && [ ! -z "$STAKE_RESULT" ]; then
        echo "✅ Validator registered successfully!"
        TX_HASH=$(echo "$STAKE_RESULT" | jq -r '.stake_tx // "N/A"')
        echo "   Stake TX: $TX_HASH"
    else
        echo "❌ Staking failed"
        STAKE_RESULT="FAILED"
    fi
fi

# Wait a bit
sleep 2

# Test 3: Restaking
echo ""
echo "============================================================"
echo "TEST 3: RESTAKING (Adding More Stake)"
echo "============================================================"

if [ "$STAKE_RESULT" = "SKIPPED" ] || [ "$STAKE_RESULT" = "FAILED" ]; then
    echo "⚠️  Skipping restaking test (staking was not successful)"
    RESTAKE_RESULT="SKIPPED"
else
    # Get validator info
    VALIDATOR_INFO=$(rpc_call "getvalidatorinfo" "\"$WALLET_ADDRESS\"")
    if [ "$VALIDATOR_INFO" = "null" ] || [ -z "$VALIDATOR_INFO" ]; then
        echo "⚠️  Could not get validator info"
        RESTAKE_RESULT="SKIPPED"
    else
        CURRENT_STAKE=$(echo "$VALIDATOR_INFO" | jq -r '.stake_amount // 0')
        echo "📊 Current stake: $CURRENT_STAKE GXC"
        
        ADDITIONAL_STAKE=50.0
        echo "📌 Adding $ADDITIONAL_STAKE GXC more stake"
        
        RESTAKE_RESULT=$(rpc_call "addstake" "\"$WALLET_ADDRESS\", $ADDITIONAL_STAKE")
        
        if [ "$RESTAKE_RESULT" != "null" ] && [ ! -z "$RESTAKE_RESULT" ]; then
            echo "✅ Additional stake added successfully!"
            ADD_AMOUNT=$(echo "$RESTAKE_RESULT" | jq -r '.additional_amount // "N/A"')
            echo "   Additional amount: $ADD_AMOUNT GXC"
        else
            echo "❌ Restaking failed"
            RESTAKE_RESULT="FAILED"
        fi
    fi
fi

# Wait a bit
sleep 2

# Test 4: Send coins
echo ""
echo "============================================================"
echo "TEST 4: SENDING COINS"
echo "============================================================"

SEND_AMOUNT=50.0
BALANCE=$(rpc_call "getbalance" "\"$WALLET_ADDRESS\"")
echo "📊 Sender balance: $BALANCE GXC"
echo "📤 Sending $SEND_AMOUNT GXC to $TARGET_ADDRESS"

# Check if we have enough balance
REQUIRED=$(echo "$SEND_AMOUNT + 0.001" | bc 2>/dev/null || echo "50.001")
if (( $(echo "$BALANCE < $REQUIRED" | bc -l 2>/dev/null || echo 1) )); then
    echo "❌ Insufficient balance. Need $REQUIRED GXC, have $BALANCE GXC"
    SEND_RESULT="FAILED"
else
    SEND_RESULT=$(rpc_call "sendtoaddress" "\"$TARGET_ADDRESS\", $SEND_AMOUNT")
    
    if [ "$SEND_RESULT" != "null" ] && [ ! -z "$SEND_RESULT" ] && [ "${SEND_RESULT:0:1}" != "{" ]; then
        echo "✅ Transaction sent successfully!"
        echo "   TX Hash: $SEND_RESULT"
        
        # Wait a bit and check recipient balance
        sleep 2
        RECIPIENT_BALANCE=$(rpc_call "getbalance" "\"$TARGET_ADDRESS\"")
        echo "📊 Recipient balance: $RECIPIENT_BALANCE GXC"
    else
        echo "❌ Send transaction failed"
        echo "   Error: $SEND_RESULT"
        SEND_RESULT="FAILED"
    fi
fi

# Summary
echo ""
echo "============================================================"
echo "TEST SUMMARY"
echo "============================================================"
echo "Mining: ✅ ATTEMPTED"
[ "$STAKE_RESULT" != "SKIPPED" ] && [ "$STAKE_RESULT" != "FAILED" ] && echo "Staking: ✅ PASSED" || echo "Staking: ⚠️  $STAKE_RESULT"
[ "$RESTAKE_RESULT" != "SKIPPED" ] && [ "$RESTAKE_RESULT" != "FAILED" ] && echo "Restaking: ✅ PASSED" || echo "Restaking: ⚠️  $RESTAKE_RESULT"
[ "$SEND_RESULT" != "FAILED" ] && echo "Sending Coins: ✅ PASSED" || echo "Sending Coins: ❌ FAILED"

echo ""
echo "🎉 Test suite completed!"
echo ""
echo "Note: For full functionality, ensure:"
echo "  1. Node is running and synced"
echo "  2. You have sufficient balance for all operations"
echo "  3. If using a different wallet, provide private key as parameter"
