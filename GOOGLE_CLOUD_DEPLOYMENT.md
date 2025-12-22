# GXC Blockchain - Google Cloud Deployment Guide

## Deploy GXC Node to Google Cloud Platform

**Version**: 3.0  
**Date**: December 22, 2025  
**Platform**: Google Cloud Platform (GCP)

---

## Overview

This guide shows how to deploy a GXC blockchain node to Google Cloud Platform.

### What You'll Deploy

- ✅ GXC Full Node (blockchain + API)
- ✅ Stock Market API
- ✅ Market Maker API
- ✅ Explorer API
- ✅ Automatic backups
- ✅ Monitoring & alerts

---

## Prerequisites

### 1. Google Cloud Account
```bash
# Create account at: https://cloud.google.com
# Enable billing
# Create new project: gxc-blockchain
```

### 2. Install Google Cloud SDK
```bash
# Install gcloud CLI
curl https://sdk.cloud.google.com | bash
exec -l $SHELL

# Initialize
gcloud init

# Login
gcloud auth login

# Set project
gcloud config set project gxc-blockchain
```

### 3. Enable Required APIs
```bash
gcloud services enable compute.googleapis.com
gcloud services enable container.googleapis.com
gcloud services enable storage-api.googleapis.com
gcloud services enable monitoring.googleapis.com
```

---

## Deployment Options

### Option 1: Compute Engine VM (Recommended)
- **Best for**: Full control, custom configuration
- **Cost**: ~$50-100/month
- **Setup time**: 15 minutes

### Option 2: Google Kubernetes Engine (GKE)
- **Best for**: High availability, auto-scaling
- **Cost**: ~$150-300/month
- **Setup time**: 30 minutes

### Option 3: Cloud Run (Serverless)
- **Best for**: API only, no mining
- **Cost**: ~$20-50/month
- **Setup time**: 10 minutes

---

## Option 1: Compute Engine VM Deployment

### Step 1: Create VM Instance

```bash
# Create VM with Ubuntu 22.04
gcloud compute instances create gxc-node \
  --zone=us-central1-a \
  --machine-type=n2-standard-4 \
  --boot-disk-size=200GB \
  --boot-disk-type=pd-ssd \
  --image-family=ubuntu-2204-lts \
  --image-project=ubuntu-os-cloud \
  --tags=gxc-node,http-server,https-server \
  --metadata=startup-script='#!/bin/bash
    apt-get update
    apt-get install -y docker.io docker-compose git
    systemctl enable docker
    systemctl start docker
  '
```

**VM Specs**:
- **CPU**: 4 vCPUs (n2-standard-4)
- **RAM**: 16 GB
- **Disk**: 200 GB SSD
- **OS**: Ubuntu 22.04 LTS
- **Cost**: ~$150/month

### Step 2: Configure Firewall

```bash
# Allow GXC node port (9333)
gcloud compute firewall-rules create gxc-node-port \
  --allow=tcp:9333 \
  --target-tags=gxc-node \
  --description="GXC blockchain node port"

# Allow RPC port (8332)
gcloud compute firewall-rules create gxc-rpc-port \
  --allow=tcp:8332 \
  --target-tags=gxc-node \
  --description="GXC RPC port"

# Allow API port (3000)
gcloud compute firewall-rules create gxc-api-port \
  --allow=tcp:3000 \
  --target-tags=gxc-node \
  --description="GXC API port"

# Allow Stock Market API port (5000)
gcloud compute firewall-rules create gxc-stock-api-port \
  --allow=tcp:5000 \
  --target-tags=gxc-node \
  --description="GXC Stock Market API port"
```

### Step 3: SSH into VM

```bash
# SSH into the VM
gcloud compute ssh gxc-node --zone=us-central1-a
```

### Step 4: Clone Repository

```bash
# Clone GXC repository
cd /opt
sudo git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Set permissions
sudo chown -R $USER:$USER /opt/GXC-goldxcoin-blockchain-
```

### Step 5: Build Node

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  libssl-dev \
  libleveldb-dev \
  libboost-all-dev \
  libsqlite3-dev \
  python3 \
  python3-pip

