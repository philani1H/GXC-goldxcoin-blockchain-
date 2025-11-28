# GXC Blockchain Build Script for Windows
# This script builds the GXC blockchain node

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  GXC Blockchain Build Script" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Check for CMake
Write-Host "Checking for CMake..." -ForegroundColor Yellow
$cmakeFound = $false
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeVersion = cmake --version | Select-Object -First 1
    Write-Host "✅ Found: $cmakeVersion" -ForegroundColor Green
    $cmakeFound = $true
} else {
    Write-Host "❌ CMake not found!" -ForegroundColor Red
    Write-Host "`nPlease install CMake:" -ForegroundColor Yellow
    Write-Host "  1. Download from: https://cmake.org/download/" -ForegroundColor White
    Write-Host "  2. Or install via Chocolatey: choco install cmake" -ForegroundColor White
    Write-Host "  3. Or install via winget: winget install Kitware.CMake" -ForegroundColor White
    exit 1
}

# Check for C++ compiler
Write-Host "`nChecking for C++ compiler..." -ForegroundColor Yellow
$compilerFound = $false
$compilerType = ""

# Check for Visual Studio
if (Get-Command cl -ErrorAction SilentlyContinue) {
    Write-Host "✅ Found: Visual Studio C++ compiler" -ForegroundColor Green
    $compilerFound = $true
    $compilerType = "msvc"
} elseif (Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat") {
    Write-Host "⚠️  Visual Studio found but not in PATH" -ForegroundColor Yellow
    Write-Host "   Run: 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat'" -ForegroundColor White
    Write-Host "   Or open 'Developer Command Prompt for VS 2022'" -ForegroundColor White
    $compilerFound = $true
    $compilerType = "msvc"
} elseif (Get-Command g++ -ErrorAction SilentlyContinue) {
    Write-Host "✅ Found: GCC compiler" -ForegroundColor Green
    $compilerFound = $true
    $compilerType = "gcc"
} elseif (Get-Command clang++ -ErrorAction SilentlyContinue) {
    Write-Host "✅ Found: Clang compiler" -ForegroundColor Green
    $compilerFound = $true
    $compilerType = "clang"
} else {
    Write-Host "❌ No C++ compiler found!" -ForegroundColor Red
    Write-Host "`nPlease install one of the following:" -ForegroundColor Yellow
    Write-Host "  1. Visual Studio 2022 (Community Edition is free)" -ForegroundColor White
    Write-Host "     Download: https://visualstudio.microsoft.com/downloads/" -ForegroundColor White
    Write-Host "     Make sure to install 'Desktop development with C++' workload" -ForegroundColor White
    Write-Host "`n  2. MinGW-w64 (GCC for Windows)" -ForegroundColor White
    Write-Host "     Download: https://www.mingw-w64.org/downloads/" -ForegroundColor White
    Write-Host "     Or via Chocolatey: choco install mingw" -ForegroundColor White
    exit 1
}

# Check for required dependencies
Write-Host "`nChecking dependencies..." -ForegroundColor Yellow

# Check for OpenSSL
$opensslFound = $false
$opensslPaths = @(
    "C:\Program Files\OpenSSL-Win64",
    "C:\Program Files (x86)\OpenSSL-Win64",
    "C:\OpenSSL-Win64",
    "C:\OpenSSL"
)

foreach ($path in $opensslPaths) {
    if (Test-Path "$path\include\openssl\ssl.h") {
        Write-Host "✅ Found OpenSSL at: $path" -ForegroundColor Green
        $env:OPENSSL_ROOT_DIR = $path
        $opensslFound = $true
        break
    }
}

if (-not $opensslFound) {
    Write-Host "⚠️  OpenSSL not found in standard locations" -ForegroundColor Yellow
    Write-Host "   CMake will try to find it automatically" -ForegroundColor White
    Write-Host "   Or download from: https://slproweb.com/products/Win32OpenSSL.html" -ForegroundColor White
}

# Create build directory
Write-Host "`nPreparing build directory..." -ForegroundColor Yellow
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "✅ Created build directory" -ForegroundColor Green
} else {
    Write-Host "✅ Build directory exists" -ForegroundColor Green
}

# Configure with CMake
Write-Host "`nConfiguring build with CMake..." -ForegroundColor Yellow
Push-Location build

try {
    # Configure for Release build
    $cmakeArgs = @(
        "..",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_MINING_CLIENT=ON",
        "-DBUILD_TESTS=OFF"
    )
    
    # Add OpenSSL path if found
    if ($opensslFound) {
        $cmakeArgs += "-DOPENSSL_ROOT_DIR=$env:OPENSSL_ROOT_DIR"
    }
    
    # For Visual Studio, generate solution
    if ($compilerType -eq "msvc") {
        $cmakeArgs += "-G", "Visual Studio 17 2022"
        $cmakeArgs += "-A", "x64"
    }
    
    Write-Host "Running: cmake $($cmakeArgs -join ' ')" -ForegroundColor Cyan
    & cmake @cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n❌ CMake configuration failed!" -ForegroundColor Red
        Write-Host "Check the error messages above for missing dependencies." -ForegroundColor Yellow
        Pop-Location
        exit 1
    }
    
    Write-Host "✅ CMake configuration successful!" -ForegroundColor Green
    
    # Build
    Write-Host "`nBuilding blockchain node..." -ForegroundColor Yellow
    Write-Host "This may take several minutes...`n" -ForegroundColor Cyan
    
    if ($compilerType -eq "msvc") {
        # Use MSBuild for Visual Studio
        $buildArgs = @(
            "--build", ".",
            "--config", "Release",
            "--target", "gxc-node",
            "-j", "4"
        )
        & cmake @buildArgs
    } else {
        # Use make for GCC/Clang
        $buildArgs = @(
            "--build", ".",
            "--config", "Release",
            "--target", "gxc-node",
            "-j", "4"
        )
        & cmake @buildArgs
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n❌ Build failed!" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    Write-Host "`n✅ Build completed successfully!`n" -ForegroundColor Green
    
    # Check for output
    $exePath = ""
    if ($compilerType -eq "msvc") {
        $exePath = "Release\gxc-node.exe"
        if (-not (Test-Path $exePath)) {
            $exePath = "gxc-node\Release\gxc-node.exe"
        }
    } else {
        $exePath = "gxc-node"
    }
    
    if (Test-Path $exePath) {
        $exeInfo = Get-Item $exePath
        Write-Host "Executable: $($exeInfo.FullName)" -ForegroundColor Green
        Write-Host "Size: $([math]::Round($exeInfo.Length / 1MB, 2)) MB" -ForegroundColor Green
        Write-Host "`nYou can now run the blockchain node with:" -ForegroundColor Cyan
        Write-Host "  .\$exePath" -ForegroundColor White
    } else {
        Write-Host "⚠️  Executable not found at expected location" -ForegroundColor Yellow
        Write-Host "   Searching for gxc-node..." -ForegroundColor Yellow
        $found = Get-ChildItem -Recurse -Filter "gxc-node*" -File | Where-Object { $_.Extension -eq ".exe" -or $_.Extension -eq "" }
        if ($found) {
            Write-Host "✅ Found: $($found.FullName)" -ForegroundColor Green
        }
    }
    
} catch {
    Write-Host "`n❌ Error during build: $_" -ForegroundColor Red
    Pop-Location
    exit 1
} finally {
    Pop-Location
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Build Complete!" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

