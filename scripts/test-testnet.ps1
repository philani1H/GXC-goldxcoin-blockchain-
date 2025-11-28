# GXC Testnet Automated Testing Script
# Runs comprehensive tests on all testnet services

param(
    [switch]$Verbose = $false,
    [switch]$StopOnError = $false
)

$ErrorActionPreference = if ($StopOnError) { "Stop" } else { "Continue" }
$testResults = @()

function Write-TestResult {
    param(
        [string]$TestName,
        [bool]$Passed,
        [string]$Message = ""
    )
    
    $result = @{
        Test = $TestName
        Passed = $Passed
        Message = $Message
        Timestamp = Get-Date
    }
    
    $script:testResults += $result
    
    if ($Passed) {
        Write-Host "✅ PASS: $TestName" -ForegroundColor Green
    } else {
        Write-Host "❌ FAIL: $TestName" -ForegroundColor Red
        if ($Message) {
            Write-Host "   Details: $Message" -ForegroundColor Yellow
        }
    }
}

function Test-ServiceAvailable {
    param(
        [string]$Name,
        [string]$Url,
        [int]$TimeoutSeconds = 5
    )
    
    try {
        $response = Invoke-WebRequest -Uri $Url -TimeoutSec $TimeoutSeconds -UseBasicParsing -ErrorAction Stop
        Write-TestResult -TestName "$Name Service Available" -Passed $true
        return $true
    } catch {
        Write-TestResult -TestName "$Name Service Available" -Passed $false -Message $_.Exception.Message
        return $false
    }
}

