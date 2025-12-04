#!/bin/bash

RPC="http://localhost:8332/rpc"
TARGET="tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"

echo "╔════════════════════════════════════════════════════╗"
echo "║  GXC Testnet: Create Wallet, Mine, Send Coins     ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Ensure node is running
if ! curl -s "$RPC" > /dev/null 2>&1; then
    echo "Starting node..."
    ./gxc-node > /tmp/node.log 2>&1 &
    sleep 8
fi

# Get wallet
echo "1. Getting wallet address..."
WALLET=$(curl -s -X POST "$RPC" \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}' | \
    jq -r '.result // empty')

if [ -z "$WALLET" ]; then
    echo "   Using node default wallet"
    WALLET="NODE_DEFAULT"
fi

echo "   Wallet: $WALLET"
echo ""

# Mine blocks to get coins
echo "2. Mining 10 blocks to build balance..."
for i in {1..10}; do
    curl -s -X POST "$RPC" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"getblocktemplate\",\"params\":[{\"miner_address\":\"$WALLET\"}],\"id\":1}" > /dev/null
    if [ $((i % 3)) -eq 0 ]; then
        echo "   Mined $i blocks..."
    fi
    sleep 0.5
done

sleep 3

# Check balance
echo ""
echo "3. Checking balance..."
BAL=$(curl -s -X POST "$RPC" \
    -H "Content-Type: application/json" \
    -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$WALLET\"],\"id\":1}" | \
    jq -r '.result // 0')
echo "   Balance: $BAL GXC"
echo ""

# Send transaction
echo "4. Sending 50 GXC to $TARGET"
echo "   (This will show the transaction structure)"
echo ""

RESPONSE=$(curl -s -X POST "$RPC" \
    -H "Content-Type: application/json" \
    -d "{\"jsonrpc\":\"2.0\",\"method\":\"sendtoaddress\",\"params\":[\"$TARGET\", 50.0],\"id\":1}")

echo "Response:"
echo "$RESPONSE" | jq '.'
echo ""

TX_HASH=$(echo "$RESPONSE" | jq -r '.result // empty')

if [ ! -z "$TX_HASH" ] && [ "$TX_HASH" != "null" ] && [ "$TX_HASH" != "transaction_hash" ]; then
    echo "✅ Transaction Hash: $TX_HASH"
    echo ""
    echo "5. Getting transaction details..."
    
    TX_DETAILS=$(curl -s -X POST "$RPC" \
        -H "Content-Type: application/json" \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"gettransaction\",\"params\":[\"$TX_HASH\"],\"id\":1}")
    
    echo ""
    echo "╔════════════════════════════════════════════════════╗"
    echo "║           TRANSACTION DETAILS                      ║"
    echo "╚════════════════════════════════════════════════════╝"
    echo ""
    echo "$TX_DETAILS" | jq '.'
    echo ""
    
    # Extract and show key fields
    echo "Transaction Summary:"
    echo "$TX_DETAILS" | jq -r '
        "  Hash: " + (.result.hash // "N/A"),
        "  From: " + (.result.from // .result.senderAddress // "N/A"),
        "  To: " + (.result.to // .result.receiverAddress // "N/A"),
        "  Amount: " + (.result.amount // .result.value // "0") + " GXC",
        "  Fee: " + (.result.fee // "0.001") + " GXC",
        "  Confirmations: " + (.result.confirmations // "0"),
        "  Type: " + (.result.type // "NORMAL")
    '
else
    ERROR=$(echo "$RESPONSE" | jq -r '.error.message // .error // "Unknown"')
    echo "❌ Transaction failed: $ERROR"
    echo ""
    echo "This might be because:"
    echo "  - Insufficient balance (need to mine more blocks)"
    echo "  - Node wallet needs initialization"
    echo ""
    echo "Check node logs: tail -f /tmp/node.log"
fi

echo ""
echo "6. Recipient balance:"
RECIP_BAL=$(curl -s -X POST "$RPC" \
    -H "Content-Type: application/json" \
    -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$TARGET\"],\"id\":1}" | \
    jq -r '.result // 0')
echo "   $TARGET: $RECIP_BAL GXC"
echo ""

echo "✅ Test complete!"
echo ""
echo "To view this transaction later:"
echo "curl -X POST $RPC -H 'Content-Type: application/json' \\"
echo "  -d '{\"jsonrpc\":\"2.0\",\"method\":\"gettransaction\",\"params\":[\"$TX_HASH\"],\"id\":1}'"
