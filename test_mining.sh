#!/bin/bash

# Test Mining Script
# This script tests the GXC miner to ensure blocks are submitted and rewards are distributed

echo "========================================"
echo "GXC Mining Test"
echo "========================================"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
MINER_ADDRESS="GXC9fab7317231b966af85ac453e168c0932"  # Genesis address for testing
NODE_URL="http://localhost:8332"
ALGORITHM="gxhash"
THREADS=2

echo ""
echo "Configuration:"
echo "  Miner Address: $MINER_ADDRESS"
echo "  Node URL: $NODE_URL"
echo "  Algorithm: $ALGORITHM"
echo "  Threads: $THREADS"
echo ""

# Check if node is running
echo "Checking if node is running..."
if ! curl -s "$NODE_URL/api/getinfo" > /dev/null 2>&1; then
    echo -e "${RED}❌ ERROR: Node is not running at $NODE_URL${NC}"
    echo "Please start the node first:"
    echo "  ./gxc-node --testnet"
    exit 1
fi

echo -e "${GREEN}✅ Node is running${NC}"

# Get initial blockchain height
INITIAL_HEIGHT=$(curl -s "$NODE_URL/api/getinfo" | jq -r '.result.height' 2>/dev/null)
echo "Initial blockchain height: $INITIAL_HEIGHT"

# Get initial balance
echo "Checking initial balance..."
INITIAL_BALANCE=$(curl -s "$NODE_URL/api/getbalance?address=$MINER_ADDRESS" | jq -r '.balance' 2>/dev/null)
echo "Initial balance: $INITIAL_BALANCE GXC"

echo ""
echo "========================================"
echo "Starting Miner"
echo "========================================"
echo "The miner will run for 30 seconds..."
echo "Press Ctrl+C to stop early"
echo ""

# Start miner in background
timeout 30 ./gxc-miner \
    --address=$MINER_ADDRESS \
    --algorithm=$ALGORITHM \
    --threads=$THREADS \
    --node=$NODE_URL \
    --verbose &

MINER_PID=$!

# Wait for miner to start
sleep 2

echo "Miner started with PID: $MINER_PID"
echo "Mining in progress..."

# Monitor progress
for i in {1..28}; do
    sleep 1
    echo -n "."
done
echo ""

# Wait for miner to finish
wait $MINER_PID 2>/dev/null

echo ""
echo "========================================"
echo "Mining Test Complete"
echo "========================================"

# Get final blockchain height
FINAL_HEIGHT=$(curl -s "$NODE_URL/api/getinfo" | jq -r '.result.height' 2>/dev/null)
echo "Final blockchain height: $FINAL_HEIGHT"

# Get final balance
FINAL_BALANCE=$(curl -s "$NODE_URL/api/getbalance?address=$MINER_ADDRESS" | jq -r '.balance' 2>/dev/null)
echo "Final balance: $FINAL_BALANCE GXC"

# Calculate results
BLOCKS_MINED=$((FINAL_HEIGHT - INITIAL_HEIGHT))
REWARD_EARNED=$(echo "$FINAL_BALANCE - $INITIAL_BALANCE" | bc 2>/dev/null)

echo ""
echo "========================================"
echo "Results"
echo "========================================"
echo "Blocks mined: $BLOCKS_MINED"
echo "Reward earned: $REWARD_EARNED GXC"
echo ""

# Verify results
if [ "$BLOCKS_MINED" -gt 0 ]; then
    echo -e "${GREEN}✅ SUCCESS: Blocks were mined and submitted!${NC}"
    
    if [ "$(echo "$REWARD_EARNED > 0" | bc)" -eq 1 ]; then
        echo -e "${GREEN}✅ SUCCESS: Rewards were distributed!${NC}"
        echo ""
        echo "Mining system is working correctly:"
        echo "  - Blocks are being mined"
        echo "  - Blocks are being submitted to blockchain"
        echo "  - Rewards are being distributed to miner"
    else
        echo -e "${YELLOW}⚠️  WARNING: No rewards received${NC}"
        echo "Blocks were mined but rewards may not be distributed yet"
        echo "This could be normal if blocks are still being processed"
    fi
else
    echo -e "${RED}❌ FAILURE: No blocks were mined${NC}"
    echo ""
    echo "Possible issues:"
    echo "  1. Difficulty too high for CPU mining"
    echo "  2. Mining algorithm not working correctly"
    echo "  3. Block submission failing"
    echo ""
    echo "Check the miner logs for more details"
fi

echo ""
echo "========================================"
echo "Blockchain Status"
echo "========================================"

# Get latest block info
LATEST_BLOCK=$(curl -s "$NODE_URL/api/getblock?height=$FINAL_HEIGHT" 2>/dev/null)
if [ ! -z "$LATEST_BLOCK" ]; then
    echo "Latest block:"
    echo "$LATEST_BLOCK" | jq '.' 2>/dev/null || echo "$LATEST_BLOCK"
fi

echo ""
echo "Test complete!"
