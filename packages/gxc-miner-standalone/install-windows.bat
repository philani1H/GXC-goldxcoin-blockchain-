@echo off
setlocal enabledelayedexpansion
title GXC Miner Installer v2.0.0

set "VERSION=2.0.0"
set "INSTALL_DIR=%LOCALAPPDATA%\GXC-Miner"

echo.
echo ===============================================================================
echo     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗
echo    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗
echo    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝
echo    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗
echo    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║
echo     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝
echo ===============================================================================
echo                         GXC MINER INSTALLER v%VERSION%
echo ===============================================================================
echo.

REM License agreement
echo ===============================================================================
echo                            LICENSE AGREEMENT
echo ===============================================================================
echo.
echo   This software is provided under the MIT License.
echo.
echo   WARNING: Mining cryptocurrency involves financial risk.
echo   No profit or earnings are guaranteed.
echo.
set /p ACCEPT="  Do you accept these terms? [Y/N]: "
if /i not "%ACCEPT%"=="Y" (
    echo Installation cancelled.
    pause
    exit /b 1
)
echo.
echo [OK] License accepted
echo.

REM Create directories
echo [1/4] Creating directories...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
echo [OK] Directories created

REM Check for pre-built binary
if exist "%~dp0bin\gxc-miner.exe" (
    echo [2/4] Installing pre-built binary...
    copy /Y "%~dp0bin\gxc-miner.exe" "%INSTALL_DIR%\" >nul
    echo [OK] Binary installed
    goto :create_shortcuts
)

REM Need to build from source
echo [2/4] Building from source...

REM Check Python
python --version >nul 2>&1
if errorlevel 1 (
    echo [INFO] Python not found. Downloading...
    
    REM Download Python installer
    powershell -Command "& {Invoke-WebRequest -Uri 'https://www.python.org/ftp/python/3.11.0/python-3.11.0-amd64.exe' -OutFile '%TEMP%\python-installer.exe'}"
    
    if exist "%TEMP%\python-installer.exe" (
        echo [INFO] Installing Python...
        start /wait "" "%TEMP%\python-installer.exe" /quiet InstallAllUsers=0 PrependPath=1
        del "%TEMP%\python-installer.exe"
    ) else (
        echo [ERROR] Failed to download Python.
        echo         Please install Python manually from https://python.org
        pause
        exit /b 1
    )
)

echo [OK] Python found

REM Install PyInstaller and build
echo [INFO] Installing build tools...
python -m pip install --quiet --upgrade pip pyinstaller >nul 2>&1

echo [INFO] Building executable...
cd /d "%~dp0src"
python -m PyInstaller --onefile --name gxc-miner --clean --noconfirm --log-level ERROR gxc_miner_cli.py 2>nul

if exist "dist\gxc-miner.exe" (
    move /Y "dist\gxc-miner.exe" "%INSTALL_DIR%\" >nul
    rmdir /S /Q build 2>nul
    rmdir /S /Q dist 2>nul
    del /Q *.spec 2>nul
    echo [OK] Binary built and installed
) else (
    echo [ERROR] Build failed
    pause
    exit /b 1
)

:create_shortcuts
echo [3/4] Creating shortcuts...

REM Create Start Menu shortcut
set "STARTMENU=%APPDATA%\Microsoft\Windows\Start Menu\Programs"

REM Create a .bat wrapper for command line
echo @echo off > "%INSTALL_DIR%\gxc-miner.bat"
echo "%INSTALL_DIR%\gxc-miner.exe" %%* >> "%INSTALL_DIR%\gxc-miner.bat"

REM Create PowerShell shortcut creator
powershell -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%STARTMENU%\GXC Miner.lnk'); $sc.TargetPath = 'cmd.exe'; $sc.Arguments = '/k cd /d %INSTALL_DIR% && echo GXC Miner - Type gxc-miner --help for usage'; $sc.WorkingDirectory = '%INSTALL_DIR%'; $sc.Save()"

REM Desktop shortcut
powershell -Command "$ws = New-Object -ComObject WScript.Shell; $sc = $ws.CreateShortcut('%USERPROFILE%\Desktop\GXC Miner.lnk'); $sc.TargetPath = 'cmd.exe'; $sc.Arguments = '/k cd /d %INSTALL_DIR% && echo GXC Miner - Type gxc-miner --help for usage'; $sc.WorkingDirectory = '%INSTALL_DIR%'; $sc.Save()"

echo [OK] Shortcuts created

REM Add to PATH
echo [4/4] Adding to PATH...

REM Add to user PATH
for /f "skip=2 tokens=3*" %%a in ('reg query HKCU\Environment /v PATH 2^>nul') do set "USERPATH=%%a%%b"
echo %USERPATH% | findstr /i /c:"%INSTALL_DIR%" >nul
if errorlevel 1 (
    setx PATH "%USERPATH%;%INSTALL_DIR%" >nul 2>&1
    echo [OK] Added to PATH
) else (
    echo [OK] Already in PATH
)

echo.
echo ===============================================================================
echo                        INSTALLATION COMPLETE!
echo ===============================================================================
echo.
echo   Location:   %INSTALL_DIR%
echo.
echo   +---------------------------------------------------------+
echo   ^|                    QUICK START                          ^|
echo   ^|                                                         ^|
echo   ^|   Open Command Prompt and run:                          ^|
echo   ^|                                                         ^|
echo   ^|   gxc-miner -o pool.gxc.io:3333 -u YOUR_WALLET -p x    ^|
echo   ^|                                                         ^|
echo   ^|   For help: gxc-miner --help                            ^|
echo   +---------------------------------------------------------+
echo.
echo   NOTE: You may need to restart your terminal for the
echo         'gxc-miner' command to be available.
echo.
echo   Happy Mining!
echo.
pause
