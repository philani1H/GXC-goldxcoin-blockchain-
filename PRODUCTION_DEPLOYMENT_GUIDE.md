# 🚀 GXC Blockchain - Production Deployment Guide

## Overview

This guide provides step-by-step instructions for deploying GXC blockchain nodes in production environments with enterprise-grade security and reliability.

**Target Audience:** DevOps Engineers, System Administrators  
**Estimated Time:** 2-4 hours  
**Difficulty:** Intermediate to Advanced

---

## Prerequisites

### Hardware Requirements

**Minimum Specifications:**
- CPU: 4 cores @ 2.5GHz
- RAM: 8GB
- Disk: 500GB SSD
- Network: 100Mbps

**Recommended Specifications:**
- CPU: 8 cores @ 3.0GHz+
- RAM: 16GB
- Disk: 1TB NVMe SSD
- Network: 1Gbps

### Software Requirements

- **Operating System:** Ubuntu 22.04 LTS (recommended) or Debian 12
- **Root Access:** sudo privileges required
- **Internet Connection:** Required for initial setup

### Network Requirements

- **Public IP Address:** Required for P2P connectivity
- **Open Ports:**
  - 8333/tcp - P2P network (must be open)
  - 8332/tcp - RPC (localhost only, use SSH tunnel)
  - 22/tcp - SSH (for management)

---

## Quick Start (Automated Deployment)

For automated deployment with all security hardening:

```bash
# 1. Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# 2. Build binaries
mkdir -p build && cd build
cmake ..
make -j$(nproc)
cd ..

# 3. Run automated deployment
sudo ./scripts/production-deploy.sh

# 4. Apply security hardening
sudo ./scripts/security-hardening.sh

# 5. Verify deployment
systemctl status gxc-node
gxc-cli -conf=/etc/gxc/gxc.conf getblockchaininfo
```

**Done!** Your production node is now running.

---

## Manual Deployment (Step-by-Step)

### Step 1: System Preparation

```bash
# Update system
sudo apt-get update
sudo apt-get upgrade -y

# Install dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libleveldb-dev \
    libboost-all-dev \
    libsqlite3-dev \
    pkg-config \
    curl \
    wget \
    ufw \
    fail2ban
```

### Step 2: Create GXC User

```bash
# Create dedicated user (security best practice)
sudo useradd -r -s /bin/false -d /var/lib/gxc gxc

# Create directories
sudo mkdir -p /var/lib/gxc
sudo mkdir -p /var/log/gxc
sudo mkdir -p /etc/gxc
sudo mkdir -p /var/lib/gxc/backups

# Set ownership
sudo chown -R gxc:gxc /var/lib/gxc
sudo chown -R gxc:gxc /var/log/gxc
sudo chown -R gxc:gxc /etc/gxc

# Set permissions
sudo chmod 700 /var/lib/gxc
sudo chmod 700 /etc/gxc
sudo chmod 755 /var/log/gxc
```

### Step 3: Build and Install Binaries

```bash
# Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Build
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Install binaries
sudo cp gxc-node /usr/local/bin/
sudo cp gxc-cli /usr/local/bin/
sudo cp gxc-miner /usr/local/bin/

# Set permissions
sudo chmod 755 /usr/local/bin/gxc-node
sudo chmod 755 /usr/local/bin/gxc-cli
sudo chmod 755 /usr/local/bin/gxc-miner
```

### Step 4: Configure Node

```bash
# Copy production configuration
sudo cp config/gxc-production.conf /etc/gxc/gxc.conf

# Generate secure RPC credentials
RPC_USER="gxcuser_$(openssl rand -hex 8)"
RPC_PASS=$(openssl rand -base64 32)

# Update configuration
sudo sed -i "s/CHANGE_ME_UNIQUE_USERNAME/$RPC_USER/" /etc/gxc/gxc.conf
sudo sed -i "s/CHANGE_ME_STRONG_PASSWORD_MIN_32_CHARS_WITH_SPECIAL_CHARS/$RPC_PASS/" /etc/gxc/gxc.conf

# Set permissions
sudo chmod 600 /etc/gxc/gxc.conf
sudo chown gxc:gxc /etc/gxc/gxc.conf

# Save credentials securely
echo "RPC Username: $RPC_USER" | sudo tee /root/gxc-credentials.txt
echo "RPC Password: $RPC_PASS" | sudo tee -a /root/gxc-credentials.txt
sudo chmod 600 /root/gxc-credentials.txt
```

