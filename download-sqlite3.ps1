# Quick SQLite3 Download Script
Write-Host "Downloading SQLite3 amalgamation..." -ForegroundColor Cyan

$sqliteDir = "third_party\sqlite3"
$zipFile = "build\sqlite3.zip"
# Try multiple URLs (SQLite changes their structure)
$urls = @(
    "https://www.sqlite.org/2024/sqlite-amalgamation-3490200.zip",
    "https://sqlite.org/2024/sqlite-amalgamation-3490200.zip",
    "https://www.sqlite.org/src/tarball/sqlite.tar.gz"
)
$url = $urls[0]  # Try first one

# Create directories
New-Item -ItemType Directory -Path $sqliteDir -Force | Out-Null
New-Item -ItemType Directory -Path "build" -Force | Out-Null

# Download - try multiple methods
Write-Host "Downloading from sqlite.org..." -ForegroundColor Yellow
$downloaded = $false
foreach ($tryUrl in $urls) {
    try {
        Write-Host "Trying: $tryUrl" -ForegroundColor Gray
        Invoke-WebRequest -Uri $tryUrl -OutFile $zipFile -UseBasicParsing -ErrorAction Stop
        Write-Host "Download complete!" -ForegroundColor Green
        $downloaded = $true
        break
    } catch {
        Write-Host "Failed: $_" -ForegroundColor Gray
        continue
    }
}

if (-not $downloaded) {
    Write-Host "`n❌ Auto-download failed!" -ForegroundColor Red
    Write-Host "`nPlease download manually:" -ForegroundColor Yellow
    Write-Host "1. Go to: https://www.sqlite.org/download.html" -ForegroundColor White
    Write-Host "2. Download: 'sqlite-amalgamation' (zip file)" -ForegroundColor White
    Write-Host "3. Extract sqlite3.h and sqlite3.c" -ForegroundColor White
    Write-Host "4. Place them in: $sqliteDir\" -ForegroundColor White
    Write-Host "`nOr run: choco install sqlite" -ForegroundColor Cyan
    exit 1
}

# Extract
Write-Host "Extracting..." -ForegroundColor Yellow
try {
    Expand-Archive -Path $zipFile -DestinationPath "build" -Force
    $extracted = Get-ChildItem "build" -Filter "sqlite-amalgamation-*" -Directory | Select-Object -First 1
    if ($extracted) {
        Copy-Item "$($extracted.FullName)\sqlite3.h" -Destination $sqliteDir -Force
        Copy-Item "$($extracted.FullName)\sqlite3.c" -Destination $sqliteDir -Force
        Write-Host "Extracted to $sqliteDir" -ForegroundColor Green
        Write-Host "`n✅ SQLite3 ready! Now run: cmake -S . -B build" -ForegroundColor Green
    } else {
        Write-Host "Extraction failed - folder not found" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "Extraction failed: $_" -ForegroundColor Red
    exit 1
}