function Test-APIEndpoint {
    param(
        [string]$Name,
        [string]$Url,
        [string]$Method = "GET",
        [hashtable]$Headers = @{},
        [string]$Body = $null
    )
    
    try {
        $params = @{
            Uri = $Url
            Method = $Method
            Headers = $Headers
            TimeoutSec = 10
            UseBasicParsing = $true
        }
        
        if ($Body) {
            $params.Body = $Body
            $params.ContentType = "application/json"
        }
        
        $response = Invoke-WebRequest @params -ErrorAction Stop
        $passed = $response.StatusCode -ge 200 -and $response.StatusCode -lt 300
        
        Write-TestResult -TestName $Name -Passed $passed
        return $response
    } catch {
        Write-TestResult -TestName $Name -Passed $false -Message $_.Exception.Message
        return $null
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " GXC TESTNET AUTOMATED TESTS" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Test 1: Check if services are running
Write-Host "`n--- Testing Service Availability ---`n" -ForegroundColor Yellow

Test-ServiceAvailable -Name "Blockchain Explorer" -Url "http://localhost:13000"
Test-ServiceAvailable -Name "Wallet API" -Url "http://localhost:15000"
Test-ServiceAvailable -Name "Market Maker" -Url "http://localhost:14000"

# Test 2: Wallet API Tests
Write-Host "`n--- Testing Wallet API ---`n" -ForegroundColor Yellow

# Test registration
$timestamp = Get-Date -Format "yyyyMMddHHmmss"
$testUser = "testuser_$timestamp"
$testEmail = "test_$timestamp@example.com"
$testPass = "TestPass123!"

$registerBody = @{
    username = $testUser
    email = $testEmail
    password = $testPass
} | ConvertTo-Json

$registerResponse = Test-APIEndpoint `
    -Name "User Registration" `
    -Url "http://localhost:15000/api/v1/register" `
    -Method "POST" `
    -Body $registerBody

# Test login
if ($registerResponse) {
    Start-Sleep -Seconds 1
    
    $loginBody = @{
        username = $testUser
        password = $testPass
    } | ConvertTo-Json
    
    $loginResponse = Test-APIEndpoint `
        -Name "User Login" `
        -Url "http://localhost:15000/api/v1/login" `
        -Method "POST" `
        -Body $loginBody
    
    # Extract token
    if ($loginResponse) {
        try {
            $loginData = $loginResponse.Content | ConvertFrom-Json
            $token = $loginData.token
            
            if ($token) {
                Write-TestResult -TestName "Token Extraction" -Passed $true
                
                # Test wallet creation
                $walletBody = @{
                    wallet_name = "Test Wallet"
                    password = "WalletPass123!"
                } | ConvertTo-Json
                
                $headers = @{
                    "Authorization" = "Bearer $token"
                }
                
                $walletResponse = Test-APIEndpoint `
                    -Name "Wallet Creation" `
                    -Url "http://localhost:15000/api/v1/wallets/create" `
                    -Method "POST" `
                    -Headers $headers `
                    -Body $walletBody
                
                # Test wallet list
                Test-APIEndpoint `
                    -Name "List Wallets" `
                    -Url "http://localhost:15000/api/v1/wallets" `
                    -Method "GET" `
                    -Headers $headers
                    
            } else {
                Write-TestResult -TestName "Token Extraction" -Passed $false -Message "No token in response"
            }
        } catch {
            Write-TestResult -TestName "Token Extraction" -Passed $false -Message $_.Exception.Message
        }
    }
}

# Test 3: Explorer Tests
Write-Host "`n--- Testing Blockchain Explorer ---`n" -ForegroundColor Yellow

Test-APIEndpoint -Name "Explorer Index Page" -Url "http://localhost:13000"
Test-APIEndpoint -Name "Explorer Blocks Page" -Url "http://localhost:13000/blocks"
Test-APIEndpoint -Name "Explorer Transactions Page" -Url "http://localhost:13000/transactions"

# Test API endpoints
Test-APIEndpoint -Name "Get Network Stats" -Url "http://localhost:13000/api/stats"
Test-APIEndpoint -Name "Get Recent Blocks" -Url "http://localhost:13000/api/blocks/recent"
Test-APIEndpoint -Name "Get Recent Transactions" -Url "http://localhost:13000/api/transactions/recent"

# Test 4: Market Maker Tests
Write-Host "`n--- Testing Market Maker ---`n" -ForegroundColor Yellow

Test-APIEndpoint -Name "Market Maker Index" -Url "http://localhost:14000"
Test-APIEndpoint -Name "Get Gold Price" -Url "http://localhost:14000/api/gold/price"
Test-APIEndpoint -Name "Get Market Stats" -Url "http://localhost:14000/api/stats"

# Test 5: RPC Tests (if node is running)
Write-Host "`n--- Testing RPC Node (if available) ---`n" -ForegroundColor Yellow

$rpcBody = @{
    method = "getblockchaininfo"
    params = @()
    id = 1
} | ConvertTo-Json

Test-APIEndpoint `
    -Name "RPC GetBlockchainInfo" `
    -Url "http://localhost:18332" `
    -Method "POST" `
    -Body $rpcBody

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$totalTests = $testResults.Count
$passedTests = ($testResults | Where-Object { $_.Passed }).Count
$failedTests = $totalTests - $passedTests
$passRate = [math]::Round(($passedTests / $totalTests) * 100, 2)

Write-Host "Total Tests:  $totalTests" -ForegroundColor White
Write-Host "Passed:       $passedTests" -ForegroundColor Green
Write-Host "Failed:       $failedTests" -ForegroundColor Red
Write-Host "Pass Rate:    $passRate%" -ForegroundColor $(if ($passRate -ge 80) { "Green" } elseif ($passRate -ge 50) { "Yellow" } else { "Red" })
Write-Host ""

# Detailed failed tests
if ($failedTests -gt 0) {
    Write-Host "Failed Tests:" -ForegroundColor Red
    $testResults | Where-Object { -not $_.Passed } | ForEach-Object {
        Write-Host "  ❌ $($_.Test)" -ForegroundColor Red
        if ($_.Message) {
            Write-Host "     $($_.Message)" -ForegroundColor Yellow
        }
    }
    Write-Host ""
}

# Export results to JSON
$resultsFile = "logs\testnet_test_results_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
$testResults | ConvertTo-Json -Depth 10 | Out-File $resultsFile
Write-Host "Test results saved to: $resultsFile" -ForegroundColor Cyan

# Exit code
if ($failedTests -eq 0) {
    Write-Host "`n✅ All tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n⚠️  Some tests failed. Review the results above." -ForegroundColor Yellow
    exit 1
}
