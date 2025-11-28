#!/bin/bash
#
# GXC Node Startup Script for Railway
# Starts the Python testnet blockchain node
#

set -e  # Exit on error

echo "=========================================="
echo "  GXC Blockchain Node - Railway Startup"
echo "=========================================="
echo ""

# Railway assigns PORT automatically
export PORT=${PORT:-18332}
echo "RPC Port: $PORT"

# Set network
NETWORK=${GXC_NETWORK:-testnet}
echo "Network: $NETWORK"

echo ""
echo "Starting Python Testnet Node..."
echo "=========================================="
echo ""

# Start the Python testnet node (proven to work)
exec python3 testnet_blockchain_node.py
