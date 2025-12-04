#!/bin/bash

# Simple script to: Start node, mine, send coins, show transaction

RPC_URL="http://localhost:8332/rpc"
TARGET="tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"

echo "🚀 GXC Testnet: Mine and Send Coins"
echo "===================================="
echo ""

# Start node in background if not running
if ! curl -s "$RPC_URL" > /dev/null 2>&1; then
    echo "Starting node..."
    ./gxc-node > /tmp/node.log 2>&1 &
    NODE_PID=$!
    echo "Waiting for node..."
    for i in {1..20}; do
        sleep 1
        if curl -s "$RPC_URL" > /dev/null 2>&1; then
            echo "✅ Node ready!"
            break
        fi
    done
fi

# Get wallet address (node's default wallet)
echo ""
echo "1. Getting wallet address..."
WALLET=$(curl -s -X POST "$RPC_URL" \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}' | \
    jq -r '.result // empty')

if [ -z "$WALLET" ] || [ "$WALLET" = "null" ]; then
    # Try to get from sendtoaddress error or use node's internal wallet
    echo "   Using node's default wallet (will be revealed in transaction)"
    WALLET="NODE_WALLET"
else
    echo "✅ Wallet: $WALLET"
fi

# Mine some blocks
echo ""
echo "2. Mining blocks..."
for i in {1..3}; do
    echo "   Mining block $i..."
    TEMPLATE=$(curl -s -X POST "$RPC_URL" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"getblocktemplate\",\"params\":[{\"miner_address\":\"$WALLET\"}],\"id\":1}")
    
    sleep 1
done

sleep 3

# Check balance
echo ""
echo "3. Checking balance..."
if [ "$WALLET" != "NODE_WALLET" ]; then
    BALANCE=$(curl -s -X POST "$RPC_URL" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$WALLET\"],\"id\":1}" | \
        jq -r '.result // 0')
    echo "   Balance: $BALANCE GXC"
fi

# Send coins
echo ""
echo "4. Sending 50 GXC to $TARGET..."
TX_RESULT=$(curl -s -X POST "$RPC_URL" \
    -H "Content-Type: application/json" \
    -d "{\"jsonrpc\":\"2.0\",\"method\":\"sendtoaddress\",\"params\":[\"$TARGET\", 50.0],\"id\":1}")

TX_HASH=$(echo "$TX_RESULT" | jq -r '.result // empty')

if [ -z "$TX_HASH" ] || [ "$TX_HASH" = "null" ]; then
    ERROR=$(echo "$TX_RESULT" | jq -r '.error.message // .error // "Unknown error"')
    echo "❌ Failed: $ERROR"
    echo ""
    echo "Full response:"
    echo "$TX_RESULT" | jq '.'
    exit 1
fi

echo "✅ Transaction sent!"
echo "   TX Hash: $TX_HASH"
echo ""

# Get transaction details
echo "5. Transaction Details:"
echo "===================================="
TX_DETAILS=$(curl -s -X POST "$RPC_URL" \
    -H "Content-Type: application/json" \
    -d "{\"jsonrpc\":\"2.0\",\"method\":\"gettransaction\",\"params\":[\"$TX_HASH\"],\"id\":1}")

echo "$TX_DETAILS" | jq '.'
echo ""

# Check recipient balance
echo "6. Recipient balance:"
RECIPIENT_BAL=$(curl -s -X POST "$RPC_URL" \
    -H "Content-Type: application/json" \
    -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$TARGET\"],\"id\":1}" | \
    jq -r '.result // 0')
echo "   $TARGET: $RECIPIENT_BAL GXC"
echo ""

echo "✅ Complete!"
echo ""
echo "View transaction:"
echo "curl -X POST $RPC_URL -H 'Content-Type: application/json' \\"
echo "  -d '{\"jsonrpc\":\"2.0\",\"method\":\"gettransaction\",\"params\":[\"$TX_HASH\"],\"id\":1}'"