### Step 5: Configure Systemd Service

```bash
# Create systemd service file
sudo tee /etc/systemd/system/gxc-node.service > /dev/null <<EOF
[Unit]
Description=GXC Blockchain Node
Documentation=https://github.com/philani1H/GXC-goldxcoin-blockchain-
After=network.target

[Service]
Type=forking
User=gxc
Group=gxc
ExecStart=/usr/local/bin/gxc-node -daemon -conf=/etc/gxc/gxc.conf -datadir=/var/lib/gxc
ExecStop=/usr/local/bin/gxc-cli -conf=/etc/gxc/gxc.conf stop
PIDFile=/var/run/gxc/gxc.pid
Restart=on-failure
RestartSec=10
TimeoutStopSec=30
LimitNOFILE=65536

# Security hardening
PrivateTmp=true
ProtectSystem=full
NoNewPrivileges=true
ProtectHome=true

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd
sudo systemctl daemon-reload

# Enable auto-start
sudo systemctl enable gxc-node
```

### Step 6: Configure Firewall

```bash
# Enable UFW
sudo ufw --force enable

# Default policies
sudo ufw default deny incoming
sudo ufw default allow outgoing

# Allow SSH
sudo ufw allow 22/tcp

# Allow GXC P2P
sudo ufw allow 8333/tcp comment 'GXC P2P'

# Deny RPC (use SSH tunnel)
sudo ufw deny 8332/tcp comment 'GXC RPC'

# Rate limit SSH
sudo ufw limit 22/tcp

# Verify rules
sudo ufw status verbose
```

### Step 7: Configure Fail2Ban

```bash
# Create GXC RPC filter
sudo tee /etc/fail2ban/filter.d/gxc-rpc.conf > /dev/null <<EOF
[Definition]
failregex = ^.*RPC.*authentication failed.*from <HOST>.*$
ignoreregex =
EOF

# Create GXC jail
sudo tee /etc/fail2ban/jail.d/gxc.conf > /dev/null <<EOF
[gxc-rpc]
enabled = true
port = 8332
filter = gxc-rpc
logpath = /var/log/gxc/gxc.log
maxretry = 5
bantime = 3600
findtime = 600
EOF

# Restart Fail2Ban
sudo systemctl restart fail2ban
sudo systemctl enable fail2ban
```

### Step 8: Configure Log Rotation

```bash
# Create logrotate configuration
sudo tee /etc/logrotate.d/gxc > /dev/null <<EOF
/var/log/gxc/*.log {
    daily
    rotate 10
    compress
    delaycompress
    missingok
    notifempty
    create 0640 gxc gxc
    sharedscripts
    postrotate
        systemctl reload gxc-node > /dev/null 2>&1 || true
    endscript
}
EOF
```

### Step 9: Configure Automatic Backups

```bash
# Create backup script
sudo tee /usr/local/bin/gxc-backup.sh > /dev/null <<'EOF'
#!/bin/bash
BACKUP_DIR="/var/lib/gxc/backups"
DATE=$(date +%Y%m%d-%H%M%S)

# Backup wallet
/usr/local/bin/gxc-cli -conf=/etc/gxc/gxc.conf backupwallet $BACKUP_DIR/wallet-$DATE.dat

# Cleanup old backups (keep last 30 days)
find $BACKUP_DIR -name "wallet-*.dat" -mtime +30 -delete
EOF

sudo chmod 755 /usr/local/bin/gxc-backup.sh

# Add to cron (every 6 hours)
echo "0 */6 * * * gxc /usr/local/bin/gxc-backup.sh" | sudo tee /etc/cron.d/gxc-backup
sudo chmod 644 /etc/cron.d/gxc-backup
```

### Step 10: Start Node

