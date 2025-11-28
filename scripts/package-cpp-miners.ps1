# Package C++ miners for installation
# Creates installable packages for all C++ miners (command-line and GUI)

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Packaging C++ Miners" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$PackagesDir = Join-Path $ProjectRoot "packages"
$BuildDir = Join-Path $ProjectRoot "build"

if (-not (Test-Path $PackagesDir)) {
    New-Item -ItemType Directory -Path $PackagesDir | Out-Null
}

$Platform = "windows"
$ExeExt = ".exe"
$ExeDir = "Release\"

# Function to package command-line miners
function Package-CliMiners {
    $PackageName = "gxc-miners-cli-$Platform"
    $PackageDir = Join-Path $PackagesDir $PackageName
    
    Write-Host "Packaging command-line miners..." -ForegroundColor Yellow
    Write-Host "================================" -ForegroundColor Yellow
    
    if (Test-Path $PackageDir) {
        Remove-Item -Recurse -Force $PackageDir
    }
    New-Item -ItemType Directory -Path $PackageDir | Out-Null
    
    # List of command-line miners
    $Miners = @(
        "gxc-node",
        "gxc-miner",
        "gxc-gxhash-miner",
        "gxc-ethash-miner",
        "gxc-sha256-miner",
        "gxc-pool-proxy"
    )
    
    $FoundCount = 0
    
    foreach ($Miner in $Miners) {
        $ExePath = Join-Path $BuildDir "$ExeDir$Miner$ExeExt"
        
        if (Test-Path $ExePath) {
            Copy-Item $ExePath $PackageDir
            Write-Host "  ✅ Copied: $Miner" -ForegroundColor Green
            $FoundCount++
        } else {
            Write-Host "  ⚠️  Not found: $Miner" -ForegroundColor Yellow
        }
    }
    
    if ($FoundCount -eq 0) {
        Write-Host "❌ No miners found! Build them first:" -ForegroundColor Red
        Write-Host "   .\build-miners.ps1" -ForegroundColor White
        Remove-Item -Recurse -Force $PackageDir
        return $false
    }
    
    # Create README
    $Readme = @"
# GXC Command-Line Miners

## Available Miners

- **gxc-node** - Blockchain node (required for mining)
- **gxc-miner** - Universal miner (supports all algorithms)
- **gxc-gxhash-miner** - GXHash CPU miner
- **gxc-ethash-miner** - Ethash GPU miner (NVIDIA/AMD)
- **gxc-sha256-miner** - SHA-256 ASIC miner
- **gxc-pool-proxy** - Mining pool proxy

## Quick Start

### 1. Start Blockchain Node

````powershell
.\gxc-node.exe --rpc-port=8545 --rest-port=8080
````

### 2. Start Miner

**Universal Miner:**
````powershell
.\gxc-miner.exe --address=YOUR_ADDRESS --algorithm=gxhash
````

**GXHash Miner:**
````powershell
.\gxc-gxhash-miner.exe --rpc-url=http://localhost:8545
````

**SHA256 Miner:**
````powershell
.\gxc-sha256-miner.exe --threads=8
````

**Ethash Miner (GPU):**
````powershell
.\gxc-ethash-miner.exe --rpc-url=http://localhost:8545 --gpu
````

## Installation

### Windows

1. Extract package:
   - Right-click `gxc-miners-cli-windows.zip`
   - Select "Extract All"
   - Choose destination folder

2. Run miner:
   - Double-click `gxc-miner.exe` or
   - Open Command Prompt in folder and run: `gxc-miner.exe --address=YOUR_ADDRESS`

## Requirements

- GXC blockchain node running
- RPC enabled on node (port 8545)
- REST API enabled on node (port 8080)

## Documentation

For detailed usage, see:
- [MINERS_README.md](../../MINERS_README.md)
- [INSTALLATION_GUIDE.md](../../INSTALLATION_GUIDE.md)
"@
    $Readme | Out-File (Join-Path $PackageDir "README.md") -Encoding UTF8
    
    # Create installation script
    $InstallScript = @"
@echo off
echo Installing GXC Command-Line Miners...
echo.

echo ✅ Installation complete!
echo.
echo Miners are ready to use. See README.md for usage instructions.
echo.
pause
"@
    $InstallScript | Out-File (Join-Path $PackageDir "install.bat") -Encoding ASCII
    
    # Create archive
    Push-Location $PackagesDir
    try {
        Compress-Archive -Path $PackageName -DestinationPath "$PackageName.zip" -Force
        Write-Host "✅ Created: $PackageName.zip" -ForegroundColor Green
    } finally {
        Pop-Location
    }
    
    Write-Host ""
    return $true
}

