#!/bin/bash

pkill -9 gxc-node 2>/dev/null
rm -rf gxc_data
PORT=8080 ./build/gxc-node --testnet > /tmp/qt.log 2>&1 &
NODE_PID=$!
sleep 5

ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

echo "=== 1. Health Check ==="
curl -s http://localhost:8080/health | jq .

echo ""
echo "=== 2. Get Template ==="
TEMPLATE=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
echo "$TEMPLATE" | jq '.result | {height, reward}'
PREV=$(echo "$TEMPLATE" | jq -r '.result.previousblockhash')

echo ""
echo "=== 3. Submit Block ==="
# Use a hash with enough leading zeros for difficulty 1000 (needs ~12 zeros)
curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_submitBlock\",\"params\":[{\"height\":1,\"hash\":\"000000000000abcdef1234567890abcdef1234567890abcdef1234567890ab\",\"previousblockhash\":\"$PREV\",\"nonce\":123456,\"miner\":\"$ADDRESS\",\"timestamp\":$(date +%s),\"difficulty\":1000}],\"id\":1}" | jq .

sleep 2

echo ""
echo "=== 4. Check Balance ==="
curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq .

echo ""
echo "=== 5. Check Logs ==="
tail -30 /tmp/qt.log | grep -E "addBlock|Balance|UTXO|validation"

kill $NODE_PID 2>/dev/null
