#!/bin/bash
# Deployment script for GXC Mining Pool to Vercel

echo "=========================================="
echo "GXC Mining Pool - Vercel Deployment"
echo "=========================================="
echo ""

# Check if Vercel CLI is installed
if ! command -v vercel &> /dev/null; then
    echo "❌ Vercel CLI not found!"
    echo "Install it with: npm install -g vercel"
    exit 1
fi

echo "✅ Vercel CLI found"
echo ""

# Check if logged in
echo "Checking Vercel login status..."
if ! vercel whoami &> /dev/null; then
    echo "Please login to Vercel:"
    vercel login
fi

echo ""
echo "📦 Deploying to Vercel..."
echo ""

# Deploy
vercel --prod

echo ""
echo "=========================================="
echo "✅ Deployment complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Set environment variables in Vercel dashboard:"
echo "   - BLOCKCHAIN_NODE_URL"
echo "   - DATABASE_PATH=/tmp/pool.db"
echo "2. Visit your deployment URL"
echo "3. Test the API: /api/stats"
echo ""

