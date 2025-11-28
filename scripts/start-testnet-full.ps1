# GXC Full Testnet Startup Script
# Starts blockchain node + all services + mining pools for complete testnet

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
        python -m pip install $package --quiet --disable-pip-version-check 2>&1 | Out-Null
    }
    
    Print-Success "Dependencies installed!"
    return $true
}

# Start the blockchain node
function Start-BlockchainNode {
    Print-Header "STARTING BLOCKCHAIN NODE"
    
    # First try C++ executable
    $nodeExe = "build\Release\gxc_blockchain.exe"
    if (Test-Path $nodeExe) {
        Print-Info "Found C++ blockchain node executable"
        Print-Info "Starting blockchain node in testnet mode..."
        Print-Info "RPC will be available at: http://localhost:18332"
        
        $configFile = "config\gxc-testnet.conf"
        if (!(Test-Path $configFile)) {
            Print-Warning "Testnet config not found, using defaults"
        }
        
        $process = Start-Process -FilePath $nodeExe `
            -ArgumentList "--testnet", "--data-dir=./gxc_testnet_data", "--rpc-port=18332", "--network-port=19333" `
            -PassThru `
            -RedirectStandardOutput "logs\blockchain_node_testnet.log" `
            -RedirectStandardError "logs\blockchain_node_testnet_error.log" `
            -WorkingDirectory $PWD
        
        Start-Sleep -Seconds 5
        
        # Check if process is still running
        if ($process.HasExited) {
            Print-Warning "C++ node failed to start, trying Python testnet node..."
        } else {
            $process.Id | Out-File "pids\blockchain_node_testnet.pid"
            Print-Success "C++ Blockchain node started (PID: $($process.Id))"
            Print-Info "Waiting for node to initialize..."
            Start-Sleep -Seconds 10
            return $true
        }
    }
    
    # Fallback to Python testnet node
    Print-Info "Using Python testnet blockchain node..."
    Print-Info "RPC will be available at: http://localhost:18332"
    
    if (!(Test-Path "testnet_blockchain_node.py")) {
        Print-Error "Python testnet node not found at testnet_blockchain_node.py"
        return $false
    }
    
    $process = Start-Process -FilePath "python" `
        -ArgumentList "testnet_blockchain_node.py" `
        -PassThru `
        -RedirectStandardOutput "logs\blockchain_node_testnet.log" `
        -RedirectStandardError "logs\blockchain_node_testnet_error.log" `
        -WorkingDirectory $PWD
    
    Start-Sleep -Seconds 3
    
    # Check if process is still running
    if ($process.HasExited) {
        Print-Error "Python testnet node failed to start. Check logs\blockchain_node_testnet_error.log"
        return $false
    }
    
    $process.Id | Out-File "pids\blockchain_node_testnet.pid"
    Print-Success "Python testnet blockchain node started (PID: $($process.Id))"
    Print-Info "Waiting for node to initialize..."
    Start-Sleep -Seconds 5
    
    # Test RPC connection
    try {
        $response = Invoke-WebRequest -Uri "http://localhost:18332" -Method POST `
            -Body '{"method":"getblockchaininfo","params":[],"id":1}' `
            -ContentType "application/json" -TimeoutSec 2 -ErrorAction SilentlyContinue
        if ($response.StatusCode -eq 200) {
            Print-Success "RPC connection verified!"
        }
    } catch {
        Print-Warning "RPC not responding yet, but node is starting..."
    }
    
    return $true
}

# Start mining pools
function Start-MiningPools {
    Print-Header "STARTING MINING POOLS"
    
    $rpcUrl = "http://localhost:18332"
    
    # General Pool
    Print-Info "Starting General Pool on port 5000..."
    $env:BLOCKCHAIN_NODE_URL = $rpcUrl
    $env:PORT = "5000"
    $process1 = Start-Process -FilePath "python" `
        -ArgumentList "mining_pool\general_pool.py" `
        -PassThru `
        -RedirectStandardOutput "logs\pool_general_testnet.log" `
        -RedirectStandardError "logs\pool_general_testnet_error.log" `
        -WorkingDirectory $PWD
    $process1.Id | Out-File "pids\pool_general_testnet.pid"
    Print-Success "General Pool started (PID: $($process1.Id))"
    Start-Sleep -Seconds 2
    
    # GPU Pool
    Print-Info "Starting GPU Pool on port 5002..."
    $env:BLOCKCHAIN_NODE_URL = $rpcUrl
    $env:PORT = "5002"
    $process2 = Start-Process -FilePath "python" `
        -ArgumentList "mining_pool\gpu_pool.py" `
        -PassThru `
        -RedirectStandardOutput "logs\pool_gpu_testnet.log" `
        -RedirectStandardError "logs\pool_gpu_testnet_error.log" `
        -WorkingDirectory $PWD
    $process2.Id | Out-File "pids\pool_gpu_testnet.pid"
    Print-Success "GPU Pool started (PID: $($process2.Id))"
    Start-Sleep -Seconds 2
    
    # ASIC Pool
    Print-Info "Starting ASIC Pool on port 5001..."
    $env:BLOCKCHAIN_NODE_URL = $rpcUrl
    $env:PORT = "5001"
    $process3 = Start-Process -FilePath "python" `
        -ArgumentList "mining_pool\asic_pool.py" `
        -PassThru `
        -RedirectStandardOutput "logs\pool_asic_testnet.log" `
        -RedirectStandardError "logs\pool_asic_testnet_error.log" `
        -WorkingDirectory $PWD
    $process3.Id | Out-File "pids\pool_asic_testnet.pid"
    Print-Success "ASIC Pool started (PID: $($process3.Id))"
    
    return $true
}

# Start the wallet API
function Start-WalletAPI {
    Print-Header "STARTING WALLET API"
    
    if (!(Test-Path "api\wallet_service.py")) {
        Print-Warning "Wallet API service not found, skipping..."
        return $false
    }
    
    Print-Info "Starting Wallet API on port 15000..."
    
    $env:FLASK_ENV = "development"
    $env:TESTNET = "true"
    $env:PORT = "15000"
    $env:BLOCKCHAIN_NODE_URL = "http://localhost:18332"
    
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
        Print-Warning "Blockchain explorer not found, skipping..."
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

# Main execution
function Main {
    Print-Header "GXC FULL TESTNET STARTUP"
    
    Print-Info "Starting complete GXC testnet with all services..."
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
    
    # Start blockchain node (REQUIRED for full testnet)
    $nodeStarted = Start-BlockchainNode
    if (!$nodeStarted) {
        Print-Error "CRITICAL: Blockchain node failed to start!"
        Print-Error "Pools cannot function without a blockchain node."
        Print-Info "Please check logs\blockchain_node_testnet_error.log for details"
        return
    }
    
    Print-Info "Blockchain node is running. Pools will connect to it."
    Start-Sleep -Seconds 5
    
    # Start mining pools
    Start-MiningPools
    Start-Sleep -Seconds 3
    
    # Start other services
    Start-WalletAPI
    Start-Sleep -Seconds 2
    
    Start-Explorer
    Start-Sleep -Seconds 2
    
    # Display information
    Print-Header "GXC FULL TESTNET SERVICES"
    Write-Host ""
    Write-Host "[RPC] Blockchain Node RPC:     http://localhost:18332" -ForegroundColor Green
    Write-Host "[POOL] General Mining Pool:     http://localhost:5000" -ForegroundColor Green
    Write-Host "[POOL] ASIC Mining Pool:        http://localhost:5001" -ForegroundColor Green
    Write-Host "[POOL] GPU Mining Pool:          http://localhost:5002" -ForegroundColor Green
    Write-Host "[API] Wallet API:              http://localhost:15000" -ForegroundColor Green
    Write-Host "[WEB] Blockchain Explorer:     http://localhost:13000" -ForegroundColor Green
    Write-Host ""
    
    Write-Host "Mining Pool Endpoints:" -ForegroundColor Cyan
    Write-Host "  - General: stratum+tcp://localhost:3333" -ForegroundColor Yellow
    Write-Host "  - ASIC:     stratum+tcp://localhost:3333" -ForegroundColor Yellow
    Write-Host "  - GPU:      stratum+tcp://localhost:4444" -ForegroundColor Yellow
    Write-Host ""
    
    Write-Host "Log Files:" -ForegroundColor Cyan
    Write-Host "  • Blockchain Node: logs\blockchain_node_testnet.log"
    Write-Host "  • General Pool:    logs\pool_general_testnet.log"
    Write-Host "  • ASIC Pool:       logs\pool_asic_testnet.log"
    Write-Host "  • GPU Pool:        logs\pool_gpu_testnet.log"
    Write-Host "  • Wallet API:      logs\wallet_api_testnet.log"
    Write-Host "  • Explorer:        logs\explorer_testnet.log"
    Write-Host ""
    
    Write-Host "Quick Start Mining:" -ForegroundColor Yellow
    Write-Host "  1. Connect miner to: stratum+tcp://localhost:3333" -ForegroundColor White
    Write-Host "  2. Username: your_worker" -ForegroundColor White
    Write-Host "  3. Password: x" -ForegroundColor White
    Write-Host "  4. View pool dashboard: http://localhost:5000" -ForegroundColor White
    Write-Host ""
    
    Write-Host "[STOP] To stop testnet: .\scripts\stop-testnet-full.ps1" -ForegroundColor Blue
    Write-Host ""
    
    Print-Header "GXC FULL TESTNET STARTED!"
    Write-Host ""
    Write-Host "Ready to start mining! Connect your miners now." -ForegroundColor Green
}

# Run main function
Main @args

