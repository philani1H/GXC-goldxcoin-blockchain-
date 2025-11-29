# Deploy ASIC Mining Pool to Vercel
Write-Host "🚀 Deploying ASIC Mining Pool to Vercel..." -ForegroundColor Green

# Copy ASIC vercel config to main vercel.json
Copy-Item -Path "mining_pool\vercel_asic.json" -Destination "mining_pool\vercel.json" -Force

# Navigate to mining_pool directory
Set-Location mining_pool

# Deploy to Vercel
Write-Host "Deploying..." -ForegroundColor Yellow
vercel --prod --yes

# Return to root
Set-Location ..

Write-Host "✅ ASIC Pool deployment complete!" -ForegroundColor Green

