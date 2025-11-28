#!/bin/bash
#
# GXC Node Startup Script for Railway
# Starts the C++ blockchain node with Railway-compatible configuration
#

set -e  # Exit on error

echo "=========================================="
echo "  GXC Blockchain Node - Railway Startup"
echo "=========================================="
echo ""

# Get Railway-assigned port (or default)
RPC_PORT=${PORT:-8332}
echo "RPC Port: $RPC_PORT"

# Set data directory
DATA_DIR=${GXC_DATA_DIR:-/app/gxc_data}
echo "Data Directory: $DATA_DIR"

# Create data directory if it doesn't exist
mkdir -p "$DATA_DIR"
echo "✓ Data directory ready"

# Set network (testnet or mainnet)
NETWORK=${GXC_NETWORK:-testnet}
echo "Network: $NETWORK"

# Set log level
LOG_LEVEL=${GXC_LOG_LEVEL:-info}
echo "Log Level: $LOG_LEVEL"

echo ""
echo "Starting GXC Node..."
echo "=========================================="
echo ""

# Start the C++ node
exec ./packages/gxc-miners-cli-linux/gxc-node \
  --config=config/gxc-railway.conf \
  --datadir="$DATA_DIR" \
  --rpc-port="$RPC_PORT" \
  --rest-port=8080 \
  $([ "$NETWORK" = "testnet" ] && echo "--testnet") \
  --verbose

# Note: exec replaces the shell process with gxc-node
# This ensures proper signal handling for Railway
