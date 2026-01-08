#!/bin/bash

NODE_URL="https://8332--019b93ff-3c8e-7a58-9a71-1a6464199b1d.eu-central-1-01.gitpod.dev"

echo "=========================================="
echo "GXC STAKING TEST"
echo "=========================================="
echo ""

# Get node info
echo "1. Node Status:"
curl -s $NODE_URL -X POST -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}' | jq '.result | {height, difficulty, connections}'
echo ""

# Get wallet address
echo "2. Get Node Wallet Address:"
WALLET=$(curl -s $NODE_URL -X POST -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"getnewaddress","params":[],"id":1}' | jq -r '.result')
echo "Wallet: $WALLET"
echo ""

# Check balance
echo "3. Check Balance:"
curl -s $NODE_URL -X POST -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"getbalance\",\"params\":[\"$WALLET\"],\"id\":1}" | jq '.result'
echo ""

# Try to stake (will fail if no balance)
echo "4. Try Staking 100 GXC:"
curl -s $NODE_URL -X POST -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"stake\",\"params\":[\"$WALLET\",100],\"id\":1}" | jq '.'
echo ""

# Check staking info
echo "5. Check Staking Info:"
curl -s $NODE_URL -X POST -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"method\":\"getstakinginfo\",\"params\":[\"$WALLET\"],\"id\":1}" | jq '.'
echo ""

# List validators
echo "6. List Validators:"
curl -s $NODE_URL -X POST -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","method":"listvalidators","params":[],"id":1}' | jq '.'
echo ""

echo "=========================================="
echo "TEST COMPLETE"
echo "=========================================="
