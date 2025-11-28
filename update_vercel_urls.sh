#!/bin/bash
# Update all localhost URLs to Vercel deployment URLs

echo "🔗 Updating URLs to Vercel deployments..."

# Your Vercel URLs
EXPLORER_URL="https://gxc-blockchain.vercel.app"
FORUM_URL="https://gxc-blockchain-forum.vercel.app"
WALLET_API_URL="https://gxc-blockchain.vercel.app"
POOL_GENERAL_URL="https://gxc-pool.vercel.app"
POOL_GPU_URL="https://gxc-gpu-pool.vercel.app"
POOL_ASIC_URL="https://gxc-asic-pool.vercel.app"
DOCS_URL="https://gxc-docs.vercel.app"
REALTIME_URL="https://forumrealtime.vercel.app"

# Find all HTML files
find . -name "*.html" -type f ! -path "*/node_modules/*" ! -path "*/.git/*" | while read file; do
    echo "Processing: $file"
    
    # Replace localhost URLs with Vercel URLs
    sed -i "s|http://localhost:3000|$EXPLORER_URL|g" "$file"
    sed -i "s|http://localhost:3001|$FORUM_URL|g" "$file"
    sed -i "s|http://localhost:5000|$WALLET_API_URL|g" "$file"
    sed -i "s|http://localhost:6000|$POOL_GENERAL_URL|g" "$file"
    sed -i "s|http://localhost:6001|$POOL_GPU_URL|g" "$file"
    sed -i "s|http://localhost:6002|$POOL_ASIC_URL|g" "$file"
    
    # Also update any references without http://
    sed -i "s|localhost:3000|gxc-blockchain.vercel.app|g" "$file"
    sed -i "s|localhost:3001|gxc-blockchain-forum.vercel.app|g" "$file"
    sed -i "s|localhost:5000|gxc-blockchain.vercel.app|g" "$file"
    sed -i "s|localhost:6000|gxc-pool.vercel.app|g" "$file"
    sed -i "s|localhost:6001|gxc-gpu-pool.vercel.app|g" "$file"
    sed -i "s|localhost:6002|gxc-asic-pool.vercel.app|g" "$file"
done

echo ""
echo "✅ URLs updated to Vercel deployments!"
echo ""
echo "Updated URLs:"
echo "  Explorer:      $EXPLORER_URL"
echo "  Forum:         $FORUM_URL"
echo "  Wallet API:    $WALLET_API_URL"
echo "  General Pool:  $POOL_GENERAL_URL"
echo "  GPU Pool:      $POOL_GPU_URL"
echo "  ASIC Pool:     $POOL_ASIC_URL"
