# GXC Blockchain Node - Complete Endpoint Test Script
# Tests all available endpoints on Railway deployment

$baseUrl = "https://gxc-chain112-blockchain-node-production.up.railway.app"

Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host "GXC Blockchain Node - Endpoint Test Suite" -ForegroundColor Cyan
Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host ""

$testResults = @()

function Test-Endpoint {
    param(
        [string]$Name,
        [string]$Method,
        [string]$Path,
        [string]$Body = $null
    )
    
    Write-Host "[TEST] $Name" -ForegroundColor Yellow
    Write-Host "  Method: $Method" -ForegroundColor Gray
    Write-Host "  Path: $Path" -ForegroundColor Gray
    
    try {
        $params = @{
            Uri = "$baseUrl$Path"
            Method = $Method
            TimeoutSec = 10
            ErrorAction = 'Stop'
        }
        
        if ($Body) {
            $params['Body'] = $Body
            $params['ContentType'] = 'application/json; charset=utf-8'
            Write-Host "  Body: $Body" -ForegroundColor Gray
        }
        
        $response = Invoke-RestMethod @params
        
        Write-Host "  ✓ SUCCESS" -ForegroundColor Green
        Write-Host "  Response:" -ForegroundColor Gray
        $response | ConvertTo-Json -Depth 5 | Write-Host -ForegroundColor White
        Write-Host ""
        
        $testResults += [PSCustomObject]@{
            Name = $Name
            Status = "PASS"
            Method = $Method
            Path = $Path
        }
        
        return $response
        
    } catch {
        Write-Host "  ✗ FAILED" -ForegroundColor Red
        Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Red
        
        if ($_.Exception.Response) {
            try {
                $reader = [System.IO.StreamReader]::new($_.Exception.Response.GetResponseStream())
                $responseBody = $reader.ReadToEnd()
                Write-Host "  Response: $responseBody" -ForegroundColor Red
            } catch {}
        }
        Write-Host ""
        
        $testResults += [PSCustomObject]@{
            Name = $Name
            Status = "FAIL"
            Method = $Method
            Path = $Path
            Error = $_.Exception.Message
        }
        
        return $null
    }
}

# ============================================================================
# HEALTH CHECK ENDPOINTS
# ============================================================================

Write-Host "HEALTH CHECK ENDPOINTS" -ForegroundColor Magenta
Write-Host "-" * 80 -ForegroundColor Magenta

Test-Endpoint -Name "Root Health Check" -Method "GET" -Path "/"
Test-Endpoint -Name "Health Endpoint" -Method "GET" -Path "/health"
Test-Endpoint -Name "Ping Endpoint" -Method "GET" -Path "/ping"

# ============================================================================
# REST API ENDPOINTS (GET)
# ============================================================================

Write-Host "REST API ENDPOINTS (GET)" -ForegroundColor Magenta
Write-Host "-" * 80 -ForegroundColor Magenta

Test-Endpoint -Name "Get Blockchain Info" -Method "GET" -Path "/api/getinfo"
Test-Endpoint -Name "Get Block Count" -Method "GET" -Path "/api/getblockcount"
Test-Endpoint -Name "Get Difficulty" -Method "GET" -Path "/api/getdifficulty"
Test-Endpoint -Name "Get Best Block Hash" -Method "GET" -Path "/api/getbestblockhash"
Test-Endpoint -Name "Get Latest Block" -Method "GET" -Path "/api/getlatestblock"
Test-Endpoint -Name "Get Block Template" -Method "GET" -Path "/api/getblocktemplate"
Test-Endpoint -Name "Get Block by Height (0)" -Method "GET" -Path "/api/getblock/0"

# Alternative paths without /api prefix
Test-Endpoint -Name "Get Info (no prefix)" -Method "GET" -Path "/getinfo"
Test-Endpoint -Name "Get Block Count (no prefix)" -Method "GET" -Path "/getblockcount"
Test-Endpoint -Name "Get Block Template (no prefix)" -Method "GET" -Path "/getblocktemplate"

