#!/bin/bash
# GXC Miners - Build Installable Packages
# Creates ready-to-use installers for all platforms

echo "🔨 Building GXC Miner Installers"
echo "======================================"

# Create output directories
mkdir -p miners/installers/windows
mkdir -p miners/installers/linux
mkdir -p miners/installers/macos
mkdir -p miners/build

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo ""
echo "======================================"
echo "Building Miners..."
echo "======================================"

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}❌ CMake not found. Installing...${NC}"
    # Try to install cmake
    if command -v apt-get &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y cmake
    elif command -v brew &> /dev/null; then
        brew install cmake
    else
        echo -e "${RED}Please install CMake manually${NC}"
        exit 1
    fi
fi

cd miners/build

# Configure with CMake
echo -e "${YELLOW}Configuring build...${NC}"
cmake ../.. -DCMAKE_BUILD_TYPE=Release

# Build all miners
echo -e "${YELLOW}Building miners...${NC}"
cmake --build . --config Release

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Miners built successfully${NC}"
else
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

cd ../..

echo ""
echo "======================================"
echo "Creating Installers..."
echo "======================================"

# Function to create Linux AppImage
create_linux_installer() {
    local miner_name=$1
    local display_name=$2
    
    echo -e "${YELLOW}Creating Linux installer for $display_name...${NC}"
    
    # Create AppDir structure
    mkdir -p miners/installers/linux/${miner_name}.AppDir/usr/bin
    mkdir -p miners/installers/linux/${miner_name}.AppDir/usr/share/applications
    mkdir -p miners/installers/linux/${miner_name}.AppDir/usr/share/icons/hicolor/256x256/apps
    
    # Copy binary
    if [ -f "miners/build/${miner_name}" ]; then
        cp miners/build/${miner_name} miners/installers/linux/${miner_name}.AppDir/usr/bin/
        chmod +x miners/installers/linux/${miner_name}.AppDir/usr/bin/${miner_name}
    else
        echo -e "${RED}❌ Binary not found: ${miner_name}${NC}"
        return 1
    fi
    
    # Create desktop file
    cat > miners/installers/linux/${miner_name}.AppDir/usr/share/applications/${miner_name}.desktop << EOF
[Desktop Entry]
Type=Application
Name=${display_name}
Comment=GXC Blockchain Miner
Exec=${miner_name}
Icon=${miner_name}
Categories=Utility;Network;
Terminal=false
EOF
    
    # Create AppRun script
    cat > miners/installers/linux/${miner_name}.AppDir/AppRun << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
cd "${HERE}/usr/bin"
exec "./${miner_name}" "$@"
EOF
    
    # Replace miner_name placeholder
    sed -i "s/\${miner_name}/${miner_name}/g" miners/installers/linux/${miner_name}.AppDir/AppRun
    chmod +x miners/installers/linux/${miner_name}.AppDir/AppRun
    
    # Create simple tar.gz package
    cd miners/installers/linux
    tar -czf ${miner_name}-linux-x64.tar.gz ${miner_name}.AppDir
    cd ../../..
    
    echo -e "${GREEN}✅ Created: miners/installers/linux/${miner_name}-linux-x64.tar.gz${NC}"
}

