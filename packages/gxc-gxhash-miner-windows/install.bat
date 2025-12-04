@echo off
setlocal enabledelayedexpansion
title GXC GxHash Miner Installer

echo ===================================================
echo     GXC GxHash CPU Miner Installer v2.0.0
echo ===================================================
echo.

python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found!
    echo Install from: https://python.org/downloads/
    pause
    exit /b 1
)
echo [OK] Python found

echo.
echo MIT License - Mining involves financial risk.
set /p ACCEPT="Accept? [Y/N]: "
if /i not "%ACCEPT%"=="Y" (
    echo Cancelled.
    pause
    exit /b 1
)

set "INSTALL_DIR=%LOCALAPPDATA%\GXC-GxHash-Miner"
mkdir "%INSTALL_DIR%" 2>nul
copy /Y gxhash_miner.py "%INSTALL_DIR%\" >nul
copy /Y requirements.txt "%INSTALL_DIR%\" >nul

echo [INFO] Installing dependencies...
python -m pip install -r requirements.txt

echo @echo off > "%INSTALL_DIR%\gxhash-miner.bat"
echo python "%INSTALL_DIR%\gxhash_miner.py" %%* >> "%INSTALL_DIR%\gxhash-miner.bat"

powershell -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%USERPROFILE%\Desktop\GXC GxHash Miner.lnk'); $sc.TargetPath = '%INSTALL_DIR%\gxhash-miner.bat'; $sc.Save()"

echo.
echo ===================================================
echo              Installation Complete!
echo ===================================================
echo.
echo Run from Desktop shortcut or:
echo   %INSTALL_DIR%\gxhash-miner.bat
echo.
pause
