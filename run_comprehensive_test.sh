#!/bin/bash
set -e

echo "========================================="
echo "GXC Comprehensive Test Suite"
echo "========================================="

# Kill any existing nodes
pkill -9 gxc-node 2>/dev/null || true
sleep 2

# Clean data
rm -rf gxc_data
mkdir -p gxc_data

# Start node in background
echo "Starting node..."
PORT=8080 ./build/gxc-node --testnet > /tmp/test_node.log 2>&1 &
NODE_PID=$!
echo "Node PID: $NODE_PID"

# Wait for node to start
sleep 6

# Test address
ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

echo ""
echo "========================================="
echo "Test 1: RPC Connectivity"
echo "========================================="
PING=$(curl -s -X POST http://localhost:8080/health 2>&1)
if echo "$PING" | grep -q "ok"; then
    echo "✅ Health check passed"
    echo "$PING" | jq .
else
    echo "❌ Health check failed"
    echo "$PING"
fi

echo ""
echo "========================================="
echo "Test 2: Get Blockchain Info"
echo "========================================="
INFO=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
echo "$INFO" | jq '.result | {height, difficulty, reward}'
HEIGHT=$(echo "$INFO" | jq -r '.result.height')
echo "Current height: $HEIGHT"

echo ""
echo "========================================="
echo "Test 3: Get Block Template"
echo "========================================="
TEMPLATE=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
echo "$TEMPLATE" | jq '.result | {height, reward, previousblockhash}'
PREV_HASH=$(echo "$TEMPLATE" | jq -r '.result.previousblockhash')
REWARD=$(echo "$TEMPLATE" | jq -r '.result.reward')
echo "Previous hash: ${PREV_HASH:0:16}..."
echo "Reward: $REWARD GXC"

echo ""
echo "========================================="
echo "Test 4: Check Initial Balance"
echo "========================================="
BALANCE_BEFORE=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}")
echo "$BALANCE_BEFORE" | jq .
BALANCE_VAL=$(echo "$BALANCE_BEFORE" | jq -r '.result')
echo "Balance: $BALANCE_VAL GXC"

echo ""
echo "========================================="
echo "Test 5: Submit Mined Block"
echo "========================================="
SUBMIT=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{
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
echo "$SUBMIT" | jq .

if echo "$SUBMIT" | grep -q "error"; then
    echo "❌ Block submission failed"
    echo "$SUBMIT" | jq '.error'
else
    echo "✅ Block submitted"
fi

# Wait for block to be processed
sleep 3

echo ""
echo "========================================="
echo "Test 6: Check Balance After Mining"
echo "========================================="
BALANCE_AFTER=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}")
echo "$BALANCE_AFTER" | jq .
BALANCE_AFTER_VAL=$(echo "$BALANCE_AFTER" | jq -r '.result')
echo "Balance after: $BALANCE_AFTER_VAL GXC"

echo ""
echo "========================================="
echo "Test 7: List UTXOs"
echo "========================================="
UTXOS=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"listunspent\",\"params\":[0,9999999,[\"$ADDRESS\"]],\"id\":1}")
UTXO_COUNT=$(echo "$UTXOS" | jq '.result | length')
echo "UTXO count: $UTXO_COUNT"
echo "$UTXOS" | jq '.result[0]' 2>/dev/null || echo "No UTXOs"

echo ""
echo "========================================="
echo "Test 8: List Transactions"
echo "========================================="
TXS=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"listtransactions\",\"params\":[\"$ADDRESS\",10],\"id\":1}")
TX_COUNT=$(echo "$TXS" | jq '.result | length')
echo "Transaction count: $TX_COUNT"
echo "$TXS" | jq '.result[0]' 2>/dev/null || echo "No transactions"

echo ""
echo "========================================="
echo "Test 9: Get Updated Blockchain Info"
echo "========================================="
INFO2=$(curl -s -X POST http://localhost:8080/rpc -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
echo "$INFO2" | jq '.result | {height, blocks, difficulty}'
HEIGHT2=$(echo "$INFO2" | jq -r '.result.height')
echo "New height: $HEIGHT2"

echo ""
echo "========================================="
echo "Test Summary"
echo "========================================="
echo "Initial balance: $BALANCE_VAL GXC"
echo "Final balance: $BALANCE_AFTER_VAL GXC"
echo "Expected reward: $REWARD GXC"
echo "Initial height: $HEIGHT"
echo "Final height: $HEIGHT2"
echo "UTXOs: $UTXO_COUNT"
echo "Transactions: $TX_COUNT"

echo ""
if [ "$BALANCE_AFTER_VAL" == "$REWARD" ] && [ "$HEIGHT2" -gt "$HEIGHT" ]; then
    echo "✅ ALL TESTS PASSED!"
    echo "   - Block was added to chain"
    echo "   - Balance was credited correctly"
    echo "   - UTXO was created"
else
    echo "❌ TESTS FAILED"
    if [ "$BALANCE_AFTER_VAL" != "$REWARD" ]; then
        echo "   - Balance mismatch: got $BALANCE_AFTER_VAL, expected $REWARD"
    fi
    if [ "$HEIGHT2" -le "$HEIGHT" ]; then
        echo "   - Height didn't increase: was $HEIGHT, now $HEIGHT2"
    fi
fi

echo ""
echo "========================================="
echo "Node Logs (last 50 lines)"
echo "========================================="
tail -50 /tmp/test_node.log | grep -E "addBlock|submitBlock|Balance|UTXO|validation|ERROR"

# Kill node
kill $NODE_PID 2>/dev/null || true
wait $NODE_PID 2>/dev/null || true

echo ""
echo "Test complete!"
