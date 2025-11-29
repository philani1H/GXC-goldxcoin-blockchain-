# Deploy all missing services: Explorer, ASIC Pool, GPU Pool
Write-Host "🚀 Deploying all missing services to Vercel..." -ForegroundColor Green
Write-Host ""

# 1. Deploy Explorer
Write-Host "1️⃣ Deploying Blockchain Explorer..." -ForegroundColor Cyan
& .\deploy-explorer.ps1
Write-Host ""

# 2. Deploy ASIC Pool
Write-Host "2️⃣ Deploying ASIC Mining Pool..." -ForegroundColor Cyan
& .\deploy-asic-pool.ps1
Write-Host ""

# 3. Deploy GPU Pool
Write-Host "3️⃣ Deploying GPU Mining Pool..." -ForegroundColor Cyan
& .\deploy-gpu-pool.ps1
Write-Host ""

Write-Host "✅ All missing services deployment complete!" -ForegroundColor Green

