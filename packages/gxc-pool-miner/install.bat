@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM
REM          FILE:  install.bat
REM
REM         USAGE:  install.bat
REM
REM   DESCRIPTION:  GXC Pool Miner Installation Script for Windows
REM
REM        AUTHOR:  GXC Blockchain
REM       VERSION:  2.0.0
REM
REM ============================================================================

title GXC Pool Miner Installer

REM Colors (Windows 10+)
set "GREEN=[92m"
set "RED=[91m"
set "YELLOW=[93m"
set "CYAN=[96m"
set "NC=[0m"

REM Configuration
set "APP_NAME=GXC Pool Miner"
set "APP_VERSION=2.0.0"
set "INSTALL_DIR=%LOCALAPPDATA%\GXC-Pool-Miner"

REM Banner
echo.
echo %CYAN%=========================================================%NC%
echo %CYAN%     _____ __  _______   __  __ _                       %NC%
echo %CYAN%    / ___/ / |/ / ___/  /  |/  (_)___  ___  ____        %NC%
echo %CYAN%   / (_ / / /|_/ / /__  / /|_/ / / _ \/ _ \/ __/        %NC%
echo %CYAN%   \___/_/_/  /_/\___/ /_/  /_/_/_//_/\___/_/           %NC%
echo %CYAN%                                                         %NC%
echo %CYAN%           POOL MINER INSTALLER v%APP_VERSION%                    %NC%
echo %CYAN%=========================================================%NC%
echo.

REM Check for Python
echo [INFO] Checking Python installation...

python --version >nul 2>&1
if errorlevel 1 (
    python3 --version >nul 2>&1
    if errorlevel 1 (
        echo %RED%[ERROR] Python is not installed!%NC%
        echo.
        echo Please install Python 3.8 or higher from:
        echo   https://www.python.org/downloads/
        echo.
        echo Make sure to check "Add Python to PATH" during installation.
        echo.
        pause
        exit /b 1
    ) else (
        set "PYTHON_CMD=python3"
    )
) else (
    set "PYTHON_CMD=python"
)

echo %GREEN%[OK] Python found%NC%

REM Check pip
echo [INFO] Checking pip...
%PYTHON_CMD% -m pip --version >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%[WARN] pip not found, attempting to install...%NC%
    %PYTHON_CMD% -m ensurepip --upgrade
    if errorlevel 1 (
        echo %RED%[ERROR] Failed to install pip%NC%
        pause
        exit /b 1
    )
)
echo %GREEN%[OK] pip is available%NC%

REM License Agreement
echo.
echo %YELLOW%=============================================================%NC%
echo %YELLOW%                    LICENSE AGREEMENT                         %NC%
echo %YELLOW%=============================================================%NC%
echo.
echo This software is provided under the MIT License.
echo.
echo IMPORTANT NOTICES:
echo   * Mining involves financial risk
echo   * Hardware utilization may affect device lifespan
echo   * You are responsible for local law compliance
echo.
echo %YELLOW%=============================================================%NC%
echo.

set /p ACCEPT="Do you accept the license terms? [Y/N]: "
if /i not "%ACCEPT%"=="Y" (
    echo %RED%[ERROR] License not accepted. Installation cancelled.%NC%
    pause
    exit /b 1
)
echo %GREEN%[OK] License accepted%NC%

REM Create installation directory
echo [INFO] Creating installation directory...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%INSTALL_DIR%\src" mkdir "%INSTALL_DIR%\src"
if not exist "%INSTALL_DIR%\logs" mkdir "%INSTALL_DIR%\logs"
if not exist "%INSTALL_DIR%\config" mkdir "%INSTALL_DIR%\config"
echo %GREEN%[OK] Directories created%NC%

REM Copy files
echo [INFO] Copying files...
xcopy /Y /E /I "src\*" "%INSTALL_DIR%\src\" >nul
copy /Y "requirements.txt" "%INSTALL_DIR%\" >nul
copy /Y "LICENSE" "%INSTALL_DIR%\" >nul
if exist "README.md" copy /Y "README.md" "%INSTALL_DIR%\" >nul
echo %GREEN%[OK] Files copied%NC%

REM Install dependencies
echo [INFO] Installing dependencies...
%PYTHON_CMD% -m pip install --upgrade pip >nul
%PYTHON_CMD% -m pip install -r requirements.txt
if errorlevel 1 (
    echo %RED%[ERROR] Failed to install dependencies%NC%
    pause
    exit /b 1
)
echo %GREEN%[OK] Dependencies installed%NC%

REM Create launcher batch file
echo [INFO] Creating launcher...
(
echo @echo off
echo cd /d "%INSTALL_DIR%"
echo %PYTHON_CMD% -m src.gxc_pool_miner %%*
) > "%INSTALL_DIR%\gxc-pool-miner.bat"
echo %GREEN%[OK] Launcher created%NC%

REM Create Start Menu shortcut
echo [INFO] Creating shortcuts...
set "SHORTCUT=%APPDATA%\Microsoft\Windows\Start Menu\Programs\GXC Pool Miner.lnk"

REM Use PowerShell to create shortcut
powershell -Command "$WScriptShell = New-Object -ComObject WScript.Shell; $Shortcut = $WScriptShell.CreateShortcut('%SHORTCUT%'); $Shortcut.TargetPath = '%INSTALL_DIR%\gxc-pool-miner.bat'; $Shortcut.WorkingDirectory = '%INSTALL_DIR%'; $Shortcut.Description = 'GXC Pool Miner'; $Shortcut.Save()"

REM Create Desktop shortcut
set "DESKTOP_SHORTCUT=%USERPROFILE%\Desktop\GXC Pool Miner.lnk"
powershell -Command "$WScriptShell = New-Object -ComObject WScript.Shell; $Shortcut = $WScriptShell.CreateShortcut('%DESKTOP_SHORTCUT%'); $Shortcut.TargetPath = '%INSTALL_DIR%\gxc-pool-miner.bat'; $Shortcut.WorkingDirectory = '%INSTALL_DIR%'; $Shortcut.Description = 'GXC Pool Miner'; $Shortcut.Save()"

echo %GREEN%[OK] Shortcuts created%NC%

REM Add to PATH (optional)
echo.
set /p ADDPATH="Add GXC Pool Miner to PATH? [Y/N]: "
if /i "%ADDPATH%"=="Y" (
    setx PATH "%PATH%;%INSTALL_DIR%" >nul 2>&1
    echo %GREEN%[OK] Added to PATH%NC%
)

echo.
echo %GREEN%=============================================================%NC%
echo %GREEN%         INSTALLATION COMPLETED SUCCESSFULLY!                 %NC%
echo %GREEN%=============================================================%NC%
echo.
echo   Installation directory: %INSTALL_DIR%
echo.
echo   To start mining:
echo     - Use the Desktop shortcut
echo     - Or run: gxc-pool-miner.bat
echo.
echo %CYAN%  Happy Mining! ⛏%NC%
echo.

pause
