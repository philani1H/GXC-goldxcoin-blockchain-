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
