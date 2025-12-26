#!/bin/bash

NODE_URL="http://localhost:8332"

echo "=========================================="
echo "Testing All RPC Methods"
echo "=========================================="
echo

# Test 1: getinfo
echo "[1] getinfo"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}' | python3 -m json.tool | head -10
echo

# Test 2: getblockcount
echo "[2] getblockcount"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}' | python3 -m json.tool
echo

# Test 3: getdifficulty
echo "[3] getdifficulty"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getdifficulty","params":[],"id":1}' | python3 -m json.tool
echo

# Test 4: getbalance
echo "[4] getbalance"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getbalance","params":["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],"id":1}' | python3 -m json.tool
echo

# Test 5: listunspent
echo "[5] listunspent"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"listunspent","params":["tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"],"id":1}' | python3 -m json.tool | head -20
echo

# Test 6: getmempoolinfo
echo "[6] getmempoolinfo"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getmempoolinfo","params":[],"id":1}' | python3 -m json.tool
echo

# Test 7: getblock
echo "[7] getblock (height 0)"
curl -s -X POST $NODE_URL -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getblock","params":[0],"id":1}' | python3 -m json.tool | head -15
echo

echo "=========================================="
echo "All RPC methods tested"
echo "=========================================="
