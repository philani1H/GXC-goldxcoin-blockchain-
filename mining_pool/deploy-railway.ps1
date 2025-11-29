# Deploy GXC Mining Pool Stratum Server to Railway

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Deploying Pool Server to Railway" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Check if Railway CLI is installed
Write-Host "Checking Railway CLI..." -ForegroundColor Yellow
$railwayCheck = railway --version 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Railway CLI found: $railwayCheck" -ForegroundColor Green
} else {
    Write-Host "⚠️  Railway CLI not found. Installing..." -ForegroundColor Yellow
    npm install -g @railway/cli
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Failed to install Railway CLI" -ForegroundColor Red
        Write-Host "Please install manually: npm install -g @railway/cli" -ForegroundColor Yellow
        exit 1
    }
}

# Login to Railway
Write-Host "`n📝 Logging in to Railway..." -ForegroundColor Yellow
railway login

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Railway login failed" -ForegroundColor Red
    exit 1
}

# Navigate to mining pool directory
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $scriptPath

# Initialize Railway project (if not already)
if (-not (Test-Path ".railway\config.json")) {
    Write-Host "`n🔧 Initializing Railway project..." -ForegroundColor Yellow
    railway init
}

# Link to project
Write-Host "`n🔗 Linking to Railway project..." -ForegroundColor Yellow
railway link

# Set environment variables
Write-Host "`n⚙️  Setting environment variables..." -ForegroundColor Yellow
railway variables set RAILWAY_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
railway variables set GXC_NETWORK=testnet
railway variables set POOL_TYPE=general
railway variables set DATABASE_PATH=/tmp/pool.db
railway variables set GENERAL_POOL_STRATUM_PORT=3333
railway variables set GENERAL_POOL_DASHBOARD_PORT=6000
railway variables set GENERAL_POOL_HOST=0.0.0.0

# Deploy
Write-Host "`n🚀 Deploying to Railway..." -ForegroundColor Yellow
railway up

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✅ Deployment complete!" -ForegroundColor Green
    
    Write-Host "`n📋 Next steps:" -ForegroundColor Yellow
    Write-Host "1. Go to Railway Dashboard → Your Service → Settings" -ForegroundColor White
    Write-Host "2. Generate a public domain" -ForegroundColor White
    Write-Host "3. Get your domain: railway domain" -ForegroundColor White
    Write-Host "4. Update environment variables with your public domain:" -ForegroundColor White
    Write-Host "   - GENERAL_POOL_STRATUM_URL = stratum+tcp://your-domain.up.railway.app:3333" -ForegroundColor Gray
    Write-Host "   - GENERAL_POOL_DASHBOARD_URL = https://your-domain.up.railway.app" -ForegroundColor Gray
    
    Write-Host "`n🔍 Check logs: railway logs" -ForegroundColor Yellow
    Write-Host "📊 Check status: railway status" -ForegroundColor Yellow
} else {
    Write-Host "`n⚠️  Deployment may have issues. Check Railway dashboard." -ForegroundColor Yellow
}

Pop-Location

Write-Host ""
