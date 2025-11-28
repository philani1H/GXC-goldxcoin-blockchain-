# PowerShell deployment script for GXC Mining Pool to Vercel

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "GXC Mining Pool - Vercel Deployment" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Check if Vercel CLI is installed
if (-not (Get-Command vercel -ErrorAction SilentlyContinue)) {
    Write-Host "❌ Vercel CLI not found!" -ForegroundColor Red
    Write-Host "Install it with: npm install -g vercel" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ Vercel CLI found" -ForegroundColor Green
Write-Host ""

# Check if logged in
Write-Host "Checking Vercel login status..."
try {
    vercel whoami | Out-Null
    Write-Host "✅ Already logged in" -ForegroundColor Green
} catch {
    Write-Host "Please login to Vercel:" -ForegroundColor Yellow
    vercel login
}

Write-Host ""
Write-Host "📦 Deploying to Vercel..." -ForegroundColor Cyan
Write-Host ""

# Deploy
vercel --prod

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "✅ Deployment complete!" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Set environment variables in Vercel dashboard:"
Write-Host "   - BLOCKCHAIN_NODE_URL"
Write-Host "   - DATABASE_PATH=/tmp/pool.db"
Write-Host "2. Visit your deployment URL"
Write-Host "3. Test the API: /api/stats"
Write-Host ""