# ============================================================================
# JSON-RPC ENDPOINTS (POST) - These may fail due to Railway proxy
# ============================================================================

Write-Host "JSON-RPC ENDPOINTS (POST)" -ForegroundColor Magenta
Write-Host "-" * 80 -ForegroundColor Magenta
Write-Host "Note: These may fail due to Railway proxy stripping POST bodies" -ForegroundColor Yellow
Write-Host ""

# Method 1: Using hashtable (recommended)
$rpcBody = @{
    jsonrpc = "2.0"
    method = "getblockchaininfo"
    params = @()
    id = 1
} | ConvertTo-Json -Compress

Test-Endpoint -Name "RPC: getblockchaininfo" -Method "POST" -Path "/" -Body $rpcBody

$rpcBody = @{
    jsonrpc = "2.0"
    method = "getblockcount"
    params = @()
    id = 2
} | ConvertTo-Json -Compress

Test-Endpoint -Name "RPC: getblockcount" -Method "POST" -Path "/" -Body $rpcBody

$rpcBody = @{
    jsonrpc = "2.0"
    method = "getblocktemplate"
    params = @(@{algorithm = "gxhash"})
    id = 3
} | ConvertTo-Json -Compress

Test-Endpoint -Name "RPC: getblocktemplate" -Method "POST" -Path "/" -Body $rpcBody

$rpcBody = @{
    jsonrpc = "2.0"
    method = "getbestblockhash"
    params = @()
    id = 4
} | ConvertTo-Json -Compress

Test-Endpoint -Name "RPC: getbestblockhash" -Method "POST" -Path "/" -Body $rpcBody

# ============================================================================
# SUMMARY
# ============================================================================

Write-Host ""
Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host "TEST SUMMARY" -ForegroundColor Cyan
Write-Host "=" * 80 -ForegroundColor Cyan
Write-Host ""

$passed = ($testResults | Where-Object { $_.Status -eq "PASS" }).Count
$failed = ($testResults | Where-Object { $_.Status -eq "FAIL" }).Count
$total = $testResults.Count

Write-Host "Total Tests: $total" -ForegroundColor White
Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor Red
Write-Host ""

if ($failed -gt 0) {
    Write-Host "Failed Tests:" -ForegroundColor Red
    $testResults | Where-Object { $_.Status -eq "FAIL" } | ForEach-Object {
        Write-Host "  - $($_.Name) ($($_.Method) $($_.Path))" -ForegroundColor Red
        if ($_.Error) {
            Write-Host "    Error: $($_.Error)" -ForegroundColor Gray
        }
    }
    Write-Host ""
}

Write-Host "RECOMMENDATIONS:" -ForegroundColor Yellow
Write-Host "  1. Use GET endpoints (/api/getinfo, /api/getblockcount, etc.)" -ForegroundColor White
Write-Host "  2. POST endpoints may not work due to Railway proxy issues" -ForegroundColor White
Write-Host "  3. For mining pools, use GET /api/getblocktemplate" -ForegroundColor White
Write-Host "  4. Health check available at /health" -ForegroundColor White
Write-Host ""

Write-Host "WORKING ENDPOINTS FOR YOUR POOL:" -ForegroundColor Green
Write-Host "  Health: $baseUrl/health" -ForegroundColor White
Write-Host "  Get Template: $baseUrl/api/getblocktemplate" -ForegroundColor White
Write-Host "  Get Info: $baseUrl/api/getinfo" -ForegroundColor White
Write-Host "  Get Block Count: $baseUrl/api/getblockcount" -ForegroundColor White
Write-Host ""

# Export results to JSON
$testResults | ConvertTo-Json -Depth 5 | Out-File "test_results.json"
Write-Host "Results saved to test_results.json" -ForegroundColor Cyan
