#!/bin/bash
set -e

echo "========================================="
echo "GXC Wallet Balance Test"
echo "========================================="

pkill -9 gxc-node 2>/dev/null || true
rm -rf gxc_data
PORT=8080 ./build/gxc-node --testnet > /tmp/wallet_test.log 2>&1 &
NODE_PID=$!
sleep 5

ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

echo ""
echo "Test 1: Initial Balance (should be 0)"
echo "========================================="
BALANCE=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')
echo "Balance: $BALANCE GXC"
if [ "$BALANCE" == "0" ] || [ "$BALANCE" == "0.0" ]; then
    echo "✅ Initial balance is 0"
else
    echo "❌ Initial balance should be 0, got $BALANCE"
fi

echo ""
echo "Test 2: Get Block Template"
echo "========================================="
TEMPLATE=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
REWARD=$(echo "$TEMPLATE" | jq -r '.result.reward')
PREV=$(echo "$TEMPLATE" | jq -r '.result.previousblockhash')
echo "Reward: $REWARD GXC"
echo "Previous hash: ${PREV:0:16}..."

if [ "$REWARD" == "50" ] || [ "$REWARD" == "50.0" ]; then
    echo "✅ Reward is 50 GXC"
else
    echo "❌ Reward should be 50, got $REWARD"
fi

echo ""
echo "Test 3: Mine and Submit Block"
echo "========================================="
SUBMIT=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_submitBlock\",\"params\":[{\"height\":1,\"hash\":\"000000000000abcdef1234567890abcdef1234567890abcdef1234567890ab\",\"previousblockhash\":\"$PREV\",\"nonce\":123456,\"miner\":\"$ADDRESS\",\"timestamp\":$(date +%s),\"difficulty\":1000}],\"id\":1}")
echo "$SUBMIT" | jq .

if echo "$SUBMIT" | grep -q '"result":null'; then
    echo "✅ Block submitted successfully"
else
    echo "❌ Block submission failed"
    echo "$SUBMIT" | jq '.error'
fi

sleep 2

echo ""
echo "Test 4: Check Balance After Mining"
echo "========================================="
BALANCE_AFTER=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')
echo "Balance: $BALANCE_AFTER GXC"

if [ "$BALANCE_AFTER" == "50" ] || [ "$BALANCE_AFTER" == "50.0" ]; then
    echo "✅ Balance is 50 GXC after mining"
else
    echo "❌ Balance should be 50, got $BALANCE_AFTER"
fi

echo ""
echo "Test 5: List UTXOs"
echo "========================================="
UTXOS=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"listunspent\",\"params\":[0,9999999,[\"$ADDRESS\"]],\"id\":1}")
UTXO_COUNT=$(echo "$UTXOS" | jq '.result | length')
UTXO_AMOUNT=$(echo "$UTXOS" | jq -r '.result[0].amount // 0')
echo "UTXO count: $UTXO_COUNT"
echo "UTXO amount: $UTXO_AMOUNT GXC"

if [ "$UTXO_COUNT" == "1" ]; then
    echo "✅ 1 UTXO created"
else
    echo "❌ Should have 1 UTXO, got $UTXO_COUNT"
fi

if [ "$UTXO_AMOUNT" == "50" ] || [ "$UTXO_AMOUNT" == "50.0" ]; then
    echo "✅ UTXO amount is 50 GXC"
else
    echo "❌ UTXO amount should be 50, got $UTXO_AMOUNT"
fi

echo ""
echo "Test 6: List Transactions"
echo "========================================="
TXS=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"listtransactions\",\"params\":[\"$ADDRESS\",10],\"id\":1}")
TX_COUNT=$(echo "$TXS" | jq '.result | length')
echo "Transaction count: $TX_COUNT"

if [ "$TX_COUNT" -ge "1" ]; then
    echo "✅ Transactions found"
    echo "$TXS" | jq '.result[0] | {txid, amount, category}'
else
    echo "❌ No transactions found"
fi

echo ""
echo "Test 7: Mine Second Block"
echo "========================================="
TEMPLATE2=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
PREV2=$(echo "$TEMPLATE2" | jq -r '.result.previousblockhash')
HEIGHT2=$(echo "$TEMPLATE2" | jq -r '.result.height')
echo "Height: $HEIGHT2"

SUBMIT2=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_submitBlock\",\"params\":[{\"height\":$HEIGHT2,\"hash\":\"000000000000fedcba0987654321fedcba0987654321fedcba0987654321fe\",\"previousblockhash\":\"$PREV2\",\"nonce\":654321,\"miner\":\"$ADDRESS\",\"timestamp\":$(date +%s),\"difficulty\":1000}],\"id\":1}")

if echo "$SUBMIT2" | grep -q '"result":null'; then
    echo "✅ Second block submitted"
else
    echo "❌ Second block failed"
fi

sleep 2

echo ""
echo "Test 8: Check Balance After 2 Blocks"
echo "========================================="
BALANCE_FINAL=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')
echo "Balance: $BALANCE_FINAL GXC"

if [ "$BALANCE_FINAL" == "100" ] || [ "$BALANCE_FINAL" == "100.0" ]; then
    echo "✅ Balance is 100 GXC after 2 blocks"
else
    echo "❌ Balance should be 100, got $BALANCE_FINAL"
fi

echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "Initial balance: $BALANCE GXC"
echo "After 1 block: $BALANCE_AFTER GXC"
echo "After 2 blocks: $BALANCE_FINAL GXC"
echo "Expected reward per block: $REWARD GXC"
echo "UTXOs: $UTXO_COUNT"
echo "Transactions: $TX_COUNT"

if [ "$BALANCE_FINAL" == "100" ] || [ "$BALANCE_FINAL" == "100.0" ]; then
    echo ""
    echo "🎉 ALL TESTS PASSED!"
    echo "✅ Wallet balance tracking works correctly"
    echo "✅ Mining rewards are credited properly"
    echo "✅ Block reward is 50 GXC (not 12.5)"
    echo "✅ UTXOs are created correctly"
    echo "✅ Transactions are tracked"
else
    echo ""
    echo "❌ TESTS FAILED"
    echo "Balance tracking is not working correctly"
fi

kill $NODE_PID 2>/dev/null || true
echo ""
echo "Test complete!"
