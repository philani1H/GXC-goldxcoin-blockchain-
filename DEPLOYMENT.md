# GXC Blockchain - Production Deployment Guide

This guide provides complete instructions for deploying the GXC blockchain in production environments, including node setup, mining configuration, wallet management, and network participation.

## Table of Contents

1. [System Requirements](#system-requirements)
2. [Installation](#installation)
3. [Node Configuration](#node-configuration)
4. [Mining Setup](#mining-setup)
5. [Wallet Management](#wallet-management)
6. [Staking Configuration](#staking-configuration)
7. [Network Participation](#network-participation)
8. [Security Considerations](#security-considerations)
9. [Monitoring and Maintenance](#monitoring-and-maintenance)
10. [Troubleshooting](#troubleshooting)

## System Requirements

### Minimum Requirements
- **OS**: Ubuntu 20.04 LTS, CentOS 8, Windows 10, macOS 10.14+
- **CPU**: 4 cores, 2.0 GHz
- **RAM**: 8 GB
- **Storage**: 100 GB SSD
- **Network**: 10 Mbps upload/download

### Recommended for Production
- **OS**: Ubuntu 22.04 LTS (server)
- **CPU**: 8+ cores, 3.0+ GHz
- **RAM**: 32+ GB
- **Storage**: 1 TB+ NVMe SSD
- **Network**: 100+ Mbps with low latency

### Mining Requirements

#### CPU Mining (GXHash)
- **CPU**: 8+ cores with AVX2 support
- **RAM**: 16+ GB
- **Recommended**: AMD Ryzen 9 or Intel Core i9

#### GPU Mining (Ethash)
- **GPU**: 8+ GB VRAM
- **Recommended**: NVIDIA RTX 3080/4080, AMD RX 6800 XT
- **Multiple GPUs**: Supported

#### ASIC Mining (SHA-256)
- **Hardware**: Bitcoin ASIC miners
- **Recommended**: Antminer S19, WhatsMiner M30S+
- **Power**: Industrial power supply required

## Installation

### From Binary Releases

#### Linux
```bash
# Download latest release
wget https://github.com/gxc-blockchain/gxc-core/releases/download/v2.0.0/gxc-linux-x86_64.tar.gz

# Extract
tar -xzf gxc-linux-x86_64.tar.gz

# Install
sudo mv gxc-* /usr/local/bin/
sudo chmod +x /usr/local/bin/gxc-*

# Create system user
sudo useradd -r -s /bin/false gxc

# Create directories
sudo mkdir -p /var/lib/gxc /var/log/gxc /etc/gxc
sudo chown gxc:gxc /var/lib/gxc /var/log/gxc
```

#### Windows
1. Download `gxc-windows-x86_64.zip`
2. Extract to `C:\Program Files\GXC\`
3. Add to PATH environment variable
4. Run as Administrator for initial setup

#### macOS
```bash
# Using Homebrew
brew tap gxc-blockchain/gxc
brew install gxc-core

# Or download directly
curl -L https://github.com/gxc-blockchain/gxc-core/releases/download/v2.0.0/gxc-macos-x86_64.tar.gz | tar -xz
```

### From Source

#### Prerequisites
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git libssl-dev libsqlite3-dev \
    libboost-all-dev qt6-base-dev qt6-charts-dev libcurl4-openssl-dev \
    pkg-config autoconf libtool

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake openssl-devel sqlite-devel boost-devel qt6-devel curl-devel

# macOS
brew install cmake openssl sqlite boost qt6
```

#### Build Process
```bash
# Clone repository
git clone https://github.com/gxc-blockchain/gxc-core.git
cd gxc-core

# Create build directory
mkdir build && cd build

# Configure build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_GUI=ON \
         -DBUILD_MINING_CLIENT=ON \
         -DBUILD_TESTS=ON

# Build (use all CPU cores)
make -j$(nproc)

# Install
sudo make install
```

## Node Configuration

### Basic Configuration

1. **Copy configuration template**:
```bash
sudo cp config/gxc.conf.example /etc/gxc/gxc.conf
```

2. **Edit configuration**:
```bash
sudo nano /etc/gxc/gxc.conf
```

3. **Essential settings to modify**:
```ini
# Network (choose mainnet or testnet)
network=mainnet

# RPC credentials (CHANGE THESE!)
rpcuser=your_username
rpcpassword=your_secure_password

# Data directory
datadir=/var/lib/gxc

# Enable services you need
server=1        # RPC server
rest=1          # REST API
websocket=1     # WebSocket for real-time updates
wallet=1        # Wallet functionality
```

### Advanced Configuration

#### High-Performance Node
```ini
# Performance optimizations
workerthreads=16
multithreading=1
dbcache=1024
mempoolsize=500
maxmempooltx=10000

# Network optimizations
maxconnections=200
maxoutbound=16
```

#### Mining Node
```ini
# Enable mining
mining=1
miningalgorithm=gxhash
miningthreads=0  # Auto-detect
miningaddress=YOUR_GXC_ADDRESS
solomining=1
```

#### Staking Node
```ini
# Enable staking
staking=1
stakingaddress=YOUR_GXC_ADDRESS
minstake=100
autostake=1
```

### Service Configuration

#### systemd Service (Linux)
```bash
# Create service file
sudo nano /etc/systemd/system/gxc-node.service
```

```ini
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=forking
User=gxc
Group=gxc
ExecStart=/usr/local/bin/gxc-node -daemon -conf=/etc/gxc/gxc.conf
ExecStop=/usr/local/bin/gxc-cli stop
Restart=always
RestartSec=30
TimeoutStopSec=60
KillMode=process

# Security settings
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/lib/gxc /var/log/gxc

[Install]
WantedBy=multi-user.target
```

```bash
# Enable and start service
sudo systemctl enable gxc-node
sudo systemctl start gxc-node
sudo systemctl status gxc-node
```

## Mining Setup

### CPU Mining (GXHash)

#### Configuration
```bash
# Create mining configuration
nano ~/.gxc/mining.conf
```

```ini
# GXHash CPU Mining Configuration
algorithm=gxhash
threads=0  # Auto-detect optimal thread count
address=YOUR_GXC_ADDRESS

# Pool mining (recommended for smaller miners)
pool=stratum+tcp://pool.gxc.network:3333
username=your_worker_name
password=x

# Solo mining (for large operations)
# solo=true
# node=127.0.0.1:8332
```

#### Start Mining
```bash
# Start CPU miner
gxc-gxhash-miner -c ~/.gxc/mining.conf

# Or integrated with node
gxc-node -mining=1 -miningalgorithm=gxhash
```

### GPU Mining (Ethash)

#### NVIDIA Setup
```bash
# Install CUDA toolkit
wget https://developer.download.nvidia.com/compute/cuda/12.0.0/local_installers/cuda_12.0.0_525.60.13_linux.run
sudo sh cuda_12.0.0_525.60.13_linux.run

# Configure GPU miner
nano ~/.gxc/gpu-mining.conf
```

```ini
# Ethash GPU Mining Configuration
algorithm=ethash
devices=0,1,2,3  # GPU device IDs
intensity=85
address=YOUR_GXC_ADDRESS

# Pool configuration
pool=stratum+tcp://gpu-pool.gxc.network:4444
username=your_worker_name.gpu
password=x

# Performance tuning
memory_tweak=1
core_clock=+100
memory_clock=+500
power_limit=250
```

#### AMD Setup
```bash
# Install ROCm
wget https://repo.radeon.com/amdgpu-install/22.40/ubuntu/jammy/amdgpu-install_5.4.50400-1_all.deb
sudo dpkg -i amdgpu-install_5.4.50400-1_all.deb
sudo amdgpu-install --usecase=rocm

# Start GPU mining
gxc-ethash-miner -c ~/.gxc/gpu-mining.conf
```

### ASIC Mining (SHA-256)

#### Configuration
```ini
# ASIC Configuration for GXC SHA-256 mining
algorithm=sha256
pool=stratum+tcp://asic-pool.gxc.network:3333
username=your_worker_name.asic
password=x
address=YOUR_GXC_ADDRESS

# ASIC-specific settings
frequency=650  # MHz
voltage=0.9    # Volts
fan_speed=80   # Percentage
```

#### Antminer Configuration
1. Access web interface: `http://MINER_IP`
2. Navigate to **Miner Configuration**
3. Set pools:
   - **Pool 1**: `stratum+tcp://asic-pool.gxc.network:3333`
   - **Worker**: `your_worker_name.asic`
   - **Password**: `x`

### Mining Pool Setup

#### Pool Operator Configuration
```bash
# Install pool software
git clone https://github.com/gxc-blockchain/gxc-pool.git
cd gxc-pool

# Configure pool
cp config.example.json config.json
nano config.json
```

```json
{
    "coin": "GXC",
    "symbol": "GXC",
    "algorithm": "gxhash",
    
    "daemon": {
        "host": "127.0.0.1",
        "port": 8332,
        "user": "pooluser",
        "password": "poolpass"
    },
    
    "stratum": {
        "enabled": true,
        "port": 3333,
        "difficulty": 32,
        "maxDifficulty": 512,
        "targetTime": 15,
        "retargetTime": 90,
        "varianceDiff": 30
    },
    
    "payouts": {
        "enabled": true,
        "interval": 600,
        "minPayment": 1.0,
        "fee": 1.0,
        "address": "POOL_FEE_ADDRESS"
    }
}
```

## Wallet Management

### Creating Wallets

#### Command Line
```bash
# Create new wallet
gxc-cli createwallet "my_wallet" "secure_password"

# Generate mnemonic for backup
gxc-cli getmnemonic

# Import from mnemonic
gxc-cli importmnemonic "word1 word2 ... word24" "password" "imported_wallet"
```

#### GUI Wallet
1. Launch `gxc-wallet`
2. Click **File → New Wallet**
3. Enter wallet name and secure password
4. **IMPORTANT**: Write down the mnemonic seed phrase
5. Verify mnemonic by re-entering it

### Security Best Practices

#### Wallet Encryption
```bash
# Encrypt existing wallet
gxc-cli encryptwallet "your_secure_passphrase"

# Unlock for transactions (auto-locks after timeout)
gxc-cli walletpassphrase "your_passphrase" 300

# Lock wallet manually
gxc-cli walletlock
```

#### Backup and Recovery
```bash
# Backup wallet
gxc-cli backupwallet "/path/to/backup/wallet_backup_$(date +%Y%m%d).dat"

# Backup private keys
gxc-cli dumpprivkey "your_address"

# Import private key
gxc-cli importprivkey "private_key" "label"
```

#### Hardware Security
- Use hardware wallets for large amounts
- Store backups in secure, offline locations
- Use multi-signature addresses for shared funds
- Regular security audits

## Staking Configuration

### Setting Up Staking

#### Prerequisites
- Minimum 100 GXC balance
- Wallet unlocked for staking
- Stable internet connection
- Node synchronized with network

#### Configuration
```bash
# Enable staking in node configuration
echo "staking=1" >> /etc/gxc/gxc.conf
echo "stakingaddress=YOUR_GXC_ADDRESS" >> /etc/gxc/gxc.conf

# Restart node
sudo systemctl restart gxc-node

# Create stake via CLI
gxc-cli createstake 1000 30  # 1000 GXC for 30 days

# Check staking status
gxc-cli getstakinginfo
```

#### Validator Setup
```bash
# Register as validator (requires significant stake)
gxc-cli registervalidator "validator_name" "description" 5000

# Set validator parameters
gxc-cli setvalidatorinfo "commission_rate" "min_delegation"

# Check validator status
gxc-cli getvalidatorinfo "your_address"
```

## Network Participation

### Full Node Setup

#### Initial Sync
```bash
# Start initial blockchain sync
gxc-node -daemon

# Monitor sync progress
watch gxc-cli getblockchaininfo

# Check connections
gxc-cli getpeerinfo
```

#### Fast Sync (Bootstrap)
```bash
# Download bootstrap file
wget https://bootstrap.gxc.network/gxc-bootstrap-latest.tar.gz

# Extract to data directory
sudo tar -xzf gxc-bootstrap-latest.tar.gz -C /var/lib/gxc/

# Start node
sudo systemctl start gxc-node
```

### Seed Node Operation

#### Configuration for Seed Nodes
```ini
# Seed node specific settings
maxconnections=1000
maxoutbound=50
listen=1
discover=1
dns=1
upnp=0

# Dedicated IP
bind=YOUR_PUBLIC_IP
externalip=YOUR_PUBLIC_IP

# Logging for monitoring
loglevel=info
logtofile=1
```

#### DNS Seeding
```bash
# Configure DNS records for seed discovery
dig TXT _gxc._tcp.seed.gxc.network
```

## Security Considerations

### Network Security

#### Firewall Configuration
```bash
# UFW (Ubuntu)
sudo ufw allow 8333/tcp  # P2P port
sudo ufw allow from YOUR_IP to any port 8332  # RPC (restricted)
sudo ufw enable

# iptables
sudo iptables -A INPUT -p tcp --dport 8333 -j ACCEPT
sudo iptables -A INPUT -p tcp --dport 8332 -s YOUR_IP -j ACCEPT
```

#### SSL/TLS Configuration
```bash
# Generate SSL certificates
openssl req -newkey rsa:4096 -nodes -keyout gxc.key -x509 -days 365 -out gxc.crt

# Configure in gxc.conf
rpcssl=1
rpcsslcertificatechainfile=/etc/ssl/certs/gxc.crt
rpcsslprivatekeyfile=/etc/ssl/private/gxc.key
```

### Operational Security

#### Regular Updates
```bash
# Update system packages
sudo apt update && sudo apt upgrade

# Update GXC node (check releases)
wget https://github.com/gxc-blockchain/gxc-core/releases/latest
```

#### Monitoring
```bash
# Monitor system resources
htop
iotop
nethogs

# Monitor blockchain sync
gxc-cli getblockchaininfo | jq '.blocks,.headers,.verificationprogress'

# Monitor connections
gxc-cli getnetworkinfo | jq '.connections'
```

#### Backup Strategy
```bash
# Daily wallet backup script
#!/bin/bash
DATE=$(date +%Y%m%d)
gxc-cli backupwallet "/backup/gxc/wallet_$DATE.dat"
find /backup/gxc/ -name "wallet_*.dat" -mtime +30 -delete
```

## Monitoring and Maintenance

### System Monitoring

#### Prometheus Metrics
```yaml
# prometheus.yml
global:
  scrape_interval: 15s

scrape_configs:
  - job_name: 'gxc-node'
    static_configs:
      - targets: ['localhost:8332']
    metrics_path: '/metrics'
    basic_auth:
      username: 'prometheus'
      password: 'password'
```

#### Grafana Dashboard
- Import GXC dashboard template
- Monitor node health, sync status, peer count
- Set up alerting for critical issues

### Log Management

#### Log Rotation
```bash
# Configure logrotate
sudo nano /etc/logrotate.d/gxc
```

```
/var/log/gxc/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 644 gxc gxc
    postrotate
        systemctl reload gxc-node
    endscript
}
```

#### Centralized Logging
```bash
# Install filebeat for log shipping
sudo apt install filebeat

# Configure for GXC logs
sudo nano /etc/filebeat/filebeat.yml
```

### Performance Optimization

#### Database Optimization
```bash
# Compact database
gxc-cli stop
gxc-db-compact /var/lib/gxc/blocks
sudo systemctl start gxc-node

# Index optimization
gxc-cli reindex
```

#### Memory Management
```bash
# Adjust system swappiness
echo 'vm.swappiness=10' | sudo tee -a /etc/sysctl.conf

# Increase file limits
echo '* soft nofile 65536' | sudo tee -a /etc/security/limits.conf
echo '* hard nofile 65536' | sudo tee -a /etc/security/limits.conf
```

## Troubleshooting

### Common Issues

#### Sync Problems
```bash
# Check sync status
gxc-cli getblockchaininfo

# Force resync
gxc-cli stop
rm -rf /var/lib/gxc/blocks /var/lib/gxc/chainstate
sudo systemctl start gxc-node

# Verify blockchain
gxc-cli verifychain 3 1000
```

#### Connection Issues
```bash
# Check firewall
sudo ufw status

# Test P2P connectivity
telnet seed1.gxc.network 8333

# Add specific peers
gxc-cli addnode "node.gxc.network:8333" "add"
```

#### RPC Issues
```bash
# Test RPC connection
curl -u user:pass -d '{"method":"getblockchaininfo"}' \
  http://localhost:8332/

# Check RPC logs
tail -f /var/log/gxc/debug.log | grep rpc
```

### Performance Issues

#### High CPU Usage
```bash
# Check mining configuration
gxc-cli getmininginfo

# Adjust thread count
gxc-cli setminingthreads 2

# Monitor processes
top -u gxc
```

#### High Memory Usage
```bash
# Check memory usage
free -h
ps aux --sort=-%mem | head

# Adjust cache settings
echo "dbcache=256" >> /etc/gxc/gxc.conf
sudo systemctl restart gxc-node
```

### Emergency Procedures

#### Node Recovery
```bash
# Create emergency backup
gxc-cli stop
tar -czf /tmp/gxc-emergency-backup.tar.gz /var/lib/gxc/

# Restore from backup
sudo systemctl stop gxc-node
tar -xzf /backup/gxc-backup.tar.gz -C /
sudo systemctl start gxc-node
```

#### Wallet Recovery
```bash
# Recover from mnemonic
gxc-cli importmnemonic "your 24 word phrase" "password" "recovered_wallet"

# Recover from private key
gxc-cli importprivkey "private_key"

# Rescan blockchain for transactions
gxc-cli rescanblockchain
```

## Support and Resources

### Documentation
- **API Reference**: https://docs.gxc.network/api/
- **Mining Guide**: https://docs.gxc.network/mining/
- **Staking Guide**: https://docs.gxc.network/staking/

### Community
- **Discord**: https://discord.gg/gxc
- **Telegram**: https://t.me/gxc_official
- **Forum**: https://forum.gxc.network/

### Support
- **GitHub Issues**: https://github.com/gxc-blockchain/gxc-core/issues
- **Email**: support@gxc.network
- **Status Page**: https://status.gxc.network/

### Emergency Contacts
- **Security Issues**: security@gxc.network
- **Critical Bugs**: critical@gxc.network
- **24/7 Support**: +1-XXX-XXX-XXXX (for enterprise customers)

---

**Note**: This deployment guide is for production use. Always test configurations in a development environment first. Keep your software updated and follow security best practices.