# GXC Miners Build Script
# Builds all mining software for download

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  GXC Miners Build Script" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

# Check for CMake
Write-Host "Checking for CMake..." -ForegroundColor Yellow
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "❌ CMake not found! Please install CMake first." -ForegroundColor Red
    Write-Host "   Run: .\build-blockchain.ps1 first" -ForegroundColor Yellow
    exit 1
}
Write-Host "✅ CMake found" -ForegroundColor Green

# Create build directory
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Create miners output directory
$minersDir = "miners_release"
if (Test-Path $minersDir) {
    Remove-Item -Recurse -Force $minersDir
}
New-Item -ItemType Directory -Path $minersDir | Out-Null

Push-Location build

try {
    # Configure build
    Write-Host "`nConfiguring build..." -ForegroundColor Yellow
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON -DBUILD_TESTS=OFF
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    # Build all miners
    Write-Host "`nBuilding miners..." -ForegroundColor Yellow
    
    $miners = @(
        @{Name="gxc-miner"; Desc="Universal Miner"},
        @{Name="gxc-gxhash-miner"; Desc="GXHash CPU Miner"},
        @{Name="gxc-ethash-miner"; Desc="Ethash GPU Miner"},
        @{Name="gxc-sha256-miner"; Desc="SHA-256 ASIC Miner"},
        @{Name="gxc-node"; Desc="Blockchain Node"}
    )
    
    foreach ($miner in $miners) {
        Write-Host "`nBuilding $($miner.Name) ($($miner.Desc))..." -ForegroundColor Cyan
        
        if ($IsWindows -or $env:OS -like "*Windows*") {
            cmake --build . --config Release --target $miner.Name -j 4
        } else {
            make -j$(nproc) $miner.Name
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ $($miner.Name) built successfully" -ForegroundColor Green
            
            # Copy executable
            $exePath = ""
            if ($IsWindows -or $env:OS -like "*Windows*") {
                $exePath = "Release\$($miner.Name).exe"
                if (-not (Test-Path $exePath)) {
                    $exePath = "$($miner.Name)\Release\$($miner.Name).exe"
                }
            } else {
                $exePath = $miner.Name
            }
            
            if (Test-Path $exePath) {
                Copy-Item $exePath "$minersDir\$($miner.Name)$(if ($IsWindows -or $env:OS -like "*Windows*") { '.exe' })"
                Write-Host "   Copied to: $minersDir\$($miner.Name)$(if ($IsWindows -or $env:OS -like "*Windows*") { '.exe' })" -ForegroundColor Gray
            }
        } else {
            Write-Host "⚠️  $($miner.Name) build failed (may not be available on this platform)" -ForegroundColor Yellow
        }
    }
    
    Pop-Location
    
    # Package Python miner
    Write-Host "`nPackaging Python GXHash miner..." -ForegroundColor Yellow
    
    $pythonMinerDir = "$minersDir\gxhash-miner-python"
    New-Item -ItemType Directory -Path $pythonMinerDir | Out-Null
    
    # Copy Python miner
    Copy-Item "mining\gxhash_miner.py" "$pythonMinerDir\gxhash_miner.py"
    
    # Create requirements.txt
    @"
requests>=2.31.0
"@ | Out-File "$pythonMinerDir\requirements.txt" -Encoding UTF8
    
    # Create README
    @"
# GXC GXHash Miner - Python

## Installation

1. Install Python 3.7 or higher
2. Install dependencies:
   pip install -r requirements.txt

## Usage

### Basic mining:
python gxhash_miner.py --rpc-url http://localhost:8545

### With options:
python gxhash_miner.py --rpc-url http://localhost:8545 --threads 4 --address GXC...

### Background/daemon mode:
python gxhash_miner.py --daemon --rpc-url http://localhost:8545

## Options

--rpc-url URL        Blockchain RPC URL (default: http://localhost:8545)
--rest-url URL       Blockchain REST URL (default: http://localhost:8080)
--threads N          Number of mining threads (0 = auto-detect CPU cores)
--address ADDRESS    Miner address for coinbase transactions (optional)
--daemon             Run in background/daemon mode

## Features

- ✅ Connects directly to blockchain node
- ✅ Gets real block templates from blockchain
- ✅ Submits mined blocks to blockchain
- ✅ No manual endpoints required
- ✅ No auto-transfers - rewards come from blockchain
- ✅ Background/daemon mode support
- ✅ Multi-threaded CPU mining
- ✅ Real-time statistics

## Requirements

- Python 3.7+
- GXC blockchain node running
- RPC enabled on blockchain node (port 8545)
- REST API enabled on blockchain node (port 8080)
"@ | Out-File "$pythonMinerDir\README.md" -Encoding UTF8
    
    # Create Windows batch file
    @"
@echo off
echo GXC GXHash Miner - Python
echo ========================
echo.
python gxhash_miner.py %*
pause
"@ | Out-File "$pythonMinerDir\run_miner.bat" -Encoding ASCII
    
    # Create Linux/Mac shell script
    @"
#!/bin/bash
echo "GXC GXHash Miner - Python"
echo "========================"
echo ""
python3 gxhash_miner.py "$@"
"@ | Out-File "$pythonMinerDir\run_miner.sh" -Encoding ASCII -NoNewline
    
    Write-Host "✅ Python miner packaged" -ForegroundColor Green
    
    # Create main README
    @"
# GXC Mining Software

This directory contains all GXC mining software ready for download.

## Available Miners

### C++ Miners (Built from Source)

1. **gxc-node.exe** - Blockchain node (required for mining)
2. **gxc-miner.exe** - Universal miner (all algorithms)
3. **gxc-gxhash-miner.exe** - GXHash CPU miner
4. **gxc-ethash-miner.exe** - Ethash GPU miner (NVIDIA/AMD)
5. **gxc-sha256-miner.exe** - SHA-256 ASIC miner

### Python Miner

**gxhash-miner-python/** - Python GXHash CPU miner
- Cross-platform (Windows, Linux, macOS)
- No compilation required
- See README.md in directory for usage

## Quick Start

1. **Start blockchain node:**
   \`\`\`
   .\gxc-node.exe --rpc-port=8545 --rest-port=8080
   \`\`\`

2. **Start miner:**
   \`\`\`
   .\gxc-gxhash-miner.exe --rpc-url=http://localhost:8545
   \`\`\`

   Or use Python miner:
   \`\`\`
   cd gxhash-miner-python
   pip install -r requirements.txt
   python gxhash_miner.py
   \`\`\`

## System Requirements

- **Windows**: Windows 10 or later
- **Linux**: Ubuntu 20.04+ or similar
- **macOS**: macOS 10.15+
- **RAM**: 4GB minimum, 8GB recommended
- **CPU**: Multi-core recommended for mining

## Documentation

See MINING_GUIDE.md in the main directory for detailed documentation.

## Support

For issues or questions, check the documentation or contact support.
"@ | Out-File "$minersDir\README.md" -Encoding UTF8
    
    Write-Host "`n✅ All miners built and packaged!" -ForegroundColor Green
    Write-Host "`nMiners are available in: $minersDir\" -ForegroundColor Cyan
    Write-Host "`nTo create a release package:" -ForegroundColor Yellow
    Write-Host "   Compress-RestPath -Path $minersDir -DestinationPath gxc-miners-release.zip" -ForegroundColor White
    
} catch {
    Write-Host "`n❌ Build failed: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Build Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

