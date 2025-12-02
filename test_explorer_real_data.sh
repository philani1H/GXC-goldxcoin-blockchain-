#!/bin/bash
set -e

echo "========================================="
echo "GXC Explorer Real Data Test"
echo "========================================="

# Clean up
pkill -9 gxc-node python3 2>/dev/null || true
sleep 2

# Clear databases
echo "Clearing old databases..."
rm -f gxc_explorer.db demo_explorer.db gxc_explorer_testnet.db gxc_data/*.db
rm -rf gxc_data
mkdir -p gxc_data

echo "✅ Databases cleared"

# Start node
echo ""
echo "Starting C++ node..."
PORT=8080 ./build/gxc-node --testnet > /tmp/explorer_node.log 2>&1 &
NODE_PID=$!
sleep 5

# Mine some blocks to have real data
echo ""
echo "Mining blocks for real data..."
ADDRESS="tGXC9fab7317231b966af85ac453e168c0932"

for i in {1..3}; do
    echo "Mining block $i..."
    
    # Get template
    TEMPLATE=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"gxc_getBlockTemplate","params":[],"id":1}')
    HEIGHT=$(echo "$TEMPLATE" | jq -r '.result.height')
    PREV=$(echo "$TEMPLATE" | jq -r '.result.previousblockhash')
    
    # Submit block
    HASH=$(printf "000000000000%050d" $i)
    curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_submitBlock\",\"params\":[{\"height\":$HEIGHT,\"hash\":\"$HASH\",\"previousblockhash\":\"$PREV\",\"nonce\":$((123456 + i)),\"miner\":\"$ADDRESS\",\"timestamp\":$(date +%s),\"difficulty\":1000}],\"id\":1}" > /dev/null
    
    echo "  Block $i submitted"
    sleep 1
done

# Check blockchain state
echo ""
echo "Checking blockchain state..."
INFO=$(curl -s -X POST http://localhost:8080/rpc -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')
HEIGHT=$(echo "$INFO" | jq -r '.result.height')
BALANCE=$(curl -s -X POST http://localhost:8080/rpc -d "{\"jsonrpc\":\"2.0\",\"method\":\"gxc_getBalance\",\"params\":[\"$ADDRESS\"],\"id\":1}" | jq -r '.result')

echo "  Height: $HEIGHT"
echo "  Balance: $BALANCE GXC"

# Start explorer
echo ""
echo "Starting explorer..."
cd web
python3 blockchain_explorer.py > /tmp/explorer_app.log 2>&1 &
EXPLORER_PID=$!
cd ..
sleep 5

# Test explorer endpoints
echo ""
echo "Testing explorer endpoints..."

echo "1. Home page..."
HOME=$(curl -s http://localhost:3000/ | grep -o "Total Blocks\|Recent Blocks\|Recent Transactions" | wc -l)
if [ "$HOME" -gt 0 ]; then
    echo "   ✅ Home page loads"
else
    echo "   ❌ Home page failed"
fi

echo "2. Blocks page..."
BLOCKS=$(curl -s http://localhost:3000/blocks | grep -o "Block #\|Height\|Miner" | wc -l)
if [ "$BLOCKS" -gt 0 ]; then
    echo "   ✅ Blocks page loads"
else
    echo "   ❌ Blocks page failed"
fi

echo "3. Transactions page..."
TXS=$(curl -s http://localhost:3000/transactions | grep -o "Transaction\|Hash\|Amount" | wc -l)
if [ "$TXS" -gt 0 ]; then
    echo "   ✅ Transactions page loads"
else
    echo "   ❌ Transactions page failed"
fi

echo "4. Address page..."
ADDR=$(curl -s "http://localhost:3000/address/$ADDRESS" | grep -o "Balance\|Transactions\|Address" | wc -l)
if [ "$ADDR" -gt 0 ]; then
    echo "   ✅ Address page loads"
else
    echo "   ❌ Address page failed"
fi

echo "5. API endpoint..."
API=$(curl -s http://localhost:3000/api/stats | jq -r '.total_blocks')
if [ "$API" != "null" ] && [ "$API" != "" ]; then
    echo "   ✅ API returns data: $API blocks"
else
    echo "   ❌ API failed"
fi

# Check database
echo ""
echo "Checking explorer database..."
if [ -f "gxc_explorer.db" ]; then
    BLOCK_COUNT=$(sqlite3 gxc_explorer.db "SELECT COUNT(*) FROM blocks;" 2>/dev/null || echo "0")
    TX_COUNT=$(sqlite3 gxc_explorer.db "SELECT COUNT(*) FROM transactions;" 2>/dev/null || echo "0")
    echo "  Blocks in DB: $BLOCK_COUNT"
    echo "  Transactions in DB: $TX_COUNT"
    
    if [ "$BLOCK_COUNT" -gt 0 ]; then
        echo "  ✅ Database has real data"
    else
        echo "  ⚠️  Database is empty"
    fi
else
    echo "  ⚠️  Database not found"
fi

# Summary
echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "Blockchain height: $HEIGHT"
echo "Miner balance: $BALANCE GXC"
echo "Explorer database: $([ -f 'gxc_explorer.db' ] && echo 'Created' || echo 'Missing')"
echo "Explorer running: $(ps aux | grep blockchain_explorer.py | grep -v grep > /dev/null && echo 'Yes' || echo 'No')"
echo ""
echo "Access explorer at: http://localhost:3000"
echo ""
echo "Press Ctrl+C to stop services..."

# Keep running
wait $NODE_PID $EXPLORER_PID
