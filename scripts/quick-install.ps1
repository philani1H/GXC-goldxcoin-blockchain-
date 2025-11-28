# Quick installation script for GXC miners
# Installs Python miner with minimal user interaction

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  GXC Miners Quick Install" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

# Detect Python
$pythonCmd = $null
$pipCmd = $null

try {
    $version = python3 --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        $pythonCmd = "python3"
        $pipCmd = "pip3"
    }
} catch {
    try {
        $version = python --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            $pythonCmd = "python"
            $pipCmd = "pip"
        }
    } catch {
        Write-Host "❌ Python not found!" -ForegroundColor Red
        Write-Host "Please install Python 3.7+ from https://www.python.org/downloads/" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "✅ Found Python: $version" -ForegroundColor Green
Write-Host ""

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$MiningDir = Join-Path (Split-Path -Parent $ScriptDir) "mining"

Write-Host "Installing GXC GXHash Miner..." -ForegroundColor Yellow
Write-Host "===============================" -ForegroundColor Yellow
Write-Host ""

Push-Location $MiningDir

try {
    if (Test-Path "setup.py") {
        Write-Host "Installing from source..." -ForegroundColor Cyan
        & $pipCmd install -e . --user --quiet
        Write-Host "✅ Installation complete!" -ForegroundColor Green
        Write-Host ""
        Write-Host "You can now run: gxc-gxhash-miner" -ForegroundColor Cyan
    } else {
        Write-Host "Installing dependencies..." -ForegroundColor Cyan
        & $pipCmd install -r gxhash_miner_requirements.txt --user --quiet
        Write-Host "✅ Dependencies installed!" -ForegroundColor Green
        Write-Host ""
        Write-Host "You can now run: $pythonCmd gxhash_miner.py" -ForegroundColor Cyan
    }
} catch {
    Write-Host "❌ Installation failed: $_" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Quick Install Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Start blockchain node:" -ForegroundColor White
Write-Host "     .\build\Release\gxc-node.exe --rpc-port=8545" -ForegroundColor Gray
Write-Host ""
Write-Host "  2. Start miner:" -ForegroundColor White
if (Test-Path (Join-Path $MiningDir "setup.py")) {
    Write-Host "     gxc-gxhash-miner" -ForegroundColor Gray
} else {
    Write-Host "     $pythonCmd gxhash_miner.py" -ForegroundColor Gray
}
Write-Host ""