# Build node
mkdir -p build
cd build
cmake ..
make -j$(nproc)

# Verify build
./gxc-node --version
```

### Step 6: Create Systemd Service

```bash
# Create service file
sudo tee /etc/systemd/system/gxc-node.service > /dev/null <<EOF
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=/opt/GXC-goldxcoin-blockchain-/build
ExecStart=/opt/GXC-goldxcoin-blockchain-/build/gxc-node \
  --datadir=/var/lib/gxc \
  --port=9333 \
  --rpc-port=8332 \
  --rpc-bind=0.0.0.0 \
  --rpc-allow-ip=0.0.0.0/0
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Create data directory
sudo mkdir -p /var/lib/gxc
sudo chown -R $USER:$USER /var/lib/gxc

# Enable and start service
sudo systemctl daemon-reload
sudo systemctl enable gxc-node
sudo systemctl start gxc-node

# Check status
sudo systemctl status gxc-node
```

### Step 7: Deploy Stock Market API

```bash
# Install Python dependencies
cd /opt/GXC-goldxcoin-blockchain-/market_maker
pip3 install -r requirements.txt

# Create service file
sudo tee /etc/systemd/system/gxc-stock-api.service > /dev/null <<EOF
[Unit]
Description=GXC Stock Market API
After=network.target gxc-node.service

[Service]
Type=simple
User=$USER
WorkingDirectory=/opt/GXC-goldxcoin-blockchain-/market_maker
ExecStart=/usr/bin/python3 api.py
Restart=always
RestartSec=10
Environment="FLASK_ENV=production"
Environment="PORT=5000"

[Install]
WantedBy=multi-user.target
EOF

# Enable and start service
sudo systemctl daemon-reload
sudo systemctl enable gxc-stock-api
sudo systemctl start gxc-stock-api

# Check status
sudo systemctl status gxc-stock-api
```

### Step 8: Setup Nginx Reverse Proxy

```bash
# Install Nginx
sudo apt-get install -y nginx certbot python3-certbot-nginx

# Create Nginx config
sudo tee /etc/nginx/sites-available/gxc > /dev/null <<EOF
server {
    listen 80;
    server_name YOUR_DOMAIN.com;

    # GXC Node RPC
    location /rpc {
        proxy_pass http://localhost:8332;
        proxy_set_header Host \$host;
        proxy_set_header X-Real-IP \$remote_addr;
    }

    # Stock Market API
    location /api/v1/stock {
        proxy_pass http://localhost:5000;
        proxy_set_header Host \$host;
        proxy_set_header X-Real-IP \$remote_addr;
    }

    # Explorer API
    location /api/v1/explorer {
        proxy_pass http://localhost:3000;
        proxy_set_header Host \$host;
        proxy_set_header X-Real-IP \$remote_addr;
    }
}
EOF

# Enable site
sudo ln -s /etc/nginx/sites-available/gxc /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx

# Setup SSL (optional but recommended)
sudo certbot --nginx -d YOUR_DOMAIN.com
```

### Step 9: Setup Monitoring

```bash
# Install monitoring agent
curl -sSO https://dl.google.com/cloudagents/add-google-cloud-ops-agent-repo.sh
sudo bash add-google-cloud-ops-agent-repo.sh --also-install

# Create monitoring config
sudo tee /etc/google-cloud-ops-agent/config.yaml > /dev/null <<EOF
logging:
  receivers:
    gxc_node:
      type: files
      include_paths:
        - /var/log/gxc-node.log
  service:
    pipelines:
      default_pipeline:
        receivers: [gxc_node]

metrics:
  receivers:
    hostmetrics:
      type: hostmetrics
      collection_interval: 60s
  service:
    pipelines:
      default_pipeline:
        receivers: [hostmetrics]
EOF

# Restart agent
sudo systemctl restart google-cloud-ops-agent
```

### Step 10: Setup Automatic Backups

```bash
# Create backup script
sudo tee /usr/local/bin/gxc-backup.sh > /dev/null <<'EOF'
#!/bin/bash
BACKUP_DIR="/var/backups/gxc"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BUCKET="gs://gxc-blockchain-backups"

