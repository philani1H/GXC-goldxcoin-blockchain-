# Master script to build and package ALL miners
# Builds C++ miners, packages Python miner, creates all installable packages

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Building ALL GXC Miners" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectRoot "build"
$PackagesDir = Join-Path $ProjectRoot "packages"

# Create directories
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}
if (-not (Test-Path $PackagesDir)) {
    New-Item -ItemType Directory -Path $PackagesDir | Out-Null
}

# Step 1: Build C++ miners
Write-Host "Step 1: Building C++ Miners..." -ForegroundColor Yellow
Write-Host "==============================" -ForegroundColor Yellow

Push-Location $BuildDir

try {
    if (-not (Test-Path "CMakeCache.txt")) {
        Write-Host "Configuring CMake..." -ForegroundColor Cyan
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON -DBUILD_GUI=ON -DBUILD_TESTS=OFF
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "⚠️  GUI build failed, trying without GUI..." -ForegroundColor Yellow
            cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON -DBUILD_TESTS=OFF
            if ($LASTEXITCODE -ne 0) {
                throw "CMake configuration failed"
            }
        }
    }
    
    Write-Host "Building all miners..." -ForegroundColor Cyan
    
    $Miners = @(
        "gxc-node",
        "gxc-miner",
        "gxc-gxhash-miner",
        "gxc-ethash-miner",
        "gxc-sha256-miner",
        "gxc-pool-proxy"
    )
    
    foreach ($Miner in $Miners) {
        Write-Host "Building $Miner..." -ForegroundColor Gray
        cmake --build . --config Release --target $Miner -j 4
        if ($LASTEXITCODE -ne 0) {
            Write-Host "⚠️  $Miner build failed" -ForegroundColor Yellow
        }
    }
    
    # Build GUI miners
    Write-Host "`nBuilding GUI miners (if Qt available)..." -ForegroundColor Cyan
    $GuiMiners = @(
        "gxc-mining-gui",
        "gxc-wallet",
        "gxc-node-gui",
        "gxc-miner-gui",
        "gxc-gxhash-miner-gui",
        "gxc-sha256-miner-gui",
        "gxc-ethash-miner-gui",
        "gxc-pool-proxy-gui"
    )
    
    foreach ($Miner in $GuiMiners) {
        Write-Host "Building $Miner..." -ForegroundColor Gray
        cmake --build . --config Release --target $Miner -j 4
        if ($LASTEXITCODE -ne 0) {
            Write-Host "⚠️  $Miner build failed (Qt may not be available)" -ForegroundColor Yellow
        }
    }
    
} catch {
    Write-Host "❌ Build failed: $_" -ForegroundColor Red
} finally {
    Pop-Location
}

# Step 2: Create Python packages
Write-Host "`nStep 2: Creating Python Packages..." -ForegroundColor Yellow
Write-Host "====================================" -ForegroundColor Yellow

if (Test-Path "$ScriptDir\create-standalone-packages.ps1") {
    & "$ScriptDir\create-standalone-packages.ps1"
} else {
    Write-Host "⚠️  Python package script not found" -ForegroundColor Yellow
}

# Step 3: Package C++ miners
Write-Host "`nStep 3: Packaging C++ Miners..." -ForegroundColor Yellow
Write-Host "===============================" -ForegroundColor Yellow

if (Test-Path "$ScriptDir\package-cpp-miners.ps1") {
    & "$ScriptDir\package-cpp-miners.ps1"
} else {
    Write-Host "⚠️  C++ package script not found" -ForegroundColor Yellow
}

# Step 4: Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Build Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "Packages created in: $PackagesDir" -ForegroundColor Yellow
Write-Host ""
Write-Host "Available packages:" -ForegroundColor Cyan
Get-ChildItem "$PackagesDir\*.zip", "$PackagesDir\*.tar.gz" -ErrorAction SilentlyContinue | ForEach-Object {
    Write-Host "  $($_.Name) ($([math]::Round($_.Length/1MB, 2)) MB)" -ForegroundColor White
}

Write-Host "`nBuilt executables in: $BuildDir\Release" -ForegroundColor Yellow
Write-Host ""
Get-ChildItem "$BuildDir\Release\gxc-*.exe" -ErrorAction SilentlyContinue | ForEach-Object {
    Write-Host "  $($_.Name)" -ForegroundColor White
}

Write-Host ""
