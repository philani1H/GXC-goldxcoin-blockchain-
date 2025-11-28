#!/bin/bash
# Oracle Cloud Free Tier - Quick Setup Script for GXC Node
# This script sets up your GXC blockchain node on Oracle Cloud's free tier

set -e

echo "=========================================="
echo "  GXC Node - Oracle Cloud Free Tier Setup"
echo "=========================================="
echo ""
echo "This will set up your GXC blockchain node"
echo "on Oracle Cloud's FREE tier (100% free forever)"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
   echo "⚠️  Please do not run as root. Using sudo when needed."
   exit 1
fi

# Update system
echo "Step 1: Updating system..."
echo "=========================="
sudo apt-get update
sudo apt-get upgrade -y

# Install dependencies
echo ""
echo "Step 2: Installing dependencies..."
echo "=================================="
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libsqlite3-dev \
    pkg-config \
    curl \
    ufw \
    g++ \
    gcc \
    make

echo "✅ Dependencies installed"

# Get current directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo ""
echo "Step 3: Building blockchain node..."
echo "=================================="
cd "$PROJECT_DIR"

if [ ! -d "build" ]; then
    mkdir -p build
fi

cd build

if [ ! -f "CMakeCache.txt" ]; then
    echo "Configuring CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MINING_CLIENT=ON -DBUILD_TESTS=OFF
fi

echo "Building gxc-node..."
cmake --build . --target gxc-node -j$(nproc)

if [ -f "gxc-node" ] || [ -f "Release/gxc-node.exe" ]; then
    echo "✅ Blockchain node built successfully"
else
    echo "❌ Build failed!"
    exit 1
fi

cd "$PROJECT_DIR"

# Create config directory
echo ""
echo "Step 4: Creating configuration..."
echo "=================================="
mkdir -p ~/.gxc
mkdir -p gxc_data

# Create config file
cat > ~/.gxc/gxc.conf << 'EOF'
network_port=9333
rpc_port=8545
rest_port=8080
data_dir=./gxc_data
testnet=false
log_level=INFO
log_file=gxc.log
EOF

echo "✅ Configuration created at ~/.gxc/gxc.conf"

# Setup firewall
echo ""
echo "Step 5: Configuring firewall..."
echo "==============================="
sudo ufw allow 22/tcp comment "SSH"
sudo ufw allow 9333/tcp comment "GXC P2P Network"
sudo ufw allow 8545/tcp comment "GXC RPC API"
sudo ufw allow 8080/tcp comment "GXC REST API"
sudo ufw --force enable

echo "✅ Firewall configured"

# Create systemd service
echo ""
echo "Step 6: Setting up systemd service..."
echo "====================================="

NODE_PATH="$(pwd)/build/gxc-node"
if [ ! -f "$NODE_PATH" ]; then
    NODE_PATH="$(pwd)/build/Release/gxc-node.exe"
fi

sudo tee /etc/systemd/system/gxc-node.service > /dev/null << EOF
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)
ExecStart=$NODE_PATH --rpc-port=8545 --rest-port=8080 --network-port=9333
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable gxc-node

echo "✅ Systemd service created"

# Get public IP
PUBLIC_IP=$(curl -s ifconfig.me || curl -s ipinfo.io/ip || echo "unknown")

# Summary
echo ""
echo "=========================================="
echo "  Setup Complete! ✅"
echo "=========================================="
echo ""
echo "Your GXC node is ready!"
echo ""
echo "📋 Important Information:"
echo "  - Public IP: $PUBLIC_IP"
echo "  - RPC URL: http://$PUBLIC_IP:8545"
echo "  - REST URL: http://$PUBLIC_IP:8080"
echo "  - P2P Port: 9333"
echo ""
echo "⚠️  IMPORTANT: Configure Oracle Cloud Security Lists!"
echo "   1. Go to: Oracle Cloud Console"
echo "   2. Networking > Virtual Cloud Networks"
echo "   3. Your VCN > Security Lists > Default Security List"
echo "   4. Add Ingress Rules for ports: 9333, 8545, 8080"
echo "   5. Source: 0.0.0.0/0"
echo ""
echo "🚀 To start the node:"
echo "   sudo systemctl start gxc-node"
echo ""
echo "📊 To check status:"
echo "   sudo systemctl status gxc-node"
echo ""
echo "📝 To view logs:"
echo "   sudo journalctl -u gxc-node -f"
echo ""
echo "🧪 To test RPC:"
echo "   curl -X POST http://localhost:8545 \\"
echo "     -H 'Content-Type: application/json' \\"
echo "     -d '{\"jsonrpc\":\"2.0\",\"method\":\"getblockchaininfo\",\"params\":[],\"id\":1}'"
echo ""
echo "⛏️  To start mining (from another computer):"
echo "   python mining/gxhash_miner.py --rpc-url http://$PUBLIC_IP:8545"
echo ""
echo "=========================================="
echo "  Happy Mining! ⛏️💰"
echo "=========================================="
