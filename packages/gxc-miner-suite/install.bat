@echo off
setlocal enabledelayedexpansion
title GXC Miner Suite Installer

set "APP_NAME=GXC Miner Suite"
set "VERSION=2.0.0"
set "INSTALL_DIR=%LOCALAPPDATA%\GXC-Miner-Suite"

echo.
echo ===================================================================
echo     GXC MINER SUITE INSTALLER v%VERSION%
echo     SHA256 - Ethash - GxHash Mining
echo ===================================================================
echo.

REM Check Python
echo [INFO] Checking Python...
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found!
    echo Install from: https://www.python.org/downloads/
    pause
    exit /b 1
)
echo [OK] Python found

REM License
echo.
echo ===================================================================
echo                     LICENSE AGREEMENT
echo ===================================================================
echo.
echo This software is provided under the MIT License.
echo.
echo WARNING: Mining involves financial risks including:
echo   * Hardware and electricity costs
echo   * No profit guarantees
echo.
set /p ACCEPT="Accept license terms? [Y/N]: "
if /i not "%ACCEPT%"=="Y" (
    echo License not accepted. Cancelled.
    pause
    exit /b 1
)
echo [OK] License accepted

REM Install
echo [INFO] Installing...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%INSTALL_DIR%\src" mkdir "%INSTALL_DIR%\src"

xcopy /Y /E /I "src\*" "%INSTALL_DIR%\src\" >nul
copy /Y "requirements.txt" "%INSTALL_DIR%\" >nul
copy /Y "LICENSE" "%INSTALL_DIR%\" >nul

REM Install dependencies
echo [INFO] Installing dependencies...
python -m pip install --upgrade pip >nul
python -m pip install -r requirements.txt
if errorlevel 1 (
    echo [ERROR] Failed to install dependencies
    pause
    exit /b 1
)

REM Create launcher
echo [INFO] Creating launcher...
(
echo @echo off
echo cd /d "%INSTALL_DIR%"
echo python -m src.gxc_miner %%*
) > "%INSTALL_DIR%\gxc-miner.bat"

REM Create shortcuts
echo [INFO] Creating shortcuts...
set "STARTMENU=%APPDATA%\Microsoft\Windows\Start Menu\Programs"
set "DESKTOP=%USERPROFILE%\Desktop"

powershell -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%STARTMENU%\GXC Miner Suite.lnk'); $sc.TargetPath = '%INSTALL_DIR%\gxc-miner.bat'; $sc.WorkingDirectory = '%INSTALL_DIR%'; $sc.Save()"
powershell -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%DESKTOP%\GXC Miner Suite.lnk'); $sc.TargetPath = '%INSTALL_DIR%\gxc-miner.bat'; $sc.WorkingDirectory = '%INSTALL_DIR%'; $sc.Save()"

echo.
echo ===================================================================
echo              INSTALLATION COMPLETE!
echo ===================================================================
echo.
echo   Run from Start Menu or Desktop shortcut
echo   Or: %INSTALL_DIR%\gxc-miner.bat
echo.
echo   Happy Mining!
echo.
pause
