#!/bin/bash
set -e

echo "========================================="
echo "GXC Data Persistence Test"
echo "========================================="

# Clean up
pkill -9 gxc-node 2>/dev/null || true
rm -rf gxc_data
mkdir -p gxc_data

ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

echo ""
echo "Phase 1: Start node and mine blocks"
echo "========================================="
PORT=8080 ./build/gxc-node --testnet > /tmp/persist_test1.log 2>&1 &
NODE_PID=$!
sleep 5

# Mine 3 blocks
for i in 1 2 3; do
    echo "Mining block $i..."
    T=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
    H=$(echo "$T" | jq -r '.result.height')
    P=$(echo "$T" | jq -r '.result.previousblockhash')
    HASH=$(printf "000000000000%052d" $i)
    curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_submitBlock\",\"params\":[{\"height\":$H,\"hash\":\"$HASH\",\"previousblockhash\":\"$P\",\"nonce\":$i,\"miner\":\"$ADDRESS\",\"timestamp\":$(date +%s),\"difficulty\":1000}],\"id\":1}" > /dev/null
    sleep 1
done

# Check state before restart
echo ""
echo "Checking state before restart..."
INFO1=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
HEIGHT1=$(echo "$INFO1" | jq -r '.result.height')
BALANCE1=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')
TXS1=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"listtransactions\",\"params\":[\"$ADDRESS\",10],\"id\":1}" | jq '.result | length')

echo "  Height: $HEIGHT1"
echo "  Balance: $BALANCE1 GXC"
echo "  Transactions: $TXS1"

# Stop node
echo ""
echo "Stopping node..."
kill $NODE_PID
wait $NODE_PID 2>/dev/null || true
sleep 2

echo ""
echo "Phase 2: Restart node and verify data"
echo "========================================="
PORT=8080 ./build/gxc-node --testnet > /tmp/persist_test2.log 2>&1 &
NODE_PID=$!
sleep 5

# Check state after restart
echo "Checking state after restart..."
INFO2=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
HEIGHT2=$(echo "$INFO2" | jq -r '.result.height')
BALANCE2=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')
TXS2=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"listtransactions\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq '.result | length')

echo "  Height: $HEIGHT2"
echo "  Balance: $BALANCE2 GXC"
echo "  Transactions: $TXS2"

# Verify persistence
echo ""
echo "========================================="
echo "Persistence Verification"
echo "========================================="

if [ "$HEIGHT1" == "$HEIGHT2" ]; then
    echo "✅ Height persisted: $HEIGHT1 == $HEIGHT2"
else
    echo "❌ Height NOT persisted: $HEIGHT1 != $HEIGHT2"
fi

if [ "$BALANCE1" == "$BALANCE2" ]; then
    echo "✅ Balance persisted: $BALANCE1 == $BALANCE2"
else
    echo "❌ Balance NOT persisted: $BALANCE1 != $BALANCE2"
fi

if [ "$TXS1" == "$TXS2" ]; then
    echo "✅ Transactions persisted: $TXS1 == $TXS2"
else
    echo "❌ Transactions NOT persisted: $TXS1 != $TXS2"
fi

# Check database file
echo ""
echo "Database files:"
ls -lh gxc_data/*.db 2>/dev/null || echo "No database files found"

# Mine one more block after restart
echo ""
echo "Phase 3: Mine after restart"
echo "========================================="
echo "Mining block 4..."
T=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
H=$(echo "$T" | jq -r '.result.height')
P=$(echo "$T" | jq -r '.result.previousblockhash')
HASH=$(printf "000000000000%050d" 4)
curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_submitBlock\",\"params\":[{\"height\":$H,\"hash\":\"$HASH\",\"previousblockhash\":\"$P\",\"nonce\":4,\"miner\":\"$ADDRESS\",\"timestamp\":$(date +%s),\"difficulty\":1000}],\"id\":1}" > /dev/null
sleep 1

# Final state
INFO3=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
HEIGHT3=$(echo "$INFO3" | jq -r '.result.height')
BALANCE3=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')

echo "  Final height: $HEIGHT3"
echo "  Final balance: $BALANCE3 GXC"

if [ "$HEIGHT3" -gt "$HEIGHT2" ]; then
    echo "✅ Can mine after restart"
else
    echo "❌ Cannot mine after restart"
fi

# Summary
echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "Before restart: Height=$HEIGHT1, Balance=$BALANCE1 GXC, TXs=$TXS1"
echo "After restart:  Height=$HEIGHT2, Balance=$BALANCE2 GXC, TXs=$TXS2"
echo "After mining:   Height=$HEIGHT3, Balance=$BALANCE3 GXC"

if [ "$HEIGHT1" == "$HEIGHT2" ] && [ "$BALANCE1" == "$BALANCE2" ] && [ "$TXS1" == "$TXS2" ] && [ "$HEIGHT3" -gt "$HEIGHT2" ]; then
    echo ""
    echo "🎉 ALL PERSISTENCE TESTS PASSED!"
    echo "✅ Blocks survive restart"
    echo "✅ Balances survive restart"
    echo "✅ Transactions survive restart"
    echo "✅ Can continue mining after restart"
else
    echo ""
    echo "❌ PERSISTENCE TESTS FAILED"
fi

kill $NODE_PID 2>/dev/null || true