```bash
# Start the node
sudo systemctl start gxc-node

# Check status
sudo systemctl status gxc-node

# View logs
sudo tail -f /var/log/gxc/gxc.log

# Check blockchain info
gxc-cli -conf=/etc/gxc/gxc.conf getblockchaininfo
```

---

## Post-Deployment Verification

### 1. Check Node Status

```bash
# Service status
systemctl status gxc-node

# Should show: active (running)
```

### 2. Check Blockchain Sync

```bash
# Get blockchain info
gxc-cli -conf=/etc/gxc/gxc.conf getblockchaininfo

# Check:
# - "blocks" should be increasing
# - "verificationprogress" should approach 1.0
```

### 3. Check Peer Connectivity

```bash
# Get peer info
gxc-cli -conf=/etc/gxc/gxc.conf getpeerinfo

# Should have at least 3-8 peers
```

### 4. Check Logs

```bash
# View recent logs
tail -100 /var/log/gxc/gxc.log

# Check for errors
grep ERROR /var/log/gxc/gxc.log
```

### 5. Check Firewall

```bash
# Verify firewall rules
sudo ufw status verbose

# Check open ports
sudo ss -tulpn | grep gxc
```

---

## Monitoring Setup

### Health Check Script

```bash
# Create health check script
sudo tee /usr/local/bin/gxc-healthcheck.sh > /dev/null <<'EOF'
#!/bin/bash
GXC_CLI="/usr/local/bin/gxc-cli"
CONFIG="/etc/gxc/gxc.conf"
LOG="/var/log/gxc/healthcheck.log"

# Check if node is responding
if ! $GXC_CLI -conf=$CONFIG getblockchaininfo > /dev/null 2>&1; then
    echo "$(date): Node not responding - restarting" >> $LOG
    systemctl restart gxc-node
fi

# Check peer count
PEERS=$($GXC_CLI -conf=$CONFIG getpeerinfo | jq length 2>/dev/null || echo 0)
if [ "$PEERS" -lt 3 ]; then
    echo "$(date): Low peer count: $PEERS" >> $LOG
fi

# Check disk space
USAGE=$(df -h /var/lib/gxc | awk 'NR==2 {print $5}' | sed 's/%//')
if [ "$USAGE" -gt 80 ]; then
    echo "$(date): Disk usage high: ${USAGE}%" >> $LOG
fi
EOF

sudo chmod 755 /usr/local/bin/gxc-healthcheck.sh

# Add to cron (every 5 minutes)
echo "*/5 * * * * root /usr/local/bin/gxc-healthcheck.sh" | sudo tee /etc/cron.d/gxc-healthcheck
```

### Monitoring Metrics

Monitor these key metrics:

- **Block Height:** Should increase every ~2 minutes
- **Peer Count:** Should be 3-8 peers
- **Memory Usage:** Should be < 80% of available RAM
- **Disk Usage:** Should be < 80% of available space
- **CPU Usage:** Should be < 70% average
- **Network Traffic:** Should show consistent P2P activity

---

## Backup and Recovery

### Manual Backup

```bash
# Backup wallet
gxc-cli -conf=/etc/gxc/gxc.conf backupwallet /secure/location/wallet-backup.dat

# Backup blockchain data
sudo tar -czf /backup/gxc-blockchain-$(date +%Y%m%d).tar.gz /var/lib/gxc/blockchain_mainnet.db

# Backup configuration
sudo cp /etc/gxc/gxc.conf /backup/gxc.conf.backup
```

### Restore from Backup

```bash
# Stop node
sudo systemctl stop gxc-node

# Restore wallet
sudo cp /backup/wallet-backup.dat /var/lib/gxc/wallet.dat
sudo chown gxc:gxc /var/lib/gxc/wallet.dat
sudo chmod 600 /var/lib/gxc/wallet.dat

# Restore blockchain (or resync)
sudo tar -xzf /backup/gxc-blockchain-*.tar.gz -C /

# Start node
sudo systemctl start gxc-node
```

---

## Troubleshooting

### Node Won't Start

```bash
# Check logs
sudo journalctl -u gxc-node -n 100

# Check configuration
sudo gxc-node -conf=/etc/gxc/gxc.conf -checkconfig

# Check permissions
ls -la /var/lib/gxc
ls -la /etc/gxc/gxc.conf
```

