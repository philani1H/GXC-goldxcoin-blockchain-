#!/bin/bash

# Complete mining test script
set -e

echo "========================================="
echo "GXC Mining Complete Test"
echo "========================================="

# Kill any existing node
pkill -9 gxc-node 2>/dev/null || true
sleep 2

# Clean data
rm -rf gxc_data
mkdir -p gxc_data

# Start node
echo "Starting C++ testnet node..."
PORT=8080 ./build/gxc-node --testnet > /tmp/test_node.log 2>&1 &
NODE_PID=$!
echo "Node PID: $NODE_PID"

# Wait for node to start
sleep 5

# Test address
ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

echo ""
echo "========================================="
echo "Step 1: Check initial balance"
echo "========================================="
BALANCE=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r .result)
echo "Initial balance: $BALANCE GXC"

echo ""
echo "========================================="
echo "Step 2: Get block template"
echo "========================================="
TEMPLATE=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
echo "Template received:"
echo "$TEMPLATE" | jq '{height, reward, previousblockhash}'

PREV_HASH=$(echo "$TEMPLATE" | jq -r .result.previousblockhash)
HEIGHT=$(echo "$TEMPLATE" | jq -r .result.height)
REWARD=$(echo "$TEMPLATE" | jq -r .result.reward)

echo ""
echo "Height: $HEIGHT"
echo "Previous hash: ${PREV_HASH:0:16}..."
echo "Reward: $REWARD GXC"

echo ""
echo "========================================="
echo "Step 3: Submit mined block"
echo "========================================="

SUBMIT_RESULT=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{
  \"jsonrpc\":\"2.0\",
  \"method\":\"gxc_submitBlock\",
  \"params\":[{
    \"height\":$HEIGHT,
    \"hash\":\"0000abcd1234567890abcdef1234567890\",
    \"previousblockhash\":\"$PREV_HASH\",
    \"nonce\":12345,
    \"miner\":\"$ADDRESS\",
    \"timestamp\":$(date +%s),
    \"difficulty\":1000.0
  }],
  \"id\":1
}")

echo "Submit result:"
echo "$SUBMIT_RESULT" | jq .

# Wait for block to be processed
sleep 3

echo ""
echo "========================================="
echo "Step 4: Check balance after mining"
echo "========================================="
BALANCE_AFTER=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r .result)
echo "Balance after mining: $BALANCE_AFTER GXC"

echo ""
echo "========================================="
echo "Step 5: List UTXOs"
echo "========================================="
UTXOS=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"listunspent\",\"params\":[0,9999999,[\"$ADDRESS\"]],\"id\":1}")
echo "UTXOs:"
echo "$UTXOS" | jq '.result | length' | xargs echo "Count:"
echo "$UTXOS" | jq '.result[0]' 2>/dev/null || echo "No UTXOs found"

echo ""
echo "========================================="
echo "Step 6: List transactions"
echo "========================================="
TXS=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"listtransactions\",\"params\":[\"$ADDRESS\",10],\"id\":1}")
echo "Transactions:"
echo "$TXS" | jq '.result | length' | xargs echo "Count:"
echo "$TXS" | jq '.result[0]' 2>/dev/null || echo "No transactions found"

echo ""
echo "========================================="
echo "Step 7: Get blockchain info"
echo "========================================="
INFO=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
echo "Blockchain info:"
echo "$INFO" | jq '.result | {height, blocks, difficulty, reward}'

echo ""
echo "========================================="
echo "Test Summary"
echo "========================================="
echo "Initial balance: $BALANCE GXC"
echo "Final balance: $BALANCE_AFTER GXC"
echo "Expected reward: $REWARD GXC"

if [ "$BALANCE_AFTER" == "$REWARD" ]; then
    echo "✅ SUCCESS: Balance matches expected reward!"
else
    echo "❌ FAIL: Balance ($BALANCE_AFTER) does not match reward ($REWARD)"
fi

echo ""
echo "Check node logs:"
echo "tail -50 /tmp/test_node.log"

# Kill node
kill $NODE_PID 2>/dev/null || true
wait $NODE_PID 2>/dev/null || true

echo ""
echo "Test complete!"
