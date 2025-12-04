@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM GXC MINER PRO - Windows Installer
REM ============================================================================

title GXC Miner Pro - Installer

REM Configuration
set VERSION=2.0.0
set PRODUCT_NAME=GXC Miner Pro
set INSTALL_DIR=%LOCALAPPDATA%\GXC-Miner-Pro

REM Colors (Windows 10+)
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"
set "CYAN=[96m"
set "WHITE=[97m"
set "NC=[0m"

cls

REM ============================================================================
REM BANNER
REM ============================================================================

echo.
echo %GREEN%   ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗ %NC%
echo %GREEN%  ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗%NC%
echo %GREEN%  ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝%NC%
echo %GREEN%  ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗%NC%
echo %GREEN%  ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║%NC%
echo %GREEN%   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝%NC%
echo %GREEN%                                 PRO%NC%
echo.
echo %WHITE%          The Ultimate Mining Software for GXC Blockchain%NC%
echo %CYAN%                        Version %VERSION%%NC%
echo.
echo %YELLOW%===============================================================================%NC%
echo.

REM ============================================================================
REM LICENSE AGREEMENT
REM ============================================================================

echo %WHITE%LICENSE AGREEMENT%NC%
echo %YELLOW%===============================================================================%NC%
echo.
echo MIT License
echo.
echo Copyright (c) 2024-2025 GXC Foundation
echo.
echo Permission is hereby granted, free of charge, to any person obtaining
echo a copy of this software and associated documentation files (the
echo "Software"), to deal in the Software without restriction, including
echo without limitation the rights to use, copy, modify, merge, publish,
echo distribute, sublicense, and/or sell copies of the Software.
echo.
echo %RED%MINING DISCLAIMER:%NC%
echo - Cryptocurrency mining profitability is NOT guaranteed
echo - Electricity costs may exceed mining rewards
echo - Hardware wear and damage is the user's responsibility
echo - Check local regulations regarding cryptocurrency mining
echo.
echo %YELLOW%===============================================================================%NC%
echo.

set /p ACCEPT="Do you accept the license agreement? [Y/N]: "
if /i not "%ACCEPT%"=="Y" (
    echo.
    echo %RED%Installation cancelled.%NC%
    pause
    exit /b 1
)

echo.

REM ============================================================================
REM CHECK PYTHON
REM ============================================================================

echo %CYAN%[1/5] Checking Python installation...%NC%

where python >nul 2>&1
if %errorlevel% neq 0 (
    echo %YELLOW%  [!] Python not found. Attempting to install...%NC%
    
    REM Try to install Python via winget
    where winget >nul 2>&1
    if %errorlevel% equ 0 (
        echo   Installing Python via winget...
        winget install Python.Python.3.11 --silent --accept-package-agreements --accept-source-agreements
    ) else (
        echo %RED%  [X] Cannot install Python automatically.%NC%
        echo.
        echo   Please install Python from: https://www.python.org/downloads/
        echo   Make sure to check "Add Python to PATH" during installation.
        echo.
        pause
        exit /b 1
    )
) else (
    for /f "tokens=*" %%i in ('python --version 2^>^&1') do set PYTHON_VER=%%i
    echo %GREEN%  [OK] !PYTHON_VER! found%NC%
)

REM ============================================================================
REM INSTALL DEPENDENCIES
REM ============================================================================

echo %CYAN%[2/5] Installing dependencies...%NC%

echo   Installing customtkinter...
python -m pip install customtkinter --quiet 2>nul

echo   Installing matplotlib...
python -m pip install matplotlib --quiet 2>nul

echo   Installing psutil...
python -m pip install psutil --quiet 2>nul

echo   Installing requests...
python -m pip install requests --quiet 2>nul

echo %GREEN%  [OK] Dependencies installed%NC%

REM ============================================================================
REM CREATE INSTALLATION DIRECTORY
REM ============================================================================

echo %CYAN%[3/5] Creating installation directory...%NC%

if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
echo %GREEN%  [OK] Created %INSTALL_DIR%%NC%

REM ============================================================================
REM COPY FILES
REM ============================================================================

echo %CYAN%[4/5] Installing files...%NC%

set SCRIPT_DIR=%~dp0
set PARENT_DIR=%SCRIPT_DIR%..

