#!/bin/bash
# Run comprehensive stress tests on GXC blockchain

set -e

echo "=========================================="
echo "GXC Blockchain Stress Test Suite"
echo "=========================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
DATA_DIR="./gxc_test_data"
NODE_PID=""

# Cleanup function
cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up...${NC}"
    if [ ! -z "$NODE_PID" ]; then
        echo -e "${YELLOW}Stopping blockchain node (PID: $NODE_PID)...${NC}"
        kill $NODE_PID 2>/dev/null || true
        wait $NODE_PID 2>/dev/null || true
    fi
    echo -e "${GREEN}Cleanup complete${NC}"
}

# Set trap to cleanup on exit
trap cleanup EXIT INT TERM

# Step 1: Start blockchain node
echo -e "${CYAN}Step 1: Starting blockchain node...${NC}"
rm -rf "$DATA_DIR"
mkdir -p "$DATA_DIR"

# Start node in background
./gxc-node > node_stress_test.log 2>&1 &
NODE_PID=$!

echo -e "${GREEN}✓ Node started (PID: $NODE_PID)${NC}"
echo -e "${YELLOW}Waiting for node to initialize...${NC}"

# Wait for node to be ready
for i in {1..30}; do
    if curl -s -X POST http://localhost:8332 \
        -H "Content-Type: application/json" \
        -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
        | grep -q "blocks"; then
        echo -e "${GREEN}✓ Node is ready${NC}"
        break
    fi
    
    if [ $i -eq 30 ]; then
        echo -e "${RED}✗ Node failed to start${NC}"
        cat node_stress_test.log
        exit 1
    fi
    
    echo -e "${BLUE}  Waiting... ($i/30)${NC}"
    sleep 2
done

# Get initial blockchain info
echo ""
echo -e "${CYAN}Blockchain Status:${NC}"
curl -s -X POST http://localhost:8332 \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
    | python3 -m json.tool

echo ""
echo "=========================================="
echo ""

# Step 2: Run 60,000 miner stress test
echo -e "${CYAN}Step 2: Running 60,000 Miner Stress Test...${NC}"
echo ""

python3 stress_test_60000_miners.py

echo ""
echo "=========================================="
echo ""

# Step 3: Run validator stress test
echo -e "${CYAN}Step 3: Running Validator Stress Test...${NC}"
echo ""

python3 stress_test_validators.py

echo ""
echo "=========================================="
echo ""

# Step 4: Check final blockchain state
echo -e "${CYAN}Step 4: Final Blockchain State${NC}"
echo ""

curl -s -X POST http://localhost:8332 \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}' \
    | python3 -m json.tool

echo ""
echo "=========================================="
echo -e "${GREEN}All stress tests completed!${NC}"
echo "=========================================="
echo ""
echo "Results saved in:"
echo "  - stress_test_results_*.json"
echo "  - validator_stress_test_results_*.json"
echo "  - node_stress_test.log"
echo ""