# Function to package GUI miners
function Package-GuiMiners {
    $PackageName = "gxc-miners-gui-$Platform"
    $PackageDir = Join-Path $PackagesDir $PackageName
    
    Write-Host "Packaging GUI miners..." -ForegroundColor Yellow
    Write-Host "=======================" -ForegroundColor Yellow
    
    if (Test-Path $PackageDir) {
        Remove-Item -Recurse -Force $PackageDir
    }
    New-Item -ItemType Directory -Path $PackageDir | Out-Null
    
    # List of GUI applications
    $GuiApps = @(
        "gxc-wallet",
        "gxc-node-gui",
        "gxc-mining-gui"
    )
    
    $FoundCount = 0
    
    foreach ($App in $GuiApps) {
        $ExePath = Join-Path $BuildDir "$ExeDir$App$ExeExt"
        
        if (Test-Path $ExePath) {
            Copy-Item $ExePath $PackageDir
            Write-Host "  ✅ Copied: $App" -ForegroundColor Green
            $FoundCount++
        } else {
            Write-Host "  ⚠️  Not found: $App (may not be built with Qt)" -ForegroundColor Yellow
        }
    }
    
    if ($FoundCount -eq 0) {
        Write-Host "⚠️  No GUI applications found!" -ForegroundColor Yellow
        Write-Host "   Build with Qt: cmake .. -DBUILD_GUI=ON -DBUILD_MINING_CLIENT=ON" -ForegroundColor White
        Remove-Item -Recurse -Force $PackageDir
        return $false
    }
    
    # Create README
    $Readme = @"
# GXC GUI Miners

## Available Applications

- **gxc-wallet** - Main wallet GUI with mining support
- **gxc-node-gui** - Node management GUI
- **gxc-mining-gui** - Dedicated mining GUI application

## Quick Start

### gxc-mining-gui (Mining GUI)

1. Start blockchain node:
   ````powershell
   .\gxc-node.exe --rpc-port=8545
   ````

2. Launch mining GUI:
   ````powershell
   .\gxc-mining-gui.exe
   ````

3. In the GUI:
   - Enter your wallet address
   - Select mining algorithm
   - Configure threads/devices
   - Click "Start Mining"

### gxc-wallet (Wallet with Mining)

1. Launch wallet:
   ````powershell
   .\gxc-wallet.exe
   ````

2. Navigate to Mining tab
3. Configure and start mining

### gxc-node-gui (Node Management)

1. Launch node GUI:
   ````powershell
   .\gxc-node-gui.exe
   ````

2. Monitor blockchain status
3. View network statistics

## Installation

### Windows

1. Extract package:
   - Right-click `gxc-miners-gui-windows.zip`
   - Select "Extract All"

2. Run application:
   - Double-click `gxc-mining-gui.exe`

**Note:** You may need:
- Visual C++ Redistributables
- Qt DLLs (if not statically linked)

## Requirements

- Qt 5 or Qt 6 (for GUI applications)
- GXC blockchain node running (for mining)
- RPC enabled on node (port 8545)

## Troubleshooting

### Application Won't Start

- Ensure Visual C++ Redistributables are installed
- Check if Qt DLLs are available (if not statically linked)
- Check Windows Event Viewer for errors

### GUI Not Showing

- Verify display settings
- Check application logs
- Ensure Qt is properly installed

## Documentation

For more information:
- [MINERS_README.md](../../MINERS_README.md)
- [INSTALLATION_GUIDE.md](../../INSTALLATION_GUIDE.md)
"@
    $Readme | Out-File (Join-Path $PackageDir "README.md") -Encoding UTF8
    
    # Create installation script
    $InstallScript = @"
@echo off
echo Installing GXC GUI Miners...
echo.

echo ✅ Installation complete!
echo.
echo GUI applications are ready to use.
echo.
echo Note: You may need Qt libraries and Visual C++ Redistributables.
echo See README.md for details.
echo.
pause
"@
    $InstallScript | Out-File (Join-Path $PackageDir "install.bat") -Encoding ASCII
    
    # Create archive
    Push-Location $PackagesDir
    try {
        Compress-Archive -Path $PackageName -DestinationPath "$PackageName.zip" -Force
        Write-Host "✅ Created: $PackageName.zip" -ForegroundColor Green
    } finally {
        Pop-Location
    }
    
    Write-Host ""
    return $true
}

# Main packaging
$CliSuccess = Package-CliMiners
$GuiSuccess = Package-GuiMiners

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Packaging Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan
Write-Host "Packages created in: $PackagesDir" -ForegroundColor Yellow
Write-Host ""
