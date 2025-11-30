# Quick Testnet Node Deployment Guide

## Option 1: Automated Deployment (Easiest)

```bash
# Run the deployment script
./deploy_testnet_node.sh
```

This will:
- ✅ Install dependencies
- ✅ Build the node
- ✅ Create configuration
- ✅ Set up directories

## Option 2: Manual Step-by-Step

### Step 1: Install Dependencies

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev libsqlite3-dev
```

**macOS:**
```bash
brew install cmake openssl sqlite3
```

### Step 2: Build the Node

```bash
# Create build directory
mkdir -p build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build (use all CPU cores)
make -j$(nproc) gxc-node

# Go back to project root
cd ..
```

### Step 3: Create Testnet Configuration

```bash
# Create directories
mkdir -p testnet_data logs

# Create config file
cat > gxc-testnet.conf << 'EOF'
network_port=19333
rpc_port=18332
rest_port=18080
data_dir=./testnet_data
testnet=true
log_level=INFO
log_file=logs/testnet.log
max_peers=20
min_peers=8
EOF
```

### Step 4: Start the Testnet Node

**Foreground (for testing):**
```bash
./build/gxc-node --testnet --config=gxc-testnet.conf
```

**Background (for production):**
```bash
nohup ./build/gxc-node --testnet --config=gxc-testnet.conf > logs/testnet.log 2>&1 &
```

**Or with explicit ports:**
```bash
./build/gxc-node \
  --testnet \
  --network-port=19333 \
  --rpc-port=18332 \
  --rest-port=18080 \
  --data-dir=./testnet_data
```

## Step 5: Verify It's Running

```bash
# Check if process is running
ps aux | grep gxc-node

# Check if ports are listening
netstat -tuln | grep -E "18332|19333|18080"

# Test RPC endpoint
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

**Expected response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "testnet",
    "blocks": 0,
    "height": 0,
    "block_reward": 50.0,
    ...
  }
}
```

## Step 6: Start Mining

Once the node is running, start mining:

```bash
# CLI Miner
python3 mine_testnet.py --address tGXC5066644d2aa70a7bf8947ee25af5e7c4e

# Or GUI Miner (if tkinter available)
python3 mine_testnet_gui.py
```

## Running as a Service (Linux)

### Create systemd Service

```bash
sudo nano /etc/systemd/system/gxc-testnet.service
```

**Add this content:**
```ini
[Unit]
Description=GXC Testnet Blockchain Node
After=network.target

[Service]
Type=simple
User=your-username
WorkingDirectory=/path/to/gxc-blockchain
ExecStart=/path/to/gxc-blockchain/build/gxc-node --testnet --config=/path/to/gxc-blockchain/gxc-testnet.conf
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

**Enable and start:**
```bash
sudo systemctl daemon-reload
sudo systemctl enable gxc-testnet
sudo systemctl start gxc-testnet
sudo systemctl status gxc-testnet
```

**View logs:**
```bash
sudo journalctl -u gxc-testnet -f
```

## Configuration Summary

### Testnet Ports
- **Network (P2P)**: 19333
- **RPC API**: 18332  
- **REST API**: 18080

### Files Created
- `gxc-testnet.conf` - Configuration file
- `testnet_data/` - Blockchain data directory
- `logs/testnet.log` - Log file

## Troubleshooting

### Build Fails
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node
```

### Ports Already in Use
```bash
# Check what's using the ports
sudo lsof -i :18332
sudo lsof -i :19333

# Use different ports
./build/gxc-node --testnet --rpc-port=18333 --network-port=19334
```

### Node Won't Start
```bash
# Check logs
tail -f logs/testnet.log

# Check permissions
chmod +x build/gxc-node
```

## Quick Commands Reference

```bash
# Start node
./build/gxc-node --testnet --config=gxc-testnet.conf

# Stop node (if running in foreground)
Ctrl+C

# Stop node (if running in background)
pkill gxc-node

# Check status
ps aux | grep gxc-node

# View logs
tail -f logs/testnet.log

# Test RPC
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

## Summary

**Quickest way:**
```bash
./deploy_testnet_node.sh
```

**Manual way:**
1. Install dependencies
2. Build: `cd build && cmake .. && make gxc-node`
3. Create config: `gxc-testnet.conf`
4. Start: `./build/gxc-node --testnet --config=gxc-testnet.conf`

Your testnet node will be running on:
- RPC: `http://localhost:18332/rpc`
- REST: `http://localhost:18080`
- Network: Port `19333`
