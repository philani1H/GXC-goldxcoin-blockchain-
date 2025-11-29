#!/bin/bash
# Quick deployment script for Stratum server to Railway

echo "🚀 Deploying GXC Mining Pool Stratum Server to Railway"
echo "=================================================="
echo ""

# Check if Railway CLI is installed
if ! command -v railway &> /dev/null; then
    echo "❌ Railway CLI not found. Installing..."
    npm i -g @railway/cli
fi

# Login to Railway
echo "📝 Logging in to Railway..."
railway login

# Navigate to mining pool directory
cd "$(dirname "$0")"

# Initialize Railway project (if not already)
if [ ! -f ".railway/config.json" ]; then
    echo "🔧 Initializing Railway project..."
    railway init
fi

# Link to project
echo "🔗 Linking to Railway project..."
railway link

# Set environment variables
echo "⚙️  Setting environment variables..."
railway variables set RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set GXC_NETWORK=testnet
railway variables set POOL_TYPE=general
railway variables set DATABASE_PATH=/tmp/pool.db
railway variables set GENERAL_POOL_STRATUM_PORT=3333
railway variables set GENERAL_POOL_DASHBOARD_PORT=6000
railway variables set GENERAL_POOL_HOST=0.0.0.0

# Deploy
echo "🚀 Deploying to Railway..."
railway up

echo ""
echo "✅ Deployment complete!"
echo ""
echo "📋 Next steps:"
echo "1. Go to Railway Dashboard → Your Service → Settings"
echo "2. Generate a public domain"
echo "3. Update environment variables with your public domain:"
echo "   - GENERAL_POOL_STRATUM_URL = stratum+tcp://your-domain.up.railway.app:3333"
echo "   - GENERAL_POOL_DASHBOARD_URL = https://your-domain.up.railway.app"
echo ""
echo "🔍 Check logs: railway logs"
echo "📊 Check status: railway status"
