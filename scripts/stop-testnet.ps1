# GXC Testnet Stop Script for Windows
# Stops all running testnet services

function Write-ColorOutput {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Message,
        [Parameter(Mandatory=$false)]
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

function Print-Info {
    param([string]$Message)
    Write-ColorOutput "[INFO] $Message" -Color Blue
}

function Print-Success {
    param([string]$Message)
    Write-ColorOutput "✅ $Message" -Color Green
}

function Print-Warning {
    param([string]$Message)
    Write-ColorOutput "[WARN] $Message" -Color Yellow
}

function Stop-ServiceByPID {
    param(
        [string]$ServiceName,
        [string]$PidFile
    )
    
    if (Test-Path $PidFile) {
        $pid = Get-Content $PidFile -ErrorAction SilentlyContinue
        
        if ($pid) {
            try {
                $process = Get-Process -Id $pid -ErrorAction SilentlyContinue
                
                if ($process) {
                    Print-Info "Stopping $ServiceName (PID: $pid)..."
                    Stop-Process -Id $pid -Force
                    Start-Sleep -Seconds 2
                    Print-Success "$ServiceName stopped"
                } else {
                    Print-Warning "$ServiceName process (PID: $pid) not running"
                }
            } catch {
                Print-Warning "Could not stop $ServiceName : $_"
            }
            
            Remove-Item $PidFile -Force -ErrorAction SilentlyContinue
        }
    } else {
        Print-Warning "No PID file found for $ServiceName"
    }
}

Write-Host ""
Write-Host "🛑 Stopping GXC Testnet Services..." -ForegroundColor Yellow
Write-Host ""

# Stop all services in reverse order
Stop-ServiceByPID "Market Maker" "pids\market_maker_testnet.pid"
Stop-ServiceByPID "Blockchain Explorer" "pids\explorer_testnet.pid"
Stop-ServiceByPID "Wallet API" "pids\wallet_api_testnet.pid"
Stop-ServiceByPID "Blockchain Node" "pids\blockchain_testnet.pid"

# Kill any remaining Python processes running testnet services
Print-Info "Cleaning up remaining processes..."

$processes = Get-Process python -ErrorAction SilentlyContinue
foreach ($proc in $processes) {
    $cmdline = (Get-WmiObject Win32_Process -Filter "ProcessId = $($proc.Id)").CommandLine
    if ($cmdline -match "wallet_service|blockchain_explorer|stock_market_maker") {
        Print-Info "Stopping remaining process: $($proc.Id)"
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
}

Write-Host ""
Print-Success "All testnet services stopped!"
Write-Host ""
