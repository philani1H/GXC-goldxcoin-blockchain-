# Deploy All Services to Railway
# Blockchain Node and Stratum Server

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Deploying to Railway" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Continue"

# Check if Railway CLI is installed
Write-Host "Checking Railway CLI..." -ForegroundColor Yellow
try {
    $railwayVersion = railway --version 2>&1
    Write-Host "✅ Railway CLI found: $railwayVersion" -ForegroundColor Green
} catch {
    Write-Host "⚠️  Railway CLI not found. Installing..." -ForegroundColor Yellow
    npm install -g @railway/cli
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Failed to install Railway CLI" -ForegroundColor Red
        Write-Host "Please install manually: npm install -g @railway/cli" -ForegroundColor Yellow
        exit 1
    }
}

# Step 1: Deploy Blockchain Node
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Step 1: Deploying Blockchain Node" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "📝 Make sure you're logged in to Railway:" -ForegroundColor Yellow
Write-Host "   railway login" -ForegroundColor Gray
Write-Host ""

$confirm = Read-Host "Have you logged in to Railway? (y/n)"
if ($confirm -ne "y" -and $confirm -ne "Y") {
    Write-Host "Please run: railway login" -ForegroundColor Yellow
    railway login
}

# Deploy blockchain node
Write-Host "`nDeploying blockchain node..." -ForegroundColor Yellow
railway up --service blockchain-node

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Blockchain node deployed!" -ForegroundColor Green
} else {
    Write-Host "⚠️  Blockchain node deployment may have issues. Check Railway dashboard." -ForegroundColor Yellow
}

# Step 2: Deploy Stratum Server (Mining Pool)
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Step 2: Deploying Stratum Server" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Push-Location mining_pool

Write-Host "Deploying Stratum server (mining pool)..." -ForegroundColor Yellow
railway up --service stratum-server

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Stratum server deployed!" -ForegroundColor Green
} else {
    Write-Host "⚠️  Stratum server deployment may have issues. Check Railway dashboard." -ForegroundColor Yellow
}

Pop-Location

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Deployment Summary" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "✅ Services deployed to Railway:" -ForegroundColor Green
Write-Host "   1. Blockchain Node (RPC)" -ForegroundColor White
Write-Host "   2. Stratum Server (Mining Pool)" -ForegroundColor White

Write-Host "`n📋 Next Steps:" -ForegroundColor Yellow
Write-Host "   1. Check Railway Dashboard for deployment status" -ForegroundColor White
Write-Host "   2. Get public URLs: railway domain" -ForegroundColor White
Write-Host "   3. Update environment variables if needed" -ForegroundColor White
Write-Host "   4. Test connections to both services" -ForegroundColor White

Write-Host "`n🔍 Check logs:" -ForegroundColor Yellow
Write-Host "   railway logs --service blockchain-node" -ForegroundColor Gray
Write-Host "   railway logs --service stratum-server" -ForegroundColor Gray

Write-Host ""

