#!/bin/bash
set -e

echo "========================================="
echo "GXC Complete System Test"
echo "========================================="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
MINER_ADDRESS="tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
RPC_URL="http://localhost:8332"

# Clean up old processes
echo "Cleaning up old processes..."
pkill -9 gxc-node 2>/dev/null || true
sleep 2

# Clean blockchain data
echo "Cleaning old blockchain data..."
rm -rf gxc_data/blockchain_testnet.db
rm -rf gxc_testnet_data/blocks gxc_testnet_data/chainstate

echo ""
echo "========================================="
echo "Step 1: Start Node"
echo "========================================="
./gxc-node --testnet > /tmp/gxc-test.log 2>&1 &
NODE_PID=$!
echo "Node started with PID: $NODE_PID"
sleep 5

# Check if node is running
if ! ps -p $NODE_PID > /dev/null; then
    echo -e "${RED}✗ Node failed to start${NC}"
    cat /tmp/gxc-test.log | tail -20
    exit 1
fi
echo -e "${GREEN}✓ Node is running${NC}"

# Check genesis block
echo ""
echo "Checking genesis block..."
grep "GENESIS BLOCK CREATED" /tmp/gxc-test.log && echo -e "${GREEN}✓ Genesis block created${NC}" || echo -e "${RED}✗ Genesis block not found${NC}"
grep "Genesis UTXO created" /tmp/gxc-test.log && echo -e "${GREEN}✓ Genesis UTXO created${NC}" || echo -e "${RED}✗ Genesis UTXO not created${NC}"

echo ""
echo "========================================="
echo "Step 2: Check Balance"
echo "========================================="
sleep 2

BALANCE=$(curl -s -X POST $RPC_URL -H "Content-Type: application/json" \
  -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$MINER_ADDRESS\"],\"id\":1}" \
  | python3 -c "import sys, json; print(json.load(sys.stdin).get('result', 0))" 2>/dev/null || echo "0")

echo "Balance for $MINER_ADDRESS: $BALANCE GXC"
if [ "$BALANCE" != "0" ] && [ "$BALANCE" != "0.0" ]; then
    echo -e "${GREEN}✓ Genesis reward received${NC}"
else
    echo -e "${YELLOW}⚠ Balance is 0 - checking UTXO set...${NC}"
    grep "total UTXOs in set" /tmp/gxc-test.log | tail -1
fi

echo ""
echo "========================================="
echo "Step 3: Test Mining"
echo "========================================="

# Create a simple mining test
python3 << 'PYTHON_EOF'
import requests
import hashlib
import time
import json

RPC_URL = "http://localhost:8332"
MINER_ADDRESS = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"

def rpc_call(method, params=None):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    try:
        response = requests.post(RPC_URL, json=payload, timeout=5)
        return response.json().get('result')
    except Exception as e:
        print(f"RPC Error: {e}")
        return None

# Get block template
print("Getting block template...")
template = rpc_call("getblocktemplate", [MINER_ADDRESS])
if not template:
    print("✗ Failed to get block template")
    exit(1)

print(f"✓ Block template received for height {template.get('height', 0)}")
print(f"  Difficulty: {template.get('difficulty', 0)}")
print(f"  Block reward: {template.get('block_reward', 0)} GXC")

# Mine a block (simple version)
print("\nMining block...")
height = template.get('height', 1)
prev_hash = template.get('previousblockhash', '0' * 64)
difficulty = template.get('difficulty', 1.0)
timestamp = int(time.time())

# Simple mining loop
nonce = 0
target = '0' * int(difficulty)
found = False

for nonce in range(100000):
    block_data = f"{height}{prev_hash}{MINER_ADDRESS}{timestamp}{nonce}"
    hash_result = hashlib.sha256(block_data.encode()).hexdigest()
    
    if hash_result.startswith(target):
        print(f"✓ Block mined! Nonce: {nonce}, Hash: {hash_result[:16]}...")
        found = True
        
        # Submit block
        block = {
            "height": height,
            "hash": hash_result,
            "previousblockhash": prev_hash,
            "nonce": nonce,
            "timestamp": timestamp,
            "difficulty": difficulty,
            "miner": MINER_ADDRESS,
            "transactions": template.get('transactions', [])
        }
        
        result = rpc_call("submitblock", [block])
        if result is None:
            print("✓ Block accepted by network")
        else:
            print(f"✗ Block rejected: {result}")
        break
    
    if nonce % 10000 == 0:
        print(f"  Mining... {nonce} attempts")

if not found:
    print("✗ Failed to mine block in 100000 attempts")

PYTHON_EOF

echo ""
echo "========================================="
echo "Step 4: Check Updated Balance"
echo "========================================="
sleep 2

NEW_BALANCE=$(curl -s -X POST $RPC_URL -H "Content-Type: application/json" \
  -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$MINER_ADDRESS\"],\"id\":1}" \
  | python3 -c "import sys, json; print(json.load(sys.stdin).get('result', 0))" 2>/dev/null || echo "0")

echo "New balance: $NEW_BALANCE GXC"
if [ "$NEW_BALANCE" != "$BALANCE" ]; then
    echo -e "${GREEN}✓ Balance updated after mining${NC}"
else
    echo -e "${YELLOW}⚠ Balance unchanged${NC}"
fi

echo ""
echo "========================================="
echo "Step 5: Test Transaction Sending"
echo "========================================="

# Create a test recipient address
RECIPIENT="tGXCtest1234567890abcdefghijklmnopqr"

python3 << PYTHON_EOF
import requests
import json

RPC_URL = "http://localhost:8332"
SENDER = "$MINER_ADDRESS"
RECIPIENT = "$RECIPIENT"

def rpc_call(method, params=None):
    payload = {"jsonrpc": "2.0", "method": method, "params": params or [], "id": 1}
    try:
        response = requests.post(RPC_URL, json=payload, timeout=5)
        return response.json().get('result')
    except Exception as e:
        print(f"RPC Error: {e}")
        return None

print(f"Sending 1 GXC from {SENDER[:20]}... to {RECIPIENT[:20]}...")

# Create transaction
tx_data = {
    "from": SENDER,
    "to": RECIPIENT,
    "amount": 1.0,
    "fee": 0.001
}

result = rpc_call("sendtoaddress", [RECIPIENT, 1.0])
if result:
    print(f"✓ Transaction created: {result[:16] if isinstance(result, str) else result}...")
else:
    print("✗ Transaction failed")

PYTHON_EOF

echo ""
echo "========================================="
echo "Step 6: Test Staking"
echo "========================================="

python3 << PYTHON_EOF
import requests

RPC_URL = "http://localhost:8332"
STAKER = "$MINER_ADDRESS"

def rpc_call(method, params=None):
    payload = {"jsonrpc": "2.0", "method": method, "params": params or [], "id": 1}
    try:
        response = requests.post(RPC_URL, json=payload, timeout=5)
        return response.json().get('result')
    except Exception as e:
        print(f"RPC Error: {e}")
        return None

print(f"Staking 10 GXC from {STAKER[:20]}...")

result = rpc_call("stake", [STAKER, 10.0])
if result:
    print(f"✓ Staking successful")
else:
    print("✗ Staking failed (may need more balance)")

PYTHON_EOF

echo ""
echo "========================================="
echo "Test Summary"
echo "========================================="
echo ""
echo "Node PID: $NODE_PID"
echo "Logs: /tmp/gxc-test.log"
echo ""
echo "To view logs: tail -f /tmp/gxc-test.log"
echo "To stop node: kill $NODE_PID"
echo ""
echo -e "${GREEN}Test completed!${NC}"
