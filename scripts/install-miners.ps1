# GXC Miners Installation Script for Windows
# Installs all miners including Python GXHash miner

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  GXC Miners Installation Script" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

# Detect Python
$pythonCmd = $null
$pipCmd = $null

# Try python3 first
try {
    $pythonVersion = python3 --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        $pythonCmd = "python3"
        $pipCmd = "pip3"
    }
} catch {
    # Try python
    try {
        $pythonVersion = python --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            $pythonCmd = "python"
            $pipCmd = "pip"
        }
    } catch {
        Write-Host "❌ Python not found! Please install Python 3.7 or higher." -ForegroundColor Red
        Write-Host "   Download from: https://www.python.org/downloads/" -ForegroundColor Yellow
        exit 1
    }
}

if ($pythonCmd) {
    Write-Host "✅ Found Python: $pythonVersion" -ForegroundColor Green
} else {
    Write-Host "❌ Python not found! Please install Python 3.7 or higher." -ForegroundColor Red
    exit 1
}

# Check Python version
$versionMatch = $pythonVersion -match "Python (\d+)\.(\d+)"
if ($versionMatch) {
    $major = [int]$matches[1]
    $minor = [int]$matches[2]
    
    if ($major -lt 3 -or ($major -eq 3 -and $minor -lt 7)) {
        Write-Host "❌ Python 3.7 or higher required. Found: $pythonVersion" -ForegroundColor Red
        exit 1
    }
}

# Install Python GXHash Miner
Write-Host "`nInstalling Python GXHash Miner..." -ForegroundColor Yellow
Write-Host "==================================" -ForegroundColor Yellow

$miningDir = Join-Path $PSScriptRoot "..\mining"
Push-Location $miningDir

try {
    # Install using pip
    if (Test-Path "setup.py") {
        Write-Host "Installing from source..." -ForegroundColor Cyan
        & $pipCmd install -e . --user
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Python GXHash Miner installed successfully!" -ForegroundColor Green
            Write-Host ""
            Write-Host "You can now run: gxc-gxhash-miner" -ForegroundColor Cyan
        } else {
            Write-Host "⚠️  Installation had issues. Trying alternative method..." -ForegroundColor Yellow
            & $pipCmd install -r gxhash_miner_requirements.txt --user
            Write-Host "✅ Dependencies installed!" -ForegroundColor Green
            Write-Host "You can now run: $pythonCmd gxhash_miner.py" -ForegroundColor Cyan
        }
    } else {
        Write-Host "Installing dependencies only..." -ForegroundColor Cyan
        if (Test-Path "gxhash_miner_requirements.txt") {
            & $pipCmd install -r gxhash_miner_requirements.txt --user
            Write-Host "✅ Dependencies installed!" -ForegroundColor Green
            Write-Host "You can now run: $pythonCmd gxhash_miner.py" -ForegroundColor Cyan
        }
    }
} catch {
    Write-Host "❌ Installation failed: $_" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}

# Check for C++ miners
Write-Host "`nChecking for C++ miners..." -ForegroundColor Yellow
Write-Host "==========================" -ForegroundColor Yellow

$buildDir = Join-Path $PSScriptRoot "..\build"
$minerExe = Join-Path $buildDir "Release\gxc-gxhash-miner.exe"

if (Test-Path $minerExe) {
    Write-Host "✅ C++ miners found in build directory" -ForegroundColor Green
    Write-Host ""
    Write-Host "C++ miners are ready to use from: $buildDir\Release\" -ForegroundColor Cyan
} else {
    Write-Host "⚠️  C++ miners not found. Build them first:" -ForegroundColor Yellow
    Write-Host "  .\build-miners.ps1" -ForegroundColor White
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Installation Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan
Write-Host "Quick Start:" -ForegroundColor Yellow
Write-Host "  1. Start GXC blockchain node:" -ForegroundColor White
Write-Host "     .\build\Release\gxc-node.exe --rpc-port=8545 --rest-port=8080" -ForegroundColor Gray
Write-Host ""
Write-Host "  2. Start Python miner:" -ForegroundColor White
Write-Host "     gxc-gxhash-miner" -ForegroundColor Gray
Write-Host "     (or: $pythonCmd gxhash_miner.py)" -ForegroundColor Gray
Write-Host ""
Write-Host "  3. Or start C++ miner:" -ForegroundColor White
Write-Host "     .\build\Release\gxc-gxhash-miner.exe --rpc-url=http://localhost:8545" -ForegroundColor Gray
Write-Host ""
