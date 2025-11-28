# GXC Blockchain Deployment Guide

## Complete Guide to Deploying the GXC Blockchain Node

This comprehensive guide covers building, configuring, and deploying the GXC blockchain node for production use.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Building the Blockchain](#building-the-blockchain)
3. [Configuration](#configuration)
4. [Running the Node](#running-the-node)
5. [Deployment Options](#deployment-options)
6. [Production Deployment](#production-deployment)
7. [Monitoring & Maintenance](#monitoring--maintenance)
8. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### System Requirements

- **OS**: Linux (Ubuntu 20.04+), Windows 10+, or macOS 10.15+
- **CPU**: 2+ cores recommended
- **RAM**: 4GB minimum, 8GB+ recommended
- **Storage**: 50GB+ free space
- **Network**: Stable internet connection

### Required Software

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libsqlite3-dev \
    pkg-config \
    curl
```

#### Windows
- **Visual Studio 2019/2022** (Community Edition)
- **CMake** 3.16+
- **vcpkg** (for package management)
- **Git for Windows**

#### macOS
```bash
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake openssl boost sqlite3
```

---

## Building the Blockchain

### Step 1: Clone Repository

```bash
git clone https://github.com/your-repo/gxc-blockchain.git
cd gxc-blockchain
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

```bash
# Release build (recommended for production)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Or Debug build (for development)
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Step 4: Compile

```bash
# Linux/macOS - parallel build
make -j$(nproc)

# Windows
cmake --build . --config Release -j

# Or single-threaded
cmake --build . --config Release
```

### Step 5: Verify Build

```bash
# Check if executable was created
ls -lh gxc-node  # Linux/macOS
dir gxc-node.exe  # Windows

# Should see: gxc-node (executable file)
```

---

## Configuration

### Configuration File

Create a configuration file at `~/.gxc/gxc.conf` or `config/gxc.conf`:

```ini
# Network Configuration
network_port=9333
rpc_port=8545
rest_port=8080
max_peers=20
min_peers=8

# Blockchain Settings
data_dir=./gxc_data
testnet=false
difficulty_adjustment_interval=10
target_block_time=2
max_block_size=1048576
block_reward=50.0

# Mining Settings
enable_mining=false
mining_threads=1
mining_algorithm=GXHash
pool_address=
miner_address=

# Wallet Settings
wallet_file=wallet.dat
wallet_backup_interval=3600
transaction_fee=0.001
min_confirmations=6

# Logging
log_level=INFO
log_file=gxc.log
log_max_size=10485760
log_max_files=5
log_to_console=true

# Security
enable_ssl=false
ssl_cert_file=
ssl_key_file=
max_connections=100
ban_threshold=10
ban_duration=86400

# Performance
cache_size=268435456
mempool_size=50000000
checkpoint_interval=1000
vacuum_interval=604800
```

### Environment Variables

```bash
export GXC_DATA_DIR=/var/lib/gxc
export GXC_RPC_PORT=8545
export GXC_REST_PORT=8080
export GXC_NETWORK_PORT=9333
export GXC_LOG_LEVEL=INFO
```

---

## Running the Node

### Local Development

```bash
# From build directory
./gxc-node

# Or with custom config
./gxc-node --config=/path/to/gxc.conf

# Or with command-line arguments
./gxc-node --rpc-port=8545 --rest-port=8080 --data-dir=./gxc_data
```

### As a Service (Linux)

#### Systemd Service

Create `/etc/systemd/system/gxc-node.service`:

```ini
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=simple
User=gxc
Group=gxc
WorkingDirectory=/opt/gxc
ExecStart=/opt/gxc/build/gxc-node --config=/etc/gxc/gxc.conf
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable gxc-node
sudo systemctl start gxc-node
sudo systemctl status gxc-node
```

#### View Logs

```bash
sudo journalctl -u gxc-node -f
```

---

## Deployment Options

### Option 1: VPS/Dedicated Server

#### Recommended Providers
- **DigitalOcean**: $12/month (2GB RAM)
- **Linode**: $12/month (2GB RAM)
- **AWS EC2**: t3.small ($15/month)
- **Hetzner**: €4.15/month (4GB RAM)

#### Setup Steps

1. **Create VPS Instance**
   ```bash
   # Ubuntu 22.04 LTS recommended
   ```

2. **SSH into Server**
   ```bash
   ssh root@your-server-ip
   ```

3. **Install Dependencies**
   ```bash
   sudo apt-get update
   sudo apt-get install -y build-essential cmake git libssl-dev libboost-all-dev libsqlite3-dev
   ```

4. **Clone and Build**
   ```bash
   git clone https://github.com/your-repo/gxc-blockchain.git
   cd gxc-blockchain
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

5. **Configure Firewall**
   ```bash
   sudo ufw allow 9333/tcp  # P2P port
   sudo ufw allow 8545/tcp  # RPC port
   sudo ufw allow 8080/tcp  # REST port
   sudo ufw enable
   ```

6. **Run as Service**
   ```bash
   # Create systemd service (see above)
   sudo systemctl start gxc-node
   ```

### Option 2: Docker Deployment

#### Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

EXPOSE 9333 8545 8080

CMD ["./build/gxc-node"]
```

#### Build and Run

```bash
# Build image
docker build -t gxc-node .

# Run container
docker run -d \
  --name gxc-node \
  -p 9333:9333 \
  -p 8545:8545 \
  -p 8080:8080 \
  -v gxc-data:/app/gxc_data \
  gxc-node
```

### Option 3: Cloud Deployment

#### AWS EC2

1. **Launch EC2 Instance**
   - AMI: Ubuntu 22.04 LTS
   - Instance Type: t3.small or larger
   - Security Group: Open ports 9333, 8545, 8080

2. **Connect and Setup**
   ```bash
   ssh -i your-key.pem ubuntu@ec2-instance-ip
   # Follow VPS setup steps above
   ```

#### Google Cloud Platform

1. **Create VM Instance**
   ```bash
   gcloud compute instances create gxc-node \
     --image-family=ubuntu-2204-lts \
     --image-project=ubuntu-os-cloud \
     --machine-type=e2-small \
     --tags=gxc-node
   ```

2. **Configure Firewall**
   ```bash
   gcloud compute firewall-rules create allow-gxc-node \
     --allow tcp:9333,tcp:8545,tcp:8080 \
     --target-tags=gxc-node
   ```

---

## Production Deployment

### Security Checklist

- [ ] Firewall configured (only necessary ports open)
- [ ] SSL/TLS enabled for RPC/REST (if exposed)
- [ ] Strong RPC credentials set
- [ ] Regular backups configured
- [ ] Monitoring and alerting set up
- [ ] Log rotation configured
- [ ] System updates automated

### RPC Security

**Never expose RPC without authentication!**

```ini
# In gxc.conf
rpc_user=your_username
rpc_password=strong_random_password
rpc_whitelist=127.0.0.1,10.0.0.0/8  # Only allow local/private network
```

### Backup Strategy

```bash
# Create backup script
#!/bin/bash
BACKUP_DIR="/backups/gxc"
DATA_DIR="/var/lib/gxc/gxc_data"
DATE=$(date +%Y%m%d_%H%M%S)

mkdir -p $BACKUP_DIR
tar -czf $BACKUP_DIR/gxc_backup_$DATE.tar.gz $DATA_DIR

# Keep only last 7 days
find $BACKUP_DIR -name "gxc_backup_*.tar.gz" -mtime +7 -delete
```

Add to crontab:
```bash
0 2 * * * /path/to/backup-script.sh
```

### Monitoring

#### Health Check Endpoint

The node exposes a health endpoint:
```bash
curl http://localhost:8080/api/v1/health
```

#### Prometheus Metrics (if implemented)

```yaml
# prometheus.yml
scrape_configs:
  - job_name: 'gxc-node'
    static_configs:
      - targets: ['localhost:9090']
```

---

## Running Commands

### Start Node

```bash
# Direct
./build/gxc-node

# With config
./build/gxc-node --config=/etc/gxc/gxc.conf

# As service
sudo systemctl start gxc-node
```

### Stop Node

```bash
# Graceful shutdown
pkill -SIGTERM gxc-node

# Or via systemd
sudo systemctl stop gxc-node
```

### Check Status

```bash
# Check if running
ps aux | grep gxc-node

# Check logs
tail -f /var/log/gxc/gxc.log

# Or systemd
sudo systemctl status gxc-node
```

### RPC Commands

```bash
# Get blockchain info
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# Get block count
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'

# Get balance
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["GXC..."],"id":1}'
```

---

## Monitoring & Maintenance

### Log Files

```bash
# View logs
tail -f /var/log/gxc/gxc.log

# Search logs
grep "ERROR" /var/log/gxc/gxc.log

# Log rotation (logrotate)
/var/log/gxc/*.log {
    daily
    rotate 7
    compress
    missingok
    notifempty
}
```

### Performance Monitoring

```bash
# CPU usage
top -p $(pgrep gxc-node)

# Memory usage
ps aux | grep gxc-node

# Disk usage
du -sh /var/lib/gxc/gxc_data
```

### Database Maintenance

```bash
# Vacuum SQLite database (if using SQLite)
sqlite3 /var/lib/gxc/gxc_data/blockchain.db "VACUUM;"

# Check database size
sqlite3 /var/lib/gxc/gxc_data/blockchain.db "SELECT page_count * page_size as size FROM pragma_page_count(), pragma_page_size();"
```

---

## Troubleshooting

### Node Won't Start

1. **Check logs**
   ```bash
   tail -100 /var/log/gxc/gxc.log
   ```

2. **Verify ports not in use**
   ```bash
   netstat -tulpn | grep -E '9333|8545|8080'
   ```

3. **Check permissions**
   ```bash
   ls -la /var/lib/gxc
   ```

### Connection Issues

1. **Check firewall**
   ```bash
   sudo ufw status
   ```

2. **Test connectivity**
   ```bash
   telnet localhost 8545
   ```

3. **Check network config**
   ```bash
   ip addr show
   ```

### High Memory Usage

1. **Reduce cache size** in config
2. **Restart node** periodically
3. **Monitor for memory leaks**

### Database Errors

1. **Check disk space**
   ```bash
   df -h
   ```

2. **Repair database** (if SQLite)
   ```bash
   sqlite3 blockchain.db ".recover" | sqlite3 blockchain_recovered.db
   ```

---

## Quick Start Script

```bash
#!/bin/bash
# quick-deploy.sh

echo "GXC Blockchain Node - Quick Deploy"
echo "=================================="

# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake git libssl-dev libboost-all-dev libsqlite3-dev

# Clone and build
git clone https://github.com/your-repo/gxc-blockchain.git
cd gxc-blockchain
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Create data directory
mkdir -p ~/.gxc
cp ../config/gxc.conf.example ~/.gxc/gxc.conf

# Configure firewall
sudo ufw allow 9333/tcp
sudo ufw allow 8545/tcp
sudo ufw allow 8080/tcp

echo "Build complete! Run: ./build/gxc-node"
```

---

## Production Checklist

- [ ] Node compiled successfully
- [ ] Configuration file created and tested
- [ ] Firewall rules configured
- [ ] Systemd service created (Linux)
- [ ] Backups configured
- [ ] Monitoring set up
- [ ] Log rotation configured
- [ ] RPC authentication enabled
- [ ] SSL/TLS configured (if needed)
- [ ] Health checks working
- [ ] Documentation reviewed

---

## Support

For issues or questions:
- Check logs: `/var/log/gxc/gxc.log`
- Review configuration: `~/.gxc/gxc.conf`
- Check system resources: `htop`, `df -h`
- Review network: `netstat -tulpn`

---

**Version**: 1.0.0  
**Last Updated**: 2024

