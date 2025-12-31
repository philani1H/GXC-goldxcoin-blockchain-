# Railway Proxy Configuration Fix

## Problem
Railway's proxy is not forwarding the request body correctly, causing parse errors:
```
Parse error at line 1, column 1: unexpected end of input
```

## Root Cause
Railway's default proxy configuration may:
1. Strip request bodies
2. Not forward Content-Length header
3. Buffer requests incorrectly
4. Timeout before body is sent

## Solutions

### Solution 1: Add Railway Configuration File

Create `railway.toml` in your project root:

```toml
[build]
builder = "NIXPACKS"

[deploy]
startCommand = "./gxc-node"
restartPolicyType = "ON_FAILURE"
restartPolicyMaxRetries = 10

# CRITICAL: Proxy settings
[proxy]
# Increase body size limit
maxBodySize = "10mb"

# Increase timeouts
readTimeout = 60
writeTimeout = 60
idleTimeout = 120

# Enable body forwarding
forwardBody = true

# Preserve headers
preserveHostHeader = true
```

### Solution 2: Update Your Node to Handle Railway Proxy

In your `src/RPCAPI.cpp` or main server file, add these headers:

```cpp
// Add to your HTTP response headers
response.setHeader("Access-Control-Allow-Origin", "*");
response.setHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
response.setHeader("Content-Type", "application/json");

// For Railway proxy compatibility
response.setHeader("X-Content-Type-Options", "nosniff");
response.setHeader("Connection", "keep-alive");
```

### Solution 3: Use Railway's Internal URL

Instead of the public URL, use Railway's internal service URL:

```powershell
# Get your Railway internal URL
railway variables

# Look for RAILWAY_PRIVATE_DOMAIN or RAILWAY_TCP_PROXY_DOMAIN
# Use format: http://servicename.railway.internal:8332
```

### Solution 4: Test with Correct PowerShell Syntax

```powershell
# Method 1: Using Invoke-WebRequest (more reliable)
$headers = @{
    "Content-Type" = "application/json"
}

$body = @{
    jsonrpc = "2.0"
    method = "getinfo"
    params = @()
    id = 1
} | ConvertTo-Json

$response = Invoke-WebRequest `
    -Uri "https://gxc-chain112-blockchain-node-production.up.railway.app/" `
    -Method Post `
    -Headers $headers `
    -Body $body `
    -UseBasicParsing

$response.Content | ConvertFrom-Json | ConvertTo-Json -Depth 10
```

```powershell
# Method 2: Using curl (if available)
curl -X POST https://gxc-chain112-blockchain-node-production.up.railway.app/ `
  -H "Content-Type: application/json" `
  -d '{\"jsonrpc\":\"2.0\",\"method\":\"getinfo\",\"params\":[],\"id\":1}'
```

```powershell
# Method 3: Explicit encoding
$json = '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":1}'
$bytes = [System.Text.Encoding]::UTF8.GetBytes($json)

Invoke-RestMethod `
    -Uri "https://gxc-chain112-blockchain-node-production.up.railway.app/" `
    -Method Post `
    -Body $bytes `
    -ContentType "application/json; charset=utf-8" `
    -Headers @{"Content-Length" = $bytes.Length} `
    -TimeoutSec 30
```

### Solution 5: Add Health Check Endpoint

Add a simple GET endpoint that doesn't require a body:

```cpp
// In your RPCAPI.cpp
if (method == "GET" && path == "/health") {
    response = "{\"status\":\"ok\",\"service\":\"gxc-node\"}";
    return response;
}

if (method == "GET" && path == "/") {
    response = "{\"jsonrpc\":\"2.0\",\"result\":{\"version\":\"2.0\",\"network\":\"mainnet\"},\"id\":null}";
    return response;
}
```

Test with:
```powershell
Invoke-RestMethod -Uri "https://gxc-chain112-blockchain-node-production.up.railway.app/health"
```

### Solution 6: Check Railway Logs

```bash
# In your Railway dashboard or CLI
railway logs

# Look for:
# - "Request body empty"
# - "Content-Length: 0"
# - "Parse error"
# - Connection timeouts
```

### Solution 7: Update Nixpacks Configuration

Create `nixpacks.toml`:

```toml
[phases.setup]
nixPkgs = ["gcc", "cmake", "openssl", "leveldb"]

[phases.build]
cmds = ["cmake -B build -DCMAKE_BUILD_TYPE=Release", "cmake --build build -j4"]

[phases.start]
cmd = "./build/gxc-node --rpc-bind=0.0.0.0 --rpc-port=8332"

[variables]
# Ensure proper encoding
LANG = "en_US.UTF-8"
LC_ALL = "en_US.UTF-8"
```