REM Copy main script
if exist "%PARENT_DIR%\src\gxc_miner_pro.py" (
    copy /Y "%PARENT_DIR%\src\gxc_miner_pro.py" "%INSTALL_DIR%\" >nul
) else if exist "%PARENT_DIR%\dist\GXC-Miner-Pro.exe" (
    copy /Y "%PARENT_DIR%\dist\GXC-Miner-Pro.exe" "%INSTALL_DIR%\" >nul
) else (
    echo %RED%  [X] Source files not found%NC%
    pause
    exit /b 1
)

REM Copy license and readme
if exist "%PARENT_DIR%\LICENSE" copy /Y "%PARENT_DIR%\LICENSE" "%INSTALL_DIR%\" >nul
if exist "%PARENT_DIR%\README.md" copy /Y "%PARENT_DIR%\README.md" "%INSTALL_DIR%\" >nul

echo %GREEN%  [OK] Files installed%NC%

REM ============================================================================
REM CREATE LAUNCHER
REM ============================================================================

echo %CYAN%[5/5] Creating launcher...%NC%

REM Create batch launcher
(
echo @echo off
echo cd /d "%INSTALL_DIR%"
echo if exist "GXC-Miner-Pro.exe" ^(
echo     GXC-Miner-Pro.exe %%*
echo ^) else ^(
echo     python gxc_miner_pro.py %%*
echo ^)
) > "%INSTALL_DIR%\gxc-miner-pro.bat"

REM Add to PATH (user environment)
for /f "tokens=2*" %%a in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "CURRENT_PATH=%%b"
echo !CURRENT_PATH! | find /i "%INSTALL_DIR%" >nul
if %errorlevel% neq 0 (
    setx PATH "%INSTALL_DIR%;!CURRENT_PATH!" >nul 2>&1
)

REM Create Start Menu shortcut
set SHORTCUT_DIR=%APPDATA%\Microsoft\Windows\Start Menu\Programs
if not exist "%SHORTCUT_DIR%\GXC Miner Pro.lnk" (
    powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%SHORTCUT_DIR%\GXC Miner Pro.lnk'); $SC.TargetPath = '%INSTALL_DIR%\gxc-miner-pro.bat'; $SC.WorkingDirectory = '%INSTALL_DIR%'; $SC.Description = 'GXC Miner Pro - Ultimate Mining Software'; $SC.Save()" 2>nul
)

REM Create Desktop shortcut
set DESKTOP=%USERPROFILE%\Desktop
powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%DESKTOP%\GXC Miner Pro.lnk'); $SC.TargetPath = '%INSTALL_DIR%\gxc-miner-pro.bat'; $SC.WorkingDirectory = '%INSTALL_DIR%'; $SC.Description = 'GXC Miner Pro - Ultimate Mining Software'; $SC.Save()" 2>nul

echo %GREEN%  [OK] Launcher and shortcuts created%NC%

REM ============================================================================
REM COMPLETE
REM ============================================================================

echo.
echo %GREEN%===============================================================================%NC%
echo %WHITE%           GXC MINER PRO INSTALLED SUCCESSFULLY!%NC%
echo %GREEN%===============================================================================%NC%
echo.
echo %CYAN%Installation Location:%NC% %INSTALL_DIR%
echo.
echo %WHITE%QUICK START:%NC%
echo.
echo   %GREEN%GUI Mode:%NC%
echo     Open "GXC Miner Pro" from Start Menu or Desktop
echo     Or run: gxc-miner-pro
echo.
echo   %GREEN%Pool Mining:%NC%
echo     gxc-miner-pro -o stratum+tcp://pool.gxc.network:3333 -u YOUR_WALLET -p x
echo.
echo   %GREEN%Solo Mining:%NC%
echo     gxc-miner-pro --solo -n http://localhost:8332 -u YOUR_WALLET
echo.
echo   %GREEN%Help:%NC%
echo     gxc-miner-pro --help
echo.
echo %YELLOW%===============================================================================%NC%
echo %CYAN%Happy Mining! For support, visit: https://docs.gxc.network%NC%
echo %YELLOW%===============================================================================%NC%
echo.
echo %WHITE%NOTE: You may need to open a new command prompt for PATH changes.%NC%
echo.
pause
