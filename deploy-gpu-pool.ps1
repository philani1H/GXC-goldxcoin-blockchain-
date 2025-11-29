# Deploy GPU Mining Pool to Vercel
Write-Host "🚀 Deploying GPU Mining Pool to Vercel..." -ForegroundColor Green

# Copy GPU vercel config to main vercel.json
Copy-Item -Path "mining_pool\vercel_gpu.json" -Destination "mining_pool\vercel.json" -Force

# Navigate to mining_pool directory
Set-Location mining_pool

# Deploy to Vercel
Write-Host "Deploying..." -ForegroundColor Yellow
vercel --prod --yes

# Return to root
Set-Location ..

Write-Host "✅ GPU Pool deployment complete!" -ForegroundColor Green

