# GXC Full Testnet Stop Script
# Stops all testnet services

function Print-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Print-Success {
    param([string]$Message)
    Write-Host "✅ $Message" -ForegroundColor Green
}

function Print-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Stop-ProcessByPid {
    param([string]$PidFile, [string]$ServiceName)
    
    if (Test-Path $PidFile) {
        $pid = Get-Content $PidFile
        try {
            $process = Get-Process -Id $pid -ErrorAction SilentlyContinue
            if ($process) {
                Stop-Process -Id $pid -Force
                Print-Success "Stopped $ServiceName (PID: $pid)"
            } else {
                Print-Info "$ServiceName was not running"
            }
            Remove-Item $PidFile -Force -ErrorAction SilentlyContinue
        } catch {
            Print-Error "Failed to stop $ServiceName: $($_.Exception.Message)"
        }
    }
}

Print-Info "Stopping GXC Full Testnet..."

# Stop all services
Stop-ProcessByPid "pids\blockchain_node_testnet.pid" "Blockchain Node"
Stop-ProcessByPid "pids\pool_general_testnet.pid" "General Pool"
Stop-ProcessByPid "pids\pool_asic_testnet.pid" "ASIC Pool"
Stop-ProcessByPid "pids\pool_gpu_testnet.pid" "GPU Pool"
Stop-ProcessByPid "pids\wallet_api_testnet.pid" "Wallet API"
Stop-ProcessByPid "pids\explorer_testnet.pid" "Blockchain Explorer"

Print-Success "All testnet services stopped!"