### No Peers Connecting

```bash
# Check firewall
sudo ufw status
sudo ss -tulpn | grep 8333

# Check network connectivity
ping -c 3 seed1.gxc.network

# Manually add peers
gxc-cli -conf=/etc/gxc/gxc.conf addnode "seed1.gxc.network:8333" "add"
```

### Blockchain Not Syncing

```bash
# Check sync status
gxc-cli -conf=/etc/gxc/gxc.conf getblockchaininfo

# Check peers
gxc-cli -conf=/etc/gxc/gxc.conf getpeerinfo

# Restart node
sudo systemctl restart gxc-node
```

### High Memory Usage

```bash
# Check memory
free -h

# Reduce database cache in config
# Edit /etc/gxc/gxc.conf
# dbcache=1024  # Reduce from 2048

# Restart node
sudo systemctl restart gxc-node
```

---

## Security Best Practices

### 1. RPC Access

**NEVER expose RPC to the internet!**

Use SSH tunnel for remote access:

```bash
# On your local machine
ssh -L 8332:localhost:8332 user@your-server

# Now access RPC via localhost:8332
```

### 2. Wallet Security

```bash
# Encrypt wallet
gxc-cli -conf=/etc/gxc/gxc.conf encryptwallet "your-strong-passphrase"

# Lock wallet
gxc-cli -conf=/etc/gxc/gxc.conf walletlock

# Unlock for transactions (60 seconds)
gxc-cli -conf=/etc/gxc/gxc.conf walletpassphrase "your-passphrase" 60
```

### 3. Regular Updates

```bash
# Update system
sudo apt-get update && sudo apt-get upgrade -y

# Update GXC (when new version available)
cd GXC-goldxcoin-blockchain-
git pull
cd build
make -j$(nproc)
sudo systemctl stop gxc-node
sudo cp gxc-node /usr/local/bin/
sudo systemctl start gxc-node
```

### 4. Audit Logs

```bash
# Review logs regularly
sudo tail -100 /var/log/gxc/gxc.log
sudo tail -100 /var/log/auth.log
sudo fail2ban-client status
```

---

## Maintenance Schedule

### Daily

- [ ] Check node status
- [ ] Review logs for errors
- [ ] Monitor disk space
- [ ] Check peer connectivity

### Weekly

- [ ] Review security alerts
- [ ] Update system packages
- [ ] Verify backups
- [ ] Check performance metrics

### Monthly

- [ ] Apply security updates
- [ ] Test disaster recovery
- [ ] Review capacity planning
- [ ] Update documentation

### Quarterly

- [ ] Security audit
- [ ] Performance review
- [ ] Infrastructure review
- [ ] Update roadmap

---

## Production Checklist

Before going live, verify:

- [ ] All security hardening applied
- [ ] Firewall configured correctly
- [ ] Fail2Ban active
- [ ] Automatic backups configured
- [ ] Monitoring setup complete
- [ ] Health checks running
- [ ] Log rotation configured
- [ ] RPC credentials changed
- [ ] Wallet encrypted
- [ ] Documentation updated
- [ ] Team trained on procedures
- [ ] Disaster recovery tested
- [ ] Support channels ready

---

## Support and Resources

### Documentation

- [Production Readiness Checklist](PRODUCTION_READINESS_CHECKLIST.md)
- [Security Hardening Guide](scripts/security-hardening.sh)
- [Configuration Reference](config/gxc-production.conf)

### Community

- GitHub: https://github.com/philani1H/GXC-goldxcoin-blockchain-
- Issues: https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues

### Emergency Contacts

- Security Issues: security@gxc.network
- Technical Support: support@gxc.network

---

## Conclusion

Your GXC blockchain node is now deployed with enterprise-grade security and reliability. Regular monitoring and maintenance will ensure optimal performance.

**Remember:**
- Keep your RPC credentials secure
- Backup your wallet regularly
- Monitor logs and metrics
- Apply security updates promptly
- Test disaster recovery procedures

**Happy Mining/Validating!** 🚀

---

*Last Updated: December 9, 2024*  
*Version: 2.1.0*  
*Status: Production Ready ✅*
