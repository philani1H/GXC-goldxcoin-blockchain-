# GXC Blockchain Deployment Script for Windows
# Quick deployment to local machine or remote server

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  GXC Blockchain Deployment" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

# Check if already built
Write-Host "Checking for existing build..." -ForegroundColor Yellow
if (Test-Path "build\Release\gxc-node.exe") {
    Write-Host "✅ Blockchain node already built" -ForegroundColor Green
    $exePath = "build\Release\gxc-node.exe"
} else {
    Write-Host "⚠️  Blockchain node not built yet" -ForegroundColor Yellow
    Write-Host "   Building blockchain first..." -ForegroundColor Cyan
    
    # Run build script
    if (Test-Path "build-blockchain.ps1") {
        & .\build-blockchain.ps1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "❌ Build failed!" -ForegroundColor Red
            exit 1
        }
        $exePath = "build\Release\gxc-node.exe"
    } else {
        Write-Host "❌ Build script not found!" -ForegroundColor Red
        Write-Host "   Please run: .\build-blockchain.ps1 first" -ForegroundColor Yellow
        exit 1
    }
}

# Create data directory
Write-Host "`nSetting up data directory..." -ForegroundColor Yellow
$dataDir = "gxc_data"
if (-not (Test-Path $dataDir)) {
    New-Item -ItemType Directory -Path $dataDir | Out-Null
    Write-Host "✅ Created data directory" -ForegroundColor Green
} else {
    Write-Host "✅ Data directory exists" -ForegroundColor Green
}

# Create config file
Write-Host "`nCreating configuration..." -ForegroundColor Yellow
$configDir = "$env:USERPROFILE\.gxc"
if (-not (Test-Path $configDir)) {
    New-Item -ItemType Directory -Path $configDir -Force | Out-Null
}

$configFile = "$configDir\gxc.conf"
@"
# GXC Blockchain Configuration
network_port=9333
rpc_port=8545
rest_port=8080
data_dir=$PWD\$dataDir
testnet=false
log_level=INFO
log_file=gxc.log
"@ | Out-File $configFile -Encoding UTF8

Write-Host "✅ Configuration created: $configFile" -ForegroundColor Green

# Check if ports are available
Write-Host "`nChecking ports..." -ForegroundColor Yellow
$ports = @(9333, 8545, 8080)
$portsInUse = @()

foreach ($port in $ports) {
    $connection = Get-NetTCPConnection -LocalPort $port -ErrorAction SilentlyContinue
    if ($connection) {
        $portsInUse += $port
        Write-Host "⚠️  Port $port is in use" -ForegroundColor Yellow
    } else {
        Write-Host "✅ Port $port is available" -ForegroundColor Green
    }
}

if ($portsInUse.Count -gt 0) {
    Write-Host "`n⚠️  Some ports are in use. The node may not start properly." -ForegroundColor Yellow
    Write-Host "   Stop other services using these ports or change config." -ForegroundColor Yellow
}

# Ask if user wants to start the node
Write-Host "`n" -NoNewline
$start = Read-Host "Start blockchain node now? (Y/N)"

if ($start -eq "Y" -or $start -eq "y") {
    Write-Host "`nStarting blockchain node..." -ForegroundColor Cyan
    Write-Host "   RPC: http://localhost:8545" -ForegroundColor Gray
    Write-Host "   REST: http://localhost:8080" -ForegroundColor Gray
    Write-Host "   Network: localhost:9333" -ForegroundColor Gray
    Write-Host "`nPress Ctrl+C to stop`n" -ForegroundColor Yellow
    
    # Start node
    & $exePath --config=$configFile --rpc-port=8545 --rest-port=8080
} else {
    Write-Host "`nTo start the node manually:" -ForegroundColor Cyan
    Write-Host "   .\$exePath --config=$configFile" -ForegroundColor White
    Write-Host "`nOr with specific ports:" -ForegroundColor Cyan
    Write-Host "   .\$exePath --rpc-port=8545 --rest-port=8080" -ForegroundColor White
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Deployment Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

