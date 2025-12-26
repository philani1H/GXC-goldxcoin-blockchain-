#!/bin/bash
# Quick stress test - smaller scale for immediate testing

set -e

echo "=========================================="
echo "GXC Quick Stress Test"
echo "=========================================="
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

DATA_DIR="./gxc_quick_test_data"
NODE_PID=""

cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up...${NC}"
    if [ ! -z "$NODE_PID" ]; then
        kill $NODE_PID 2>/dev/null || true
        wait $NODE_PID 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

# Start node
echo -e "${CYAN}Starting node...${NC}"
rm -rf "$DATA_DIR"
mkdir -p "$DATA_DIR"

./gxc-node > quick_test.log 2>&1 &
NODE_PID=$!

echo -e "${YELLOW}Waiting for node...${NC}"
for i in {1..20}; do
    if curl -s -X POST http://localhost:8332 \
        -H "Content-Type: application/json" \
        -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
        | grep -q "blocks"; then
        echo -e "${GREEN}✓ Node ready${NC}"
        break
    fi
    sleep 2
done

echo ""
echo -e "${CYAN}Running quick tests...${NC}"
echo ""

# Test 1: 100 miners
echo -e "${CYAN}Test 1: 100 concurrent miners${NC}"
python3 test_multi_miner.py --miners 100 --template-test

echo ""

# Test 2: 10 mining miners
echo -e "${CYAN}Test 2: 10 miners competing${NC}"
python3 test_multi_miner.py --miners 10 --max-attempts 5000

echo ""
echo -e "${GREEN}Quick stress test complete!${NC}"
