# Redeploy Blockchain Explorer to Vercel
Write-Host "🚀 Redeploying Blockchain Explorer to Vercel..." -ForegroundColor Green

# Navigate to web directory
Set-Location web

# Deploy to Vercel
Write-Host "Deploying..." -ForegroundColor Yellow
vercel --prod --yes

# Return to root
Set-Location ..

Write-Host "✅ Explorer deployment complete!" -ForegroundColor Green

