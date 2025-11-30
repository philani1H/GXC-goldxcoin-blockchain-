# Deploy GXC Testnet Node - Complete Guide

## Quick Start

### Option 1: Automated Deployment (Recommended)

```bash
# Run the deployment script
./deploy_testnet_node.sh
```

This script will:
- ✅ Check and install dependencies
- ✅ Build the testnet node
- ✅ Create configuration files
- ✅ Set up data directories
- ✅ Optionally create systemd service

### Option 2: Manual Deployment

#### Step 1: Install Dependencies

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev libsqlite3-dev pkg-config
```

**Linux (RHEL/CentOS):**
```bash
sudo yum install -y gcc-c++ cmake openssl-devel sqlite-devel
```

**macOS:**
```bash
brew install cmake openssl sqlite3
```

#### Step 2: Build the Node

```bash
# Create build directory
mkdir -p build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc) gxc-node

# Verify build
ls -lh gxc-node
```

#### Step 3: Create Testnet Configuration

```bash
cd ..

# Create data directory
mkdir -p testnet_data logs

# Create config file
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
```

#### Step 4: Start the Testnet Node

```bash
# Start in foreground (for testing)
./build/gxc-node --testnet --config=gxc-testnet.conf

# Or start in background
nohup ./build/gxc-node --testnet --config=gxc-testnet.conf > logs/testnet.log 2>&1 &
```

## Configuration

### Testnet Ports

- **Network (P2P)**: 19333
- **RPC API**: 18332
- **REST API**: 18080

### Configuration File: `gxc-testnet.conf`

```ini
# Network Configuration
network_port=19333      # P2P network port
rpc_port=18332          # RPC API port
rest_port=18080         # REST API port

# Data Storage
data_dir=./testnet_data # Blockchain data directory
testnet=true            # Enable testnet mode

# Logging
log_level=INFO          # Log level: DEBUG, INFO, WARNING, ERROR
log_file=logs/testnet.log

# Network Settings
max_peers=20            # Maximum peer connections
min_peers=8             # Minimum peer connections
connection_timeout=30   # Connection timeout in seconds
```

## Verification

### Check if Node is Running

```bash
# Check process
ps aux | grep gxc-node

# Check ports
netstat -tulpn | grep -E '18332|19333|18080'
```

### Test RPC Endpoint

```bash
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

**Expected Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "testnet",
    "blocks": 0,
    "height": 0,
    "block_reward": 50.0,
    "difficulty": 1000.0,
    ...
  },
  "id": 1
}
```

### Test REST Endpoint

```bash
curl http://localhost:18080/api/v1/blockchain/info
```

## Running as a Service

### Linux (systemd)

```bash
# Create service file
sudo nano /etc/systemd/system/gxc-testnet.service
```

**Service file content:**
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

## Connecting Miners

Once the node is running, miners can connect:

### GUI Miner
```bash
python3 mine_testnet_gui.py
# Set RPC URL: http://localhost:18332/rpc
```

### CLI Miner
```bash
python3 mine_testnet.py \
  --address tGXC5066644d2aa70a7bf8947ee25af5e7c4e \
  --rpc-url http://localhost:18332/rpc
```

## Troubleshooting

### Node Won't Start

**Check logs:**
```bash
tail -f logs/testnet.log
```

**Check if ports are in use:**
```bash
netstat -tulpn | grep -E '18332|19333|18080'
```

**Try different ports:**
```bash
./build/gxc-node --testnet \
  --network-port=19334 \
  --rpc-port=18333 \
  --rest-port=18081
```

### Build Errors

**Missing dependencies:**
```bash
# Install missing libraries
sudo apt-get install -y libssl-dev libsqlite3-dev
```

**CMake errors:**
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node
```

### Connection Issues

**Firewall:**
```bash
# Allow testnet ports
sudo ufw allow 19333/tcp
sudo ufw allow 18332/tcp
sudo ufw allow 18080/tcp
```

**Test connectivity:**
```bash
# Test RPC
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'
```

## Monitoring

### View Real-time Logs

```bash
tail -f logs/testnet.log
```

### Check Node Status

```bash
# Get blockchain info
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# Get latest block
curl -X POST http://localhost:18332/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblock","params":["latest",true],"id":1}'
```

### Monitor Resources

```bash
# CPU and memory usage
top -p $(pgrep gxc-node)

# Disk usage
du -sh testnet_data/
```

## Next Steps

After deploying the testnet node:

1. **Start Mining**: Use `mine_testnet_gui.py` or `mine_testnet.py`
2. **Connect Explorer**: Point explorer to `http://localhost:18080`
3. **Connect Wallets**: Update wallet RPC URL to `http://localhost:18332/rpc`
4. **Monitor**: Watch logs and blockchain growth

## Summary

✅ **Deployment Script**: `./deploy_testnet_node.sh`  
✅ **Configuration**: `gxc-testnet.conf`  
✅ **Testnet Ports**: RPC 18332, Network 19333, REST 18080  
✅ **Data Directory**: `./testnet_data`  
✅ **Logs**: `logs/testnet.log`  

**Quick Start Command:**
```bash
./build/gxc-node --testnet --config=gxc-testnet.conf
```

Your testnet node is ready! 🚀
