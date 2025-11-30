#!/bin/bash
# GXC Testnet Node Deployment Script
# This script builds and deploys the GXC testnet blockchain node

set -e  # Exit on error

echo "=========================================="
echo "GXC Testnet Node Deployment"
echo "=========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found. Please run this script from the project root.${NC}"
    exit 1
fi

# Step 1: Install dependencies
echo -e "${YELLOW}Step 1: Checking dependencies...${NC}"
echo ""

# Check for required tools
command -v cmake >/dev/null 2>&1 || { echo -e "${RED}Error: cmake is required but not installed.${NC}"; exit 1; }
command -v make >/dev/null 2>&1 || { echo -e "${RED}Error: make is required but not installed.${NC}"; exit 1; }
command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || { echo -e "${RED}Error: C++ compiler (g++ or clang++) is required.${NC}"; exit 1; }

echo -e "${GREEN}✓ Dependencies check passed${NC}"
echo ""

# Step 2: Install system dependencies (if needed)
echo -e "${YELLOW}Step 2: Installing system dependencies...${NC}"
echo ""

if command -v apt-get >/dev/null 2>&1; then
    echo "Installing dependencies via apt-get..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libssl-dev libsqlite3-dev pkg-config || true
elif command -v yum >/dev/null 2>&1; then
    echo "Installing dependencies via yum..."
    sudo yum install -y gcc-c++ cmake openssl-devel sqlite-devel || true
elif command -v brew >/dev/null 2>&1; then
    echo "Installing dependencies via brew..."
    brew install cmake openssl sqlite3 || true
fi

echo -e "${GREEN}✓ System dependencies installed${NC}"
echo ""

# Step 3: Build the node
echo -e "${YELLOW}Step 3: Building testnet node...${NC}"
echo ""

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake .. -DCMAKE_BUILD_TYPE=Release || {
    echo -e "${RED}CMake configuration failed. Trying with reduced features...${NC}"
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF -DBUILD_TESTS=OFF || {
        echo -e "${RED}Build configuration failed. Please check errors above.${NC}"
        exit 1
    }
}

# Build the node
echo "Building gxc-node..."
make -j$(nproc 2>/dev/null || echo 4) gxc-node || {
    echo -e "${RED}Build failed. Please check errors above.${NC}"
    exit 1
}

# Check if build succeeded
if [ ! -f "gxc-node" ]; then
    echo -e "${RED}Error: gxc-node executable not found after build.${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful${NC}"
echo ""

# Step 4: Create testnet configuration
echo -e "${YELLOW}Step 4: Creating testnet configuration...${NC}"
echo ""

cd ..

# Create testnet data directory
mkdir -p testnet_data
mkdir -p logs

# Create testnet config file
cat > gxc-testnet.conf << EOF
# GXC Testnet Configuration
network_port=19333
rpc_port=18332
rest_port=18080
data_dir=./testnet_data
testnet=true
log_level=INFO
log_file=logs/testnet.log
max_peers=20
min_peers=8
connection_timeout=30
EOF

echo -e "${GREEN}✓ Configuration created: gxc-testnet.conf${NC}"
echo ""

# Step 5: Create systemd service (optional, for Linux)
if command -v systemctl >/dev/null 2>&1 && [ "$EUID" -eq 0 ]; then
    echo -e "${YELLOW}Step 5: Creating systemd service...${NC}"
    echo ""
    
    cat > /etc/systemd/system/gxc-testnet.service << EOF
[Unit]
Description=GXC Testnet Blockchain Node
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)
ExecStart=$(pwd)/build/gxc-node --testnet --config=$(pwd)/gxc-testnet.conf
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

    systemctl daemon-reload
    echo -e "${GREEN}✓ Systemd service created${NC}"
    echo ""
    echo "To start the service: sudo systemctl start gxc-testnet"
    echo "To enable on boot: sudo systemctl enable gxc-testnet"
    echo ""
fi

# Step 6: Display deployment summary
echo "=========================================="
echo -e "${GREEN}Deployment Complete!${NC}"
echo "=========================================="
echo ""
echo "Node executable: $(pwd)/build/gxc-node"
echo "Configuration: $(pwd)/gxc-testnet.conf"
echo "Data directory: $(pwd)/testnet_data"
echo "Log file: $(pwd)/logs/testnet.log"
echo ""
echo "Testnet Ports:"
echo "  - Network (P2P): 19333"
echo "  - RPC API: 18332"
echo "  - REST API: 18080"
echo ""
echo "To start the testnet node:"
echo "  ./build/gxc-node --testnet --config=gxc-testnet.conf"
echo ""
echo "Or run in background:"
echo "  nohup ./build/gxc-node --testnet --config=gxc-testnet.conf > logs/testnet.log 2>&1 &"
echo ""
echo "To verify the node is running:"
echo "  curl -X POST http://localhost:18332/rpc \\"
echo "    -H 'Content-Type: application/json' \\"
echo "    -d '{\"jsonrpc\":\"2.0\",\"method\":\"getblockchaininfo\",\"params\":[],\"id\":1}'"
echo ""
echo "=========================================="
