# GXC Testnet Startup Script for Windows
# Starts all testnet services with proper configuration

# Color output helpers
function Write-ColorOutput {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Message,
        [Parameter(Mandatory=$false)]
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

function Print-Header {
    param([string]$Title)
    Write-Host ""
    Write-Host "================================" -ForegroundColor Cyan
    Write-Host " $Title" -ForegroundColor Cyan
    Write-Host "================================" -ForegroundColor Cyan
}

function Print-Success {
    param([string]$Message)
    Write-ColorOutput "✅ $Message" -Color Green
}

function Print-Info {
    param([string]$Message)
    Write-ColorOutput "[INFO] $Message" -Color Blue
}

function Print-Warning {
    param([string]$Message)
    Write-ColorOutput "[WARN] $Message" -Color Yellow
}

function Print-Error {
    param([string]$Message)
    Write-ColorOutput "[ERROR] $Message" -Color Red
}

# Create necessary directories
function Setup-Directories {
    Print-Info "Creating necessary directories..."
    
    $dirs = @(
        "logs",
        "pids",
        "gxc_testnet_data",
        "testnet_backups"
    )
    
    foreach ($dir in $dirs) {
        if (!(Test-Path $dir)) {
            New-Item -ItemType Directory -Path $dir -Force | Out-Null
            Print-Info "Created directory: $dir"
        }
    }
    
    Print-Success "Directories created!"
}

# Check Python installation
function Check-Python {
    Print-Info "Checking Python installation..."
    
    try {
        $pythonVersion = python --version 2>&1
        Print-Success "Python found: $pythonVersion"
        return $true
    } catch {
        Print-Error "Python not found! Please install Python 3.8+"
        return $false
    }
}

# Install Python dependencies
function Install-Dependencies {
    Print-Header "INSTALLING PYTHON DEPENDENCIES"
    
    if (!(Check-Python)) {
        return $false
    }
    
    Print-Info "Installing required Python packages..."
    
    $packages = @(
        "flask",
        "flask-cors",
        "flask-socketio",
        "bcrypt",
        "pyjwt",
        "cryptography",
        "qrcode",
        "requests",
        "yfinance",
        "numpy",
        "pillow"
    )
    
    foreach ($package in $packages) {
        Print-Info "Installing $package..."
        python -m pip install $package --quiet --disable-pip-version-check
    }
    
    Print-Success "Dependencies installed!"
    return $true
}

# Build the blockchain if needed
function Build-Blockchain {
    Print-Header "BUILDING BLOCKCHAIN"
    
    if (Test-Path "build\Release\gxc_blockchain.exe") {
        Print-Info "Blockchain already built"
        return $true
    }
    
    Print-Info "Building blockchain from source..."
    
    if (!(Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" -Force | Out-Null
    }
    
    Push-Location build
    
    try {
        Print-Info "Running CMake..."
        cmake .. -DCMAKE_BUILD_TYPE=Release
        
        if ($LASTEXITCODE -ne 0) {
            Print-Warning "CMake configuration skipped or failed"
            Print-Info "Checking for pre-built executables..."
        } else {
            Print-Info "Building with CMake..."
            cmake --build . --config Release
        }
        
        Pop-Location
        return $true
    } catch {
        Print-Error "Build failed: $_"
        Pop-Location
        return $false
    }
}

# Start the wallet API
function Start-WalletAPI {
    Print-Header "STARTING WALLET API"
    
    if (!(Test-Path "api\wallet_service.py")) {
        Print-Error "Wallet API service not found!"
        return $false
    }
    
    Print-Info "Starting Wallet API on port 15000..."
    
    $env:FLASK_ENV = "development"
    $env:TESTNET = "true"
    $env:PORT = "15000"
    
    $process = Start-Process -FilePath "python" `
        -ArgumentList "api\wallet_service.py" `
        -PassThru `
        -RedirectStandardOutput "logs\wallet_api_testnet.log" `
        -RedirectStandardError "logs\wallet_api_testnet_error.log" `
        -WorkingDirectory $PWD
    
    $process.Id | Out-File "pids\wallet_api_testnet.pid"
    Print-Success "Wallet API started (PID: $($process.Id))"
    
    return $true
}

# Start the blockchain explorer
function Start-Explorer {
    Print-Header "STARTING BLOCKCHAIN EXPLORER"
    
    if (!(Test-Path "web\blockchain_explorer.py")) {
        Print-Error "Blockchain explorer not found!"
        return $false
    }
    
    Print-Info "Starting Blockchain Explorer on port 13000..."
    
    $env:FLASK_ENV = "development"
    $env:TESTNET = "true"
    $env:PORT = "13000"
    $env:BLOCKCHAIN_NODE_URL = "http://localhost:18332"
    
    $process = Start-Process -FilePath "python" `
        -ArgumentList "web\blockchain_explorer.py" `
        -PassThru `
        -RedirectStandardOutput "logs\explorer_testnet.log" `
        -RedirectStandardError "logs\explorer_testnet_error.log" `
        -WorkingDirectory $PWD
    
    $process.Id | Out-File "pids\explorer_testnet.pid"
    Print-Success "Blockchain Explorer started (PID: $($process.Id))"
    
    return $true
}

# Start the market maker
function Start-MarketMaker {
    Print-Header "STARTING MARKET MAKER"
    
    if (!(Test-Path "market_maker\stock_market_maker.py")) {
        Print-Error "Market maker not found!"
        return $false
    }
    
    Print-Info "Starting Market Maker on port 14000..."
    
    $env:FLASK_ENV = "development"
    $env:TESTNET = "true"
    $env:PORT = "14000"
    
    $process = Start-Process -FilePath "python" `
        -ArgumentList "market_maker\stock_market_maker.py" `
        -PassThru `
        -RedirectStandardOutput "logs\market_maker_testnet.log" `
        -RedirectStandardError "logs\market_maker_testnet_error.log" `
        -WorkingDirectory $PWD
    
    $process.Id | Out-File "pids\market_maker_testnet.pid"
    Print-Success "Market Maker started (PID: $($process.Id))"
    
    return $true
}

# Main execution
function Main {
    Print-Header "GXC BLOCKCHAIN TESTNET STARTUP"
    
    Print-Info "Starting GXC Testnet..."
    Write-Host ""
    
    # Setup
    Setup-Directories
    
    # Install dependencies (skip if --skip-deps argument provided)
    if ($args -notcontains "--skip-deps") {
        if (!(Install-Dependencies)) {
            Print-Error "Failed to install dependencies"
            return
        }
    }
    
    # Build blockchain
    # Build-Blockchain
    
    # Start services
    Start-WalletAPI
    Start-Sleep -Seconds 3
    
    Start-Explorer
    Start-Sleep -Seconds 3
    
    Start-MarketMaker
    Start-Sleep -Seconds 3
    
    # Display information
    Print-Header "GXC TESTNET SERVICES"
    Write-Host ""
    Write-Host "🔗 Blockchain Node RPC:     http://localhost:18332" -ForegroundColor Green
    Write-Host "💼 Wallet API:              http://localhost:15000" -ForegroundColor Green
    Write-Host "🔍 Blockchain Explorer:     http://localhost:13000" -ForegroundColor Green
    Write-Host "📈 Market Maker:            http://localhost:14000" -ForegroundColor Green
    Write-Host ""
    
    Write-Host "📁 Log Files:" -ForegroundColor Cyan
    Write-Host "  • Wallet API:     logs\wallet_api_testnet.log"
    Write-Host "  • Explorer:       logs\explorer_testnet.log"
    Write-Host "  • Market Maker:   logs\market_maker_testnet.log"
    Write-Host ""
    
    Write-Host "🆔 Process IDs:" -ForegroundColor Cyan
    if (Test-Path "pids\wallet_api_testnet.pid") {
        $pid = Get-Content "pids\wallet_api_testnet.pid"
        Write-Host "  • Wallet API:     $pid"
    }
    if (Test-Path "pids\explorer_testnet.pid") {
        $pid = Get-Content "pids\explorer_testnet.pid"
        Write-Host "  • Explorer:       $pid"
    }
    if (Test-Path "pids\market_maker_testnet.pid") {
        $pid = Get-Content "pids\market_maker_testnet.pid"
        Write-Host "  • Market Maker:   $pid"
    }
    Write-Host ""
    
    Write-Host "💡 Quick Start:" -ForegroundColor Yellow
    Write-Host "  1. Visit explorer:  http://localhost:13000"
    Write-Host "  2. Create wallet:   http://localhost:15000"
    Write-Host "  3. View market:     http://localhost:14000"
    Write-Host ""
    
    Write-Host "🛑 To stop testnet: .\scripts\stop-testnet.ps1" -ForegroundColor Blue
    Write-Host ""
    
    Print-Header "🎉 GXC TESTNET STARTED! 🎉"
}

# Run main function
Main @args