# Function to create Windows installer script
create_windows_installer() {
    local miner_name=$1
    local display_name=$2
    
    echo -e "${YELLOW}Creating Windows installer for $display_name...${NC}"
    
    # Create installer directory
    mkdir -p miners/installers/windows/${miner_name}
    
    # Copy binary (if exists)
    if [ -f "miners/build/${miner_name}.exe" ]; then
        cp miners/build/${miner_name}.exe miners/installers/windows/${miner_name}/
    fi
    
    # Create installer batch script
    cat > miners/installers/windows/${miner_name}/install.bat << EOF
@echo off
echo ====================================
echo ${display_name} Installer
echo ====================================
echo.

REM Create installation directory
set INSTALL_DIR=%LOCALAPPDATA%\\GXC\\Miners\\${miner_name}
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

REM Copy files
echo Installing ${display_name}...
copy /Y "${miner_name}.exe" "%INSTALL_DIR%\\" >nul
copy /Y "config.json" "%INSTALL_DIR%\\" >nul 2>nul

REM Create desktop shortcut
echo Creating desktop shortcut...
powershell -Command "\$WshShell = New-Object -ComObject WScript.Shell; \$Shortcut = \$WshShell.CreateShortcut('%USERPROFILE%\\Desktop\\${display_name}.lnk'); \$Shortcut.TargetPath = '%INSTALL_DIR%\\${miner_name}.exe'; \$Shortcut.WorkingDirectory = '%INSTALL_DIR%'; \$Shortcut.Save()"

REM Create start menu shortcut
set START_MENU=%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\GXC
if not exist "%START_MENU%" mkdir "%START_MENU%"
powershell -Command "\$WshShell = New-Object -ComObject WScript.Shell; \$Shortcut = \$WshShell.CreateShortcut('%START_MENU%\\${display_name}.lnk'); \$Shortcut.TargetPath = '%INSTALL_DIR%\\${miner_name}.exe'; \$Shortcut.WorkingDirectory = '%INSTALL_DIR%'; \$Shortcut.Save()"

echo.
echo ====================================
echo Installation Complete!
echo ====================================
echo.
echo ${display_name} has been installed to:
echo %INSTALL_DIR%
echo.
echo Shortcuts created on:
echo - Desktop
echo - Start Menu
echo.
echo You can now close this window and launch ${display_name}
echo from your desktop or start menu.
echo.
pause
EOF

    # Create uninstaller
    cat > miners/installers/windows/${miner_name}/uninstall.bat << EOF
@echo off
echo ====================================
echo ${display_name} Uninstaller
echo ====================================
echo.

set INSTALL_DIR=%LOCALAPPDATA%\\GXC\\Miners\\${miner_name}

echo Removing ${display_name}...
if exist "%INSTALL_DIR%" rmdir /S /Q "%INSTALL_DIR%"

echo Removing shortcuts...
del "%USERPROFILE%\\Desktop\\${display_name}.lnk" 2>nul
del "%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\GXC\\${display_name}.lnk" 2>nul

echo.
echo Uninstallation complete!
echo.
pause
EOF

    # Create default config
    cat > miners/installers/windows/${miner_name}/config.json << EOF
{
    "pool_url": "localhost:3333",
    "wallet_address": "YOUR_GXC_ADDRESS_HERE",
    "worker_name": "worker1",
    "threads": 4,
    "algorithm": "auto"
}
EOF

    # Create README
    cat > miners/installers/windows/${miner_name}/README.txt << EOF
${display_name}
====================================

Installation:
1. Double-click install.bat
2. Follow the prompts
3. Launch from desktop or start menu

Configuration:
- Edit config.json to set your wallet address and pool
- Default pool: localhost:3333

Uninstallation:
- Double-click uninstall.bat

Support:
- Forum: http://localhost:3001
- Documentation: See project README

====================================
EOF

    # Create ZIP package
    cd miners/installers/windows
    if command -v zip &> /dev/null; then
        zip -r ${miner_name}-windows-x64.zip ${miner_name}/
        echo -e "${GREEN}✅ Created: miners/installers/windows/${miner_name}-windows-x64.zip${NC}"
    else
        echo -e "${YELLOW}⚠️  zip not found, skipping archive creation${NC}"
    fi
    cd ../../..
}

# Build installers for each miner
echo ""
echo "Creating SHA256 Miner installers..."
create_linux_installer "sha256_miner_gui" "GXC SHA256 Miner"
create_windows_installer "sha256_miner_gui" "GXC SHA256 Miner"

echo ""
echo "Creating Ethash Miner installers..."
create_linux_installer "ethash_miner_gui" "GXC Ethash Miner"
create_windows_installer "ethash_miner_gui" "GXC Ethash Miner"

echo ""
echo "Creating GXHash Miner installers..."
create_linux_installer "gxhash_miner_gui" "GXC GXHash Miner"
create_windows_installer "gxhash_miner_gui" "GXC GXHash Miner"

echo ""
echo "======================================"
echo "✅ All Installers Created!"
echo "======================================"
echo ""
echo "📦 Installers available in:"
echo ""
echo "  LINUX:"
echo "    • miners/installers/linux/sha256_miner_gui-linux-x64.tar.gz"
echo "    • miners/installers/linux/ethash_miner_gui-linux-x64.tar.gz"
echo "    • miners/installers/linux/gxhash_miner_gui-linux-x64.tar.gz"
echo ""
echo "  WINDOWS:"
echo "    • miners/installers/windows/sha256_miner_gui-windows-x64.zip"
echo "    • miners/installers/windows/ethash_miner_gui-windows-x64.zip"
echo "    • miners/installers/windows/gxhash_miner_gui-windows-x64.zip"
echo ""
echo "📖 Installation Instructions:"
echo ""
echo "  LINUX:"
echo "    1. Extract: tar -xzf miner-linux-x64.tar.gz"
echo "    2. Run: ./miner.AppDir/AppRun"
echo ""
echo "  WINDOWS:"
echo "    1. Extract the ZIP file"
echo "    2. Double-click install.bat"
echo "    3. Launch from desktop or start menu"
echo ""
echo "🎉 Miners are ready to distribute!"
echo ""
