# Create standalone installable packages for all miners
# Creates packages for Windows, Linux, and macOS

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Creating Standalone Miner Packages" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$PackagesDir = Join-Path $ProjectRoot "packages"
$MiningDir = Join-Path $ProjectRoot "mining"

# Create packages directory
if (-not (Test-Path $PackagesDir)) {
    New-Item -ItemType Directory -Path $PackagesDir | Out-Null
}

function Create-PythonPackage {
    param(
        [string]$Platform
    )
    
    $PackageName = "gxc-gxhash-miner-$Platform"
    $PackageDir = Join-Path $PackagesDir $PackageName
    
    Write-Host "Creating Python package for $Platform..." -ForegroundColor Yellow
    
    if (Test-Path $PackageDir) {
        Remove-Item -Recurse -Force $PackageDir
    }
    New-Item -ItemType Directory -Path $PackageDir | Out-Null
    
    # Copy miner files
    Copy-Item (Join-Path $MiningDir "gxhash_miner.py") $PackageDir
    Copy-Item (Join-Path $MiningDir "gxhash_miner_requirements.txt") (Join-Path $PackageDir "requirements.txt")
    if (Test-Path (Join-Path $MiningDir "setup.py")) {
        Copy-Item (Join-Path $MiningDir "setup.py") $PackageDir
    }
    
    # Create installation script
    if ($Platform -eq "windows") {
        $installScript = @"
@echo off
echo Installing GXC GXHash Miner...
echo.

python --version >nul 2>&1
if errorlevel 1 (
    echo Python not found! Please install Python 3.7+ from https://www.python.org/downloads/
    pause
    exit 1
)

echo Installing dependencies...
pip install -r requirements.txt --user
if errorlevel 1 (
    echo Installation failed!
    pause
    exit 1
)

echo.
echo ✅ Installation complete!
echo.
echo You can now run: python gxhash_miner.py
echo.
pause
"@
        $installScript | Out-File (Join-Path $PackageDir "install.bat") -Encoding ASCII
    } else {
        $installScript = @"
#!/bin/bash
echo "Installing GXC GXHash Miner..."
echo ""

if ! command -v python3 &> /dev/null; then
    echo "Python 3 not found! Please install Python 3.7+"
    exit 1
fi

echo "Installing dependencies..."
pip3 install -r requirements.txt --user

echo ""
echo "✅ Installation complete!"
echo ""
echo "You can now run: python3 gxhash_miner.py"
"@
        $installScript | Out-File (Join-Path $PackageDir "install.sh") -Encoding ASCII -NoNewline
    }
    
    # Create README
    $readme = @"
# GXC GXHash Miner - Standalone Package

## Installation

### Windows
1. Double-click `install.bat`
2. Or run manually:
   ````
   pip install -r requirements.txt --user
   python gxhash_miner.py
   ````

### Linux/macOS
1. Run: `bash install.sh`
2. Or run manually:
   ````
   pip3 install -r requirements.txt --user
   python3 gxhash_miner.py
   ````

## Usage

````bash
python gxhash_miner.py --rpc-url http://localhost:8545
````

## Options

- `--rpc-url URL` - Blockchain RPC URL (default: http://localhost:8545)
- `--rest-url URL` - Blockchain REST URL (default: http://localhost:8080)
- `--threads N` - Number of mining threads (0 = auto-detect)
- `--address ADDRESS` - Miner address for rewards
- `--daemon` - Run in background mode

## Requirements

- Python 3.7 or higher
- GXC blockchain node running
- RPC enabled on node (port 8545)
"@
    $readme | Out-File (Join-Path $PackageDir "README.md") -Encoding UTF8
    
    # Create archive
    Push-Location $PackagesDir
    try {
        if ($Platform -eq "windows") {
            Compress-Archive -Path $PackageName -DestinationPath "$PackageName.zip" -Force
            Write-Host "✅ Created: $PackageName.zip" -ForegroundColor Green
        } else {
            # For cross-platform, we'll create zip for all
            Compress-Archive -Path $PackageName -DestinationPath "$PackageName.zip" -Force
            Write-Host "✅ Created: $PackageName.zip" -ForegroundColor Green
        }
    } finally {
        Pop-Location
    }
}

# Create packages for all platforms
Create-PythonPackage "windows"
Create-PythonPackage "linux"
Create-PythonPackage "macos"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Package Creation Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan
Write-Host "Packages created in: $PackagesDir" -ForegroundColor Yellow
Write-Host ""
