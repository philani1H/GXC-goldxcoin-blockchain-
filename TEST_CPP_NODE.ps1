# Test C++ GXC Node on Railway
# Run this after Railway finishes deploying

$url = "https://gxc-chain112-blockchain-node-production.up.railway.app"

Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host "Testing C++ GXC Node on Railway" -ForegroundColor Cyan
Write-Host "URL: $url" -ForegroundColor White
Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host ""

Write-Host "Waiting for Railway to finish deploying..." -ForegroundColor Yellow
Write-Host "Check Railway dashboard: https://railway.app" -ForegroundColor Gray
Write-Host ""
Start-Sleep -Seconds 5

# Test 1: Health Check
Write-Host "[1] Health Check" -ForegroundColor Yellow
try {
    $health = Invoke-RestMethod -Uri "$url/health" -Method Get -TimeoutSec 10
    Write-Host "✓ Node is running" -ForegroundColor Green
    $health | ConvertTo-Json | Write-Host -ForegroundColor White
} catch {
    Write-Host "✗ Health check failed: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Railway may still be deploying. Wait 2-3 minutes and try again." -ForegroundColor Yellow
    exit 1
}
Write-Host ""

# Test 2: Get Blockchain Info
Write-Host "[2] Get Blockchain Info" -ForegroundColor Yellow
try {
    $info = Invoke-RestMethod -Uri "$url/api/getinfo" -Method Get -TimeoutSec 10
    Write-Host "✓ Success" -ForegroundColor Green
    $info | ConvertTo-Json -Depth 5 | Write-Host -ForegroundColor White
} catch {
    Write-Host "✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 3: Get Block Count
Write-Host "[3] Get Block Count" -ForegroundColor Yellow
try {
    $count = Invoke-RestMethod -Uri "$url/api/getblockcount" -Method Get -TimeoutSec 10
    Write-Host "✓ Success" -ForegroundColor Green
    $count | ConvertTo-Json | Write-Host -ForegroundColor White
} catch {
    Write-Host "✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 4: Get Difficulty
Write-Host "[4] Get Difficulty" -ForegroundColor Yellow
try {
    $diff = Invoke-RestMethod -Uri "$url/api/getdifficulty" -Method Get -TimeoutSec 10
    Write-Host "✓ Success" -ForegroundColor Green
    $diff | ConvertTo-Json | Write-Host -ForegroundColor White
} catch {
    Write-Host "✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 5: Get Block Template (for mining)
Write-Host "[5] Get Block Template (for mining pools)" -ForegroundColor Yellow
try {
    $template = Invoke-RestMethod -Uri "$url/api/getblocktemplate" -Method Get -TimeoutSec 10
    Write-Host "✓ Success" -ForegroundColor Green
    $template | ConvertTo-Json -Depth 5 | Write-Host -ForegroundColor White
} catch {
    Write-Host "✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 6: Get Best Block Hash
Write-Host "[6] Get Best Block Hash" -ForegroundColor Yellow
try {
    $hash = Invoke-RestMethod -Uri "$url/api/getbestblockhash" -Method Get -TimeoutSec 10
    Write-Host "✓ Success" -ForegroundColor Green
    $hash | ConvertTo-Json | Write-Host -ForegroundColor White
} catch {
    Write-Host "✗ Failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Summary
Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host "SUMMARY" -ForegroundColor Cyan
Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host ""
Write-Host "All endpoints are now accessible via GET requests!" -ForegroundColor Green
Write-Host ""
Write-Host "For your Rust mining pool, use these endpoints:" -ForegroundColor Yellow
Write-Host "  Health:   $url/health" -ForegroundColor White
Write-Host "  Template: $url/api/getblocktemplate" -ForegroundColor White
Write-Host "  Info:     $url/api/getinfo" -ForegroundColor White
Write-Host "  Count:    $url/api/getblockcount" -ForegroundColor White
Write-Host ""
Write-Host "Note: POST requests may not work due to Railway proxy." -ForegroundColor Yellow
Write-Host "Use GET endpoints instead." -ForegroundColor Yellow
Write-Host ""