# Create backup directory
mkdir -p $BACKUP_DIR

# Stop node
systemctl stop gxc-node

# Backup blockchain data
tar -czf $BACKUP_DIR/gxc-data-$TIMESTAMP.tar.gz /var/lib/gxc

# Start node
systemctl start gxc-node

# Upload to Google Cloud Storage
gsutil cp $BACKUP_DIR/gxc-data-$TIMESTAMP.tar.gz $BUCKET/

# Keep only last 7 days locally
find $BACKUP_DIR -name "gxc-data-*.tar.gz" -mtime +7 -delete

# Keep only last 30 days in cloud
gsutil ls $BUCKET/gxc-data-*.tar.gz | head -n -30 | xargs -r gsutil rm
EOF

# Make executable
sudo chmod +x /usr/local/bin/gxc-backup.sh

# Create Cloud Storage bucket
gsutil mb gs://gxc-blockchain-backups

# Add to crontab (daily at 2 AM)
(crontab -l 2>/dev/null; echo "0 2 * * * /usr/local/bin/gxc-backup.sh") | crontab -
```

---

## Option 2: Docker Deployment

### Step 1: Create Dockerfile

```dockerfile
# Create Dockerfile
cat > Dockerfile <<'EOF'
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libleveldb-dev \
    libboost-all-dev \
    libsqlite3-dev \
    python3 \
    python3-pip \
    git \
    && rm -rf /var/lib/apt/lists/*

# Clone repository
WORKDIR /opt
RUN git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
WORKDIR /opt/GXC-goldxcoin-blockchain-

# Build node
RUN mkdir -p build && cd build && cmake .. && make -j$(nproc)

# Install Python dependencies
WORKDIR /opt/GXC-goldxcoin-blockchain-/market_maker
RUN pip3 install -r requirements.txt

# Expose ports
EXPOSE 9333 8332 5000 3000

# Create entrypoint
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
EOF
```

### Step 2: Create Entrypoint Script

```bash
cat > entrypoint.sh <<'EOF'
#!/bin/bash

# Start GXC node in background
/opt/GXC-goldxcoin-blockchain-/build/gxc-node \
  --datadir=/var/lib/gxc \
  --port=9333 \
  --rpc-port=8332 \
  --rpc-bind=0.0.0.0 &

# Wait for node to start
sleep 10

# Start Stock Market API
cd /opt/GXC-goldxcoin-blockchain-/market_maker
python3 api.py &

# Keep container running
wait
EOF
```

### Step 3: Create Docker Compose

```yaml
cat > docker-compose.yml <<'EOF'
version: '3.8'

services:
  gxc-node:
    build: .
    container_name: gxc-node
    ports:
      - "9333:9333"
      - "8332:8332"
      - "5000:5000"
      - "3000:3000"
    volumes:
      - gxc-data:/var/lib/gxc
      - ./logs:/var/log/gxc
    restart: unless-stopped
    environment:
      - NODE_ENV=production
    networks:
      - gxc-network

  nginx:
    image: nginx:alpine
    container_name: gxc-nginx
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
      - ./ssl:/etc/nginx/ssl:ro
    depends_on:
      - gxc-node
    restart: unless-stopped
    networks:
      - gxc-network

volumes:
  gxc-data:
    driver: local

networks:
  gxc-network:
    driver: bridge
EOF
```

### Step 4: Build and Deploy

```bash
# Build image
docker-compose build

# Start services
docker-compose up -d

# Check logs
docker-compose logs -f gxc-node

# Check status
docker-compose ps
```

### Step 5: Push to Google Container Registry

```bash
# Tag image
docker tag gxc-node gcr.io/gxc-blockchain/gxc-node:latest

# Configure Docker for GCR
gcloud auth configure-docker

# Push image
docker push gcr.io/gxc-blockchain/gxc-node:latest
```

---

## Verification

### Check Node Status

```bash
# Check if node is running
curl http://YOUR_VM_IP:8332/api/v1/blockchain/info

# Expected response:
{
  "success": true,
  "height": 12345,
  "difficulty": 1.23,
  "network_hashrate": "1.5 TH/s",
  "peers": 15
}
```

### Check Stock Market API

```bash
# Check stock API
curl http://YOUR_VM_IP:5000/api/v1/health

# Expected response:
{
  "success": true,
  "status": "healthy",
  "features": [
    "Iceberg Orders",
    "TWAP Orders",
    "VWAP Orders",
    ...
  ]
}
```

### Check Logs

```bash
# Node logs
sudo journalctl -u gxc-node -f

# Stock API logs
sudo journalctl -u gxc-stock-api -f

# Nginx logs
sudo tail -f /var/log/nginx/access.log
```

---

## Cost Estimation

### Compute Engine VM
- **VM (n2-standard-4)**: ~$120/month
- **200 GB SSD**: ~$30/month
- **Network egress**: ~$10/month
- **Backups (Cloud Storage)**: ~$5/month
- **Total**: ~$165/month

### Optimization Tips
1. Use preemptible VMs (70% cheaper) for non-critical nodes
2. Use standard persistent disk instead of SSD (50% cheaper)
3. Use committed use discounts (up to 57% off)
4. Use Cloud CDN for API responses

---

## Security Best Practices

### 1. Firewall Rules
```bash
# Restrict RPC access to specific IPs
gcloud compute firewall-rules update gxc-rpc-port \
  --source-ranges=YOUR_IP/32
```

### 2. SSH Key Authentication
```bash
# Disable password authentication
sudo sed -i 's/PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config
sudo systemctl restart sshd
```

### 3. Automatic Updates
```bash
# Enable automatic security updates
sudo apt-get install -y unattended-upgrades
sudo dpkg-reconfigure -plow unattended-upgrades
```

### 4. Monitoring Alerts
```bash
# Create alert policy for high CPU
gcloud alpha monitoring policies create \
  --notification-channels=YOUR_CHANNEL_ID \
  --display-name="GXC Node High CPU" \
  --condition-display-name="CPU > 80%" \
  --condition-threshold-value=0.8 \
  --condition-threshold-duration=300s
```

---

## Troubleshooting

### Node Won't Start
```bash
# Check logs
sudo journalctl -u gxc-node -n 100

# Check disk space
df -h

# Check permissions
ls -la /var/lib/gxc
```

### Can't Connect to Node
```bash
# Check firewall
sudo ufw status

# Check if port is open
sudo netstat -tulpn | grep 9333

# Test connection
telnet YOUR_VM_IP 9333
```

### High Memory Usage
```bash
# Check memory
free -h

# Restart node
sudo systemctl restart gxc-node

# Consider upgrading VM
gcloud compute instances stop gxc-node
gcloud compute instances set-machine-type gxc-node --machine-type=n2-standard-8
gcloud compute instances start gxc-node
```

---

## Maintenance

### Update Node
```bash
# SSH into VM
gcloud compute ssh gxc-node

# Pull latest code
cd /opt/GXC-goldxcoin-blockchain-
git pull

# Rebuild
cd build
make -j$(nproc)

# Restart services
sudo systemctl restart gxc-node
sudo systemctl restart gxc-stock-api
```

### Restore from Backup
```bash
# Stop node
sudo systemctl stop gxc-node

# Download backup
gsutil cp gs://gxc-blockchain-backups/gxc-data-TIMESTAMP.tar.gz /tmp/

# Extract
sudo tar -xzf /tmp/gxc-data-TIMESTAMP.tar.gz -C /

# Start node
sudo systemctl start gxc-node
```

---

## Next Steps

1. ✅ Deploy node to Google Cloud
2. ✅ Configure firewall and security
3. ✅ Setup monitoring and alerts
4. ✅ Configure automatic backups
5. ✅ Test all APIs
6. ✅ Point domain to VM IP
7. ✅ Setup SSL certificate
8. ✅ Monitor performance

---

**🚀 GXC Node Deployed to Google Cloud! 🚀**

**Version**: 3.0  
**Date**: December 22, 2025  
**Platform**: Google Cloud Platform
