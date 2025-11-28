#!/bin/bash
#
# GXC Railway Deployment Script
# Run this script to deploy your C++ blockchain node to Railway
#

set -e  # Exit on error

echo "╔════════════════════════════════════════════════════════╗"
echo "║                                                        ║"
echo "║   🚂 GXC Railway Deployment                            ║"
echo "║                                                        ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Check if Railway CLI is installed
if ! command -v railway &> /dev/null; then
    echo "❌ Railway CLI not found!"
    echo ""
    echo "Installing Railway CLI..."
    npm install -g @railway/cli
    echo "✅ Railway CLI installed!"
    echo ""
fi

echo "Railway CLI version: $(railway --version)"
echo ""

# Step 1: Login
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 1: Login to Railway"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "This will open your browser for authentication..."
echo ""
read -p "Press Enter to continue..."

railway login

echo ""
echo "✅ Logged in successfully!"
echo ""

# Step 2: Initialize project
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 2: Initialize Railway Project"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Project name: gxc-cpp-blockchain-node"
echo ""

railway init

echo ""
echo "✅ Project initialized!"
echo ""

# Step 3: Create volume
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 3: Create Persistent Volume"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Creating volume for blockchain data..."
echo ""

railway volume create gxc_blockchain_data

echo ""
echo "✅ Volume created!"
echo ""

# Step 4: Set environment variables
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 4: Set Environment Variables"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

railway variables set GXC_NETWORK=testnet
echo "✅ Set GXC_NETWORK=testnet"

railway variables set GXC_LOG_LEVEL=info
echo "✅ Set GXC_LOG_LEVEL=info"

railway variables set GXC_DATA_DIR=/app/gxc_data
echo "✅ Set GXC_DATA_DIR=/app/gxc_data"

echo ""
echo "✅ Environment variables configured!"
echo ""

# Step 5: Deploy
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 5: Deploy to Railway"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Deploying C++ blockchain node..."
echo ""

railway up

echo ""
echo "✅ Deployment complete!"
echo ""

# Step 6: Get URL
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 6: Get Your Node URL"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

RAILWAY_URL=$(railway domain 2>&1 | grep -o 'https://[^[:space:]]*' | head -1)

if [ -z "$RAILWAY_URL" ]; then
    echo "Generating domain..."
    railway domain
    RAILWAY_URL=$(railway domain 2>&1 | grep -o 'https://[^[:space:]]*' | head -1)
fi

echo ""
echo "✅ Your node is live at:"
echo ""
echo "   $RAILWAY_URL"
echo ""

# Save URL to file
echo "$RAILWAY_URL" > RAILWAY_NODE_URL.txt
echo "✅ URL saved to RAILWAY_NODE_URL.txt"
echo ""

# Step 7: Check logs
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 7: Check Deployment Logs"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Fetching logs..."
echo ""

railway logs --limit 50

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║                                                        ║"
echo "║   ✅ DEPLOYMENT SUCCESSFUL!                            ║"
echo "║                                                        ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Your C++ blockchain node is now live!"
echo ""
echo "Node URL: $RAILWAY_URL"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Next Steps:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "1. Test your node:"
echo "   python test_railway_node.py $RAILWAY_URL"
echo ""
echo "2. Update your miner:"
echo "   Edit mine_testnet_fixed.py:"
echo "   RPC_URL = \"$RAILWAY_URL\""
echo ""
echo "3. Start mining:"
echo "   python mine_testnet_fixed.py"
echo ""
echo "4. View logs:"
echo "   railway logs --follow"
echo ""
echo "5. Open dashboard:"
echo "   railway open"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🎉 Happy mining!"
echo ""
