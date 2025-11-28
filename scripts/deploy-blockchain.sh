#!/bin/bash
# Deploy GXC Blockchain Node
# Quick deployment script for local or server deployment

set -e

echo "========================================"
echo "  GXC Blockchain Deployment"
echo "========================================"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
   echo "⚠️  Please do not run as root. Using sudo when needed."
fi

# Install dependencies
echo "Step 1: Installing dependencies..."
echo "=================================="
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libsqlite3-dev \
    pkg-config \
    curl \
    g++ \
    gcc \
    make

echo "✅ Dependencies installed"
echo ""

# Build blockchain
echo "Step 2: Building blockchain..."
echo "=============================="
cd "$(dirname "$0")/.."

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

cd ..

# Create config directory
echo ""
echo "Step 3: Creating configuration..."
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
echo ""

# Create systemd service (if systemd available)
if command -v systemctl &> /dev/null && [ -d "/etc/systemd/system" ]; then
    echo "Step 4: Setting up systemd service..."
    echo "======================================"
    
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
    echo ""
    echo "To start the node:"
    echo "  sudo systemctl start gxc-node"
    echo ""
    echo "To check status:"
    echo "  sudo systemctl status gxc-node"
    echo ""
    echo "To view logs:"
    echo "  sudo journalctl -u gxc-node -f"
    echo ""
else
    echo "Step 4: Systemd not available, creating startup script..."
    echo "========================================================"
    
    cat > start-gxc-node.sh << 'EOF'
#!/bin/bash
# Start GXC Blockchain Node

cd "$(dirname "$0")"

if [ -f "build/gxc-node" ]; then
    ./build/gxc-node --rpc-port=8545 --rest-port=8080 --network-port=9333
elif [ -f "build/Release/gxc-node.exe" ]; then
    ./build/Release/gxc-node.exe --rpc-port=8545 --rest-port=8080 --network-port=9333
else
    echo "❌ gxc-node not found! Build it first."
    exit 1
fi
EOF

    chmod +x start-gxc-node.sh
    echo "✅ Startup script created: start-gxc-node.sh"
    echo ""
    echo "To start the node:"
    echo "  ./start-gxc-node.sh"
    echo ""
fi

# Configure firewall (if ufw available)
if command -v ufw &> /dev/null; then
    echo "Step 5: Configuring firewall..."
    echo "==============================="
    sudo ufw allow 9333/tcp comment "GXC P2P Network"
    sudo ufw allow 8545/tcp comment "GXC RPC API"
    sudo ufw allow 8080/tcp comment "GXC REST API"
    echo "✅ Firewall configured"
    echo ""
fi

# Summary
echo "========================================"
echo "  Deployment Complete!"
echo "========================================"
echo ""
echo "Blockchain node is ready!"
echo ""
echo "Access points:"
echo "  - RPC API: http://localhost:8545"
echo "  - REST API: http://localhost:8080"
echo "  - P2P Network: localhost:9333"
echo ""
echo "Test RPC:"
echo "  curl -X POST http://localhost:8545 \\"
echo "    -H 'Content-Type: application/json' \\"
echo "    -d '{\"jsonrpc\":\"2.0\",\"method\":\"getblockchaininfo\",\"params\":[],\"id\":1}'"
echo ""
echo "Test REST:"
echo "  curl http://localhost:8080/api/v1/blockchain/info"
echo ""
echo "Next steps:"
echo "  1. Start the blockchain node"
echo "  2. Download and install miners from packages/ directory"
echo "  3. Start mining!"
echo ""