### Solution 8: Add Request Logging

Temporarily add logging to see what Railway is sending:

```cpp
// In your request handler
LOG_API(LogLevel::INFO, "Received request:");
LOG_API(LogLevel::INFO, "  Method: " + method);
LOG_API(LogLevel::INFO, "  Path: " + path);
LOG_API(LogLevel::INFO, "  Content-Type: " + contentType);
LOG_API(LogLevel::INFO, "  Content-Length: " + std::to_string(body.length()));
LOG_API(LogLevel::INFO, "  Body: " + body);
```

### Solution 9: Use Railway Environment Variables

Set these in Railway dashboard:

```bash
# Railway Dashboard > Variables
PORT=8332
RPC_BIND=0.0.0.0
RPC_PORT=8332
RAILWAY_STATIC_URL=true
```

### Solution 10: Test Locally First

```powershell
# Test against local node first
$body = @{
    jsonrpc = "2.0"
    method = "getinfo"
    params = @()
    id = 1
} | ConvertTo-Json

# Local test
Invoke-RestMethod -Uri "http://localhost:8332" -Method Post -Body $body -ContentType "application/json"

# If local works but Railway doesn't, it's a proxy issue
```

## Recommended Fix Order

1. **First**: Add `railway.toml` with proxy settings
2. **Second**: Test with the corrected PowerShell syntax (Method 1 above)
3. **Third**: Add health check endpoint
4. **Fourth**: Check Railway logs
5. **Fifth**: Add request logging to see what's being received

## Quick Test Script

Save as `test-railway.ps1`:

```powershell
# Test Railway RPC endpoint
$url = "https://gxc-chain112-blockchain-node-production.up.railway.app/"

Write-Host "Testing Railway RPC endpoint..." -ForegroundColor Cyan

# Test 1: Health check (if implemented)
try {
    Write-Host "`n[Test 1] Health check..." -ForegroundColor Yellow
    $health = Invoke-RestMethod -Uri "$url/health" -Method Get -TimeoutSec 5
    Write-Host "✓ Health check passed" -ForegroundColor Green
    $health | ConvertTo-Json
} catch {
    Write-Host "✗ Health check failed: $($_.Exception.Message)" -ForegroundColor Red
}

# Test 2: RPC with proper encoding
try {
    Write-Host "`n[Test 2] RPC getinfo..." -ForegroundColor Yellow
    
    $body = @{
        jsonrpc = "2.0"
        method = "getinfo"
        params = @()
        id = 1
    } | ConvertTo-Json -Compress
    
    Write-Host "Request body: $body" -ForegroundColor Gray
    
    $response = Invoke-WebRequest `
        -Uri $url `
        -Method Post `
        -Body $body `
        -ContentType "application/json; charset=utf-8" `
        -Headers @{"Accept" = "application/json"} `
        -UseBasicParsing `
        -TimeoutSec 30
    
    Write-Host "✓ RPC call succeeded" -ForegroundColor Green
    Write-Host "Status: $($response.StatusCode)" -ForegroundColor Gray
    $response.Content | ConvertFrom-Json | ConvertTo-Json -Depth 10
    
} catch {
    Write-Host "✗ RPC call failed: $($_.Exception.Message)" -ForegroundColor Red
    if ($_.Exception.Response) {
        $reader = [System.IO.StreamReader]::new($_.Exception.Response.GetResponseStream())
        $responseBody = $reader.ReadToEnd()
        Write-Host "Response: $responseBody" -ForegroundColor Red
    }
}

# Test 3: Check if service is running
try {
    Write-Host "`n[Test 3] Connection test..." -ForegroundColor Yellow
    $tcp = New-Object System.Net.Sockets.TcpClient
    $uri = [System.Uri]$url
    $tcp.Connect($uri.Host, 443)
    Write-Host "✓ TCP connection successful" -ForegroundColor Green
    $tcp.Close()
} catch {
    Write-Host "✗ TCP connection failed: $($_.Exception.Message)" -ForegroundColor Red
}
```

Run with:
```powershell
.\test-railway.ps1
```

## Most Likely Solution

Based on the error, the most likely fix is **Solution 4** - using the correct PowerShell syntax with proper headers:

```powershell
$body = @{
    jsonrpc = "2.0"
    method = "getinfo"
    params = @()
    id = 1
} | ConvertTo-Json

Invoke-RestMethod `
    -Uri "https://gxc-chain112-blockchain-node-production.up.railway.app/" `
    -Method Post `
    -Body $body `
    -ContentType "application/json; charset=utf-8" `
    -TimeoutSec 30
```

The issue with your original command was using `[System.Text.Encoding]::UTF8.GetBytes()` which may not set the Content-Type header correctly for Railway's proxy.
